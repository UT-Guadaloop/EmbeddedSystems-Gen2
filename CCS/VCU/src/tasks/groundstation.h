/*
 * groundstation.h
 *
 * Authors: Dario Jimenez
 *
 * @file header for tasks and isr's related to groundstation communication
 */

#ifndef TASKS_GROUNDSTATION_H_
#define TASKS_GROUNDSTATION_H_


/*
 * @brief task that handles received messages from groundstation
 */
void groundstation_receive_task(void *args);

/*
 * @brief task that handles the sending of messages to ground station
 */
void groundstation_send_task(void *args);

/*
 * @brief isr that periodically sends ack to groundstation unit
 */
void groundstation_sendAck_isr(void);

/*
 * @brief isr that periodically checks if ack from groundstation was sent
 */
void groundstation_checkAck_isr(void);

#endif /* TASKS_GROUNDSTATION_H_ */
