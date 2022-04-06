/*********************************************************************************
 * @file  event_system.c
 * @brief Handle the behaviour of the alarm. Event handler.
 * @project Techem-P200 Firmware
 * @date  29 Mar 2022
 * @author  NDI

 *******************************************************************************/

/* Includes required by this module */

#include "em_cmu.h"
#include "em_emu.h"

#include  <cpu/include/cpu.h>
#include  <common/include/common.h>
#include  <kernel/include/os.h>
#include  <kernel/source/os_priv.h>
#include  <common/include/lib_def.h>
#include  <common/include/rtos_utils.h>
#include  <common/include/toolchains.h>
#include "em_core.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_rmu.h"
#include "hal_BURTCTimer.h"
#include "events.h"
#include "app.h"
#include "system_events.h"
#include "diagnostics.h"
#include "SwitchTask.h"
#include "Acquisition_CO.h"
#include "Acquisition_Smoke.h"
#include "Acquisition_Heat.h"
/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
/* default System Mode & System Operational State & device is de-mounted*/
static behaviour_state_enum_System_modes behavioural_system_mode = Standby_Mode;
static behaviour_state_enum_operational_States behavioural_operational_State =    state_Idle;
static behaviour_state_enum_System_modes behavioural_system_mode_previous =    Standby_Mode;
static ADS_operate_state_enum ADS_operateState = operate_disabled;
static co_state_enum coState = co_none;
static Smoke_state_enum SmokeState = Smoke_none;
static heat_state_enum HeatState = Heat_none;
static bool alarmSilenceLimitReached = false;

/* NDI For Dummy Function
 *
 */
button_state_t switch_test = NO_PRESS;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

/*Function definitions for ADS Switch*/
static void handleDeviceEnable (void);
static void handleDeviceDisable (void);
void Start_Diagnostic_BIST(void);
void Stop_Diagnostic_BIST(void);
/*Function definitions for System Mode handling*/
static void handle_System_Standby_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_System_Commisioning_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_System_Operational_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_System_Transport_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_System_ShutDown_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1);

/*Function definitions for Operational System Mode different State handling*/
static void handle_State_idle (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_state_Smoke_Alarm (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_state_Smoke_Silence (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_state_Heat_Alarm (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_state_Heat_Silence (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_state_CO_Alarm (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_state_CO_Silence (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_state_Remote_Alarm (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_state_Domestic_Test (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_State_Airing_Configuration (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void handle_System_Functional_Test_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1);
static void jumpToSmokeAlarm (bool newAlarm);
static void jumpToHeatAlarm (bool newAlarm);
static void jumpToCOAlarm (bool newAlarm);
static void jumpToRemoteAlarm (bool newAlarm);

/**
 * @brief This function handles control of the operate state machine
 * @details The operate state machine is used to control the behaviour of the
 *
 * @return nothing to return.
 */
void runOperateModule (OS_FLAGS flags_0, OS_FLAGS flags_1)
{

  (void)flags_1;
  /* Handle The reset Conditions*/

  switch (ADS_operateState)
    {
    case operate_active:
      /*
       * In active on-base/shipping tag removed state
       * From here can either enter off-base state or shutdown(very low battery) state
       */
      if ((flags_0 & FLAGS_BIT_INDEX(TMR_Device_ADS_disable_event_0)) != 0u)
        {
          DEBUG_APP("Enter DISABLED", false, 0u);
          handleDeviceDisable ();
          ADS_operateState = operate_disabled;

        }

      /* NOTE: device shutdown should not be triggered for product with replaceable batteries,
       as quick battery replacement may not cause device reset and shutdown exit. */

      if ((flags_0 & FLAGS_BIT_INDEX(TMR_Device_Shutdown_event_0)) != 0u)
        {
          DEBUG_APP("Enter SHUTDOWN", false, 0u);
          behavioural_system_mode = Shutdown_Mode;
        }

      break;

    case operate_disabled:
      /*
       * In Disable/Off-base/shipping tag inserted state
       * From here can either enter on base state
       */
      if ((flags_0 & FLAGS_BIT_INDEX(TMR_Device_ADS_enable_event_0)) != 0u)
        {
          DEBUG_APP("Enter ACTIVE", false, 0u);
          ADS_operateState = operate_active;/* Device has been placed on base so switch to operate_active State */
          handleDeviceEnable ();

        }
      break;

    default:
      /*Shouldn't reach here*/
      break;
    }
}

/**
 * @brief This function handles control of the behaviour state machine
 * @details The behavioural state machine is used to control all aspects of
 *      the device behaviour in relation to non- disable input events.
 * @req
 * @return nothing to return
 */
void runBehaviouralModule (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  switch (behavioural_system_mode)
    {
    case Standby_Mode:/* Standby_Mode: This is The mode when we received a device from Factory*/
      handle_System_Standby_Mode(flags_0, flags_1);
      break;
    case Commisioning_Mode:/* Commisioning_Mode : This is The mode for when Mounted event received in standby mode*/
      handle_System_Commisioning_Mode(flags_0, flags_1);
      break;
    case Operational_Mode:/* Operational_Mode : This is The mode for when device is in operational mode*/
      handle_System_Operational_Mode(flags_0, flags_1);
      break;
    case Functional_Test_Mode: /*Functional_Test_Mode: This is The mode for validation testing*/
      handle_System_Functional_Test_Mode(flags_0, flags_1);
      break;
    case Transport_Mode:/* Transport_Mode: This is The mode for returned devices*/
      handle_System_Transport_Mode(flags_0, flags_1);
      break;
      /* No need to Have Shutdown mode*/
    case Shutdown_Mode:/*Shutdown_Mode  This is The mode for battery critically low and safe shutdown*/
      handle_System_ShutDown_Mode(flags_0, flags_1);
      break;
    default:
      /*Shouldn't reach here*/
      break;
    }
  /* End of Switch*/
}

/**
 * @brief Handle the Standby Mode & events
 * @param flags_0 & flags_1
 */
static void handle_System_Standby_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;

  if ((flags_0 & FLAGS_BIT_INDEX(TMR_BUTTON_PRESS_0)) != 0u)
    {
      DEBUG_APP("Display Standby Mode", false, 0u);
      /* Dummy Function*/
       /* Get the Type of Button press */
      if (get_switches_type()==SHORT_PRESS)
        {
            /* Take a Action */
          /*LED Post for Standby Mode */
          /*ledBuzz_Post (Stanby_pattern)*/
        }
    }
  /* Mode Change By service tool : SPI handler function will set the mode & will set Mode change event*/
}
/**
 * @brief Handle the Standby Mode & events
 * @param flags_0 & flags_1
 */
static void handle_System_Transport_Mode(OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;


  if ((flags_0 & FLAGS_BIT_INDEX(TMR_BUTTON_PRESS_0)) != 0u)
    {
      DEBUG_APP("Display Transport Mode", false, 0u);
      /* Dummy Function*/
            /* Get the Type of Button press */
           if (get_switches_type()==SHORT_PRESS)
             {

                 /* Take a Action */
               /*LED Post for Standby Mode */
               /*ledBuzz_Post (Stanby_pattern)*/
             }

    }

  /* Mode Change By service tool : SPI handler function will set the mode & will set Mode change event accondingly*/
}
/**
 * @brief Handle the Commissioning Mode & events
 * @param flags_0 & flags_1
 */
static void handle_System_Commisioning_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;
  (void)flags_0;

  DEBUG_APP("Stuck in Commisioning mode", false, 0u);
  /* If demounted then it's handle by handleDeviceDisable()
   * disable all signals and change the system state = Standby Mode
   */
  /* if Mounted*/

}
/**
 * @brief Handle the Operational Mode & events
 * @param flags_0 & flags_1
 */
static void handle_System_Operational_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  switch (behavioural_operational_State)
    {
    case state_Idle:/* Idle State: This is where the product will spend most of its life.*/
      handle_State_idle (flags_0, flags_1);
      break;
    case state_Smoke_Alarm:/* CO Low Level Warning State: This state will handle the unit being in warning mode.*/
      handle_state_Smoke_Alarm (flags_0, flags_1);
      break;
    case state_Smoke_Alarm_Silence:/* CO Low Level Warning Silence State: This state will handle when the warning mode is silenced.*/
      handle_state_Smoke_Silence (flags_0, flags_1);
      break;
    case state_Heat_Alarm: /*LLW during Alarm Memory: variant of LLW state but with handling of alarm memory timeout period.*/
      handle_state_Heat_Alarm (flags_0, flags_1);
      break;
    case state_Heat_Alarm_Silence:/* CO_LocalAlarm: This state will be used to manage the device when in Local Alarm*/
      handle_state_Heat_Silence (flags_0, flags_1);
      break;
    case state_CO_Alarm:/* CO_LocalAlarmSilence: This state will be used to manage the device when in Silent Local Alarm*/
      handle_state_CO_Alarm (flags_0, flags_1);
      break;
    case state_CO_Alarm_Silence:/* CO_LocalAlarmSilence: This state will be used to manage the device when in Silent Local Alarm*/
      handle_state_CO_Silence (flags_0, flags_1);
      break;
    case state_Remote_Alarm:/* CO_LocalAlarmSilence: This state will be used to manage the device when in Silent Local Alarm*/
      handle_state_Remote_Alarm (flags_0, flags_1);
      break;
    case state_Domestic_Test:/* CO_LocalAlarmSilence: This state will be used to manage the device when in Silent Local Alarm*/
      handle_state_Domestic_Test (flags_0, flags_1);
      break;
    case State_Airing_Configuration:/* CO_LocalAlarmSilence: This state will be used to manage the device when in Silent Local Alarm*/
      handle_State_Airing_Configuration (flags_0, flags_1);
      break;

    default:
      /*Shouldn't reach here*/
      break;
    }
  /* End of Switch*/
}
/**
 * @brief Handle the Transport Mode & events
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_System_Functional_Test_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;

   /*ledBuzz_Post (Idle_pattern)*/
  /* After Timeout Change System mode To Previous mode Set Mode & Start the Timers*/
  /* Stop All Timers */
  Stop_All_timers_except_timestamp();
  DEBUG_APP("Start Functional Test Mode", false, 0u);
  if ((flags_0 & FLAGS_BIT_INDEX(TMR_State_Timeout_event_0)) != 0u)
    {
      /* All generic Timers are started */
      Start_All_timers();
      /* check the button press Type & perform the LED*/
      DEBUG_APP("Exit Functional Test Mode", false, 0u);
      /* This will swicthback the FTM Previous mode*/
      /* Start All Timers if operational mode*/
      if ((behavioural_system_mode_previous==Operational_Mode)&&(ADS_operateState==operate_active))
        {

          Start_Diagnostic_BIST();
          setBehavioural_Operational_State(state_Idle);
        }
      else
        {

          setBehavioural_System_Modes(behavioural_system_mode_previous);
        }

    }
  else
    {
      /*LDRA */
    }
  /* Get the request & start the Timers BURTC or LETIMERS in periodic as per FTM requirements*/

}
/**
 * @brief Handle the Shutdown Mode & events
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_System_ShutDown_Mode (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;
  (void)flags_0;

  /* Stop All Timers */
  /* Check Voltage level */
  /* If possible write on EEPROM inform MCU-2*/
  //log_advancedEvent(eventType_shutdownEnter, NULL); /*Log event to EEPROM*/

  /*stop LED/buzz*/
 // halSounder_OFF(); /*Stop the sounder*/
 // ledSwitch_configHardware (ledSwitch_hwMode_idle); /*turn LEDs off and configure switches*/


  /*Stop BURTC and letimer*/
  BURTCTimer_StopFrom(0);
  BURTC_Enable(false);
 // LETIMER_Enable(LETIMER0, false);

  /*Disable Watchdog*/
 // WDOGn_Enable(WDOG0, false);

  /*stop all tasks*/
//  OSTaskSuspend(&CommsTaskTCB, &err);
  //OSTaskSuspend(&LedBuzzTaskTCB, &err);
//  OSTaskSuspend(&switchTaskTCB, &err);


  /*Go to sleep in lowest possible mode*/

  (void) SLEEP_Sleep();

}


 /**
 * @brief Function definitions for Operational System Mode different State handling
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_State_idle (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;
  static int data =0;
  DEBUG_APP("Idle mode", true, data);
  data++;
  if ((flags_0 & FLAGS_BIT_INDEX(TMR_Smoke_HIGH_SUPER_event_0)) != 0u)
    {
      DEBUG_APP("Smoke Alarm", false, 0u);
      /*check the  Smoke level and set the variables accordingly*/
      if (getSmokeAfterCompensation()>=SUPER_SMOKE)
        {
          SmokeState= Smoke_super ;
        }
      else
        {
          SmokeState=Smoke_high;
        }
      /* Smoke Alarm Post + Logging */
      jumpToSmokeAlarm (true);
    }
  else
      {
        /*LDRA */
      }

  /* As the Smoke increase rate is 2 sec, Need to set LETIMER in Periodic
    Can Handle different by calling Letimer & Smoke Acqusition */
#if 0
  if ((flags_0 & FLAGS_BIT_INDEX (TMR_Smoke_IncreasedSampleRate_event_0)) != 0u)
    {
        /*check the Smoke level set the variable Smoke_super*/
        DEBUG_APP("Smoke Alarm", false, 0u);
         /*jumpToSmokeAlarm();*/
       }
 #endif
  if ((flags_0 & FLAGS_BIT_INDEX(TMR_Heat_HIGH_SUPER_event_0)) != 0u)
    {
      /*check the heat level set the variable Smoke_super*/
      DEBUG_APP("Heat Alarm", false, 0u);


      /*check the  Heat level and set the variables accordingly
       *  HeatState=Heat_high or Heat_super
       */
         if (getHeatAfterCompensation()>=SUPER_HEAT)
           {
             HeatState= Heat_super ;
           }
         else
           {
             HeatState=Heat_high;
           }
         /* Heat Alarm Post + Logging */
      jumpToHeatAlarm (true);
    }


  if ((flags_0 & FLAGS_BIT_INDEX(TMR_COHB_HIGH_SUPER_event_0)) != 0u)
    {

      /*check the  Co level and set the variables accordingly
       *  coState=co_high or co_super
       */
         if (getCoAfterCompensation()>=SUPER_CO)
           {
             coState= co_super ;
           }
         else
           {
             coState=co_high;
           }

      DEBUG_APP("Co Alarm", false, 0u);
      /* CO Alarm Post + Logging */
      jumpToCOAlarm (true);
    }

  if ((flags_0 & FLAGS_BIT_INDEX(TMR_CO_IntelligentsampleRate_event_0)) != 0u)
    {
      /*check the heat level set the variable Smoke_super*/
      DEBUG_APP("Co Increased Sample ", false, 0u);
      /* Call Only Co  acquisition */
      /*acquisition_co()*/

    }
  if ((flags_0 & FLAGS_BIT_INDEX(TMR_RADIO_Alarm_event_0)) != 0u)
    {
      DEBUG_APP("Remote Alarm", false, 0u);
      /* Remote Alarm Post + Logging */
      jumpToRemoteAlarm (true);
    }
  if ((flags_0 & FLAGS_BIT_INDEX(TMR_BUTTON_PRESS_0)) != 0u)
    {

      switch_test=get_switches_type();
     if (switch_test==SHORT_PRESS)
       {
          DEBUG_APP(" Short Button Press", false, 0u);
         /* Perform Any Action on it */
         /* Fault Silence */
       }

      if (switch_test==LONG_PRESS)
        {
        /* Call Any Action */
         // diagnostics(behavioural_system_mode, ADS_operateState, 0);
         }
    /* Long Press & Hold*/
      if (switch_test==Sequense_PRESS_Domestic_user)
        {
          /* Call Diagnostic Bist and POST LED pattern from Diagnostic after completion  */
           diagnostics(behavioural_system_mode, ADS_operateState, 0);
           /*
        }
      if (switch_test==Sequense_PRESS_Extended_Domestic_user)
        {
          Call Diagnostic Bist and POST LED pattern from Diagnostic after completion
         Call Diagnostic with extended*/
          diagnostics(behavioural_system_mode, ADS_operateState, 0);

        }
      if (switch_test==Sequense_PRESS_Airing_Configuration)
        {
            /* Stop All Diagnostic Timers*/
             Stop_Diagnostic_BIST();
             /* Change System Mode = Airing Configuration*/
             behavioural_operational_State=State_Airing_Configuration;
            /* Change Set System Mode event to Configuration with Timeout of 30 Sec*/
            /* Start LEtimer for 30 sec timeout on time out change system mode to operational mode Idle state*/
            }
      else
          {
            /*LDRA */
          }
    }


}

 /**
 * @brief Function definitions for Operational System Mode different State handling
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_state_Smoke_Alarm (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;

  if ((flags_0 & FLAGS_BIT_INDEX(TMR_Smoke_HIGH_SUPER_event_0)) != 0u)
    {
      DEBUG_APP("Smoke Alarm", false, 0u);
      /*check the  Smoke level and set the variables accordingly*/
      if (getSmokeAfterCompensation()>=SUPER_SMOKE)
        {
          SmokeState= Smoke_super ;
        }
      else
        {
          DEBUG_APP("Do nothing in Smoke Alarm", false, 0u);
        }

    }
  if ((flags_0 & FLAGS_BIT_INDEX(TMR_Smoke_NONE_event_0)) != 0u)
    {
      /*check the Smoke level set the variable Smoke_super*/
      DEBUG_APP("Smoke LOW", false, 0u);
      /* Clear all events*/
      SmokeState = Smoke_none; /*Update local Smoke state*/
      /* Log The Alarm Stop*/
      behavioural_operational_State = state_Idle; /*Change state*/
      /* ledBuzz_Post (pattern_idle); Stop alarm pattern */

    }
  /* Any Button Press Event will consider As Silence */
  if ((flags_0 & FLAGS_BIT_INDEX(TMR_BUTTON_PRESS_0)) != 0u)
    {
      /*check the heat level set the variable Smoke_super*/
      DEBUG_APP("Smoke Alarm Button Press", false, 0u);
      if ((SmokeState != Smoke_super) && (alarmSilenceLimitReached == false))
        { /*Check if silence is permitted*/
          alarmSilenceLimitReached = true; /*Mark that silence action has been used, only 1 silence allowed per alarm*/
          DEBUG_APP("Enter Local Smoke Alarm Silence Mode", false, 0u);
          BURTCTimer_Start (TMR_State_Timeout_event_0, false,ALARM_SILENCE_TIMEOUT); /*Start silence timeout*/
          behavioural_operational_State = state_Smoke_Alarm_Silence; /*Change state*/
          /*ledBuzz_Post (pattern_alarm_Smoke_silence); Start silence pattern
          *log_advancedEvent(eventType_localAlarmSilenceActivated, NULL);Log event to EEPROM*/
        }
      else
        {
          DEBUG_APP("Silence not allowed when in SUPER Smoke", false, 0u);
        }
    }

}

 /**
 * @brief Function definitions for Operational System Mode different State handling
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_state_Smoke_Silence (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;

  if ((flags_0 & FLAGS_BIT_INDEX(TMR_Smoke_HIGH_SUPER_event_0)) != 0u)
    {
      if (getSmokeAfterCompensation()>=SUPER_SMOKE)
            {
              SmokeState= Smoke_super ;
             /* Smoke Alarm Post + Logging , break the silence */
              jumpToSmokeAlarm (false);/*Enter alarm state, but not as a new alarm*/
            }
          else
            {
              DEBUG_APP("Do nothing in Smoke Alarm", false, 0u);
            }
    }
  if ((flags_0 & FLAGS_BIT_INDEX(TMR_Smoke_NONE_event_0)) != 0u)
    {
      /*check the Smoke level set the variable Smoke_super*/
      DEBUG_APP("Smoke LOW", false, 0u);
      /* Clear all events*/
      SmokeState = Smoke_none; /*Update local Smoke state*/
      behavioural_operational_State = state_Idle; /*Change state*/
      /* Log The Alarm Stop*/
      /* ledBuzz_Post (pattern_idle); Stop alarm pattern */

    }

  if ((flags_0 & FLAGS_BIT_INDEX(TMR_State_Timeout_event_0)) != 0u)
    {
      /*Silence timeout so back to full alarm state*/
      DEBUG_APP("Local Smoke Alarm Silence Timeout", false, 0u);
      /* log_advancedEvent(eventType_localAlarmSilenceExit, NULL); Log to EEPROM*/
      jumpToSmokeAlarm(false); /*Enter alarm state, but not as a new alarm*/

}

}

 /**
 * @brief Function definitions for Operational System Mode different State handling
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_state_Heat_Alarm (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;

if ((flags_0 & FLAGS_BIT_INDEX(TMR_Smoke_HIGH_SUPER_event_0)) != 0u)
{
    /*check the  Smoke level and set the variables accordingly*/
       if (getSmokeAfterCompensation()>=SUPER_SMOKE)
         {
           SmokeState= Smoke_super ;
         }
       else
         {
           SmokeState=Smoke_high;
         }
       /* Smoke Alarm Post + Logging */
       jumpToSmokeAlarm (true);/*Enter alarm state, as a new alarm*/
}

/* As the Smoke increase rate is 2 sec, Need to set LETIMER in Periodic
 * No Need to handle here */
#if 0
    if ((flags_0 & FLAGS_BIT_INDEX (TMR_Smoke_IncreasedSampleRate_event_0)) != 0u)
      {
          /*check the Smoke level set the variable Smoke_super*/
          DEBUG_APP("Smoke Alarm", false, 0u);
           /*jumpToSmokeAlarm();*/
         }
   #endif

    if ((flags_0 & FLAGS_BIT_INDEX(TMR_Heat_HIGH_SUPER_event_0)) != 0u)
      {
        DEBUG_APP("Heat Alarm", false, 0u);
        /*check the  Smoke level and set the variables accordingly*/
        if (getHeatAfterCompensation()>=SUPER_HEAT)
          {
            HeatState= Heat_super ;
          }
        else
          {
            DEBUG_APP("Do nothing in Heat Alarm", false, 0u);
          }

      }
if ((flags_0 & FLAGS_BIT_INDEX(TMR_Heat_NONE_event_0)) != 0u)
{
  /*check the Smoke level set the variable Smoke_super*/
  DEBUG_APP("Smoke LOW", false, 0u);
  /* Clear all events*/
  HeatState = Heat_none; /*Update local Smoke state*/
  behavioural_operational_State = state_Idle; /*Change state*/
  /* Log The Alarm Stop
 ledBuzz_Post (pattern_idle); Stop alarm pattern */

}
/* Any Button Press Event will consider As Silence */
if ((flags_0 & FLAGS_BIT_INDEX(TMR_BUTTON_PRESS_0)) != 0u)
{
  if ((HeatState != Heat_super) && (alarmSilenceLimitReached == false))
    { /*Check if silence is permitted*/
      alarmSilenceLimitReached = true; /*Mark that silence action has been used, only 1 silence allowed per alarm*/
      DEBUG_APP("Enter Local Heat Alarm Silence Mode", false, 0u);
      BURTCTimer_Start (TMR_State_Timeout_event_0, false,ALARM_SILENCE_TIMEOUT); /*Start silence timeout*/
      behavioural_operational_State = state_Heat_Alarm_Silence; /*Change state*/
      /*ledBuzz_Post (pattern_alarm_Smoke_silence); Start silence pattern
    log_advancedEvent(eventType_localAlarmSilenceActivated, NULL);Log event to EEPROM*/
    }
  else
    {
      DEBUG_APP("Silence not allowed when in SUPER Heat", false, 0u);
    }
}
}

 /**
 * @brief Function definitions for Operational System Mode different State handling
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_state_Heat_Silence (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;

if ((flags_0 & FLAGS_BIT_INDEX(TMR_Smoke_HIGH_SUPER_event_0)) != 0u)
{
  DEBUG_APP("Smoke Alarm", false, 0u);
  /*check the  Smoke level and set the variables accordingly*/
        if (getSmokeAfterCompensation()>=SUPER_SMOKE)
          {
            SmokeState= Smoke_super ;
          }
        else
          {
            SmokeState=Smoke_high;
          }
        /* Smoke Alarm Post + Logging */
        jumpToSmokeAlarm (true);/*Enter alarm state, as a new alarm*/
}

/* As the Smoke increase rate is 2 sec, Need to set LETIMER in Periodic
 * No Need to handle here */
#if 0
     if ((flags_0 & FLAGS_BIT_INDEX (TMR_Smoke_IncreasedSampleRate_event_0)) != 0u)
       {
           /*check the Smoke level set the variable Smoke_super*/
           DEBUG_APP("Smoke Alarm", false, 0u);
            /*jumpToSmokeAlarm();*/
          }
    #endif
if ((flags_0 & FLAGS_BIT_INDEX(TMR_Heat_HIGH_SUPER_event_0)) != 0u)
{

        DEBUG_APP("Heat Alarm", false, 0u);
        /*check the  Heat level and set the variables accordingly*/
        if (getHeatAfterCompensation()>=SUPER_HEAT)
          {
            HeatState= Heat_super ;
            /* Heat Alarm Post + Logging */
            jumpToHeatAlarm(false);/*Enter alarm state, not as a new alarm*/
          }
        else
          {
            DEBUG_APP("Do nothing in Heat Alarm", false, 0u);
          }


}
if ((flags_0 & FLAGS_BIT_INDEX(TMR_Heat_NONE_event_0)) != 0u)
{
  DEBUG_APP("Heat LOW", false, 0u);
  /* Clear all events*/
  HeatState = Heat_none; /*Update local Smoke state*/
  /* Log The Alarm Stop*/
  behavioural_operational_State = state_Idle; /*Change state*/
  /* ledBuzz_Post (pattern_idle) Stop alarm pattern */

}

if ((flags_0 & FLAGS_BIT_INDEX(TMR_State_Timeout_event_0)) != 0u)
{
  /*Silence timeout so back to full alarm state*/
  DEBUG_APP("Local Heat Alarm Silence Timeout", false, 0u);
  /* log_advancedEvent(eventType_localAlarmSilenceExit, NULL); Log to EEPROM*/
jumpToHeatAlarm(false); /*Enter alarm state, but not as a new alarm*/
}
}

 /**
 * @brief Function definitions for Operational System Mode different State handling
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_state_CO_Alarm (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;

if ((flags_0 & FLAGS_BIT_INDEX(TMR_Smoke_HIGH_SUPER_event_0)) != 0u)
{
    DEBUG_APP("Smoke Alarm", false, 0u);
    /*check the  Smoke level and set the variables accordingly*/
    if (getSmokeAfterCompensation()>=SUPER_SMOKE)
      {
        SmokeState= Smoke_super ;
      }
    else
      {
        SmokeState=Smoke_high;
      }
    /* Smoke Alarm Post + Logging */
    jumpToSmokeAlarm (true);/*Enter alarm state, as a new alarm*/


}

/* As the Smoke increase rate is 2 sec, Need to set LETIMER in Periodic
 * No Need to handle here */
#if 0
  if ((flags_0 & FLAGS_BIT_INDEX (TMR_Smoke_IncreasedSampleRate_event_0)) != 0u)
    {
        /*check the Smoke level set the variable Smoke_super*/
        DEBUG_APP("Smoke Alarm", false, 0u);
         /*jumpToSmokeAlarm();*/
       }
 #endif
if ((flags_0 & FLAGS_BIT_INDEX(TMR_Heat_HIGH_SUPER_event_0)) != 0u)
{
    /*check the  Heat level and set the variables accordingly
         *  HeatState=Heat_high or Heat_super
         */
           if (getHeatAfterCompensation()>=SUPER_HEAT)
             {
               HeatState= Heat_super ;
             }
           else
             {
               HeatState=Heat_high;
             }
           /* Heat Alarm Post + Logging */
        jumpToHeatAlarm (true); /*Enter alarm state, as a new alarm*/
}


if ((flags_0 & FLAGS_BIT_INDEX(TMR_COHB_HIGH_SUPER_event_0)) != 0u)
{
    /*check the Smoke level set the variable Smoke_super*/
    DEBUG_APP("CO Alarm", false, 0u);
     /*check the  Smoke level and set the variables accordingly*/
    if (getCoAfterCompensation()>=SUPER_CO)
      {
        coState= co_super ;
      }
    else
      {
        coState=co_high;
      }
 }

if ((flags_0 & FLAGS_BIT_INDEX(TMR_BUTTON_PRESS_0)) != 0u)
{
    if ((coState != co_super) && (alarmSilenceLimitReached == false))
      { /*Check if silence is permitted*/
        alarmSilenceLimitReached = true; /*Mark that silence action has been used, only 1 silence allowed per alarm*/
        DEBUG_APP("Enter Local CO Alarm Silence Mode", false, 0u);
        BURTCTimer_Start (TMR_State_Timeout_event_0, false, ALARM_SILENCE_TIMEOUT); /*Start silence timeout*/
        behavioural_operational_State = state_CO_Alarm_Silence; /*Change state*/
        /*log_advancedEvent(eventType_localAlarmSilenceActivated, NULL);Log event to EEPROM*/
        /*ledBuzz_Post (pattern_alarm_CO_silence); Start silence pattern*/
      }
    else
      {
        DEBUG_APP("Silence not allowed when in SUPER CO", false, 0u);
      }

}

if ((flags_0 & FLAGS_BIT_INDEX(TMR_COHB_NONE_event_0)) != 0u)
{
    /*check the Smoke level set the variable Smoke_super*/
    DEBUG_APP("CO LOW", false, 0u);
    /* Clear all events*/
    coState = co_none; /*Update local co state*/
    behavioural_operational_State = state_Idle; /*Change state*/
    /* Log The Alarm Stop*/
    /* ledBuzz_Post (pattern_idle); Stop alarm pattern */
}

}

 /**
 * @brief Function definitions for Operational System Mode different State handling
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_state_CO_Silence (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;

  if ((flags_0 & FLAGS_BIT_INDEX(TMR_Smoke_HIGH_SUPER_event_0)) != 0u)
  {
      DEBUG_APP("Smoke Alarm", false, 0u);
      /*check the  Smoke level and set the variables accordingly*/
      if (getSmokeAfterCompensation()>=SUPER_SMOKE)
        {
          SmokeState= Smoke_super ;
        }
      else
        {
          SmokeState=Smoke_high;
        }
      /* Smoke Alarm Post + Logging */
      jumpToSmokeAlarm (true); /*Enter alarm state, as a new alarm*/
  }

/* As the Smoke increase rate is 2 sec, Need to set LETIMER in Periodic
 * No Need to handle here */
#if 0
    if ((flags_0 & FLAGS_BIT_INDEX (TMR_Smoke_IncreasedSampleRate_event_0)) != 0u)
      {
          /*check the Smoke level set the variable Smoke_super*/
          DEBUG_APP("Smoke Alarm", false, 0u);
           /*jumpToSmokeAlarm();*/
         }
   #endif
    if ((flags_0 & FLAGS_BIT_INDEX(TMR_Heat_HIGH_SUPER_event_0)) != 0u)
    {
        /*check the  Heat level and set the variables accordingly
                *  HeatState=Heat_high or Heat_super
                */
                  if (getHeatAfterCompensation()>=SUPER_HEAT)
                    {
                      HeatState= Heat_super ;
                    }
                  else
                    {
                      HeatState=Heat_high;
                    }
                  /* Heat Alarm Post + Logging */
               jumpToHeatAlarm (true); /*Enter alarm state, as a new alarm*/}


    if ((flags_0 & FLAGS_BIT_INDEX(TMR_COHB_HIGH_SUPER_event_0)) != 0u)
    {
        if (getCoAfterCompensation()>=SUPER_CO)
          {
            coState= co_super ;
            /* CO Alarm Post + Logging break the Silence*/
            jumpToCOAlarm (false);
          }
        else
          {
            coState=co_high;
          }
    }


if ((flags_0 & FLAGS_BIT_INDEX(TMR_State_Timeout_event_0)) != 0u)
{
    /*Silence timeout so back to full alarm state*/
    DEBUG_APP("Local CO Alarm Silence Timeout", false, 0u);
    /* log_advancedEvent(eventType_localAlarmSilenceExit, NULL); Log to EEPROM*/
    jumpToCOAlarm(false); /*Enter alarm state, but not as a new alarm*/

}

if ((flags_0 & FLAGS_BIT_INDEX(TMR_COHB_NONE_event_0)) != 0u)
{
    /*check the Smoke level set the variable Smoke_super*/
     DEBUG_APP("CO LOW", false, 0u);
     /* Clear all events*/
     coState = co_none; /*Update local co state*/
     behavioural_operational_State = state_Idle; /*Change state*/
     /* Log The Alarm Stop
     ledBuzz_Post (pattern_idle); Stop alarm pattern */

}

}
/*
 /**
 * @brief Function definitions for Operational System Mode different State handling
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_state_Remote_Alarm (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_1;

  if ((flags_0 & FLAGS_BIT_INDEX(TMR_Smoke_HIGH_SUPER_event_0)) != 0u)
     {
          DEBUG_APP("Smoke Alarm", false, 0u);
           /*check the  Smoke level and set the variables accordingly*/
           if (getSmokeAfterCompensation()>=SUPER_SMOKE)
             {
               SmokeState= Smoke_super ;
             }
           else
             {
               SmokeState=Smoke_high;
             }
               /* Smoke Alarm Post + Logging */
           jumpToSmokeAlarm (true); /*Enter alarm state, as a new alarm*/
     }
     /* As the Smoke increase rate is 2 sec, Need to set LETIMER in Periodic
    * No Need to handle here */
 #if 0
   if ((flags_0 & FLAGS_BIT_INDEX (TMR_Smoke_IncreasedSampleRate_event_0)) != 0u)
     {
         /*check the Smoke level set the variable Smoke_super*/
         DEBUG_APP("Smoke Alarm", false, 0u);
          /*jumpToSmokeAlarm();*/
        }
  #endif
   if ((flags_0 & FLAGS_BIT_INDEX(TMR_Heat_HIGH_SUPER_event_0)) != 0u)
     {
       /*check the  Heat level and set the variables accordingly
               *  HeatState=Heat_high or Heat_super
               */
                 if (getHeatAfterCompensation()>=SUPER_HEAT)
                   {
                     HeatState= Heat_super ;
                   }
                 else
                   {
                     HeatState=Heat_high;
                   }
                 /* Heat Alarm Post + Logging */
              jumpToHeatAlarm (true); /*Enter alarm state, as a new alarm*/
     }

   if ((flags_0 & FLAGS_BIT_INDEX(TMR_COHB_HIGH_SUPER_event_0)) != 0u)
     {
       /*check the  Co level and set the variables accordingly
            *  coState=co_high or co_super
            */
              if (getCoAfterCompensation()>=SUPER_CO)
                {
                  coState= co_super ;
                }
              else
                {
                  coState=co_high;
                }

           DEBUG_APP("Co Alarm", false, 0u);
           /* CO Alarm Post + Logging */
           jumpToCOAlarm (true);
     }

   if ((flags_0 & FLAGS_BIT_INDEX(TMR_CO_IntelligentsampleRate_event_0)) != 0u)
     {
       /*check the heat level set the variable Smoke_super*/
       DEBUG_APP("Co Acquisition ", false, 0u);
       //Call Only Co  acquisition */
       /*acquisition_co()*/

     }

if ((flags_0 & FLAGS_BIT_INDEX(TMR_BUTTON_PRESS_0)) != 0u)
{
    if ((alarmSilenceLimitReached == false))
         { /*Check if silence is permitted*/

        alarmSilenceLimitReached = true; /*Mark that silence action has been used, only 1 silence allowed per alarm*/
        DEBUG_APP("Enter Local Remote Alarm Silence Mode", false, 0u);
        BURTCTimer_Start (TMR_State_Timeout_event_0, false,ALARM_SILENCE_TIMEOUT); /*Start silence timeout*/
          /*ledBuzz_Post (pattern_alarm_Smoke_silence); Start silence pattern
         log_advancedEvent(eventType_localAlarmSilenceActivated, NULL);Log event to EEPROM*/
         }
}

if ((flags_0 & FLAGS_BIT_INDEX(TMR_State_Timeout_event_0)) != 0u)
{
    /*Silence timeout so back to full alarm state*/
     DEBUG_APP("Remote Alarm Silence Timeout", false, 0u);
     /* log_advancedEvent(eventType_localAlarmSilenceExit, NULL); Log to EEPROM*/
     /* Check If remote Alarm condition is still Available  */
     /* bool Remote_Alarm=get_remote_alarm_status_MCU_2();
     if (Remote_Alarm==true)
     jumpToRemoteAlarm(false); /*Enter alarm state, but not as a new alarm*/
    // else
//           /* Clear all events*/
//            /* Log The Alarm Stop*/
//            /* ledBuzz_Post (pattern_idle); /*Stop alarm pattern */
//            behavioural_operational_State = state_Idle; /*Change state*/
//            */
}
}
#if 1
 /**
 * @brief Function definitions for Operational System Mode different State handling
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_state_Domestic_Test (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  (void)flags_0;
  (void)flags_1;

}

#endif
 /**
 * @brief Function definitions for Operational System Mode different State handling
 * @param flags_0 & flags_1
 * @return nothing to return
 */
static void handle_State_Airing_Configuration (OS_FLAGS flags_0, OS_FLAGS flags_1)
{
  if ((flags_1 & FLAGS_BIT_INDEX(TMR_BUTTON_PRESS_0)) != 0u)
  {
    /*check the heat level set the variable Smoke_super*/
    DEBUG_APP("Change the Configuration ", false, 0u);

  }
  if ((flags_0 & FLAGS_BIT_INDEX(TMR_State_Timeout_event_0)) != 0u)
  {
    /*check the heat level set the variable Smoke_super*/
    DEBUG_APP("End of Configuration", false, 0u);
    behavioural_operational_State = state_Idle; /*Change state*/
  }
  if ((flags_0 & FLAGS_BIT_INDEX(TMR_BUTTON_PRESS_0)) != 0u)
  {
    /*check the heat level set the variable Smoke_super*/
    DEBUG_APP("End of Configuration ", false, 0u);
    behavioural_operational_State = state_Idle; /*Change state*/

  }
}

/**
 * @brief Handle the common transition actions for entering the CO alarm state
 * @param newAlarm - true if function called when new alarm started ie not from silence to alarm state transition
 */
static void jumpToCOAlarm (bool newAlarm)
{
  if (newAlarm == true)
  {
    /*If newly triggered alarm reset variables*/
     //SetIntelligentSampleRate(false);
    /*log_localAlarmEvent (alarmType_CO);*/
      alarmSilenceLimitReached = false;
      DEBUG_APP("Enter LocalCOAlarm state", false, 0u);
  }

  behavioural_operational_State = state_CO_Alarm;
  // ledBuzz_Post (pattern_alarm_CO);
}

/**
 * @brief Handle the common transition actions for entering the Smoke alarm state
 * @param newAlarm - true if function called when new alarm started ie not from silence to alarm state transition
 */
static void jumpToSmokeAlarm (bool newAlarm)
{
  if (newAlarm == true)
    {
      /*If newly triggered alarm reset variables*/
      // Clear_the Increased sample Rate  ;
      alarmSilenceLimitReached = false;
    //  DEBUG_APP("Enter LocalSmokeAlarm state", false, 0u);
      /* Stop Any State_Timeouts*/
      BURTCTimer_Stop(TMR_State_Timeout_event_0);

  /* If No Smoke Alarm or Alarm Silence Then Check and Log the end and clear Any Alarm Pattern*/
if((behavioural_operational_State !=state_Smoke_Alarm) &&  (behavioural_operational_State !=state_Smoke_Alarm_Silence))
 {
   if((behavioural_operational_State==state_CO_Alarm) ||  (behavioural_operational_State== state_CO_Alarm_Silence ))
   {
       /* ledBuzz_Post(alarmType_Idle);
     log_localAlarmEvent (alarmType_CO_END);*/
    }
if((behavioural_operational_State==state_Heat_Alarm )||  (behavioural_operational_State==state_Heat_Alarm_Silence) )
   {
    /* ledBuzz_Post(alarmType_Idle);
     log_localAlarmEvent (alarmType_Heat_END);*/
  }
if((behavioural_operational_State==state_Remote_Alarm ) )
   {

     /* log_localAlarmEvent (alarmType_Remote_END);*/
    /* ledBuzz_Post(alarmType_Idle);*/

  }
 }
 }
  behavioural_operational_State = state_Smoke_Alarm;
/*log_localAlarmEvent (alarmType_Smoke);*/
// ledBuzz_Post (pattern_alarm_Smoke);
}

/**
 * @brief Handle the common transition actions for entering the Heat alarm state
 * @param newAlarm - true if function called when new alarm started ie not from silence to alarm state transition
 */
static void jumpToHeatAlarm (bool newAlarm)
{
if (newAlarm == true)
{
    /*If newly triggered alarm reset variables*/
    /*if required  Acknowledge or reset any Module variable*/
    /*log_localAlarmEvent (alarmType_Heat);*/
    /* reSet the Variable for Alarm silence */
    alarmSilenceLimitReached = false;
  //  DEBUG_APP("Enter LocalHeatAlarm state", false, 0u);
    /* Stop Any State_Timeouts*/
     BURTCTimer_Stop(TMR_State_Timeout_event_0);
 /*If the alarm switch from  Other Low Priority Alarm to High Priority then Log Alarm Stop*/
  if((behavioural_operational_State==state_CO_Alarm) ||  (behavioural_operational_State== state_CO_Alarm_Silence ))
  {
      /* ledBuzz_Post(alarmType_Idle);
      log_localAlarmEvent (alarmType_CO_END);*/
   }
  if((behavioural_operational_State==state_Remote_Alarm ) )
     {

       /* log_localAlarmEvent (alarmType_Remote_END);*/
      /* ledBuzz_Post(alarmType_Idle);*/

    }
}
  behavioural_operational_State = state_Heat_Alarm;
  /*log_localAlarmEvent (alarmType_Heat);*/
  // ledBuzz_Post (pattern_alarm_Heat);
}
/**
 * @brief Handle the common transition actions for entering the Remote alarm state
 * Remote Alarm is only available when System is not in Local alarm
 * @param newAlarm - true if function called when new alarm started ie not from silence to alarm state transition
 */
static void jumpToRemoteAlarm (bool newAlarm)
{
if (newAlarm == true)
{
    /*If newly triggered alarm reset variables*/
    /*if required  Acknowledge or reset any Module variable*/
    /*log_localAlarmEvent (alarmType_Remote);*/
    /* reSet the Variable for Alarm silence */
    /* Stop Any State_Timeouts*/
     BURTCTimer_Stop(TMR_State_Timeout_event_0);
    alarmSilenceLimitReached = false;
    DEBUG_APP("Enter LocalRemote Alarm state", false, 0u);
}

behavioural_operational_State = state_Remote_Alarm;
// ledBuzz_Post (pattern_alarm_Remote);
}


/**
 * @brief Handle device disable
 * @details LED/Buzzer actions should be stopped and pins put into lowest possible current state.
 * The switch task should be set to only look at the ADS.
 * The event should be logged to the EEPROM
 * All  Diagnostic events should be , except timestamp and ambient light,
 *  @note This should be called at powerup to ensure all modules in correct state
 *
 */

static void handleDeviceDisable(void)
{

  /*Stop LED/Buzz and put into ADS only low power mode*/
  //ledBuzz_Post (pattern_idle);
 /* log_advancedEvent_Mode(eventType_deactivated, System Mode,NULL);*/

  if (getBehavioural_System_Modes(false)==Standby_Mode)
    {

    }
   if (getBehavioural_System_Modes(false)==Operational_Mode)
    {
       /* Start the Timer for demounting & set the result if it is >21 days */
       setBehavioural_Operational_State(state_Idle);
    }
   if (getBehavioural_System_Modes(false)==Transport_Mode)
     {
           /* No Handling required only Stop any indications*/
     }
   /* This Need Special Handling if we dismount in Functional test then timeout should happen ?
    * or  After switch testing switch back to previous mode*/
   if (getBehavioural_System_Modes(false)==Functional_Test_Mode)
     {
       /* Switch back to  Functional test and after timeout back to operational mode */
       setBehavioural_System_Modes(getBehavioural_previouse_System_Modes());

     }
   /* due to fault if device in commissioning mode , then de-mount will put it back to standby mode */
   if (getBehavioural_System_Modes(false)==Commisioning_Mode)
     {

       setBehavioural_System_Modes(Standby_Mode);

     }
  /*defaultVariableCO();*/

  /*Stop all  timers except timestamp, variance, Heart Beat*/
    Stop_Diagnostic_BIST();

}


/**
 * @brief Handle the device enable
 * @details Log the event.
 * Set the switch pins into the appropriate state
 *  Resume the LED/BUZZ task
 * Start the diagnostics and environmental update timers
 *
 */
static void handleDeviceEnable(void)
{

  bool result=false;
  /*Log Mounting event */
 /* log_activationEvent();*/
  /*defaultVariableSmoke();  reset the default State */
  /*defaultVariableHeat();  reset the default State */
  /*defaultVariableCO();  reset the default State */
    if (getBehavioural_System_Modes(false)==Standby_Mode)
     {
        result=diagnostics(Standby_Mode , operate_active,0);
         /* Get the result */
         if(result==true)
           {
            /*Set the mode & Operational State*/
             setBehavioural_System_Modes(Operational_Mode);
             setBehavioural_Operational_State(state_Idle);
            /* Start The Diagnostic  Timers */
             Start_Diagnostic_BIST();
           }
         else
           {
             /* either diagnostic will post Major or Minor Pattern or
              * Get the Status & Post the Pattern
              */
             setBehavioural_System_Modes(Commisioning_Mode);
             /*Stop The Diagnostic  Timers */
             Stop_Diagnostic_BIST();
           }
     }
    if (getBehavioural_System_Modes(false)==Operational_Mode)
     {
        setBehavioural_Operational_State(state_Idle);
        /* Perform All BIST only*/
        result=diagnostics(Operational_Mode , operate_active,0);
        /*Start The Diagnostic  Timers */
         Start_Diagnostic_BIST();
     }
    if (getBehavioural_System_Modes(false)==Transport_Mode)
      {
       /* ledBuzz_Post (pattern_Trasport_mode);*/
        /*Stop The Diagnostic  Timers */
        Stop_Diagnostic_BIST();
      }

}

/**
 * @brief get the state of the behavioural Operational state
 * return  system state
 */
behaviour_state_enum_operational_States getBehavioural_Operational_State(void)
{

  return behavioural_operational_State;
}
/**
 * @brief Set the state of the behavioural Operational state
 * @param  New system state
 * @return none
 */
void setBehavioural_Operational_State(behaviour_state_enum_operational_States state)
{

  behavioural_operational_State=state;

}
/**
 * @brief get the state of the behavioural state mode
 * @param  none
 * @return behavioural state mode
 */
behaviour_state_enum_System_modes getBehavioural_previouse_System_Modes(void)
{

  return behavioural_system_mode_previous;
}
/**
 * @brief get the state of the behavioural state mode
 * @param  true= read from eeprom after power cycle
 * @return behavioural state mode
 */
behaviour_state_enum_System_modes getBehavioural_System_Modes(bool read_From_eeprom)
{
  if(read_From_eeprom==true)
    {
    /* read form EEROM */
      //If the System Mode is FF then set the system mode to Standby_Mode
      //behavioural_system_mode_from_EEPROM()
    }

  return behavioural_system_mode;
}


/**
 * @brief Stop All Timers except Timestamp
 * @param  none
 * @return none
 */
void Stop_All_timers_except_timestamp(void)
 {
    /* Stop All Diagnostic Timers */
    Stop_Diagnostic_BIST();
    /* Stop general Timers */
    BURTCTimer_Stop(TMR_Battery_Measurement_BIST_event_0); /* For Testing */
    BURTCTimer_Stop(TMR_CO_Variance_Acquisition_event_0);
    /* When System is Commissioning mode & With fault */
    if(behavioural_system_mode!=Commisioning_Mode)
      {
        BURTCTimer_Stop(TMR_heartbeat_event_0);
      }
    BURTCTimer_Stop(TMR_TempHum_measure_BIST_event_0);


 }
/**
 * @brief Start All general Timers
 * @param  none
 * @return none
 */
void Start_All_timers(void)
 {

  /*Start-up: Start the three general timers */
   BURTCTimer_Start(TMR_Battery_Measurement_BIST_event_0, periodical, SMOKE_MEASUREMENT); /* For Testing */
   BURTCTimer_Start(TMR_CO_Variance_Acquisition_event_0, periodical, VARIANCE_PERIOD_NON_OPERATIONAL);
   BURTCTimer_Start(TMR_heartbeat_event_0, periodical, HEARTBEAT_PERIOD);
   BURTCTimer_Start(TMR_TempHum_measure_BIST_event_0, periodical, TEMP_HUMIDITY_PERIOD);

 }
/**
 * @brief Stop The Diagnostic BIST & Sensor Detection Timers
 * @param  none
 * @return none
 */
void Stop_Diagnostic_BIST(void)
 {
    /* Stop All Diagnostic Timers */
     BURTCTimer_StopFrom(TMR_Smoke_measure_event_0);

 }
/**
 * @brief Start The Diagnostic BIST & Sensor Detection Timers
 * @param  None
 * @return none
 */
void Start_Diagnostic_BIST(void)
 {
           /* Start All Diagnostic Timers */
            /* Smoke detection*/
            BURTCTimer_Start (TMR_Smoke_measure_event_0,periodical,SMOKE_MEASUREMENT);
            /* Smoke BIST*/
            BURTCTimer_Start (TMR_Smoke_BIST_event_0,periodical,SMOKE_BIST_MEASUREMENT);
            /* Soling detection + BIST*/
            BURTCTimer_Start (TMR_Soling_Measurement_BIST_event_0,periodical,SOILING_MEASUREMNT_BIST_PERIOD);
            /* Heat detection + BIST*/
            BURTCTimer_Start (TMR_Heat_measure_BIST_event_0,periodical,HEAT_MEASURMENT_BIST);
            /* CO detection*/
            BURTCTimer_Start (TMR_CO_measure_event_0,periodical,CO_MEASUREMENT_PERIOD);
            /* Co  BIST*/
            BURTCTimer_Start (TMR_CO_BIST_event_0,periodical,CO_BIST_PERIOD);
            /* Buzzer BIST*/
            BURTCTimer_Start (TMR_BUZZER_BIST_event_0,periodical,STORT_PERIOD); /* test*/
            /* Obstacle/Laser  BIST*/
            BURTCTimer_Start (TMR_Obstacle_Coverage_BIST_event_0,periodical,STORT_PERIOD); /* test*/
            /* Stop off base Variennce */
            BURTCTimer_Stop (TMR_CO_Variance_Acquisition_event_0);
 }
/**
 * @brief Set the mode of the behavioural state mode
 * @param  New system mode
 * @return none
 */
extern void setBehavioural_System_Modes(behaviour_state_enum_System_modes system_mode)
{
  /* This will be required to switch from FTM to Previous Mode*/
  if (system_mode== Functional_Test_Mode)
    {
      behavioural_system_mode_previous=behavioural_system_mode;
    }
  behavioural_system_mode = system_mode;
}

/**
 * @brief get the button Type
 * @param  none
 * @return button_state_t
 */
button_state_t get_switches_type(void)
{

  return switch_test;
}


/**
 * @brief get the operational State Type
 * @param  none
 * @return button_state_t
 */
ADS_operate_state_enum getOperateState(void)
{

  return ADS_operateState;
}
