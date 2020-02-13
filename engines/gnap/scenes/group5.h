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

#ifndef GNAP_GROUP5_H
#define GNAP_GROUP5_H

#include "gnap/debugger.h"
#include "gnap/scenes/scenecore.h"

namespace Gnap {

enum {
	kHS53Platypus		= 0,
	kHS53Device			= 1,
	kHS53PhoneKey1		= 2,
	kHS53PhoneKey2		= 3,
	kHS53PhoneKey3		= 4,
	kHS53PhoneKey4		= 5,
	kHS53PhoneKey5		= 6,
	kHS53PhoneKey6		= 7,
	kHS53PhoneKey7		= 8,
	kHS53PhoneKey8		= 9,
	kHS53PhoneKey9		= 10,
	kHS53PhoneKey0		= 11,
	kHS53PhoneKeySharp	= 12,
	kHS53PhoneKeyStar	= 13,
	kHS53PhoneExit		= 14
};

/*****************************************************************************/

class GnapEngine;

class Scene53: public Scene {
public:
	Scene53(GnapEngine *vm);
	~Scene53() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override {};
	void updateAnimationsCb() override {};

private:
	bool _isGnapPhoning;
	int _currHandSequenceId;
	int _callsMadeCtr;
	uint _callsRndUsed;

	int pressPhoneNumberButton(int phoneNumber, int buttonNum);
	int getRandomCallIndex();
	void runRandomCall();
	void runChitChatLine();
};

} // End of namespace Gnap
#endif // GNAP_GROUP5_H
