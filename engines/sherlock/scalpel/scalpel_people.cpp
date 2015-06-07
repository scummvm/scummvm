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

#include "sherlock/scalpel/scalpel_people.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Scalpel {

void ScalpelPeople::setTalking(int speaker) {
	Resources &res = *_vm->_res;

	// If no speaker is specified, then we can exit immediately
	if (speaker == -1)
		return;

	if (_portraitsOn) {
		delete _talkPics;
		Common::String filename = Common::String::format("%s.vgs", _characters[speaker]._portrait);
		_talkPics = new ImageFile(filename);

		// Load portrait sequences
		Common::SeekableReadStream *stream = res.load("sequence.txt");
		stream->seek(speaker * MAX_FRAME);

		int idx = 0;
		do {
			_portrait._sequences[idx] = stream->readByte();
			++idx;
		} while (idx < 2 || _portrait._sequences[idx - 2] || _portrait._sequences[idx - 1]);

		delete stream;

		_portrait._maxFrames = idx;
		_portrait._frameNumber = 0;
		_portrait._sequenceNumber = 0;
		_portrait._images = _talkPics;
		_portrait._imageFrame = &(*_talkPics)[0];
		_portrait._position = Common::Point(_portraitSide, 10);
		_portrait._delta = Common::Point(0, 0);
		_portrait._oldPosition = Common::Point(0, 0);
		_portrait._goto = Common::Point(0, 0);
		_portrait._flags = 5;
		_portrait._status = 0;
		_portrait._misc = 0;
		_portrait._allow = 0;
		_portrait._type = ACTIVE_BG_SHAPE;
		_portrait._name = " ";
		_portrait._description = " ";
		_portrait._examine = " ";
		_portrait._walkCount = 0;

		if (_holmesFlip || _speakerFlip) {
			_portrait._flags |= 2;

			_holmesFlip = false;
			_speakerFlip = false;
		}

		if (_portraitSide == 20)
			_portraitSide = 220;
		else
			_portraitSide = 20;

		_portraitLoaded = true;
	}
}

} // End of namespace Scalpel

} // End of namespace Sherlock
