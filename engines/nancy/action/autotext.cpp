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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/action/autotext.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/util.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"

namespace Nancy {
namespace Action {

void Autotext::readData(Common::SeekableReadStream &stream) {
	_transparency = stream.readUint16LE();
	_surfaceID = stream.readUint16LE();
	_fontID = stream.readUint16LE();
	_defaultTextColor = stream.readUint16LE();
	_offset.x = stream.readUint16LE();
	_offset.y = stream.readUint16LE();
	_surfWidth = stream.readUint16LE();
	_surfHeight = stream.readUint16LE();

	readFilename(stream, _imageName);

	uint16 numImages = stream.readUint16LE();

	_imageLineIDs.resize(numImages);
	_imageSrcs.resize(numImages);
	for (uint i = 0; i < numImages; ++i) {
		_imageLineIDs[i] = stream.readUint16LE();
		readRect(stream, _imageSrcs[i]);
	}

	stream.skip((5 - numImages) * (2 + 16));

	_useAutotextChunk = stream.readByte();
	readFilename(stream, _textKey);

	uint sizeText = stream.readUint16LE();

	if (sizeText) {
		char *buf = new char[sizeText];
		stream.read(buf, sizeText);
		assembleTextLine(buf, _embeddedText, sizeText);
		delete[] buf;
	}
}

void Autotext::execute() {
	g_nancy->_resource->loadImage(_imageName, _image);

	if (_surfaceID > 2) {
		// Surfaces 3+ are journal surfaces, and their text contents are saved. Texts MUST be in CONVO chunk,
		// so we do not check _useAutotextChunk
		Nancy::JournalData *journalData = (Nancy::JournalData *)NancySceneState.getPuzzleData(Nancy::JournalData::getTag());
		assert(journalData);
		const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
		assert(autotext);

		bool isLIFO = (g_nancy->getGameType() == kGameTypeNancy7) && _surfaceID > 5; // This is nancy7-specific, later games implement LIFO in a different way
		if (isLIFO) {
			_surfaceID -= 3;
		}

		Common::String stringToPush;
		auto &entriesForSurface = journalData->journalEntries[_surfaceID];
		bool foundThisKey = false;
		for (auto &stringID : entriesForSurface) {
			stringToPush += autotext->texts[stringID];
			if (stringID == _textKey) {
				foundThisKey = true;
			}
		}

		if (!foundThisKey) {
			// Key inside this Autotext instance wasn't found inside existing list, push it back and add it to string to draw
			if (!isLIFO) {
				// Push at end
				entriesForSurface.push_back(_textKey);
				stringToPush += autotext->texts[_textKey];
			} else {
				// Insert at front
				entriesForSurface.insert_at(0, _textKey);
				stringToPush = autotext->texts[_textKey] + stringToPush;
			}
		}

		addTextLine(stringToPush);
	} else {
		// Surfaces 0-2 have their contents cleared every scene (though we only bother doing so when we need to reuse)
		if (_useAutotextChunk) {
			// We have a key into the AUTOTEXT chunk
			const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
			assert(autotext);

			addTextLine(autotext->texts[_textKey]);
		} else {
			// We have text embedded inside this Autotext instance
			addTextLine(_embeddedText);
		}
	}

	// A height of zero means the surface doesn't need to be drawn
	if (_surfHeight) {
		// Guesstimate the height of the surface
		uint surfHeight = _textLines[0].size() / 144 * _surfWidth;
		surfHeight = MAX<uint>(surfHeight, _surfHeight + 20);
		Graphics::ManagedSurface &surf = g_nancy->_graphicsManager->getAutotextSurface(_surfaceID);
		surf.create(_surfWidth + 1, surfHeight, g_nancy->_graphicsManager->getInputPixelFormat());
		if (_transparency) {
			surf.clear(g_nancy->_graphicsManager->getTransColor());
		}

		_fullSurface.create(surf, surf.getBounds());
		if(_transparency == kPlayOverlayTransparent) {
			_fullSurface.setTransparentColor(g_nancy->_graphicsManager->getTransColor());
		}

		Common::Rect textBounds = surf.getBounds();
		textBounds.left += _offset.x;
		textBounds.top += _offset.y;
		drawAllText(textBounds, _fontID, _fontID);
	}

	_isDone = true;
}

} // End of namespace Action
} // End of namespace Nancy
