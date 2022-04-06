/*
 * comms_handler.h
 *
 *  Created on: 14 Mar 2022
 *      Author: uhegde
 */

#ifndef COMMS_HANDLER_H_
#define COMMS_HANDLER_H_

#include <stdbool.h>
#include <stdint.h>

#define DEBUG_MSG_MAX_LENGTH      32u
#define STX                       0x02u
#define ETX                       0x03u
#define ESC                       0x1Bu
#define ESC_OFFSET                0x20u
#define XMODEM_CMD_NAK            0x15u /* Not Acknowledge*/
#define RX_BUFFER_SIZE            32u
#define PACKET_CRC_MSB_OFFSET     2u
#define PACKET_CRC_LSB_OFFSET     3u
#define ASCII_CONVERSION          (0x41u - 0x39u - 0x01u)
#define PACKET_DATA_MAX_LENGTH    16u
#define COMMS_PACKET_MAX_LENGTH   (PACKET_DATA_MAX_LENGTH +4u)
#define TX_BUFFER_SIZE            136u /*this needs to be large enough to handle an XMODEM packet (133bytes)*/
#define LDMA_CHANNEL_EUSART0_TX   0u
#define TX_MSG_BUFFER_SIZE        32u

typedef enum {
  DEBUG_BUFF, TRANSMIT_BUFF
} txBuff_t;

typedef struct _commsMsg_t {
    uint8_t bufferLength;/**<Length of data in buffer*/
    uint8_t buffer[DEBUG_MSG_MAX_LENGTH];/**<buffer containing message data*/
    uint8_t cmd; /**<for packet output*/
    bool numMode; /**<for debug output*/
    uint32_t num; /**<for debug output*/
} commsMsg_t;

extern void debug_out(const char* str, bool numMode, uint32_t dataValue);
extern void eusart_enter_sleep_mode(void);
extern void eusart_send_data(uint8_t * string, uint8_t dataLen);
extern void comms_initTask(void);
extern void buildAndSend_DebugPacket(void);

#endif /* COMMS_HANDLER_H_ */
