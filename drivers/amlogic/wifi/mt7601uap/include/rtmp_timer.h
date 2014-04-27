/*
 *************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                       *
 *************************************************************************/


#ifndef __RTMP_TIMER_H__
#define  __RTMP_TIMER_H__

#include "rtmp_os.h"

#define DECLARE_TIMER_FUNCTION(_func)			\
	void rtmp_timer_##_func(unsigned long data)

#define GET_TIMER_FUNCTION(_func)				\
	(PVOID)rtmp_timer_##_func

/* ----------------- Timer Related MARCO ---------------*/
/* In some os or chipset, we have a lot of timer functions and will read/write register, */
/*   it's not allowed in Linux USB sub-system to do it ( because of sleep issue when */
/*  submit to ctrl pipe). So we need a wrapper function to take care it. */

#ifdef RTMP_TIMER_TASK_SUPPORT
typedef VOID(
	*RTMP_TIMER_TASK_HANDLE) (
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /* RTMP_TIMER_TASK_SUPPORT */

typedef struct _RALINK_TIMER_STRUCT {
	RTMP_OS_TIMER TimerObj;	/* Ndis Timer object */
	BOOLEAN Valid;		/* Set to True when call RTMPInitTimer */
	BOOLEAN State;		/* True if timer cancelled */
	BOOLEAN PeriodicType;	/* True if timer is periodic timer */
	BOOLEAN Repeat;		/* True if periodic timer */
	ULONG TimerValue;	/* Timer value in milliseconds */
	ULONG cookie;		/* os specific object */
	void *pAd;
#ifdef RTMP_TIMER_TASK_SUPPORT
	RTMP_TIMER_TASK_HANDLE handle;
#endif				/* RTMP_TIMER_TASK_SUPPORT */
} RALINK_TIMER_STRUCT, *PRALINK_TIMER_STRUCT;


#ifdef RTMP_TIMER_TASK_SUPPORT
typedef struct _RTMP_TIMER_TASK_ENTRY_ {
	RALINK_TIMER_STRUCT *pRaTimer;
	struct _RTMP_TIMER_TASK_ENTRY_ *pNext;
} RTMP_TIMER_TASK_ENTRY;

#define TIMER_QUEUE_SIZE_MAX	128
typedef struct _RTMP_TIMER_TASK_QUEUE_ {
	unsigned int status;
	unsigned char *pTimerQPoll;
	RTMP_TIMER_TASK_ENTRY *pQPollFreeList;
	RTMP_TIMER_TASK_ENTRY *pQHead;
	RTMP_TIMER_TASK_ENTRY *pQTail;
} RTMP_TIMER_TASK_QUEUE;

#define BUILD_TIMER_FUNCTION(_func)										\
void rtmp_timer_##_func(unsigned long data)										\
{																			\
	PRALINK_TIMER_STRUCT	_pTimer = (PRALINK_TIMER_STRUCT)data;				\
	RTMP_TIMER_TASK_ENTRY	*_pQNode;										\
	RTMP_ADAPTER			*_pAd;											\
																			\
	_pTimer->handle = _func;													\
	_pAd = (RTMP_ADAPTER *)_pTimer->pAd;										\
	_pQNode = RtmpTimerQInsert(_pAd, _pTimer); 								\
	if ((_pQNode == NULL) && (_pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT))	\
		RTMP_OS_Add_Timer(&_pTimer->TimerObj, OS_HZ);               					\
}
#else /* !RTMP_TIMER_TASK_SUPPORT */
#define BUILD_TIMER_FUNCTION(_func)										\
void rtmp_timer_##_func(unsigned long data)										\
{																			\
	PRALINK_TIMER_STRUCT	pTimer = (PRALINK_TIMER_STRUCT) data;				\
																			\
	_func(NULL, (PVOID) pTimer->cookie, NULL, pTimer); 							\
	if (pTimer->Repeat)														\
		RTMP_OS_Add_Timer(&pTimer->TimerObj, pTimer->TimerValue);			\
}
#endif /* RTMP_TIMER_TASK_SUPPORT */

DECLARE_TIMER_FUNCTION(MlmePeriodicExec);
DECLARE_TIMER_FUNCTION(MlmeRssiReportExec);
DECLARE_TIMER_FUNCTION(AsicRxAntEvalTimeout);
DECLARE_TIMER_FUNCTION(APSDPeriodicExec);
DECLARE_TIMER_FUNCTION(EnqueueStartForPSKExec);


#ifdef RTMP_MAC_USB
DECLARE_TIMER_FUNCTION(BeaconUpdateExec);
#endif /* RTMP_MAC_USB */

#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(APDetectOverlappingExec);

#ifdef DOT11N_DRAFT3
DECLARE_TIMER_FUNCTION(Bss2040CoexistTimeOut);
#endif /* DOT11N_DRAFT3 */

DECLARE_TIMER_FUNCTION(GREKEYPeriodicExec);
DECLARE_TIMER_FUNCTION(CMTimerExec);
DECLARE_TIMER_FUNCTION(WPARetryExec);
#ifdef AP_SCAN_SUPPORT
DECLARE_TIMER_FUNCTION(APScanTimeout);
#endif /* AP_SCAN_SUPPORT */
DECLARE_TIMER_FUNCTION(APQuickResponeForRateUpExec);

#ifdef IDS_SUPPORT
DECLARE_TIMER_FUNCTION(RTMPIdsPeriodicExec);
#endif /* IDS_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */


#ifdef TXBF_SUPPORT
DECLARE_TIMER_FUNCTION(eTxBfProbeTimerExec);
#endif // TXBF_SUPPORT //





#ifdef RALINK_ATE
DECLARE_TIMER_FUNCTION(ATEPeriodicExec);
#endif /* RALINK_ATE */

#endif /* __RTMP_TIMER_H__ */
