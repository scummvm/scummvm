/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef AMBIENT_H_
#define AMBIENT_H_

#include "common/array.h"

namespace Myst3 {

class Myst3Engine;

class Ambient {
public:
	Ambient(Myst3Engine *vm);
	virtual ~Ambient();

	void playCurrentNode(uint32 volume, uint32 fadeOutDelay);
	void loadNode(uint32 node, uint32 room, uint32 age);
	void applySounds(uint32 fadeOutDelay);
	void scaleVolume(uint32 volume);

	void addSound(uint32 id, int32 volume, int32 heading, int32 headingAngle, int32 u1, int32 fadeOutDelay);

	void setCueSheet(uint32 id, int32 volume, int32 heading, int32 headingAngle);
	void updateCue();

	uint32 _scriptAge;
	uint32 _scriptRoom;

private:
	Myst3Engine *_vm;

	struct AmbientSound {
		uint32 id;
		//char name[32];

		int32 volume;
		int32 volumeFlag;
		int32 heading;
		int32 headingAngle;
		int32 u1;
		int32 fadeOutDelay;

		void reset() {
			id = 0;
			volume = 0;
			volumeFlag = 0;
			heading = 0;
			headingAngle = 0;
			u1 = 0;
			fadeOutDelay = 0;
		}
	};

	uint16 delayForCue(uint32 id);
	uint32 nextCueSound(uint32 id);

	Common::Array<AmbientSound> _sounds;

	AmbientSound _cueSheet;
	uint32 _cueStartTick;
};

} // End of namespace Myst3

#endif // AMBIENT_H_
