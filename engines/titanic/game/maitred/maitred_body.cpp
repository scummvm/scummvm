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

#include "titanic/game/maitred/maitred_body.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMaitreDBody, CMaitreDProdReceptor)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(AnimateMaitreDMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

void CMaitreDBody::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_armed, indent);
	CMaitreDProdReceptor::save(file, indent);
}

void CMaitreDBody::load(SimpleFile *file) {
	file->readNumber();
	_armed = file->readNumber();
	CMaitreDProdReceptor::load(file);
}

bool CMaitreDBody::EnterViewMsg(CEnterViewMsg *msg) {
	return true;
}

bool CMaitreDBody::AnimateMaitreDMsg(CAnimateMaitreDMsg *msg) {
	static const char *const ARMED_CLIPS[5] = {
		"Talking 1", "Talking 2", "Talking 3", "Talking 4", nullptr
	};
	static const char *const UNARMED_CLIPS[5] = {
		"Armless Talking 1", "Armless Talking 2", "Armless Talking 3",
		"Armless Talking 4", nullptr
	};

	if (!hasActiveMovie()) {
		playRandomClip(_armed ? ARMED_CLIPS : UNARMED_CLIPS);
	}

	return true;
}

bool CMaitreDBody::ActMsg(CActMsg *msg) {
	if (msg->_action == "LoseArm") {
		_armed = false;
		loadFrame(262);
		playSound(TRANSLATE("c#75.wav", "c#57.wav"));
	}

	return true;
}

} // End of namespace Titanic
