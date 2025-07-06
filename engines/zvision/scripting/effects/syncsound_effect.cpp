/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "audio/decoders/wave.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/scripting/effects/syncsound_effect.h"
#include "zvision/sound/zork_raw.h"


namespace ZVision {

SyncSoundNode::SyncSoundNode(ZVision *engine, uint32 key, Common::Path &filename, int32 syncto)
	: ScriptingEffect(engine, key, SCRIPTING_EFFECT_AUDIO) {
	_syncto = syncto;
	_sub = 0;

	Audio::RewindableAudioStream *audioStream = NULL;

	if (filename.baseName().contains(".wav")) {
		Common::File *file = new Common::File();
		if (file->open(filename)) {
			audioStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
		}
	} else {
		audioStream = makeRawZorkStream(filename, _engine);
	}

	_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, audioStream);

	Common::String subname = filename.baseName();
	subname.setChar('s', subname.size() - 3);
	subname.setChar('u', subname.size() - 2);
	subname.setChar('b', subname.size() - 1);

	Common::Path subpath(filename.getParent().appendComponent(subname));
	if (SearchMan.hasFile(subpath))
		_sub = _engine->getSubtitleManager()->create(subpath, _handle); // NB automatic subtitle!
}

SyncSoundNode::~SyncSoundNode() {
	_engine->_mixer->stopHandle(_handle);
	if (_sub)
		_engine->getSubtitleManager()->destroy(_sub);
}

bool SyncSoundNode::process(uint32 deltaTimeInMillis) {
	if (! _engine->_mixer->isSoundHandleActive(_handle))
		return stop();
	else {

		if (_engine->getScriptManager()->getSideFX(_syncto) == NULL)
			return stop();
	}
	return false;
}

} // End of namespace ZVision
