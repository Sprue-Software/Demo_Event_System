

/*
 *
 *  @file diagnostics.h
 *  @brief: This File describe variable required for diagnostic & fault
 *  Created on: 4 Apr 2022
 *  Author:
 */


#ifndef APP_INC_DIAGNOSTICS_H_
#define APP_INC_DIAGNOSTICS_H_

#define DEBUG_ENABLE_DIAG
#ifdef DEBUG_ENABLE_DIAG
#define DEBUG_DIAG(str, numMode, dataValue) debug_out(str,numMode,dataValue)
#else
#define DEBUG_DIAG(str, numMode, dataValue)
#endif
/*DUMMY*/
extern  bool diagnostics(behaviour_state_enum_System_modes modes , ADS_operate_state_enum ADS_status,OS_FLAGS flags);


#endif /* APP_INC_DIAGNOSTICS_H_ */
