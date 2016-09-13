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

#include "common/md5.h"
#include "xeen/music.h"
#include "xeen/xeen.h"
#include "xeen/files.h"

namespace Xeen {

#define ADLIB_CHANNEL_COUNT 9
#define CALLBACKS_PER_SECOND 60

Music::Music(Audio::Mixer *mixer) : _mixer(mixer), _effectsData(nullptr) {
	_mixer = mixer;
	_opl = OPL::Config::create();
	_opl->init();
	_opl->start(new Common::Functor0Mem<void, Music>(this, &Music::onTimer), CALLBACKS_PER_SECOND);

	loadEffectsData();
}

Music::~Music() {
	_opl->stop();
	delete _opl;
	delete[] _effectsData;
}

void Music::loadEffectsData() {
	File file("admus");
	Common::String md5str = Common::computeStreamMD5AsString(file, 8192);
	
	if (md5str != "be8989a5e868913f0e53963046e3ea13")
		error("Unknown music driver encountered");

	// Load in the driver data
	const int EFFECTS_OFFSET = 0x91D;
	byte *effectsData = new byte[file.size() - EFFECTS_OFFSET];
	file.seek(EFFECTS_OFFSET);
	file.read(effectsData, file.size() - EFFECTS_OFFSET);
	file.close();
	_effectsData = effectsData;

	// Extract the effects offsets
	_effectsOffsets.resize(180);
	for (int idx = 0; idx < 180; ++idx)
		_effectsOffsets[idx] = READ_LE_UINT16(&effectsData[idx * 2]) - EFFECTS_OFFSET;
}

void Music::onTimer() {
	Common::StackLock slock(_driverMutex);
	update();
	flush();
}

void Music::flush() {
	Common::StackLock slock(_driverMutex);

	while (!_queue.empty()) {
		RegisterValue v = _queue.pop();
		_opl->writeReg(v._regNum, v._value);
	}
}

void Music::update() {
	// TODO
}

} // End of namespace Xeen
