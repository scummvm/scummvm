/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_PET_MESSAGES_H
#define TITANIC_PET_MESSAGES_H

#include "titanic/messages/messages.h"

namespace Titanic {

MESSAGE0(CPETDeliverMsg);
MESSAGE0(CPETGainedObjectMsg);
MESSAGE0(CPETHelmetOnOffMsg);
MESSAGE0(CPETKeyboardOnOffMsg);
MESSAGE0(CPETLostObjectMsg);
MESSAGE0(CPETObjectSelectedMsg);
MESSAGE1(CPETObjectStateMsg, int, value, 0);
MESSAGE0(CPETPhotoOnOffMsg);
MESSAGE1(CPETPlaySoundMsg, int, soundNum, 0);
MESSAGE0(CPETReceiveMsg);
MESSAGE0(CPETSetStarDestinationMsg);
MESSAGE1(CPETStarFieldLockMsg, int, value, 0);
MESSAGE0(CPETStereoFieldOnOffMsg);
MESSAGE2(CPETTargetMsg, CString, name, "", int, numValue, -1);

#define PET_MESSAGE(NAME) \
	class NAME: public CPETTargetMsg { \
	public: \
	NAME() : CPETTargetMsg() {} \
	NAME(const CString &name, int num) : CPETTargetMsg(name, num) {} \
	CLASSDEF; \
	static bool isSupportedBy(const CTreeItem *item) { \
		return supports(item, _type); \
	} \
}

PET_MESSAGE(CPETDownMsg);
PET_MESSAGE(CPETUpMsg);
PET_MESSAGE(CPETLeftMsg);
PET_MESSAGE(CPETRightMsg);
PET_MESSAGE(CPETActivateMsg);

} // End of namespace Titanic

#endif /* TITANIC_PET_MESSAGES_H */
