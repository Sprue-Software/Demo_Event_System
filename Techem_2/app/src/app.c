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
#include <time.h>
#include <stdlib.h>

#include "em_device.h"
#include "em_common.h"
#include "em_core.h"
#include "sl_sleeptimer.h"
#include "sl_power_manager.h"
#include "sl_atomic.h"
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"
#include "os.h"
#include "em_emu.h"
#include "comms_handler.h"
#include "app.h"
#include "hal_BURTCTimer.h"
#include "system_events.h"
#include "events.h"
#include "diagnostics.h"
#include "sl_simple_button_instances.h"


/*****************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define  ABR_LOGIC 0

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
//ABR Techem. Each of the following needs to be moved to relevant module.

static OS_TCB   eventsTasktcb;

#ifdef ABR
static OS_TCB   ledBuzzTasktcb;
static OS_TCB   spiCommsTasktcb;
static OS_TCB   switchTasktcb;

static OS_TCB   wdogTimerTasktcb;
#endif
uint32_t reset_cause=0;

static CPU_STK  eventsTaskstack[EVENTS_TASK_STK_SIZE];

#ifdef ABR
static CPU_STK  ledBuzzTaskstack[LED_BUZZ_TASK_STK_SIZE];
static CPU_STK  spiCommsTaskstack[SPI_COMMS_TASK_STK_SIZE];
static CPU_STK  switchTaskstack[SWITCH_TASK_STK_SIZE];

static CPU_STK  wdogTimerTaskstack[WDOG_TIMER_TASK_STK_SIZE];
#endif

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void events_task(void *arg);
#ifdef ABR
static void ledBuzz_task(void *arg);
static void spiComms_task(void *arg);
static void switch_task(void *arg);
static void uartComms_task(void *arg);
static void wdogTimer_task(void *arg);
#endif
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
OS_FLAG_GRP Event_Flags_TopGroup;
OS_FLAG_GRP Event_Flags_SubGroup[2];
OS_FLAGS flags_0;
OS_FLAGS flags_1;
void OSIdleHook(void);
void OSIdleEnterHook(void);
void OSIdleExitHook(void);


////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**
 * Initialise application.
 ******************************************************************************/
void app_init(void)
{
  RTOS_ERR err;

  /* Use LPM 2 */
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM2);

  GPIO_init();
  BURTC_init();

  /* Create Events Task for behavioural model & event system */
  /* This is starting point of the FW */
  OSTaskCreate(&eventsTasktcb,
               "Events Task",
               events_task,
               DEF_NULL,
               EVENTS_TASK_PRIO,
               &eventsTaskstack[0],
               (EVENTS_TASK_STK_SIZE / 10u),
               EVENTS_TASK_STK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
}


/**
 * @brief Run the bootup procedures, initialising all hardware, RTOS tasks and variables
 */
static void app_bootup(void)
{
  /* Init Code for hardware  */
  /* init Debug Message */
  comms_initTask();
  GPIO_PinOutClear(BSP_GPIO_LED0_PORT, BSP_GPIO_LED1_PIN);

#if 0
/* Restart Handling */
  /* Read EEPROM and get the system mode -reset Condition */
  /*Step 1: get the System mode from EEPROM & set The System mode for first Time it will be standby */
   /* This will Handle the reset in differnt modes and after switch back to pervious mode*/
     setBehavioural_System_Modes(getBehavioural_System_Modes(true));
    /* This Function should return the System Time if the time is less than calibration time then request the Time from MCU-2*/
    //Step 2: get The System Time
    /*get_System_Time_from_EEPROM()
     * After than set_the_system_time() & Update the Eeprom
   //Step 2: get The reset Cause
     reset_cause=EMU->RSTCAUSE;
  //Step 3: get Reason for reset */
  /*record the EEPROM */
  /* log_Reset_cause();
   Clear the cause of the reset. */
   RMU_ResetCauseClear();
#endif
  //Start the Letimer for 3-5 sec (This Will give Enough Time to Settle All tasks and accept the events) to switches_getADS () and trigger the ADS event
  //runOperateModule()*/

}






/**
 * @brief    The system task is responsible for starting all other tasks in the system, and then
 *           monitoring the state of the system to ensure all tasks are behaving as expected.
 * @param    p_arg   Pointer to an optional data area which can pass parameters to the task when the
 *                   task executes. Unused here.
 */
static void events_task(void *arg)
{
  RTOS_ERR err;
  static uint32_t data=0; /*test*/
  (void)&arg; /*Unused parameters */
  /*Initialise the hardware and other tasks*/
  app_bootup();

  /* Create the event flag Sub group for Sub group 0                          */
  OSFlagCreate(&Event_Flags_SubGroup[0], /*   Pointer to user-allocated event flag.         */
               "EventFlags_0", /*   Name used for debugging.                  */
               0, /*   Initial flags, all cleared.                   */
               &err);
  /*   Check error code.                                  */
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  /* Create the event flag for Sub group 1                        */
  OSFlagCreate(&Event_Flags_SubGroup[1], /*   Pointer to user-allocated event flag.         */
               "EventFlags_1", /*   Name used for debugging.                  */
               0, /*   Initial flags, all cleared.                   */
               &err);
  /*   Check error code.                                  */
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);


#if ABR_LOGIC



  //SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
  //ABR. Call different init function for different peripherals and move
              //following task create into relevant init function

  //ABR. Create LED & Buzzer Task.
  OSTaskCreate(&ledBuzzTasktcb,
               "LedBuzz Task",
               ledBuzz_task,
               DEF_NULL,
               LED_BUZZ_TASK_PRIO,
               &ledBuzzTaskstack[0],
               (LED_BUZZ_TASK_STK_SIZE / 10u),
               LED_BUZZ_TASK_STK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);

  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  //ABR. Create SPI Comms Task.
  OSTaskCreate(&spiCommsTasktcb,
               "SpiComms Task",
               spiComms_task,
               DEF_NULL,
               SPI_COMMS_TASK_PRIO,
               &spiCommsTaskstack[0],
               (SPI_COMMS_TASK_STK_SIZE / 10u),
               SPI_COMMS_TASK_STK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);

  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  //ABR. Create Switch Task.
  OSTaskCreate(&switchTasktcb,
               "Switch Task",
               switch_task,
               DEF_NULL,
               SWITCH_TASK_PRIO,
               &switchTaskstack[0],
               (SWITCH_TASK_STK_SIZE / 10u),
               SWITCH_TASK_STK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);

  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
#endif


  /*Start-up: Start the three general timers */
  BURTCTimer_Start(TMR_timestamp_event_0, periodical, TIMESTAMP_PERIOD);
  /* if reset in functional test mode */
  if(getBehavioural_System_Modes(false)!=Functional_Test_Mode)
    {
      BURTCTimer_Start(TMR_Battery_Measurement_BIST_event_0, periodical, SMOKE_MEASUREMENT); /* For Testing */
      BURTCTimer_Start(TMR_CO_Variance_Acquisition_event_0, periodical, VARIANCE_PERIOD_NON_OPERATIONAL);
      BURTCTimer_Start(TMR_heartbeat_event_0, periodical, HEARTBEAT_PERIOD);
      BURTCTimer_Start(TMR_TempHum_measure_BIST_event_0, periodical, TEMP_HUMIDITY_PERIOD);
    }
/* testing for idle mode*/
 // Start_Diagnostic_BIST();

  /* This  is blocking function for Event Task, This function will unblock once Event received */
  while (true){
      //ABR. Will add TopGroup checking later.
      flags_0 = OSFlagPend(&Event_Flags_SubGroup[0], /* Pointer to user-allocated event flag. */
                       0xffffffffu, /* Flag bitmask to match. */
                       0,           /* Wait indefinitely. */
                       OS_OPT_PEND_FLAG_SET_ANY | /* Wait until ANY flags are set and */
                       OS_OPT_PEND_BLOCKING |     /* task will block and */
                       OS_OPT_PEND_FLAG_CONSUME,  /* consume flags */
                       DEF_NULL, /* Timestamp is not used. */
                       &err);
    /* Check error code. */
    if (RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){
     /* every 1 minute */
    if ((flags_0 & FLAGS_BIT_INDEX(TMR_timestamp_event_0)) != 0u)
      {
        GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
        debug_out("\n Time Stamp ", true, data);
        data++;
        /* Suggested steps */
        /* Step:1 time_handleTimestamp();*/
      }
    /* every 3 minute */
    if ((flags_0 & FLAGS_BIT_INDEX (TMR_AmbientLight_measure_event_0)) != 0u)
      {
        debug_out("\n Ambient Light ", true, data);
        /* Suggested steps */
        /* Step:1 Read the Ambient level from sensor and set the variable
         * Step:2 Night detected: which will be useful for all alarms
         */
        /*(void) LightSensor_LevelMeasure();*/
      }
    /* every 3 minute */
    if ((flags_0 & FLAGS_BIT_INDEX (TMR_heartbeat_event_0)) != 0u)
      {
        debug_out("\n heart beat  ", true, data);
         /* Suggested steps
         Step:1 check the Night detected flag NightTime_Detected() ?
         *Check the req. if Night Time  Heatbeat need to disabled else
         * Step 2: if (Operational_Mode==getBehavioural_System_Modes ()
         * If System mode is Operational then (Heart beat will be active in Operational Mode)
         * Step 3: ledBuzz_Post (pattern_heartbeat);
         */
      }
     /* ADS switch Conditions*/
#if 1
    if ((flags_0 & OPERATE_EVENTS) != 0u){
          /*Evaluate the operational state*/
           runOperateModule(flags_0, flags_1);
        }
#endif
    if ((flags_0 & EVENT_MODE_CHANGE_0) != 0u){
        setBehavioural_System_Modes(Transport_Mode);
        /* This Function will be call if the System Need changes & event Task Need Calling + Run Behavioural
         SPI-MCU-2 App layer will check the mode and set the Event
         * (Supported Mode)
         * standby <-> Operational , Standby <-> Transport, Operational > Transport
         * FTM mode is allowed from Operational, Transport, Commissioning mode only
         * Record the Time Stamp with who requested mode change
         record_System_Mode_with_Time_Stamp(getBehavioural_System_Modes());*/
    }

    if ((flags_0 & EVENT_FAULT_SILENCE_TIMEOUT_0) != 0u){
      DEBUG_APP("Fault Silence Timeout", false, 0u);
      /*Handle fault silence timeout and check if silence period should be extended*/
     /* faults_handleFaultSilenceTimeout();*/
    }
    if (((flags_0 & DIAGNOSTIC_EVENTS)|| (flags_0 & FLAGS_BIT_INDEX (TMR_Battery_Measurement_BIST_event_0))) != 0u){
       DEBUG_APP("\n Diagnostic", false, 0u);
       /* Suggested steps */
       /* Step:1 check the ADS switch Status ? Mounted or demounted (i.e operate_active, operate_disabled);
       * Step 3 System mode =getBehavioural_System_Modes ();
       *diagnostics(System mode ,Mounted,flags_0);*/
        /* Flag_0 will be use to decide which mode need to handle */
       diagnostics(getBehavioural_System_Modes(false),getOperateState(), flags_0);
     }
    if ((flags_0 & EVENT_CO_VARIENCE_0) != 0u){
      DEBUG_APP("\n off base variance  ", false, false);
      /* Suggested steps */
       /* below function will be use to get off Base variance (ADS- OFF & Diagnostic is stoped)
        offbase_variance();*/
    }
      /* Pass the events to the State machine */
    runBehaviouralModule(flags_0,flags_1);
    SLEEP_Sleep();
  }

}

}/*End */

#if ABR_LOGIC
//ABR Move the following tasks into relevant modules
/**
 * @brief    The ledBuzz_task is responsible for starting all other tasks in the system, and then
 *           monitoring the state of the system to ensure all tasks are behaving as expected.
 * @param    p_arg   Pointer to an optional data area which can pass parameters to the task when the
 *                   task executes. Unused here.
 */
static void ledBuzz_task(void *arg)
{
  RTOS_ERR err;
  OS_FLAGS flags;

  (void)&arg;


  //ABR. Next timer start is for testing purpose only.
  BURTCTimer_Start(LedBuzz_event, periodical, 2u);

  while (true){
    //ABR. Will be changed to pend the corresponding semaphore or mutex as it is not periodical.
    flags = OSFlagPend(&Event_Flags_SubGroup[0], /* Pointer to user-allocated event flag. */
                       0xffffffffu, /* Flag bitmask to match. */
                       0,           /* Wait indefinitely. */
                       OS_OPT_PEND_FLAG_SET_ANY | /* Wait until ANY flags are set and */
                       OS_OPT_PEND_BLOCKING |     /* task will block and */
                       OS_OPT_PEND_FLAG_CONSUME,  /* consume flags */
                       DEF_NULL, /* Timestamp is not used. */
                       &err);
    /* Check error code. */
    if (RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){

    if ((flags & FLAGS_BIT_INDEX(LedBuzz_event)) != 0u){
        GPIO_PinOutToggle(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
      }
    }
  }

}

/**
 * @brief    The ledBuzz_task is responsible for starting all other tasks in the system, and then
 *           monitoring the state of the system to ensure all tasks are behaving as expected.
 * @param    p_arg   Pointer to an optional data area which can pass parameters to the task when the
 *                   task executes. Unused here.
 */
static void spiComms_task(void *arg)
{
  RTOS_ERR err;
  OS_FLAGS flags;

  (void)&arg;



  while (true){
    //ABR. Will be changed to pend the corresponding semaphore or mutex as it is not periodical.
    flags = OSFlagPend(&Event_Flags_SubGroup[0], /* Pointer to user-allocated event flag. */
                       0xffffffffu, /* Flag bitmask to match. */
                       0,           /* Wait indefinitely. */
                       OS_OPT_PEND_FLAG_SET_ANY | /* Wait until ANY flags are set and */
                       OS_OPT_PEND_BLOCKING |     /* task will block and */
                       OS_OPT_PEND_FLAG_CONSUME,  /* consume flags */
                       DEF_NULL, /* Timestamp is not used. */
                       &err);
    /* Check error code. */
    if (RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){

    if ((flags & FLAGS_BIT_INDEX(SpiComms_event)) != 0u){
        //ABR. Add functionality code
                ;
      }
    }
  }

}

/**
 * @brief    The uartComms_task is responsible for starting all other tasks in the system, and then
 *           monitoring the state of the system to ensure all tasks are behaving as expected.
 * @param    p_arg   Pointer to an optional data area which can pass parameters to the task when the
 *                   task executes. Unused here.
 */
static void uartComms_task(void *arg)
{
  RTOS_ERR err;
  OS_FLAGS flags;

  (void)&arg;



  while (true){
    //ABR. Will be changed to pend the corresponding semaphore or mutex as it is not periodical.
    flags = OSFlagPend(&Event_Flags_SubGroup[0], /* Pointer to user-allocated event flag. */
                       0xffffffffu, /* Flag bitmask to match. */
                       0,           /* Wait indefinitely. */
                       OS_OPT_PEND_FLAG_SET_ANY | /* Wait until ANY flags are set and */
                       OS_OPT_PEND_BLOCKING |     /* task will block and */
                       OS_OPT_PEND_FLAG_CONSUME,  /* consume flags */
                       DEF_NULL, /* Timestamp is not used. */
                       &err);
    /* Check error code. */
    if (RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){

    if ((flags & FLAGS_BIT_INDEX(UartComms_event)) != 0u){
        //ABR. Add functionality code
                ;
      }
    }
  }

}
/**
 * @brief    The uartComms_task is responsible for starting all other tasks in the system, and then
 *           monitoring the state of the system to ensure all tasks are behaving as expected.
 * @param    p_arg   Pointer to an optional data area which can pass parameters to the task when the
 *                   task executes. Unused here.
 */

static void switch_task(void *arg)
{
  RTOS_ERR err;
  OS_FLAGS flags;

  (void)&arg;



  while (true){
    //ABR. Will be changed to pend the corresponding semaphore or mutex as it is not periodical.
    flags = OSFlagPend(&Event_Flags_SubGroup[0], /* Pointer to user-allocated event flag. */
                       0xffffffffu, /* Flag bitmask to match. */
                       0,           /* Wait indefinitely. */
                       OS_OPT_PEND_FLAG_SET_ANY | /* Wait until ANY flags are set and */
                       OS_OPT_PEND_BLOCKING |     /* task will block and */
                       OS_OPT_PEND_FLAG_CONSUME,  /* consume flags */
                       DEF_NULL, /* Timestamp is not used. */
                       &err);
    /* Check error code. */
    if (RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE){

    if ((flags & FLAGS_BIT_INDEX(Switch_event)) != 0u){
        //ABR. Add functionality code
                ;
      }
    }
  }

}
#endif
////////////////////////////////////////////////////////////////////////////////




/**
 * @brief    This will be called by the Micrium OS idle task when there is no other task ready to run.
 *           We enter the lowest possible energy mode; usually EM2.
 */
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT

#else
  void OSIdleHook(void)
  {
    //EMU_EnterEM2(true);
    (void) SLEEP_Sleep();
  }
#endif


void OSIdleEnterHook(void)
{
  //EMU_EnterEM2(true);
  //(void) SLEEP_Sleep();
}

void OSIdleExitHook(void)
{
  //EMU_EnterEM1();
}


/***************************************************************************//**
 * Callback on button change  For testing
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  RTOS_ERR err;
   if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
        debug_out("\n  Button Press ", false, 0u);
        OSFlagPost(&Event_Flags_SubGroup[0],  /*Pointer to user-allocated event flag.*/
                   EVENT_BUTON_PRESS_0,
                   OS_OPT_POST_FLAG_SET,    //Set the flag
                   &err);
        /*   Check error code.                                  */
        if (RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
            RTOS_ERR_SET(err, RTOS_ERR_FAIL);
        }

    }
    if (&sl_button_btn1 == handle) {
           debug_out("\n  Heat ", false, 0u);
           OSFlagPost(&Event_Flags_SubGroup[0],  /*Pointer to user-allocated event flag.*/
                      EVENT_HEAT_HIGH_SUPER_0,
                      OS_OPT_POST_FLAG_SET,    //Set the flag
                      &err);
           /*   Check error code.                                  */
           if (RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE){
               RTOS_ERR_SET(err, RTOS_ERR_FAIL);
           }
       }

  }
}




//OSIdle
