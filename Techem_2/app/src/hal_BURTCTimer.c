/***************************************************************************//**
 * @file  hal_BURTCTimer.c
 * @brief This project uses the BURTC (Backup Real Time Counter) to tick every
 *        10 seconds.
 *
 * @project P0200 Techem Core Firmware
 * @date    16 March 2022
 * @author  abenrashed
 *******************************************************************************/
#include  <cpu/include/cpu.h>
#include  <common/include/common.h>
#include  <kernel/include/os.h>
//#include  <kernel/include/os_core.h>
#include  <kernel/source/os_priv.h>

#include  <common/include/lib_def.h>
#include  <common/include/rtos_utils.h>
#include  <common/include/toolchains.h>

//#include "peripheral_sysrtc.h"
//#include "sl_sleeptimer.h"
//#include "sli_sleeptimer_hal.h"
#include "em_core.h"
//#include "em_cmu.h"

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
//#include "em_burtc.h"
#include "em_rmu.h"
#include "hal_BURTCTimer.h"
#include "events.h"
#include "app.h"
static int count=0;
BURTCTimer_TypeDef Event_Timer[(uint8_t) NO_OF_EVENTS];

/**
 * BURTCTimer Callback
 * @req
 * @brief   Sets relevant event flags.
 * @details Each event arising is flagged by the relevant flag, semaphore, etc.
 *          When that timer period passes, the event is flagged in a callback function.
 */
#if 0
static void BURTCTimer_CallBack(uint64_t event)
{
  RTOS_ERR err;

  if ((event & (1u << (uint32_t) Smoke_measure_event)) != 0u){
      OSFlagPost(&Event_Flags_SubGroup[FLAGS_SUBGROUP_INDEX(Smoke_measure_event)],  /*Pointer to user-allocated event flag.*/
                 FLAGS_BIT_INDEX(Smoke_measure_event),
                 OS_OPT_POST_FLAG_SET,    //Set the flag
                 &err);
    /*   Check error code.                                  */
      if (RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
          RTOS_ERR_SET(err, RTOS_ERR_FAIL);
      }
    //APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
    //OS_ASSERT_DBG_NO_ERR((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), RTOS_ERR_ASSERT_CRITICAL_FAIL, 1);
  }

  if ((event & (1u << (uint32_t) AmbientLight_measure_event)) != 0u){
      OSFlagPost(&Event_Flags_SubGroup[FLAGS_SUBGROUP_INDEX(AmbientLight_measure_event)],  /*Pointer to user-allocated event flag.*/
                 FLAGS_BIT_INDEX(Smoke_measure_event),
                 OS_OPT_POST_FLAG_SET,    //Set the flag
                 &err);
    /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  }

  if ((event & (1u << (uint32_t) Heartbeat_event)) != 0u){
      OSFlagPost(&Event_Flags_SubGroup[FLAGS_SUBGROUP_INDEX(Heartbeat_event)],  /*Pointer to user-allocated event flag.*/
                 FLAGS_BIT_INDEX(Heartbeat_event),
                 OS_OPT_POST_FLAG_SET,    //Set the flag
                 &err);
    /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  }

  if ((event & (1u << (uint32_t) LedBuzz_event)) != 0u){
      OSFlagPost(&Event_Flags_SubGroup[FLAGS_SUBGROUP_INDEX(LedBuzz_event)],  /*Pointer to user-allocated event flag.*/
                 FLAGS_BIT_INDEX(LedBuzz_event),
                 OS_OPT_POST_FLAG_SET,    //Set the flag
                 &err);
    /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  }

}
#else
static void BURTCTimer_CallBack(uint64_t event)
{
  RTOS_ERR err;
  uint32_t subgroub[2];
/* testing*/
  count++ ;

  subgroub[0] = event & 0xFFFFFFFF;
  subgroub[1] = (event >> 32) & 0xFFFFFFFF;


  OSFlagPost(&Event_Flags_SubGroup[0],  /*Pointer to user-allocated event flag.*/
             subgroub[0],
             OS_OPT_POST_FLAG_SET,    //Set the flag
             &err);
  /*   Check error code.                                  */
  if (RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
      RTOS_ERR_SET(err, RTOS_ERR_FAIL);
  }
  //APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  OSFlagPost(&Event_Flags_SubGroup[1],  /*Pointer to user-allocated event flag.*/
             subgroub[1],
             OS_OPT_POST_FLAG_SET,    //Set the flag
             &err);
  /*   Check error code.                                  */
  if (RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
      RTOS_ERR_SET(err, RTOS_ERR_FAIL);
  }
  //APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

//ABR. Will add the topGroup here later after testing.
}
#endif


/**************************************************************************//**
 * @brief  BURTC Handler
 *****************************************************************************/
void BURTC_IRQHandler(void)
{
  CORE_DECLARE_IRQ_STATE;
  uint8_t index;
  uint64_t events_detected = 0U; /*Initialise detected events to none*/

  //OSIntEnter();
  CORE_ENTER_ATOMIC();

  for (index = 0U; index < (uint8_t) NO_OF_EVENTS; index++)/*Check all events timers if expired*/
  {
    if (Event_Timer[index].enabled) /*Only active timers checked*/
    {
      if (Event_Timer[index].period > 0U) /*When timer elapsed*/
      {
        Event_Timer[index].period--;
        if (Event_Timer[index].period == 0U){
          events_detected |= (1U << index); /*Set detected event index*/
          if (Event_Timer[index].periodic){
            Event_Timer[index].period = Event_Timer[index].value; /*Restart the periodic timers*/
          }
          else{
            Event_Timer[index].enabled = false; /*Disabled the one shot timers when expired*/
          }
        }
      }
    }
  }

  //events_detected = 1;

  if (events_detected > 0U) /*Some events triggered*/
  {
      BURTCTimer_CallBack(events_detected);
  }

  BURTC_IntClear(BURTC_IF_COMP); // compare match
  //GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);

  //OSIntExit();
  CORE_EXIT_ATOMIC();
}


/**
 * BURTCTimer Start
 * @req
 * @brief Starts an event timer.
 * @param event: the event of concern
 * @param periodic: True, periodic, False one shot
 * @param period: The timer period in counts
 */

void BURTCTimer_Start(BURTCTimer_Events_TypeDef event, bool periodic, uint32_t period)
{
  RTOS_ERR err;
  OSSchedLock(&err);

  Event_Timer[event].periodic = periodic; /*Set to periodic or one shot*/
  Event_Timer[event].period = period;     /*Set the period counts*/
  Event_Timer[event].value = period;      /*Keep the period value*/
  Event_Timer[event].enabled = true;      /*Enable the timer*/
  OSSchedUnlock(&err);
}

/**
 * BURTCTimer Stop
 * @req
 * @brief Stops an event timer.
 * @param event: the event of concern
 */
uint32_t BURTCTimer_Stop(BURTCTimer_Events_TypeDef event)
{
  RTOS_ERR err;
  uint32_t stoppedValue;

  OSSchedLock(&err);
  Event_Timer[(uint8_t) event].enabled = false; /*Stop the timer*/
  stoppedValue = Event_Timer[(uint8_t) event].period;
  Event_Timer[(uint8_t) event].period = 0u;
  OSSchedUnlock(&err);

  return stoppedValue;                          /*Return value at moment stopped*/
}

/**
 * BURTCTimer Stop a range of timers
 * @req
 * @brief Stops all events timers after a certain event.
 * @param event: the event index after which all events timers to be stopped
 */

void BURTCTimer_StopFrom(BURTCTimer_Events_TypeDef event)
{
  RTOS_ERR err;
  uint8_t index;

  OSSchedLock(&err);
  for (index = (uint8_t) event; index < (uint8_t) NO_OF_EVENTS; index++) /*All specified timers after the event index*/
  {
    Event_Timer[index].enabled = false; /*Stop the timer*/
    Event_Timer[(uint8_t) event].period = 0u;
  }
  OSSchedUnlock(&err);
}



/**************************************************************************//**
 * @brief  Initialize GPIOs for push button and LED
 *****************************************************************************/
void GPIO_init(void)
{
  //GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeInput, 1);
  GPIO_PinModeSet(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN, gpioModePushPull, 1); // LED0 on
  GPIO_PinModeSet(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN, gpioModePushPull, 1); // LED0 on
}

/**************************************************************************//**
 * @brief  Configure BURTC to interrupt every BURTC_IRQ_PERIOD
 *
 *****************************************************************************/
void BURTC_init(void)
{
  CMU_ClockSelectSet(cmuClock_EM4GRPACLK, cmuSelect_LFXO);//ABR Techem.
  CMU_ClockEnable(cmuClock_BURTC, true);
  //CMU_ClockEnable(cmuClock_BURAM, true); //ABR Techem. not needed as we do not use EM4

  BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
  burtcInit.compare0Top = true; // reset counter when counter reaches compare value
  //burtcInit.em4comp = true;     // BURTC compare interrupt wakes from EM4 (causes reset)//ABR Techem. not needed as we do not use EM4
  burtcInit.clkDiv = burtcClkDiv_128;//ABR Techem.
  BURTC_Init(&burtcInit);

  BURTC_CounterReset();
  BURTC_CompareSet(0, BURTC_COMPARE_FOR_IRQ_PERIOD_10SEC);

  BURTC_IntEnable(BURTC_IEN_COMP);    // compare match
  NVIC_EnableIRQ(BURTC_IRQn);
  BURTC_Enable(true);
}




