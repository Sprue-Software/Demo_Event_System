/*
 * diagnostics.h
 *
 *  Created on: 31 Mar 2022
 *      Author: ndiwathe
 */

#ifndef APP_INC_DIAGNOSTICS_H_
#define APP_INC_DIAGNOSTICS_H_

#ifdef DEBUG_ENABLE_DIAG
#define DEBUG_DIAG(str, numMode, dataValue) sendDebugMsg(str,numMode,dataValue)
#else
#define DEBUG_DIAG(str, numMode, dataValue)
#endif
extern  bool diagnostics(behaviour_state_enum_System_modes modes , ADS_operate_state_enum ADS_status,OS_FLAGS flags);

#endif /* APP_INC_DIAGNOSTICS_H_ */
