/*
 * SwitchTask.h
 *
 *  Created on: 4 Apr 2022
 *      Author:
 */

#ifndef APP_INC_SWITCHTASK_H_
#define APP_INC_SWITCHTASK_H_
#define DEBUG_ENABLE_SWITCHES
#ifdef DEBUG_ENABLE_SWITCHES
#define DEBUG_SWITCHES(str, numMode, dataValue) debug_out(str,numMode,dataValue)
#else
#define DEBUG_SWITCHES(str, numMode, dataValue)
#endif
/*DUMMY*/

/**
 * State options for ADS switch
 */
typedef enum {
  ADS_OFF_BASE = 0u, /**< ADS_OFF_BASE*/
  ADS_ON_BASE = 1u, /**< ADS_ON_BASE*/
} ADS_state_t;

/**
 * State options of SelfTest switch
 */
typedef enum {
  NO_PRESS, /**< No BUTTON PRESSED*/
  SHORT_PRESS = 1u, /**< Short*/
  LONG_PRESS = 2u, /**< Long Press*/
  LONG_PRESS_HOLD= 3u, /**< Long Press & hold*/
  Sequense_PRESS_Operate_disable= 4u, /**< Button Sequence to Switch from  Operational mode to Standby Mode*/
  Sequense_PRESS_Airing_Configuration= 5u,/**< Button Sequence for Airing configuration */
  Sequense_PRESS_Domestic_user=6u, /**< Button  for Domestic User */
  Sequense_PRESS_Extended_Domestic_user=7u, /**< Button  for Domestic User Extended test */
  Button_STUCK=8u,/**< Button stuck*/
  END_BUTTON
} button_state_t;


extern ADS_state_t switches_getADS(void);
extern button_state_t get_switches_type(void);
//DUMMY testing
button_state_t get_switches_type_1(void);
extern void switches_checkPollStatus(void);



#endif /* APP_INC_SWITCHTASK_H_ */
