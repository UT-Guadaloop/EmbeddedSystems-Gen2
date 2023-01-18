/*
 * CAN.h
 *
 * Authors: Dario Jimenez
 */

#ifndef CAN_H_
#define CAN_H_

typedef enum {
    CAN0,
    CAN1
}CAN_MODULE_t;

/*
 * @ brief set up can bus controller to trigger interrupts
 *         when messages are received.
 *
 * @note check msp432 datasheet for info. SIE bit on CANCTL
 *       has to be set.
 */
void CAN_interrupt_setup(CAN_MODULE_t can, void *isr);

/*
 * @brief set up can bus controller to store received
 *        messages in hardware FIFO. *
 */
void CAN_fifo_setup(CAN_MODULE_t can);


#endif /* CAN_H_ */
