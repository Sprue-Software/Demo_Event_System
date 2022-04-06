/*
 * comms_handler.c
 *
 *  Created on: 14 Mar 2022
 *      Author: uhegde
 */
#include "os.h"
#include <string.h>
#include "em_chip.h"
#include "em_eusart.h"
#include "em_ldma.h"
#include "comms_handler.h"
#include "sl_power_manager.h"
#include "sl_sleeptimer.h"
#include "dmadrv.h"
#include "hal_BURTCTimer.h"
#include "events.h"

#define COMMS_TASK_STK_SIZE   256u
#define COMMS_TASK_PRIO       7u

#define COMMS_UART_TX_DEBUG   (1u << 1)
#define COMMS_UART_TX     (1u << 2)
#define COMMS_UART_RX     (1u << 3)
#define COMMS_UART_XMODEM   (1u << 4)
#define COMMS_EVENT_ALL     (COMMS_UART_TX_DEBUG | COMMS_UART_TX | COMMS_UART_RX | COMMS_UART_XMODEM)

#ifndef WAKEUP_DELAY_MS
#define WAKEUP_DELAY_MS           5000
#endif

//
static unsigned int tx_channel, rx_channel;

// Transfer and reception buffers
static char tx_buffer[TX_BUFFER_SIZE];
//static char rx_buffer[RX_BUFFER_SIZE];

// Flag indicating that the DMA data transfer is completed on reception channel
static volatile bool rx_transfer_complete;

OS_SEM SEM_LDMAtxComplete; /*Semaphore for LDMA transfer complete*/

// Flag indicating that the DMA data transfer is completed on reception channel
static volatile bool rx_transfer_complete;

/*task variables*/
OS_TCB CommsTaskTCB;
static CPU_STK CommsTaskStk[COMMS_TASK_STK_SIZE];
OS_FLAG_GRP CommsEventFlags;

//sl_sleeptimer_timer_handle_t timer;

struct _debugMsgBuffer{
  volatile commsMsg_t buffer[TX_MSG_BUFFER_SIZE];
  volatile uint16_t writeIndex;
  volatile uint16_t readIndex;
}debugMsgBuffer ={.writeIndex = 0u, .readIndex = 0u};

struct _transmitMsgBuffer{
  volatile commsMsg_t buffer[TX_MSG_BUFFER_SIZE];/**<Buffer of comms messages*/
  volatile uint16_t writeIndex; /**<*/
  volatile uint16_t readIndex;
}transmitMsgBuffer ={.writeIndex = 0u, .readIndex = 0u};

/**
 * XMODEM states
 */
enum {
  XMODEM_IDLE, /**< XMODEM_IDLE*/
  XMODEM_WAIT_FOR_C, /**< XMODEM_WAIT_FOR_C*/
  XMODEM_WAIT_FOR_ACK, /**< XMODEM_WAIT_FOR_ACK*/
  XMODEM_WAIT_FOR_EOT_ACK/**< XMODEM_WAIT_FOR_EOT_ACK*/
} xmodemState;

static void CommsTask(void *p_arg);
void buildAndSend_DebugPacket(void);
static bool getTxMsgBuffer(commsMsg_t *msg, txBuff_t buff);
static bool on_timeout(void);


// Function to transfer transmission buffer to USART via DMA
static void transmit_data(void)
{
  bool active;

  DMADRV_TransferActive(tx_channel, &active);

  // wait for any active transfers to finish
  while (active) {
    sl_sleeptimer_delay_millisecond(1);
    DMADRV_TransferActive(tx_channel, &active);
  }

  // Transfer data from tx buffer to USART peripheral
  DMADRV_MemoryPeripheral(tx_channel,
                          dmadrvPeripheralSignal_EUSART0_TXBL,
                          (void*)&(EUSART0->TXDATA),
                          tx_buffer,
                          true,
                          strlen(tx_buffer),
                          dmadrvDataSize1,
                          NULL,
                          NULL);
}

/**
 * @brief dmadrv initialisation
 */
void dmadrv_app_init(void)
{
  DMADRV_Init();
  uint32_t status;

  // Allocate channels for transmission and reception
  status = DMADRV_AllocateChannel(&tx_channel, NULL);
  EFM_ASSERT(status == ECODE_EMDRV_DMADRV_OK);
  status = DMADRV_AllocateChannel(&rx_channel, NULL);
  EFM_ASSERT(status == ECODE_EMDRV_DMADRV_OK);

  // Initialise transfer complete flag
  rx_transfer_complete = false;

  char *ptr = "DMADRV application\n";

  int i=0;
  while(*ptr != '\0'){
      tx_buffer[i++] = *ptr;
      ptr++;
  }
  transmit_data();
}

/**
 * @brief comms handler task initialisation
 */
void comms_initTask(void)
{
  RTOS_ERR err;

  dmadrv_app_init();  // dmadrv initialisation

  OSFlagCreate(&CommsEventFlags, /*   Pointer to user-allocated event flag.         */
                 "CommsFlags", /*   Name used for debugging.                      */
                 0, /*   Initial flags, all cleared.                   */
                 &err);
  /*   Check error code.                                  */
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  OSTaskCreate(&CommsTaskTCB,
               "CommsTask",
               CommsTask,
               DEF_NULL,
               COMMS_TASK_PRIO,
               &CommsTaskStk[0],
               (COMMS_TASK_STK_SIZE / 10u),
               COMMS_TASK_STK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK), /* Task options.                  */
               &err);
  /*   Check error code.              */
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
}

static bool on_timeout(void)
{
  //sendDebugMsg("Second", false, 0);
  return true;
}
/**
 * @brief Communications task.
 * @details Task pends on Comms Event Flags
 * @param p_arg Unused.
 */
static void CommsTask(void *p_arg)
{
  RTOS_ERR err;
  OS_FLAGS flags;
  bool active;

  PP_UNUSED_PARAM(p_arg); /* Prevent compiler warning.                  */

  /* build and send the debug packet */
  DMADRV_TransferActive(tx_channel, &active);

  // wait for any active transfers to finish
  while (active) {
   sl_sleeptimer_delay_millisecond(1);
   DMADRV_TransferActive(tx_channel, &active);
  }

  while (DEF_TRUE)
  {
     flags = OSFlagPend(&CommsEventFlags,
                       COMMS_UART_TX_DEBUG,
                       (OS_TICK) 0,
                       OS_OPT_PEND_FLAG_SET_ANY | /*   Wait until all flags are set and             */
                       OS_OPT_PEND_BLOCKING | /*    task will block and                         */
                       OS_OPT_PEND_FLAG_CONSUME, /*    function will clear the flags.               */
                       DEF_NULL, /*   Time stamp is not used.                        */
                       &err);

     if (RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){
         if ((flags & COMMS_UART_TX_DEBUG) > 0){
             buildAndSend_DebugPacket();
         }
     }
  }
} /*comms task*/

/**
 * @brief Build debug message into a packet and send via the LEUART
 */
void buildAndSend_DebugPacket(void)
{
  static commsMsg_t debugMsg;
  static uint8_t debugOutBuffer[DEBUG_MSG_MAX_LENGTH]={0u,};
  bool active=false;

  uint8_t i, x;

  while ((getTxMsgBuffer(&debugMsg, DEBUG_BUFF)) == true){
    i = 0u;
    debugOutBuffer[i] = (uint8_t) '>'; /*start char*/
    i++;
    for (x = 0u; x < debugMsg.bufferLength; x++){
      debugOutBuffer[i] = debugMsg.buffer[x];
      i++;
    }

    if (debugMsg.numMode == DEF_ENABLED){
      debugOutBuffer[i] = (uint8_t) ' '; /*add space between string and number*/
      i++;
      uint8_t ch;
      /* go through each nibble of the number and convert to ascii*/
      for (x = 0u; x < 8u; x++){
        ch = (uint8_t) (debugMsg.num >> 28u) + (uint8_t) '0'; /*get nibble and add '0' to get ascii*/
        if (ch > (uint8_t) '9'){ /*if greater than 9 needs to be A-F*/
          ch = ch + ASCII_CONVERSION;
        }
        debugOutBuffer[i] = ch; /*add to output buffer*/
        i++;
        debugMsg.num <<= 4u;
      }
    }
    debugOutBuffer[i] = (uint8_t) '<'; /*stop char*/
    i++;
    eusart_send_data(debugOutBuffer, i);
  }

  DMADRV_TransferActive(tx_channel, &active);

  // wait for any active transfers to finish
  while (active)
  {
    sl_sleeptimer_delay_millisecond(1);
    DMADRV_TransferActive(tx_channel, &active);
  }
}

/**
 * Transmit data via the EUSART/LDMA. Pends on semaphore to ensure all data in LDMA buffers has been sent
 * @param string Data to copy into LDMA buffer for transmission
 * @param dataLen Length of data to transmit, if data is longer than buffer it is truncated.
 */
void eusart_send_data(uint8_t * string, uint8_t dataLen)
{
  static uint8_t txBuffer[32];

  uint8_t i = 0u;
  uint8_t * ptr = string;
  uint8_t len = dataLen;
  bool active;

  /* Resource acquired. */
  if (len > TX_BUFFER_SIZE){
    len = TX_BUFFER_SIZE; /*truncate messages longer than permitted size*/
  }
  while (i < len){ /*copy data into the buffer*/
    txBuffer[i] = *ptr++;
    i++;
  }

  DMADRV_TransferActive(tx_channel, &active);

  // wait for any active transfers to finish
  while (active) {
    sl_sleeptimer_delay_millisecond(1);
    DMADRV_TransferActive(tx_channel, &active);
  }

  // Transfer data from tx buffer to USART peripheral
  DMADRV_MemoryPeripheral(tx_channel,
                          dmadrvPeripheralSignal_EUSART0_TXBL,
                          (void*)&(EUSART0->TXDATA),
                          txBuffer,
                          true,
                          i,
                          dmadrvDataSize1,
                          on_timeout,
                          NULL);
}

/**
 * Get message from transmit buffer
 * @param msg Pointer to message location
 * @param buff  Buffer to read message from (txBuff_t)
 * @return True if success in reading buffer, False if buffer empty
 */
static bool getTxMsgBuffer(commsMsg_t *msg, txBuff_t buff)
{
  bool success;
  if (buff == DEBUG_BUFF){

    if (debugMsgBuffer.readIndex != debugMsgBuffer.writeIndex){
      *msg = debugMsgBuffer.buffer[debugMsgBuffer.readIndex];
      debugMsgBuffer.readIndex = (debugMsgBuffer.readIndex + 1u) % TX_MSG_BUFFER_SIZE;
      success = true;
    }
    else{
      success = false; /*buffer empty*/
    }
  }
  else if (buff == TRANSMIT_BUFF){
    if (transmitMsgBuffer.readIndex != transmitMsgBuffer.writeIndex){
      *msg = transmitMsgBuffer.buffer[transmitMsgBuffer.readIndex];
      transmitMsgBuffer.readIndex = (transmitMsgBuffer.readIndex + 1u) % TX_MSG_BUFFER_SIZE;
      success = true;
    }
    else{
      success = false; /*buffer empty*/
    }
  }
  else{
    success = false;/*unknown buff type*/
  }
  return success;
}

/**
 * @brief Put a transmit message into a buffer ready for output
 * @param msg Comms message to send
 * @param buff Buffer to add message to (txBuff_t)
 */
static void putTxMsgBuffer(commsMsg_t msg, txBuff_t buff)
{
  if (buff == DEBUG_BUFF){

    if (debugMsgBuffer.writeIndex == ((debugMsgBuffer.readIndex - 1u + TX_MSG_BUFFER_SIZE) % TX_MSG_BUFFER_SIZE)){
      /*Buffer full - ignore any new data until space available*/
    }
    else{
      debugMsgBuffer.buffer[debugMsgBuffer.writeIndex] = msg;
      debugMsgBuffer.writeIndex = (debugMsgBuffer.writeIndex + 1u) % TX_MSG_BUFFER_SIZE;
    }
  }
  else if (buff == TRANSMIT_BUFF){
    if (transmitMsgBuffer.writeIndex
            == ((transmitMsgBuffer.readIndex - 1u + TX_MSG_BUFFER_SIZE) % TX_MSG_BUFFER_SIZE)){
      /*Buffer full - ignore any new data until space available*/
    }
    else{
      transmitMsgBuffer.buffer[transmitMsgBuffer.writeIndex] = msg;
      transmitMsgBuffer.writeIndex = (transmitMsgBuffer.writeIndex + 1u) % TX_MSG_BUFFER_SIZE;
    }
  }
  else{
    /*Unknown buff type*/
  }
}

/**
 * @brief Format and send debug message to the commsHandler to output
 * @param str   String to be output
 * @param numMode DEF_TRUE, if using number; DEF_FALSE if want to skip number
 * @param dataValue   Number to add to message. Number will be output in ASCII representation of hex value
 */
void debug_out(const char* str, bool numMode, uint32_t dataValue)
{
  RTOS_ERR err;
  commsMsg_t
  msgDebug ={.bufferLength = 0u, .buffer ={0u,}, .cmd = 0u, .numMode = 0u, .num = 0u};
  uint8_t i = 0u;

  while (*str){
    msgDebug.buffer[i] = *str++;
    i++;
    if (i > (DEBUG_MSG_MAX_LENGTH - 1u)){
      break;/*truncate message if too long*/
    }
  }

  msgDebug.bufferLength = i;
  msgDebug.numMode = numMode;
  msgDebug.num = dataValue;

  putTxMsgBuffer(msgDebug, DEBUG_BUFF);

  OSFlagPost(&CommsEventFlags,  /*Pointer to user-allocated event flag.*/
             COMMS_UART_TX_DEBUG,
             OS_OPT_POST_FLAG_SET,    //Set the flag
             &err);
  /*   Check error code.                                  */
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
}



