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

#include "titanic/game/maitred/maitred_legs.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMaitreDLegs, CMaitreDProdReceptor)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(AnimateMaitreDMsg)
END_MESSAGE_MAP()

void CMaitreDLegs::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	CMaitreDProdReceptor::save(file, indent);
}

void CMaitreDLegs::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	CMaitreDProdReceptor::load(file);
}

bool CMaitreDLegs::EnterViewMsg(CEnterViewMsg *msg) {
	_flag = true;
	loadFrame(0);
	return true;
}

bool CMaitreDLegs::AnimateMaitreDMsg(CAnimateMaitreDMsg *msg) {
	static const char *const WIGGLE_CLIPS[4] = {
		"Hip Wiggle", "Knee Bend", "Wire Wiggle", nullptr
	};
	static const char *const FIGHTING_CLIPS[4] = {
		"Fighting 1", "Fighting 2", "Leg Fidget", nullptr
	};
	static const char *const ARCING_SOUNDS[9] = {
		"MaitreD Arcing 1.wav", "MaitreD Arcing 2.wav",
		"MaitreD Arcing 3.wav", "MaitreD Arcing 4.wav",
		"MaitreD Arcing 5.wav", "MaitreD Arcing 6.wav",
		"MaitreD Arcing 7.wav", "MaitreD Arcing 8.wav",
		"MaitreD Arcing 9.wav"
	};

	switch (msg->_value) {
	case 0:
		if (_flag) {
			playRandomClip(FIGHTING_CLIPS);

			if (getRandomNumber(2) != 0)
				playSound(ARCING_SOUNDS[getRandomNumber(8)],
					40 + getRandomNumber(30));
		} else {
			playClip("Walk Right");
			_flag = true;
		}
		break;

	case 1:
		if (_flag) {
			playClip("Walk Left");
			_flag = false;
		} else {
			playRandomClip(WIGGLE_CLIPS);
		}
		break;

	default:
		break;
	}

	return true;
}

} // End of namespace Titanic
