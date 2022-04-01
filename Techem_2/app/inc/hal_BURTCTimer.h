/**
 * @file  hal_BURTCTimer.h
 * @brief
 * @project P0200 Techem Core Firmware
 * @date    16 March 2022
 * @author  abenrashed
 */

#ifndef BURTCTIMER_H_
#define BURTCTIMER_H_

#include "em_burtc.h"

#define burtcClk                  32768u/burtcClkDiv_128 /**< Divide clock by 128. */
// BURTC interrupts period
#define BURTC_COMPARE_FOR_IRQ_PERIOD_10SEC    (burtcClk * 10)       //ABR Techem.
#define BURTC_COMPARE_FOR_IRQ_PERIOD_8SEC     (burtcClk * 8)        //ABR Techem.
#define BURTC_COMPARE_FOR_IRQ_PERIOD_5SEC     (burtcClk * 5)        //ABR Techem.
#define BURTC_COMPARE_FOR_IRQ_PERIOD_4SEC     (burtcClk * 4)        //ABR Techem.
#define BURTC_COMPARE_FOR_IRQ_PERIOD_2SEC     (burtcClk * 2)       //ABR Techem.
#define BURTC_COMPARE_FOR_IRQ_PERIOD_1SEC     (burtcClk * 1)       //ABR Techem.

typedef struct {
    bool enabled;
    bool periodic;
    uint32_t period;
    uint32_t value;
} BURTCTimer_TypeDef;


/* ************************************************* */
/* Behavioural Events                                */
/*                                                   */
/* ************************************************* */
typedef enum {
  TMR_Battery_Measurement_BIST_event_0,  /**<0 Not stopped during disable, started on power-up, periodic */
  TMR_timestamp_event_0,  /**<1 Not stopped during disable, started on power-up, periodic */
  TMR_CO_Variance_Acquisition_event_0, /**<2 Not stopped during disable, started on power-up, periodic*/
  TMR_heartbeat_event_0,/**<3 Started when entering active state, periodic*/
  TMR_TempHum_measure_BIST_event_0,/**<4 Temperature & Humidity BIST & measurement,periodic */
  TMR_Smoke_measure_event_0,/**<5 Smoke Detection available in Operation Mode, periodic */
  TMR_Smoke_BIST_event_0,   /**<6 Smoke Chamber BIST available in Operation Mode, periodic */
  TMR_Smoke_IncreasedSampleRate_event_0, /**<7 if Increased smoke detected in Operation Mode, */
  TMR_Soling_Measurement_BIST_event_0,/**<8  Soiling Measurement & BIST is done during same time in Operation Mode, periodic */
  TMR_Heat_measure_BIST_event_0, /**<9 Heat Measurement & BIST, periodic */
  TMR_CO_measure_event_0, /**<10 CO Measurement, periodic */
  TMR_CO_BIST_event_0, /**<11 CO BIST, periodic */
  TMR_CO_IntelligentsampleRate_event_0,/**<12 Increased CO detected change rate of acquisition  , periodic */
  TMR_BUZZER_BIST_event_0,/**<13 Buzzer BIST  , periodic */
  TMR_Obstacle_Coverage_BIST_event_0,/**<14 Obstacle & Coverage  Detection/BIST , periodic */
  TMR_faultSilence_event_0, /**<15 Started when button pressed to silence battery fault chirp, one-shot*/
  TMR_Airing_Configuration_Timeout_event_0, /**<16 Airing Configuration, Non-periodic */
  TMR_AmbientLight_measure_event_0,  /**<17 AmbientLight Power, periodic */
  TMR_AmbientLigt_test_event_0,   /**<18 AmbientLight Measure */
  TMR_Smoke_HIGH_SUPER_event_0,/**<19 Smoke detected, Non-periodic */
  TMR_Smoke_NONE_event_0,/**<20 No Smoke , Non-periodic */
  TMR_Heat_HIGH_SUPER_event_0,/**<21 Heat detected, Non-periodic */
  TMR_Heat_NONE_event_0, /**<22 No Heat Configuration, Non-periodic */
  TMR_COHB_HIGH_SUPER_event_0,/**<23 CO detected, Non-periodic */
  TMR_COHB_NONE_event_0, /**<24 No CO , Non-periodic */
  TMR_RADIO_Alarm_event_0,/**<25 Remote Alarm , Non-periodic */
  TMR_MODE_Change_event_0 ,/**<26 Mode change  , Non-periodic */
  TMR_State_Timeout_event_0 ,/**<27 State  timeout , Non-periodic */
  TMR_BUTTON_PRESS_0,/**<28 State  timeout , Non-periodic */
  /* Common Button Press Event Once button is press */
  /*Then Enum value can be set to identify type of button Press this will save the events*/
  //TMR_Device_Button_Short_Press_event_0,/**<28 Short Button Press By Switch Module, Non-periodic */
  //TMR_Device_Button_Long_Press_event_1,/**<0 Long Button Press By Switch Module, Non-periodic */
  //TMR_Device_Button_Long_Press_And_Hold_event_1,/**<1 Long Button Press & Hold By Switch Module, Non-periodic */
 // TMR_Device_Button_Sequence_Press_event_1, /**<2 Sequence Press and Call function Type of sequence, Non-periodic */
  TMR_Device_ADS_enable_event_0,/**<29 ADS Enabled, Non-periodic */
  TMR_Device_ADS_disable_event_0,/**<30 ADS Disabled, Non-periodic */
  TMR_Device_Shutdown_event_0,/**<31 device shutdown low Voltage below 2.7/2.4 v, Non-periodic */
  SpiComms_event_1, /**<1 SPI Comm Task , periodic */
  WdogTimer_event_1,  /**<2 Watchdog Task , periodic */
  NO_OF_EVENTS                  /** Insert any new event before this item. This item represents the enum length*/
} BURTCTimer_Events_TypeDef;



/*BURTC TICK= 10 seconds, Please change the period if BURTC_COMPARE_FOR_IRQ_PERIOD_XSEC  */
#define BURTC_PERIOD  10u    /**<BURTC TICK period BURTC_PERIOD = X SEC*/
#define PERIOD_SET(p)    (p / BURTC_PERIOD)

/*Periods are defined in multiples of 10 seconds  Please re-verify*/
#define BATTERY_MEASURMENT_BIST_PERIOD  PERIOD_SET(86400u)  /**<24 Hr =86400 sec*/
#define TIMESTAMP_PERIOD PERIOD_SET(60u) /**<1 min  =60 sec*/
#define VARIANCE_PERIOD_NON_OPERATIONAL  PERIOD_SET(180u)  /**< 3 min*/
#define HEARTBEAT_PERIOD PERIOD_SET(60u),/**<3 every 60 sec */
#define FAULT_SILENCE_TIMEOUT PERIOD_SET(86400u)/**< PTR-267  24 Hr =86400 sec */
#define SMOKE_MEASUREMENT PERIOD_SET(10u)/**< Smoke Detection 10 sec */
#define SMOKE_BIST_MEASUREMENT PERIOD_SET(3600u)   /**<6 PTR-1188  Smoke – Degraded Chamber every hour Need to change*/
#define SMOKE_INCRESED_RATE 2u /**<if Increased smoke detected, This need to be handled by LETIMER because of 2 s time BURTC is 10 sec*/
#define SOILING_MEASUREMNT_BIST_PERIOD  PERIOD_SET(180u)/**< PTR-1376 3 min, Need to change periodic rate  */
#define HEAT_MEASURMENT_BIST PERIOD_SET(10u)  /**<Heat Measurement & BIST, periodic 10 sec */
#define CO_MEASUREMENT_PERIOD PERIOD_SET(50u) /**< CO Measurement, periodic 50 sec */
#define CO_BIST_PERIOD PERIOD_SET(180) /**< CO BIST, periodic= 3 min */
#define CO_INCREASED_SAMPLE_RATE PERIOD_SET(10u)/**< Increased CO detected change rate of acquisition , periodic  10sec */
#define BUZZER_BIST_PERIOD PERIOD_SET(604800u)/**< Buzzer BIST  , periodic  every 7 days*/
#define OBSTACLE_COVARAGE_PERIOD PERIOD_SET(604800u)//**< Obstacle & Coverage  Detection/BIST , periodic every 7 days*/
#define TEMP_HUMIDITY_PERIOD PERIOD_SET(120u) /**<Temperature & Humidity BIST & measurement,periodic= 2 min */
#define AMBIENT_MEASUREMENT_PERIOD PERIOD_SET(360u)  /**< AmbientLight Power, periodic= every 6 min */
#define AIRING_CONFIGURATION_TIMEOUT PERIOD_SET(30u)/**< Airing Configuration, Non-periodic 30 sec */
#define SPI_COMMUMICATION_  PERIOD_SET(180u) /**< SPI Comm Task , periodic = 3min */
#define WATCHDOG_TIMER PERIOD_SET(180)  /**<4 Watchdog Task , periodic periodic = 3min, Need to change  */
#define ALARM_SILENCE_TIMEOUT PERIOD_SET (870)  /**< TimeOut  = 14.30 min, Need to change  */


#define periodical          true
#define one_shot            false

#define FLAGS_SUBGROUP_INDEX(x)   (x/32)
#define FLAGS_BIT_INDEX(x)        (1u << (x%32))

extern void GPIO_init(void);
extern void BURTC_init(void);
extern void BURTCTimer_Start(BURTCTimer_Events_TypeDef event, bool periodic, uint32_t period);
extern uint32_t BURTCTimer_Stop(BURTCTimer_Events_TypeDef event);
extern void BURTCTimer_StopFrom(BURTCTimer_Events_TypeDef event);


#endif
