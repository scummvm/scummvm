/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#ifndef __SKIN_H__
#define __SKIN_H__

// skin
#define	sknInfoState		0
#define	sknInfoPosX			1
#define	sknInfoPosY			2

#define	sknInfoMaxWOrH		3
#define	sknInfoDrawMode		4
#define	sknInfoKeepXOrY1	5
#define	sknInfoKeepXOrY2	7

#define sknInfoListWidth	sknInfoMaxWOrH
#define sknInfoListHeight	sknInfoDrawMode
#define sknInfoListSize		sknInfoListHeight
#define sknInfoListItemSize	12

#define	sknPosRsc			'sPos'
#define	sknColorsRsc		'sCol'
#define	sknDepthRsc			'sDep'

#define sknStateNormal		0
#define sknStateSelected	10
#define sknStateDisabled	20

#define sknSelectedState(bmp)	(bmp + sknStateSelected)
#define sknDisabledState(bmp)	(bmp + sknStateDisabled)

// skin elements
#define skinList					500
#define	skinColors					600
#define skinDepth					700

#define skinButtonNone				0

#define skinSliderUpArrow			2000
#define skinSliderDownArrow			2100

#define skinButtonGameParams		3000
#define skinButtonGameAdd			3100
#define skinButtonGameEdit			3200
#define skinButtonGameAudio			3300
#define skinButtonGameDelete		4000
#define skinButtonGameStart			7000

#define	skinBackgroundImageTop			1000
#define	skinBackgroundImageBottom		1010
///


#define skinToolsBackground			1100

#define	skinListUpArrowNormal		1500
#define	skinListUpArrowOver			1510
#define	skinListUpArrowDisabled		1540

#define skinListDownArrowNormal		1800
#define	skinListDownArrowOver		1810
#define	skinListDownArrowDisabled	1840

#define skinButtonGameInfoNormal	2000
#define skinButtonGameInfoOver		2010
#define skinButtonGameInfoDisabled	2040

#define skinButtonGameParamNormal	3000
#define skinButtonGameParamOver		3010

#define skinButtonGameDeleteNormal		4000
#define skinButtonGameDeleteOver		4010
#define skinButtonGameDeleteDisabled	4040

#define skinButtonVibratorNormal		5000
#define skinButtonVibratorOver			5010
#define skinButtonVibratorSelected		5020
#define skinButtonVibratorSelectedOver	5030
#define skinButtonVibratorDisabled		5040


#define skinButtonSleepNormal		6000
#define skinButtonSleepOver			6010
#define skinButtonSleepSelected		6020
#define skinButtonSleepSelectedOver	9030

#define skinButtonPlayNormal		7000
#define skinButtonPlayOver			7010
#define skinButtonPlayDisabled		7040

// protos
void SknApplySkin();
void SknGetObjectBounds(DmOpenRef skinDBP, DmResID resID, RectangleType *rP);
DmOpenRef SknOpenSkin();
void SknCloseSkin(DmOpenRef skinDBP);
UInt8 SknSetState(DmOpenRef skinDBP, DmResID resID, UInt8 newState);
void SknShowObject(DmOpenRef skinDBP, DmResID resID);
UInt8 SknGetState(DmOpenRef skinDBP, DmResID resID);
void SknUpdateList();
void SknGetListBounds(RectangleType *rAreaP, RectangleType *rArea2xP);
UInt16 SknCheckClick(DmOpenRef skinDBP, Coord mx, Coord my);
void SknSelect(Coord x, Coord y);
Boolean SknProcessArrowAction(UInt16 button);
UInt8 SknGetDepth(DmOpenRef skinDBP);

extern UInt16 lastIndex;

#endif
