/**
 * @file	timeHandler.c
 * @brief	Handle the monitoring of the time since production
 * @project SA2888 Gen5 Core Firmware
 * @date	26 Sep 2019
 * @author  cbaker
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
#include "timeHandler.h"
#include "event_system.h"


static uint32_t currentTime; /**< Unit = mins*/
static uint8_t inProductionLockout = true;

/**
 * Set the current time
 *  * @param time
 */
void time_setCurrentTime(uint32_t time)
{
	currentTime = time;
	if (currentTime < TIME_PRODUCTION_LOCKOUT){
		inProductionLockout = true;
	}
	else{
		inProductionLockout = false;
	}
	/*log_operatingTime(currentTime); */
}

/**
 * @brief Handle the update of the current time
 */
void time_handleTimestamp(void)
{
	currentTime++; /*Increment minute counter*/
	DEBUG_TIME("-----Current Time:", true, currentTime);

	if ((currentTime % TIME_HOUR) == 0u){
		/*Update to EEPROM every hour*/
		DEBUG_TIME("***Timestamp EEPROM update***", DEF_NULL, DEF_NULL);
		/*log_operatingTime(currentTime); */
	}

	if ((currentTime % TIME_MONTH) == 0u){
		/*Every month log battery and environmental data if required*/
		DEBUG_TIME("***Month Elapsed***", DEF_NULL, DEF_NULL);
		if (inProductionLockout == false){
#ifdef REPLACEABLE_BATTERY
			if ((getOperateState() == operate_active)&&
					(IsBattVoltAboveShutdown() == true)){
#else
			if (getOperateState() == operate_active){
#endif
				log_batteryVoltagesEvent();
				Environmental_Log30dayData();
			}
		}
	}
	if (currentTime >= TIME_EOL){
		/*Check for End Of Life*/
		DEBUG_TIME("***END OF LIFE***", DEF_NULL, DEF_NULL);
	/*	faults_changeState(FAULT_ID_EOL, FAULT_ACTION_SETFAULT);*/
	/*	log_advancedEvent(eventType_endOfLifeEvent, NULL); */
	}
	if ((inProductionLockout == true) && (currentTime >= TIME_PRODUCTION_LOCKOUT)){
		/*Check for production lockout end*/
		DEBUG_TIME("***Production Lockout ended***", DEF_NULL, DEF_NULL);
		inProductionLockout = false;
		/*log_advancedEvent(eventType_productionLockoutEnd, NULL);*/
	}
}

/**
 * Check if in production lockout (time<120hours)
 * @req 	Production Lockout
 * @return True if in production lockout
 */
uint8_t time_inProductionLockout(void)
{
	return inProductionLockout;
}

/**
 * Return the current time
 * @return
 */
uint32_t get_currentTime(void)
{
	return currentTime;
}
