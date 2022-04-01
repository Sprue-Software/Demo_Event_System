/*
 * event_system.h
 *
 *  Created on: 29 Mar 2022
 *      Author: NDI
 */

#ifndef APP_INC_EVENT_SYSTEM_H_
#define APP_INC_EVENT_SYSTEM_H_

#ifdef DEBUG_ENABLE_APP
#define DEBUG_APP(str, numMode, dataValue) sendDebugMsg(str,numMode,dataValue)
#else
#define DEBUG_APP(str, numMode, dataValue)
#endif

/* Enum defines */

/**
 * These are the various behavioural System modes
 */
typedef enum {
  Standby_Mode, /**< System default Mode, device is de-mounted*/
  Commisioning_Mode,/**< System Mode, device is mounted first Time*/
  Operational_Mode,/**< System Operating Mode, device Successfully Pass Commissioning mode*/
  Functional_Test_Mode, /**< System Test Mode, All Test Functionality is possible*/
  Transport_Mode,/**< System Mode, When device is returned/damaged/Faulty*/
  Shutdown_Mode,/**< System Mode, When device is critically Low Battery Safe Shutdown*/
  NUM_Modes
} behaviour_state_enum_System_modes;

/**
 * These are the various behavioural Operational states
 */
typedef enum {
  state_Idle,/**< System default Operational State, */
  state_Smoke_Alarm,/**< System  Operational State, When Smoke Alarm conditions occurs */
  state_Smoke_Alarm_Silence,/**< System  Operational State, When Button press for silence Smoke Alarm */
  state_Heat_Alarm,/**< System  Operational State, When Heat Alarm conditions occurs */
  state_Heat_Alarm_Silence,/**< System  Operational State, When Button press for silence heat Alarm */
  state_CO_Alarm,/**< System  Operational State, When CO Alarm conditions occurs */
  state_CO_Alarm_Silence,/**< System  Operational State, When Button press for silence CO Alarm */
  state_Remote_Alarm,/**< System  Operational State, When CO Alarm conditions occurs */
  state_Domestic_Test,/**< System  Operational State, When Button Press to test the Limited BIST functions */
  state_BISTMode,/**< System  Operational State, When MCU-2 requests or Serial Commands */
  State_Airing_Configuration,/**< System  Operational State, When Button Press initiated Airing Configuration with 30 sec timeout */
  NUM_State
} behaviour_state_enum_operational_States;


/**
 * These are the various operate states the device can find its-self in
 */
typedef enum {
  operate_active, /**< operate_active, ADS on-base, shipping tag removed*/
  operate_disabled /**< operate_disabled, ADS off-base, shipped tag inserted*/
} ADS_operate_state_enum;

/**
 * CO modes/state
 */
typedef enum {
  co_none, /**< co_none, no CO detected*/
  co_llw, /**< co_llw, CO detected at a low level warning */
  co_high, /**< co_high, high levels of CO detected*/
  co_super /**< co_super, very high levels of CO detected*/
} co_state_enum;


/**
 * Smoke modes/state
 */
typedef enum {
  Smoke_none, /**< Smoke_none, no Smoke detected*/
  Smoke_high, /**< Smoke_high, high levels of Smoke detected*/
  Smoke_super /**< Smoke_super, very high levels of Smoke detected*/
} Smoke_state_enum;


/**
 * Heat modes/state
 */
typedef enum {
  Heat_none, /**< Heat_none, no Heat detected*/
  Heat_high, /**< Heat_high, high levels of Heat detected*/
  Heat_super /**< Heat_super, very high levels of Heat detected*/
} heat_state_enum;


/* Function prototypes */
extern void runOperateModule(OS_FLAGS flags_0, OS_FLAGS flags_1);
extern void runBehaviouralModule(OS_FLAGS flags_0, OS_FLAGS flags_1);
extern void clearEventFlag(OS_FLAGS flags_0, OS_FLAGS flags_1);
extern ADS_operate_state_enum getOperateState(void);
extern behaviour_state_enum_operational_States getBehavioural_Operational_State(void);
extern void setBehavioural_Operational_State(behaviour_state_enum_operational_States state);
extern behaviour_state_enum_System_modes getBehavioural_System_Modes(bool read_From_eeprom);
extern void setBehavioural_System_Modes(behaviour_state_enum_System_modes system_mode);
extern behaviour_state_enum_System_modes getBehavioural_previouse_System_Modes(void);

#endif /* APP_INC_EVENT_SYSTEM_H_ */
