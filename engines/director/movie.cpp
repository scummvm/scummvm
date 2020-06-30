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

#include "common/config-manager.h"
#include "common/substream.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/lingo/lingo.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/util.h"

namespace Director {

Movie::Movie(DirectorEngine *vm) {
	_vm = vm;
	_lingo = _vm->getLingo();

	_flags = 0;

	_currentMouseDownSpriteId = 0;
	_currentClickOnSpriteId = 0;
	_lastEventTime = _vm->getMacTicks();
	_lastKeyTime = _lastEventTime;
	_lastClickTime = _lastEventTime;
	_lastRollTime = _lastEventTime;
	_lastTimerReset = _lastEventTime;

	_movieArchive = nullptr;

	_cast = new Cast(_vm, this);
	_score = new Score(_vm, this);
}

Movie::~Movie() {
	// _movieArchive is shared with the cast, so the cast will free it
	delete _cast;
	delete _score;
}

void Movie::setArchive(Archive *archive) {
	_movieArchive = archive;

	if (archive->hasResource(MKTAG('M', 'C', 'N', 'M'), 0)) {
		_macName = archive->getName(MKTAG('M', 'C', 'N', 'M'), 0).c_str();
	} else {
		_macName = archive->getFileName();
	}

	_cast->setArchive(archive);

	// Frame Labels
	if (archive->hasResource(MKTAG('V', 'W', 'L', 'B'), -1)) {
		Common::SeekableSubReadStreamEndian *r;
		_score->loadLabels(*(r = archive->getFirstResource(MKTAG('V', 'W', 'L', 'B'))));
		delete r;
	}
}

bool Movie::loadArchive() {
	Common::SeekableSubReadStreamEndian *r = nullptr;

	// File Info
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'F', 'I'), -1)) {
		loadFileInfo(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'F', 'I'))));
		delete r;
	}

	// Cast
	_cast->loadArchive();

	// _movieRect and _stageColor are in VWCF, which the cast handles

	Stage *stage = g_director->getStage();

	// If the stage dimensions are different, delete it and start again.
	// Otherwise, do not clear it so there can be a nice transition.
	if (stage->getSurface()->w != _movieRect.width() || stage->getSurface()->h != _movieRect.height()) {
		stage->resize(_movieRect.width(), _movieRect.height());
	}

	stage->setStageColor(_stageColor);

	// Score
	if (!_movieArchive->hasResource(MKTAG('V', 'W', 'S', 'C'), -1)) {
		warning("Score::loadArchive(): Wrong movie format. VWSC resource missing");
		return false;
	}
	_score->loadFrames(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'S', 'C'))));
	delete r;

	// Action list
	if (_movieArchive->hasResource(MKTAG('V', 'W', 'A', 'C'), -1)) {
		_score->loadActions(*(r = _movieArchive->getFirstResource(MKTAG('V', 'W', 'A', 'C'))));
		delete r;
	}

	_score->setSpriteCasts();

	return true;
}

Common::Rect Movie::readRect(Common::ReadStreamEndian &stream) {
	Common::Rect rect;
	rect.top = stream.readUint16();
	rect.left = stream.readUint16();
	rect.bottom = stream.readUint16();
	rect.right = stream.readUint16();

	return rect;
}

Common::Array<Common::String> Movie::loadStrings(Common::SeekableSubReadStreamEndian &stream, uint32 &entryType, bool hasHeader) {
	Common::Array<Common::String> strings;
	uint32 offset = 0;

	if (hasHeader) {
		offset = stream.readUint32();
		/*uint32 unk1 = */ stream.readUint32();
		/*uint32 unk2 = */ stream.readUint32();
		entryType = stream.readUint32();
		stream.seek(offset);
	}

	uint16 count = stream.readUint16() + 1;

	debugC(3, kDebugLoading, "Score::loadStrings(): Strings: %d entries", count);

	uint32 *entries = (uint32 *)calloc(count, sizeof(uint32));

	for (uint i = 0; i < count; i++)
		entries[i] = stream.readUint32();

	byte *data = (byte *)malloc(entries[count - 1]);
	stream.read(data, entries[count - 1]);

	for (uint16 i = 0; i < count - 1; i++) {
		Common::String entryString;

		uint start = i == 1 ? entries[i] + 1 : entries[i]; // Skip first byte which is string length

		for (uint j = start; j < entries[i + 1]; j++)
			if (data[j] == '\r')
				entryString += '\n';
			else if (data[j] >= 0x20)
				entryString += data[j];

		strings.push_back(entryString);

		debugC(6, kDebugLoading, "String %d:\n%s\n", i, Common::toPrintable(entryString).c_str());
	}

	free(data);
	free(entries);

	return strings;
}

void Movie::loadFileInfo(Common::SeekableSubReadStreamEndian &stream) {
	debugC(2, kDebugLoading, "****** Loading FileInfo VWFI");

	Common::Array<Common::String> fileInfoStrings = Movie::loadStrings(stream, _flags);
	_script = fileInfoStrings[0];

	if (!_script.empty() && ConfMan.getBool("dump_scripts"))
		_cast->dumpScript(_script.c_str(), kMovieScript, _cast->_movieScriptCount);

	if (!_script.empty())
		_lingo->addCode(_script.c_str(), _cast->_lingoArchive, kMovieScript, _cast->_movieScriptCount);

	_cast->_movieScriptCount++;
	_changedBy = fileInfoStrings[1];
	_createdBy = fileInfoStrings[2];
	_directory = fileInfoStrings[3];
}

} // End of namespace Director
