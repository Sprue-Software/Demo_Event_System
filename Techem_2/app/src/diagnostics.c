/*
 * diagnostics.c
 *
 *  Created on: 31 Mar 2022
 *      Author: ndiwathe
 */
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
/**
 * @brief Diagnostics routine, run acquisition routines and check for faults.
 * @details

 */
/* This is just for reference to test the logic , Need to implement as per the requirement*/
 bool diagnostics(behaviour_state_enum_System_modes modes ,ADS_operate_state_enum ADS_status,OS_FLAGS flags)
{
  bool diagPerformed;

 // if (ledBuzz_checkForGap() == false){
 //   /*LED/Buzzer is active so delay the diagnostics until complete*/
  //  diagPerformed = false;
 //   LETimer_start(LETIMER_DIAG_DELAY, DIAG_DELAY_PERIOD);
 //   delayType = output;
 // }
  /* This will Periodic when device- Operational mode + ADS On +event are set by BURTC timers */
 if ( ((modes==Operational_Mode) && (ADS_status==operate_active)) && (flags!=0))
   {
     DEBUG_DIAG("\n ",false,0u);
  if ((flags & FLAGS_BIT_INDEX(TMR_Battery_Measurement_BIST_event_0)) != 0u){
         GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
         DEBUG_DIAG("\n Battery Measurement",false,0u);
       }
  if ((flags & FLAGS_BIT_INDEX(TMR_TempHum_measure_BIST_event_0)) != 0u){
         GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
         DEBUG_DIAG("\n Temp Measurement",false,0u);
       }
  if ((flags & FLAGS_BIT_INDEX(TMR_Smoke_measure_event_0)) != 0u){
         GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
         DEBUG_DIAG("\n Smoke Measurement",false,0u);
       }
  if ((flags & FLAGS_BIT_INDEX(TMR_Smoke_BIST_event_0)) != 0u){
         GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
         DEBUG_DIAG("\n Smoke BIST",false,0u);
       }
  if ((flags & FLAGS_BIT_INDEX(TMR_Soling_Measurement_BIST_event_0)) != 0u){
         GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
         DEBUG_DIAG("\n Soiling  Measurement",false,0u);
       }
  if ((flags & FLAGS_BIT_INDEX(TMR_Heat_measure_BIST_event_0)) != 0u){
         GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
         DEBUG_DIAG("\n Heat BIST + Measurement ",false,0u);
       }
  if ((flags & FLAGS_BIT_INDEX(TMR_CO_measure_event_0)) != 0u){
         GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
         DEBUG_DIAG("\n Co Measurement",false,0u);
       }
     if ((flags & FLAGS_BIT_INDEX(TMR_CO_BIST_event_0)) != 0u){
         DEBUG_DIAG("\n Co BIST ",false,0u);
         ;
       }
     if ((flags & FLAGS_BIT_INDEX(TMR_BUZZER_BIST_event_0)) != 0u){
            GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
            DEBUG_DIAG("\n Buzzer BIST",false,0u);
          }
     if ((flags & FLAGS_BIT_INDEX(TMR_Obstacle_Coverage_BIST_event_0)) != 0u){
            GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
            DEBUG_DIAG("\n Obstacle + Coverage BIST  ",false,0u);
          }
   }
 /* This will happen only when device- Standby mode + ADS off only Battery +Temp+SPI will be active */
 if ( (modes==Standby_Mode) && (ADS_status==operate_disabled) )
    {

     if ((flags & FLAGS_BIT_INDEX(TMR_TempHum_measure_BIST_event_0)) != 0u){
             GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
             DEBUG_DIAG("\n Standby Temp Measurement",false,0u);
           }
     if ((flags & FLAGS_BIT_INDEX(TMR_Battery_Measurement_BIST_event_0)) != 0u){
               GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
               DEBUG_DIAG("\n Standby Battery Measurement",false,0u);
             }

     }
 /* This will happen only when device- Transport mode */
 if ( (modes==Transport_Mode) ||(modes==Commisioning_Mode) )
    {

     if ((flags & FLAGS_BIT_INDEX(TMR_TempHum_measure_BIST_event_0)) != 0u){
             GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
             DEBUG_DIAG("\n Transport_Mode Temp Measurement",false,0u);
           }
     if ((flags & FLAGS_BIT_INDEX(TMR_Battery_Measurement_BIST_event_0)) != 0u){
               GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
               DEBUG_DIAG("\n Transport_Mode Battery Measurement",false,0u);
             }

     }
 /* This will happen only when device- Standby mode + ADS enabled first time */
 if ( (modes==Standby_Mode) && (ADS_status==operate_active) )
    {

          DEBUG_DIAG("\n Perform BIST  ALL",false,0u);

     }
 /* This will happen only when device- Operational mode + ADS enabled first time */
 if ( ((modes==Operational_Mode) && (ADS_status==operate_active)) && (flags==0) )
     {


           DEBUG_DIAG("\n BIST ALL",false,0u);
           /*Check Button Press status if pattern is required if for Domestic or Extended */

      }

    diagPerformed = true;



    DEBUG_DIAG("\n ********************** ",false,0u);
    DEBUG_DIAG("\n ",false,0u);
  return diagPerformed;
}
