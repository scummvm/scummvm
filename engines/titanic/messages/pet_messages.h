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

RAW_MESSAGE(CPETDeliverMsg);
RAW_MESSAGE(CPETGainedObjectMsg);
RAW_MESSAGE(CPETHelmetOnOffMsg);
RAW_MESSAGE(CPETKeyboardOnOffMsg);
RAW_MESSAGE(CPETLostObjectMsg);
RAW_MESSAGE(CPETObjectSelectedMsg);
NUM_MESSAGE(CPETObjectStateMsg, _value);
RAW_MESSAGE(CPETPhotoOnOffMsg);
NUM_MESSAGE(CPETPlaySoundMsg, _value);
RAW_MESSAGE(CPETReceiveMsg);
RAW_MESSAGE(CPETSetStarDestinationMsg);
NUM_MESSAGE(CPETStarFieldLockMsg, _value);
RAW_MESSAGE(CPETStereoFieldOnOffMsg);
SNUM_MESSAGE_VAL(CPETTargetMsg, _strValue, _numValue, nullptr, -1);

} // End of namespace Titanic

#endif /* TITANIC_PET_MESSAGES_H */
