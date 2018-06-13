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

#ifndef PINK_AUDIO_INFO_MGR_H
#define PINK_AUDIO_INFO_MGR_H

#include "common/str.h"

namespace Pink {

class Actor;
class LeadActor;
class Archive;

class AudioInfoMgr {
public:
	AudioInfoMgr(LeadActor *lead);

	void loadState(Archive &archive);
	void saveState(Archive &archive);

	void start(Actor *actor);
	void stop();

	void onLeftClick();

private:
	void playAudio();
	void stopAudio();

	void showPDAButton();
	void hidePDAButton();

private:
	Common::String _aboutWhom;
	LeadActor *_lead;
};

} // End of namespace Pink


#endif
