/*
 * vcu_communication.h
 *
 * Authors: Dario Jimenez
 *
 * @file header file for all the vcu communication methods
 */

#ifndef VCU_COMMUNICATION_H_
#define VCU_COMMUNICATION_H_

/*
 * @brief task to handle VCU messages and requests received.
 */
void *vcu_receive_task(void *args);

/*
 * @brief task to handle sending sensor data to VCU.
 *        higher priority data should be sent first.
 */
void *vcu_send_task(void *args);

/*
 * @brief isr to periodically send ack to vcu
 */
void vcu_sendAck_isr(void);

/*
 * @brief isr to periodically check if ack was received from vcu
 */
void vcu_checkAck_isr(void);

#endif /* VCU_COMMUNICATION_H_ */
