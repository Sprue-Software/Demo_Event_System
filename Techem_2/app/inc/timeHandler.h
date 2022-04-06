/*
 * timeHandler.h
 *
 *  Created on: 4 Apr 2022
 *      Author:
 */

#ifndef APP_INC_TIMEHANDLER_H_
#define APP_INC_TIMEHANDLER_H_

#ifdef DEBUG_ENABLE_TIME
#define DEBUG_TIME(str, numMode, dataValue) sendDebugMsg(str,numMode,dataValue)
#else
#define DEBUG_TIME(str, numMode, dataValue)
#endif

/*DUMMY*/
#define TIME_HOUR 60u     /**< 1 hour in min*/
#define TIME_MONTH  43200u    /**< 30 day in min*/
#define TIME_EOL  6307200u  /**< 12 year in min*/
#define TIME_PRODUCTION_LOCKOUT 7200u /**< 120 hour in min*/

extern void time_setCurrentTime(uint32_t time);
extern void time_handleTimestamp(void);
extern uint8_t time_inProductionLockout(void);
extern uint32_t get_currentTime(void);

#endif /* APP_INC_TIMEHANDLER_H_ */
