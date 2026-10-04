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

#include "common/file.h"
#include "ultima/ultima3/views/title.h"
#include "ultima/ultima3/gfx/pic_decoder.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

// Box regions within EXOD.IBM, in pixels -- (srcY, dstY, x, width, height),
// derived directly from drawTitleBox1-6's drawSparkleBox parameters
// (bl/bh=source/dest row, cx=starting byte column *4, dh=width in *words*
// i.e. *8, dl=row count)
static const TitleBox BOX1 = { 8, 8, 28, 264, 77 };
static const TitleBox BOX4 = { 85, 85, 28, 264, 43 };
static const TitleBox BOX5 = { 132, 132, 68, 72, 4 };
static const TitleBox BOX6 = { 146, 146, 20, 40, 32 };
static const TitleBox BOX2 = { 168, 168, 116, 88, 16 };
static const TitleBox BOX3 = { 152, 168, 116, 96, 16 };

// runBootFlagAnimation's per-si (si=2,1,0) step tables, read directly from
// the 12 raw bytes sitting between drawSparkleBox's end and
// runBootFlagAnimation's start (0x15FE7-0x15FF2 in ultima.idb)
static const int8 FLAG_STEP[3]       = { 1, -1, 1 };
static const int FLAG_AL_LIMIT[3]    = { 4, -1, 4 }; // -1 stands for the 0FFh byte-wrap sentinel
static const byte FLAG_BL_LIMIT[3]   = { 0x31, 0x2B, 0x34 };
static const byte FLAG_INITIAL_AL[3] = { 0, 3, 0 };

constexpr int WAIT_FRAMES_PER_UNIT = 5; // ~waitFrames(1)'s real-hardware duration, approximated

Title::Title() : View("Title") {
}

void Title::loadPic(Graphics::ManagedSurface &surf, const Common::String &filename) {
	Gfx::PicDecoder decoder;
	Common::File f;
	if (!f.open(filename.c_str()) || !decoder.loadStream(f))
		error("Could not load %s", filename.c_str());

	surf.create(320, 200);
	surf.blitFrom(*decoder.getSurface());
}

void Title::loadRaw(Common::Array<byte> &data, const Common::String &filename) {
	Common::File f;
	if (!f.open(filename.c_str()))
		error("Could not load %s", filename.c_str());

	data.resize((uint)f.size());
	f.read(&data[0], data.size());
}

bool Title::msgFocus(const FocusMessage &msg) {
	loadPic(_portrait, "EXOD.IBM");
	loadRaw(_nameData, "NAME.DAT");
	loadRaw(_animateData, "ANIMATE.DAT");

	_canvas.create(320, 200);
	loadPic(_canvas, "BLANK.IBM");

	_phase = PH_BOX1_SPARKLE;
	_sparklePass = 0;
	_keyPending = false;
	_prngState = 0x9DE3;

	delayFrames(1);
	return View::msgFocus(msg);
}

bool Title::msgUnfocus(const UnfocusMessage &msg) {
	_canvas.free();
	_portrait.free();
	_nameData.clear();
	_animateData.clear();
	return View::msgUnfocus(msg);
}

void Title::draw() {
	getSurface().blitFrom(_canvas);
}

void Title::drawBox(const TitleBox &box, bool sparkle, int threshold) {
	int wordCols = box.width / 8;

	for (int row = 0; row < box.height; ++row) {
		for (int col = 0; col < wordCols; ++col) {
			int px = box.x + col * 8;
			bool reveal = true;

			if (sparkle) {
				// The original's additive congruential generator:
				// al' = (al+0x1D); al'' = al'+ah; ah' = al'
				byte al = _prngState & 0xFF, ah = _prngState >> 8;
				byte step1 = al + 0x1D;
				byte newAl = step1 + ah;
				_prngState = (step1 << 8) | newAl;
				reveal = newAl <= threshold;
			}

			for (int dx = 0; dx < 8; ++dx) {
				byte color = reveal ?
					*(const byte *)_portrait.getBasePtr(px + dx, box.srcY + row) : 0;
				_canvas.setPixel(px + dx, box.dstY + row, color);
			}
		}
	}

	redraw();
}

void Title::drawFlagFrame(int bxParam, int axParam) {
	int px = ((bxParam * 2 + 20) / 4) * 4;
	uint32 srcOffset = (uint32)axParam * 1472 + (uint32)(bxParam & 1) * 736;
	const byte *src = &_animateData[srcOffset];

	for (int row = 0; row < 16; ++row) {
		int y = 168 + row;
		const byte *rowBytes = src + row * 23;

		for (int dx = -4; dx < 0; ++dx)
			_canvas.setPixel(px + dx, y, 0);

		for (int b = 0; b < 23; ++b) {
			byte v = rowBytes[b];
			for (int bit = 0; bit < 4; ++bit)
				_canvas.setPixel(px + b * 4 + bit, y, (v >> (6 - bit * 2)) & 3);
		}

		for (int dx = 0; dx < 4; ++dx)
			_canvas.setPixel(px + 23 * 4 + dx, y, 0);
	}

	redraw();
}

bool Title::stepPixelPath() {
	if (_pixelPathPos + 1 >= _nameData.size())
		return false;

	byte lengthByte = _nameData[_pixelPathPos];
	if (lengthByte == 0)
		return false;
	byte rowByte = _nameData[_pixelPathPos + 1];
	_pixelPathPos += 2;

	int x = lengthByte + 20, y = 0xC0 - rowByte;
	_canvas.setPixel(x, y, 3);
	_canvas.setPixel(x + 1, y, 3);

	redraw();
	return true;
}

bool Title::stepFlagAnimation() {
	if (_flagSi < 0)
		return false;

	if (!_flagAlActive) {
		_flagAl = FLAG_INITIAL_AL[_flagSi];
		_flagAlActive = true;
	}

	drawFlagFrame(_flagBl, _flagAl);
	_flagAl += FLAG_STEP[_flagSi];
	int alLimit = FLAG_AL_LIMIT[_flagSi] < 0 ? 0x100 + FLAG_AL_LIMIT[_flagSi] : FLAG_AL_LIMIT[_flagSi];
	if ((_flagAl & 0xFF) != alLimit)
		return true;

	_flagAlActive = false;
	_flagBl = (_flagBl + FLAG_STEP[_flagSi]) & 0xFF;
	if (_flagBl != FLAG_BL_LIMIT[_flagSi])
		return true;

	--_flagSi;
	return _flagSi >= 0;
}

bool Title::tickWait() {
	if (_keyPending)
		return true;
	if (--_waitCounter <= 0)
		return true;

	delayFrames(1);
	return false;
}

void Title::playSoundBurst() {
	// The original toggles the PC speaker's raw output bit ~1024 times at a
	// CPU-speed-dependent rate (no fixed PIT divisor is ever programmed);
	// approximated here as a short buzzy burst rather than an exact tone
	for (int i = 0; i < 8; ++i) {
		g_engine->queueTone(1500, 20);
		g_engine->queueTone(1800, 20);
	}
}

void Title::timeout() {
	switch (_phase) {
	case PH_BOX1_SPARKLE:
		if (_keyPending || _sparklePass >= 64) {
			drawBox(BOX1, false, 0);
			_phase = PH_BOX1_WAIT;
			_waitCounter = 2 * WAIT_FRAMES_PER_UNIT;
		} else {
			drawBox(BOX1, true, _sparklePass * 4);
			++_sparklePass;
		}
		delayFrames(1);
		break;

	case PH_BOX1_WAIT:
		if (tickWait()) {
			_phase = PH_BOX4_SPARKLE;
			_sparklePass = 0;
			delayFrames(1);
		}
		break;

	case PH_BOX4_SPARKLE:
		if (_keyPending || _sparklePass >= 64) {
			drawBox(BOX4, false, 0);
			_phase = PH_BOX4_WAIT;
			_waitCounter = 5 * WAIT_FRAMES_PER_UNIT;
		} else {
			drawBox(BOX4, true, _sparklePass * 4);
			++_sparklePass;
		}
		delayFrames(1);
		break;

	case PH_BOX4_WAIT:
		if (tickWait()) {
			_phase = PH_BOX5;
			delayFrames(1);
		}
		break;

	case PH_BOX5:
		drawBox(BOX5, false, 0);
		_phase = PH_BOX5_WAIT;
		_waitCounter = 3 * WAIT_FRAMES_PER_UNIT;
		delayFrames(1);
		break;

	case PH_BOX5_WAIT:
		if (tickWait()) {
			_phase = PH_PIXEL_PATH;
			_pixelPathPos = 0;
			delayFrames(1);
		}
		break;

	case PH_PIXEL_PATH:
		// ~6 signature steps per tick approximates the original's very
		// short (~9ms) inter-step busy-wait at a modern 20fps frame rate
		if (_keyPending) {
			_phase = PH_PIXEL_PATH_WAIT;
			_waitCounter = 2 * WAIT_FRAMES_PER_UNIT;
		} else {
			for (int i = 0; i < 6; ++i) {
				if (!stepPixelPath()) {
					_phase = PH_PIXEL_PATH_WAIT;
					_waitCounter = 2 * WAIT_FRAMES_PER_UNIT;
					break;
				}
			}
		}
		delayFrames(1);
		break;

	case PH_PIXEL_PATH_WAIT:
		if (tickWait()) {
			_phase = PH_BOX6;
			delayFrames(1);
		}
		break;

	case PH_BOX6:
		drawBox(BOX6, false, 0);
		_phase = PH_BOX6_WAIT;
		_waitCounter = 6 * WAIT_FRAMES_PER_UNIT;
		delayFrames(1);
		break;

	case PH_BOX6_WAIT:
		if (tickWait()) {
			_phase = PH_FLAG_ANIM;
			_flagSi = 2;
			_flagBl = 0x2A;
			_flagAlActive = false;
			delayFrames(1);
		}
		break;

	case PH_FLAG_ANIM:
		if (_keyPending || !stepFlagAnimation()) {
			_phase = PH_FLAG_ANIM_WAIT;
			_waitCounter = 4 * WAIT_FRAMES_PER_UNIT;
		}
		delayFrames(3); // ~drawAnimationFrameRow's own ~130ms inter-frame wait
		break;

	case PH_FLAG_ANIM_WAIT:
		if (tickWait()) {
			_phase = PH_BOX2;
			delayFrames(1);
		}
		break;

	case PH_BOX2:
		drawBox(BOX2, false, 0);
		_phase = PH_SOUND_BURST;
		delayFrames(1);
		break;

	case PH_SOUND_BURST:
		if (!_keyPending)
			playSoundBurst();
		_phase = PH_BOX3;
		delayFrames(1);
		break;

	case PH_BOX3:
		drawBox(BOX3, false, 0);
		_phase = PH_BOX3_WAIT;
		_waitCounter = 5 * WAIT_FRAMES_PER_UNIT;
		delayFrames(1);
		break;

	case PH_BOX3_WAIT:
		if (tickWait()) {
			_phase = PH_DRAIN;
			delayFrames(1);
		}
		break;

	case PH_DRAIN:
		_keyPending = false;
		draw();
		showMainMenu();
		break;
	}
}

void Title::showMainMenu() {
	replaceView("MainMenu");
}

bool Title::msgKeypress(const KeypressMessage &msg) {
	_keyPending = true;
	return true;
}

bool Title::msgMouseDown(const MouseDownMessage &msg) {
	_keyPending = true;
	return true;
}

bool Title::msgAction(const ActionMessage &msg) {
	_keyPending = true;
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
