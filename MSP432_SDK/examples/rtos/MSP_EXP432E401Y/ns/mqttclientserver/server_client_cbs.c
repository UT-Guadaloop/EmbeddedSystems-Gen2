/*
 * Copyright (c) 2016-2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//*****************************************************************************
//
//! \addtogroup mqtt_server
//! @{
//
//*****************************************************************************
/* Standard includes                                                          */
#include <stdlib.h>

/* Kernel (Non OS/Free-RTOS/TI-RTOS) includes                                 */
#include "pthread.h"
#include "mqueue.h"

#include <ti/display/Display.h>

/* Application includes                                                       */
#include "server_client_cbs.h"

extern bool gResetApplication;
extern Display_Handle display;

//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************
#define OS_WAIT_FOREVER         (0xFFFFFFFF)
#define OS_NO_WAIT              (0)
#define OS_OK                   (0)

#define MQTTClientCbs_ConnackRC(data) (data & 0xff) /**< CONNACK: Return Code (LSB) */

//*****************************************************************************
//                 GLOBAL VARIABLES
//*****************************************************************************

/* Message Queue                                                              */
extern mqd_t g_PBQueue;
extern char *topic[];
struct client_info client_info_table[MAX_CONNECTION];

//*****************************************************************************
//                 Queue external function
//*****************************************************************************
extern int32_t MQTT_SendMsgToQueue(struct msgQueue *queueElement);

//*****************************************************************************
//                          SERVER CALLBACKS
//*****************************************************************************

void MqttServerCallback(int32_t event, void * metaData, uint32_t metaDateLen, void *data, uint32_t dataLen)
{
    switch(event)
    {
        case MQTTServer_RECV_CB_EVENT:
        {
            MQTTServer_RecvMetaDataCB *meta = (MQTTServer_RecvMetaDataCB*)metaData;

            uint32_t bufSizeReqd = 0;
            uint32_t topicOffset;
            uint32_t payloadOffset;

            struct publishMsgHeader msgHead;

            char *pubBuff = NULL;
            struct msgQueue queElem;

            topicOffset = sizeof(struct publishMsgHeader);
            payloadOffset = sizeof(struct publishMsgHeader) + (meta->topLen + 1);

            bufSizeReqd += sizeof(struct publishMsgHeader);
            bufSizeReqd += meta->topLen + 1;
            bufSizeReqd += dataLen + 1;
            pubBuff = (char *) malloc(bufSizeReqd);

            if (pubBuff == NULL)
            {
                Display_printf(display, 0, 0, "malloc failed: recv_cb");
                return;
            }

            msgHead.topicLen = meta->topLen;
            msgHead.payLen = dataLen;
            msgHead.retain = meta->retain;
            msgHead.dup = meta->dup;
            msgHead.qos = meta->qos;
            memcpy((void*) pubBuff, &msgHead, sizeof(struct publishMsgHeader));

            /* copying the topic name into the buffer                         */
            memcpy((void*) (pubBuff + topicOffset), (const void*) meta->topic, meta->topLen);
            memset((void*) (pubBuff + topicOffset + meta->topLen), '\0', 1);

            /* copying the payload into the buffer                            */
            memcpy((void*) (pubBuff + payloadOffset), (const void*) data, dataLen);
            memset((void*) (pubBuff + payloadOffset + dataLen), '\0', 1);

            Display_printf(display, 0, 0, "\n\rMsg Recvd. by server");
            Display_printf(display, 0, 0, "TOPIC: %s", pubBuff + topicOffset);
            Display_printf(display, 0, 0, "PAYLOAD: %s",
                    pubBuff + payloadOffset);
            Display_printf(display, 0, 0, "QOS: %d", meta->qos);

            if (meta->retain)
            {
                Display_printf(display, 0, 0, "Retained");
            }

            if (meta->dup)
            {
                Display_printf(display, 0, 0, "Duplicate");
            }

            /* filling the queue element details                              */
            queElem.event = MSG_RECV_BY_SERVER;
            queElem.msgPtr = pubBuff;

            /* signal to the main task                                        */
            if (MQTT_SendMsgToQueue(&queElem))
            {
                Display_printf(display, 0, 0, "\n\n\rQueue is full\n\r");
            }
        }
        break;

        case MQTTServer_CONNECT_CB_EVENT:
        {
            MQTTServer_ConnectMetaDataCB *meta = (MQTTServer_ConnectMetaDataCB*)metaData;
            Display_printf(display, 0, 0, "CONNECTION");
            Display_printf(display, 0, 0, "client ID = %s", meta->clientId);
            Display_printf(display, 0, 0, "username = %s",  meta->userName);
            Display_printf(display, 0, 0, "password = %s",  meta->password);
            /* Check if Conn Ack return value is Success (NULL) or Error -    */
            /* Negative value                                                 */
            if (data == NULL)
            {
                meta->usr = (void**)&client_info_table[0];
            }
            else
            {
                Display_printf(display, 0, 0, "Connection Error = %d",  (int)data);
            }

        }
        break;

        case MQTTServer_DISCONNECT_CB_EVENT:
        {
            struct msgQueue queElem;
            Display_printf(display, 0, 0, "DISCONNECTION");
            /* filling the queue element details                              */
            queElem.event  = REMOTE_CLIENT_DISCONNECTION;
            queElem.msgPtr = NULL;
            /* signal to the main task                                        */
            if (MQTT_SendMsgToQueue(&queElem))
            {
                Display_printf(display, 0, 0, "\n\n\rQueue is full\n\r");
            }
        }
        break;

    }
}

//*****************************************************************************
//
//! Callback in case the client receives a msg from the remote broker
//!
//! This function:
//! 1. packs the info of the incoming message from the remote broker (along with
//!    the payload) and signals the main task about occurrence of this event.
//!
//! \param[in]  event       - is a event occurred
//! \param[in]  metaData    - is the pointer for the message buffer
//!                           (for this event)
//! \param[in]  metaDateLen - is the length of the message buffer
//! \param[in]  data        - is the pointer to the buffer for data
//!                           (for this event)
//! \param[in]  dataLen     - is the length of the buffer data
//! \param[in]  app_hndl    - is the ctx handl for the connection
//! \param[in]  topstr      - is a pointer to the topic string
//! \param[in]  top_len     - is the length of the topic string
//! \param[in]  payload     - is the pointer to the msg payload
//! \param[in]  pay_len     - is the length of the username string
//! \param[in]  dup         - is the flag indicating if the msg is a duplicate
//! \param[in]  qos         - is the qos of the incoming msg
//! \param[in]  retain      - flag will decide if this message has to be retained
//!                           or not
//!
//! return none
//
//*****************************************************************************
void MqttClientCallback(int32_t event , void * metaData , uint32_t metaDateLen , void *data , uint32_t dataLen)
{
    int32_t i = 0;

    switch((MQTTClient_EventCB)event)
    {
        case MQTTClient_OPERATION_CB_EVENT:
        {
            switch (((MQTTClient_OperationMetaDataCB*)metaData)->messageType)
            {
                case MQTTCLIENT_OPERATION_CONNACK:
                {
                    uint16_t *ConnACK = (uint16_t*) data;
                    Display_printf(display, 0, 0, "CONNACK:");
                    /* Check if Conn Ack return value is Success (0) or       */
                    /* Error - Negative value                                 */
                    if ( 0 == (MQTTClientCbs_ConnackRC(*ConnACK)) )
                    {
                        Display_printf(display, 0, 0, "Connection Success");
                    }
                    else
                    {
                        Display_printf(display, 0, 0,
                                "Connection Error: %d", *ConnACK);
                    }
                    break;
                }

                case MQTTCLIENT_OPERATION_EVT_PUBACK:
                {
                    char *PubAck = (char *) data;
                    Display_printf(display, 0, 0, "PubAck:");
                    Display_printf(display, 0, 0, "%s", PubAck);
                    break;
                }

                case MQTTCLIENT_OPERATION_SUBACK:
                {
                    Display_printf(display, 0, 0, "Sub Ack:");
                    Display_printf(display, 0, 0, "Granted QoS Levels are:");
                    for (i = 0; i < dataLen; i++)
                    {
                        Display_printf(display, 0, 0, "%s :QoS %d", topic[i],
                                ((unsigned char*) data)[i]);
                    }
                    break;
                }

                case MQTTCLIENT_OPERATION_UNSUBACK:
                {
                    char *UnSub = (char *) data;
                    Display_printf(display, 0, 0, "UnSub Ack ");
                    Display_printf(display, 0, 0, "%s", UnSub);
                    break;
                }

                default:
                    break;
            }
            break;
        }
        case MQTTClient_RECV_CB_EVENT:
        {
            MQTTClient_RecvMetaDataCB *recvMetaData =  (MQTTClient_RecvMetaDataCB *)metaData;
            uint32_t bufSizeReqd = 0;
            uint32_t topicOffset;
            uint32_t payloadOffset;

            struct publishMsgHeader msgHead;

            char *pubBuff = NULL;
            struct msgQueue queElem;

            topicOffset = sizeof(struct publishMsgHeader);
            payloadOffset = sizeof(struct publishMsgHeader) + recvMetaData->topLen + 1;

            bufSizeReqd += sizeof(struct publishMsgHeader);
            bufSizeReqd += recvMetaData->topLen + 1;
            bufSizeReqd += dataLen + 1;
            pubBuff = (char *) malloc(bufSizeReqd);

            if (pubBuff == NULL)
            {
                Display_printf(display, 0, 0, "malloc failed: recv_cb");
                return;
            }

            msgHead.topicLen = recvMetaData->topLen;
            msgHead.payLen = dataLen;
            msgHead.retain = recvMetaData->retain;
            msgHead.dup = recvMetaData->dup;
            msgHead.qos = recvMetaData->qos;
            memcpy((void*) pubBuff, &msgHead, sizeof(struct publishMsgHeader));

            /* copying the topic name into the buffer                         */
            memcpy((void*) (pubBuff + topicOffset), (const void*)recvMetaData->topic, recvMetaData->topLen);
            memset((void*) (pubBuff + topicOffset + recvMetaData->topLen), '\0', 1);

            /* copying the payload into the buffer                            */
            memcpy((void*) (pubBuff + payloadOffset), (const void*) data, dataLen);
            memset((void*) (pubBuff + payloadOffset + dataLen), '\0', 1);

            Display_printf(display, 0, 0, "\n\rMsg Recvd. by client");
            Display_printf(display, 0, 0, "TOPIC: %s", pubBuff + topicOffset);
            Display_printf(display, 0, 0, "PAYLOAD: %s",
                    pubBuff + payloadOffset);
            Display_printf(display, 0, 0, "QOS: %d", recvMetaData->qos);

            if (recvMetaData->retain)
            {
                Display_printf(display, 0, 0, "Retained");
            }

            if (recvMetaData->dup)
            {
                Display_printf(display, 0, 0, "Duplicate");
            }

            /* filling the queue element details                              */
            queElem.event  = MSG_RECV_BY_CLIENT;
            queElem.msgPtr = pubBuff;

            /* signal to the main task                                        */
            if (MQTT_SendMsgToQueue(&queElem))
            {
                Display_printf(display, 0, 0, "\n\n\rQueue is full\n\r");
            }
            break;
        }
        case MQTTClient_DISCONNECT_CB_EVENT:
        {
            Display_printf(display, 0, 0, "BRIDGE DISCONNECTION");
            break;
        }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
