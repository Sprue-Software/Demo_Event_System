/*
 * Acquisition_Smoke.c
 *
 *  Created on: 1 Apr 2022
 *      Author:
 *
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

uint32_t current_Smoke_value=0;

/**
 * @brief  This Function will return the CO Value.(Which include Compensation )
 * @return  Co value
 */
extern uint32_t getSmokeAfterCompensation(void)
{

  return current_Smoke_value;
}
