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


#ifndef __RADAR_H__
#define __RADAR_H__

/* RESTRICTION_BAND_1: 5600MHz ~ 5650MHz */
#define RESTRICTION_BAND_1(_pAd)												\
	_pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40 ? 						\
	((_pAd->CommonCfg.Channel >= 116) && (_pAd->CommonCfg.Channel <= 128)) :	\
	((_pAd->CommonCfg.Channel >= 120) && (_pAd->CommonCfg.Channel <= 128))

/* 802.11H */
typedef struct _DOT11_H {
	/* 802.11H and DFS related params */
	UCHAR CSCount;		/*Channel switch counter */
	UCHAR CSPeriod; 	/*Channel switch period (beacon count) */
	USHORT RDCount; 	/*Radar detection counter, if RDCount >  ChMovingTime, start to send beacons*/
	UCHAR RDMode;		/*Radar Detection mode */
	USHORT ChMovingTime;
	BOOLEAN bDFSIndoor;
	ULONG InServiceMonitorCount;	/* unit: sec */
} DOT11_H, *PDOT11_H;

BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch);

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd);

#ifdef CONFIG_AP_SUPPORT
VOID ChannelSwitchingCountDownProc(
	IN PRTMP_ADAPTER	pAd);
#endif /* CONFIG_AP_SUPPORT */

VOID RadarDetectPeriodic(
	IN PRTMP_ADAPTER	pAd);

int	Set_CSPeriod_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

int Set_ChMovingTime_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg);

int Set_BlockChReset_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg);

#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)
int	Set_RadarShow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

VOID CckMrcStatusCtrl(
	IN PRTMP_ADAPTER pAd);

VOID RadarGLRTCompensate(
	IN PRTMP_ADAPTER pAd);

#endif /*defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)*/

#endif /* __RADAR_H__ */
