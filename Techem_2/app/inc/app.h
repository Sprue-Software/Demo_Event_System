/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef APP_H
#define APP_H
#define  DEBUG_ENABLE_APP

#ifdef DEBUG_ENABLE_APP
#define DEBUG_APP(str, numMode, dataValue) debug_out(str,numMode,dataValue)
#else
#define DEBUG_APP(str, numMode, dataValue)
#endif
#define BSP_GPIO_LEDS
#define BSP_NO_OF_LEDS          2
#define BSP_GPIO_LED0_PORT      gpioPortC
#define BSP_GPIO_LED0_PIN       8
#define BSP_GPIO_LED1_PORT      gpioPortC
#define BSP_GPIO_LED1_PIN       9

#define TOOGLE_DELAY_MS         60000

/*
 *********************************************************************************************************
 *                                      APPLICATION TASK PRIORITIES
 *********************************************************************************************************
*/
#define LED_BUZZ_TASK_PRIO          10u
#define EVENTS_TASK_PRIO            11u
#define SPI_COMMS_TASK_PRIO         12u
#define SWITCH_TASK_PRIO            13u
#define UART_COMMS_TASK_PRIO        14u
#define WDOG_TIMER_TASK_PRIO        15u

/*
 *********************************************************************************************************
 *                                      APPLICATION TASK STACK SIZES
 *********************************************************************************************************
*/
#define LED_BUZZ_TASK_STK_SIZE      256u
#define EVENTS_TASK_STK_SIZE        512u
#define SPI_COMMS_TASK_STK_SIZE     512u
#define SWITCH_TASK_STK_SIZE        256u
#define UART_COMMS_TASK_STK_SIZE    512u
#define WDOG_TIMER_TASK_STK_SIZE    256u


/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
extern void app_init(void);
extern void blink_init(void);

#endif  // APP_H
