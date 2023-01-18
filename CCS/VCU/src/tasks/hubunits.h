/*
 * hubunits.h
 *
 * Authors: Dario Jimenez
 *
 * @file header containing tasks and isr's for communication with hub units
 */

#ifndef TASKS_INC_HUBUNITS_H_
#define TASKS_INC_HUBUNITS_H_

/*
 * @brief task that handles received messages from hub units
 */
void hubunits_receive_task(void *args);

/*
 * @brief task that handles the sending of messages to hub units
 */
void hubunits_send_task(void *args);

/*
 * @brief isr that periodically sends ack's to hub units
 */
void hubunits_sendAck_isr(void);

/*
 * @brief isr that periodically checks acks from the hub units
 */
void hubunits_checkAck_isr(void);

#endif /* TASKS_INC_HUBUNITS_H_ */
