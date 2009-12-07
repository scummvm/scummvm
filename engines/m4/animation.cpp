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
 * $URL$
 * $Id$
 *
 */

#include "m4/assets.h"
#include "m4/animation.h"
#include "m4/compression.h"

namespace M4 {

// TODO: this code needs cleanup

Animation::Animation(M4Engine *vm) {
	_vm = vm;
	_playing = false;
}

void Animation::loadFullScreen(const char *filename) {
	_vm->_palette->deleteAllRanges();
	load(filename);
}

void Animation::load(const char *filename) {
	MadsPack anim(filename, _vm);
	char buffer[20];

	// Chunk 1: header
	// header
	// TODO: there are some unknown fields here, plus we don't read
	// the entire chunk
	Common::SeekableReadStream *animStream = anim.getItemStream(0);
	Common::SeekableReadStream *spriteSeriesStream;
	//printf("Chunk 0, size %i\n", animStream->size());
	_seriesCount = animStream->readUint16LE();
	_frameCount = animStream->readUint16LE();
	_frameEntryCount = animStream->readUint16LE();

	// Unknown
	for (int i = 0; i < 43; i++)
		animStream->readByte();

	_spriteSeriesNames = new Common::String[_seriesCount];
	printf("%i sprite series\n", _seriesCount);

	// TODO: for now, we only load the first sprite series
	if (_seriesCount > 1)
		printf("TODO: Anim has %i sprite series, for now, we only load the first one\n", _seriesCount);
	_seriesCount = 1;		// TODO

	for (int i = 0; i < _seriesCount; i++) {
		animStream->read(buffer, 13);
		_spriteSeriesNames[i] = Common::String(buffer);
		//printf("%03d: %s\n", i, _spriteSeriesNames[i].c_str());

		spriteSeriesStream = _vm->res()->get(_spriteSeriesNames[i].c_str());
		_spriteSeries = new SpriteAsset(_vm, spriteSeriesStream,
										spriteSeriesStream->size(), _spriteSeriesNames[i].c_str());
		_vm->res()->toss(_spriteSeriesNames[i].c_str());

		// Adjust the palette of the sprites in the sprite series
		// so that they can be displayed on screen correctly
		RGBList *palData = new RGBList(_spriteSeries->getColorCount(), _spriteSeries->getPalette(), true);
		_vm->_palette->addRange(palData);

		for (int k = 0; k < _spriteSeries->getCount(); k++) {
			M4Sprite *spr = _spriteSeries->getFrame(k);
			spr->translate(palData);		// sprite pixel translation
		}
	}

	//printf("End pos: %i\n", animStream->pos());

	delete animStream;

	// ------------------

	// Chunk 2: anim info
	AnimationFrame frame;
	animStream = anim.getItemStream(1);
	//printf("Chunk 1, size %i\n", animStream->size());

	_frameEntries = new AnimationFrame[_frameEntryCount];

	for (int i = 0; i < _frameEntryCount; i++) {

		frame.animFrameIndex = animStream->readUint16LE();
		frame.u = animStream->readByte();
		frame.seriesIndex = animStream->readByte();
		frame.seriesFrameIndex = animStream->readUint16LE();
		frame.x = animStream->readUint16LE();
		frame.y = animStream->readUint16LE();
		frame.v = animStream->readByte();
		frame.w = animStream->readByte();

		_frameEntries[i] = frame;

		/*
		printf(
		"animFrameIndex = %4d, "
		"u = %3d, "
		"seriesIndex = %3d, "
		"seriesFrameIndex = %6d, "
		"x = %3d, "
		"y = %3d, "
		"v = %3d, "
		"w = %3d\n",

		frame.animFrameIndex,
		frame.u,
		frame.seriesIndex,
		frame.seriesFrameIndex,
		frame.x,
		frame.y,
		frame.v,
		frame.w
		);
		*/
	}
	//printf("End pos: %i\n", animStream->pos());

	delete animStream;

	// Chunk 3: unknown (seems to be sound data?)
	// TODO
}

Animation::~Animation() {
	//delete[] _spriteSeriesNames;
	//delete[] _spriteSeries;
	//delete[] _frameEntries;
}

void Animation::start() {
	_curFrame = 0;
	_curFrameEntry = 0;
	//for (int i = 0; i < _seriesCount; i++) {
		//_spriteSeries[i] = new SpriteSeries((char*)_spriteSeriesNames[i].c_str());
	//}
	_playing = true;
	updateAnim();
}

bool Animation::updateAnim() {
	if (!_playing)
		return true;

	// Get the scene background surface
	M4Surface *bg = _vm->_scene->getBackgroundSurface();

	while (_frameEntries[_curFrameEntry].animFrameIndex == _curFrame) {
		AnimationFrame *frame = &_frameEntries[_curFrameEntry];
		int seriesFrameIndex = (frame->seriesFrameIndex & 0x7FFF) - 1;

		// Write the sprite onto the screen
		M4Sprite *spr = _spriteSeries->getFrame(seriesFrameIndex);

		// FIXME: We assume that the transparent color is the color of the top left pixel
		byte *transparentColor = spr->getBasePtr(0, 0);

		// FIXME: correct x, y
		spr->copyTo(bg, frame->x, frame->y, (int)*transparentColor);

		// HACK: wait a bit
		g_system->delayMillis(100);

		//printf("_curFrameEntry = %d\n", _curFrameEntry);
		_curFrameEntry++;
	}

	//printf("_curFrame = %d\n", _curFrame);

	_curFrame++;
	if (_curFrame >= _frameCount)		// anim done
		stop();

	return _curFrame >= _frameCount;
}

void Animation::stop() {
	_playing = false;

	for (int i = 0; i < _seriesCount; i++) {
		// TODO: cleanup
		//delete _spriteSeries[i];
		//_spriteSeries[i] = NULL;
	}
}

} // End of namespace M4
