/*
 * CAN_message_schema.h
 *
 * Authors: Dario Jimenez
 *
 * @file header file containing methods and data structures for
 *       CAN bus message schema
 */

#ifndef GUADALOOP_LIB_CAN_MESSAGE_SCHEMA_H_
#define GUADALOOP_LIB_CAN_MESSAGE_SCHEMA_H_

/** macros and constants **/
#define MAX_NUM_BYTES 8     /* max number of bytes that can be sent through can bus at one time */


/*
 * typedef for a byte
 */
typedef unsigned char byte;

/*
 * enum for all types of messages
 */
typedef enum message_type {
    ACK,
    //TODO: add additional msg types
}msg_type_t;

/*
 * @brief converts a message type and data into an array of bytes that can be sent through
 *        CAN bus.
 *
 * @return array of bytes
 */
byte *canMsgSchema_message(msg_type_t msg_type, void *data);


#endif /* GUADALOOP_LIB_CAN_MESSAGE_SCHEMA_H_ */
