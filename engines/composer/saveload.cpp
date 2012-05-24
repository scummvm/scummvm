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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef SAVING_ANYWHERE
#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/zlib.h"
#include "graphics/palette.h"

#include "composer/composer.h"
#include "composer/graphics.h"

namespace Composer {
Common::String ComposerEngine::makeSaveGameName(int slot) {
	return (_targetName + Common::String::format(".%02d", slot));
}

Common::Error ComposerEngine::loadGameState(int slot) {
	Common::String filename = makeSaveGameName(slot);
	Common::InSaveFile *in;
	if (!(in = _saveFileMan->openForLoading(filename)))
		return Common::kPathNotFile;

	Common::Serializer ser(in, NULL);
	byte magic[4];
	ser.syncBytes(magic, 4);
	if (magic[0] != 'C' || magic[1] != 'M' || magic[2] != 'P' || magic[3] != 'S') 
		return Common::kUnknownError;

	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
		delete *i;
	}
	_anims.clear();

	for (Common::List<Pipe *>::iterator i = _pipes.begin(); i != _pipes.end(); i++) {
		delete *i;
	}
	_pipes.clear();

	ser.syncVersion(0);
	Common::String desc;
	ser.syncString(desc);
	uint32 tmp;
	ser.syncAsUint32LE(tmp);
	_rnd->setSeed(tmp);
	ser.syncAsUint32LE(_currentTime);
	uint32 timeDelta = _system->getMillis() - _currentTime;
	_currentTime += timeDelta;
	ser.syncAsUint32LE(_lastTime);
	_lastTime += timeDelta;
	ser.syncString(_bookGroup);
	_libraries.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 id;
		ser.syncAsUint16LE(id);
		loadLibrary(id);
	}
	_sprites.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 id;
		ser.syncAsUint16LE(id);
		Sprite sprite;
		sprite._id = id;
		initSprite(sprite);
		_sprites.push_back(sprite);
	}

	_pendingPageChanges.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 id;
		bool remove;
		ser.syncAsUint16LE(id);
		ser.syncAsByte(remove);
		_pendingPageChanges.push_back(PendingPageChange(id, remove));
	}
	_stack.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 svar;
		ser.syncAsUint16LE(svar);
		_stack.push_back(svar);
	}
	_vars.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 var;
		ser.syncAsUint16LE(var);
		_vars.push_back(var);
	}

	for (Common::List<OldScript *>::iterator i = _oldScripts.begin(); i != _oldScripts.end(); i++) {
		delete *i;
	}
	_oldScripts.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		uint16 id;
		uint32 delay;
		ser.syncAsUint16LE(id);
		ser.syncAsUint32LE(delay);
		OldScript *oTmp = new OldScript(id, getResource(ID_SCRP, id));
		oTmp->_currDelay = delay;
		_oldScripts.push_back(oTmp);
	}
	_queuedScripts.clear();
	ser.syncAsUint32LE(tmp);
	for (uint32 i = tmp; i > 0; i--) {
		QueuedScript qTmp;
		ser.syncAsUint32LE(qTmp._baseTime);
		qTmp._baseTime += timeDelta;
		ser.syncAsUint32LE(qTmp._duration);
		ser.syncAsUint32LE(qTmp._count);
		if(qTmp._count != 0) {
			assert(qTmp._count != 0);
		}
		ser.syncAsUint16LE(qTmp._scriptId);
		_queuedScripts.push_back(qTmp);
	}

	ser.syncAsByte(_mouseEnabled);
	ser.syncAsByte(_mouseVisible);
	ser.syncAsUint16LE(_mouseSpriteId);
	_dirtyRects.clear();

	_dirtyRects.push_back(Common::Rect(0, 0, 640, 480));
	byte palbuf[256 * 3];
	ser.syncBytes(palbuf, 256 * 3);
	_system->getPaletteManager()->setPalette(palbuf, 0, 256);
	_needsUpdate = true;

	return Common::kNoError;
}

Common::Error ComposerEngine::saveGameState(int slot, const Common::String &desc) {
	Common::String filename = makeSaveGameName(slot);
	Common::OutSaveFile *out;
	if (!(out = _saveFileMan->openForSaving(filename)))
		return Common::kWritingFailed;

	Common::Serializer ser(NULL, out);
	byte magic[4] = {'C', 'M', 'P', 'S'};
	ser.syncBytes(magic, 4);
	ser.syncVersion(0);
	Common::String desctmp = desc;
	ser.syncString(desctmp);
	uint32 tmp = _rnd->getSeed();
	ser.syncAsUint32LE(tmp);
	ser.syncAsUint32LE(_currentTime);
	ser.syncAsUint32LE(_lastTime);
	ser.syncString(_bookGroup);
	tmp = _libraries.size();
	ser.syncAsUint32LE(tmp);
	for (Common::List<Library>::const_iterator i = _libraries.begin(); i != _libraries.end(); i++) {
		uint16 tmp = (*i)._id;
		ser.syncAsUint16LE(tmp);
	}
	tmp = _sprites.size();
	ser.syncAsUint32LE(tmp);
	for (Common::List<Sprite>::const_iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		uint16 tmp = (*i)._id;
		ser.syncAsUint16LE(tmp);
	}
	tmp = _pendingPageChanges.size();
	ser.syncAsUint32LE(tmp);
	for (Common::Array<PendingPageChange>::const_iterator i = _pendingPageChanges.begin(); i != _pendingPageChanges.end(); i++) {
		uint16 tmp = (*i)._pageId;
		bool tmpb = (*i)._remove;
		ser.syncAsUint16LE(tmp);
		ser.syncAsByte(tmpb);
	}
	tmp = _stack.size();
	ser.syncAsUint32LE(tmp);
	for (Common::Array<uint16>::const_iterator i = _stack.begin(); i != _stack.end(); i++) {
		uint16 tmp = (*i);
		ser.syncAsUint16LE(tmp);
	}
	tmp = _vars.size();
	ser.syncAsUint32LE(tmp);
	for (Common::Array<uint16>::const_iterator i = _vars.begin(); i != _vars.end(); i++) {
		uint16 tmp = (*i);
		ser.syncAsUint16LE(tmp);
	}
	tmp = _oldScripts.size();
	ser.syncAsUint32LE(tmp);
	for (Common::List<OldScript *>::const_iterator i = _oldScripts.begin(); i != _oldScripts.end(); i++) {
		uint16 tmp16 = (*i)->_id;
		tmp = (*i)->_currDelay;
		ser.syncAsUint16LE(tmp16);
		ser.syncAsUint32LE(tmp);
	}
	tmp = _queuedScripts.size();
	ser.syncAsUint32LE(tmp);
	for (Common::Array<QueuedScript>::const_iterator i = _queuedScripts.begin(); i != _queuedScripts.end(); i++) {
		tmp = (*i)._baseTime;
		ser.syncAsUint32LE(tmp);
		tmp = (*i)._duration;
		ser.syncAsUint32LE(tmp);
		tmp = (*i)._count;
		if(tmp != 0) {
			assert(tmp != 0);
		}
		uint16 tmp16 = (*i)._scriptId;
		ser.syncAsUint32LE(tmp);
		ser.syncAsUint16LE(tmp16);
	}
	ser.syncAsByte(_mouseEnabled);
	ser.syncAsByte(_mouseVisible);
	ser.syncAsUint16LE(_mouseSpriteId);

	byte palbuf[256 * 3];
	_system->getPaletteManager()->grabPalette(palbuf, 0, 256);
	ser.syncBytes(palbuf, 256 * 3);

	out->finalize();
	return Common::kNoError;
}
}
#endif
