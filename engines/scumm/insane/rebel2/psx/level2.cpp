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
 */

#include "common/events.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/util.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel2/shared.h"
#include "scumm/insane/rebel2/psx/psx.h"
#include "scumm/insane/rebel2/psx/ui.h"

namespace Scumm {

// Level 2 is a cover shooter, and the original runs its three parts as separate levels:
// 2, 0xc9 and 0xca. The same engine drives chapters 11 and 12.
const RA2PSXLevel2PartInfo kRA2PSXLevel2Parts[kRA2PSXLevel2PartCount] = {
	{ "l2P1bg", "r1L2P1", 0x26, 0x68, 0, 0x26, true,
		0x2a, 0x32, 0xcf, 0x8d,
		{ 63, 104, 145, 186, 320 }, { 73, 88, 98, 118, 240 },
		{
		{ 255, 65, 78, 116 }, { 250, 68, 76, 113 }, { 234, 68, 76, 113 },
		{ 219, 68, 70, 113 }, { 200, 72, 80, 109 }, { 169, 73, 88, 108 },
		{ 169, 73, 88, 108 }, { 167, 73, 90, 108 }, { 165, 73, 92, 108 },
		{ 165, 73, 92, 108 }, { 179, 73, 78, 108 }, { 179, 73, 78, 108 },
		{ 178, 73, 80, 108 }, { 174, 74, 84, 107 }, { 174, 74, 84, 107 },
		{ 181, 73, 78, 109 }, { 181, 73, 78, 109 }, { 181, 73, 78, 108 },
		{ 181, 73, 80, 108 }, { 181, 73, 80, 108 }, { 180, 73, 80, 108 },
		{ 180, 73, 80, 108 }, { 181, 73, 82, 108 }, { 182, 73, 80, 108 },
		{ 182, 73, 80, 108 }, { 181, 73, 78, 109 }, { 181, 73, 78, 109 },
		{ 182, 73, 78, 108 }, { 183, 73, 80, 108 }, { 183, 73, 80, 108 }
		} },
	{ "l2P2bg", "r1L2P2", 0x00, 0x00, -10, 0x12, false,
		0x1c, 0x26, 0x11e, 0xa0,
		{ 63, 104, 145, 186, 320 }, { 73, 88, 98, 118, 240 },
		{
		{ 255, 65, 78, 116 }, { 250, 68, 76, 113 }, { 234, 68, 76, 113 },
		{ 219, 68, 70, 113 }, { 200, 72, 80, 109 }, { 169, 73, 88, 108 },
		{ 169, 73, 88, 108 }, { 167, 73, 90, 108 }, { 165, 73, 92, 108 },
		{ 165, 73, 92, 108 }, { 179, 73, 78, 108 }, { 179, 73, 78, 108 },
		{ 178, 73, 80, 108 }, { 174, 74, 84, 107 }, { 174, 74, 84, 107 },
		{ 181, 73, 78, 109 }, { 181, 73, 78, 109 }, { 181, 73, 78, 108 },
		{ 181, 73, 80, 108 }, { 181, 73, 80, 108 }, { 180, 73, 80, 108 },
		{ 180, 73, 80, 108 }, { 181, 73, 82, 108 }, { 182, 73, 80, 108 },
		{ 182, 73, 80, 108 }, { 181, 73, 78, 109 }, { 181, 73, 78, 109 },
		{ 182, 73, 78, 108 }, { 183, 73, 80, 108 }, { 183, 73, 80, 108 }
		} },
	{ "l2P3bg", "r1L2P3", 0x00, 0x00, 0, 0x23, false,
		0x1a, 0x43, 0xc9, 0x92,
		{ 63, 104, 145, 186, 320 }, { 73, 88, 98, 118, 240 },
		{
		{ 255, 65, 78, 116 }, { 250, 68, 76, 113 }, { 234, 68, 76, 113 },
		{ 219, 68, 70, 113 }, { 200, 72, 80, 109 }, { 169, 73, 88, 108 },
		{ 169, 73, 88, 108 }, { 167, 73, 90, 108 }, { 165, 73, 92, 108 },
		{ 165, 73, 92, 108 }, { 179, 73, 78, 108 }, { 179, 73, 78, 108 },
		{ 178, 73, 80, 108 }, { 174, 74, 84, 107 }, { 174, 74, 84, 107 },
		{ 181, 73, 78, 109 }, { 181, 73, 78, 109 }, { 181, 73, 78, 108 },
		{ 181, 73, 80, 108 }, { 181, 73, 80, 108 }, { 180, 73, 80, 108 },
		{ 180, 73, 80, 108 }, { 181, 73, 82, 108 }, { 182, 73, 80, 108 },
		{ 182, 73, 80, 108 }, { 181, 73, 78, 109 }, { 181, 73, 78, 109 },
		{ 182, 73, 78, 108 }, { 183, 73, 80, 108 }, { 183, 73, 80, 108 }
		} }
};

RA2PSXLevel2Scene::RA2PSXLevel2Scene() : _part(0), _frame(0), _delay(0), _out(false),
		_moving(false) {
	for (int layer = 0; layer < 2; ++layer) {
		for (int axis = 0; axis < 2; ++axis)
			_scroll[layer][axis] = _scrollTarget[layer][axis] = _scrollStep[layer][axis] = 0;
	}
}

const RA2PSXLevel2PartInfo &RA2PSXLevel2Scene::info() const {
	return kRA2PSXLevel2Parts[_part];
}

bool RA2PSXLevel2Scene::load(const RA2PSXArchive &archive, int part) {
	if (part < 0 || part >= kRA2PSXLevel2PartCount)
		return false;

	Common::Array<byte> data;
	_textures.clear();
	if (!archive.getMember(kRA2PSXLevel2Parts[part].sheet, data) || !_textures.append(data))
		return false;

	static const char *const required[] = { "BACK_L", "BACK_R", "PARA_L", "PARA_R" };
	for (uint i = 0; i < ARRAYSIZE(required); ++i) {
		if (!_textures.has(required[i]))
			return false;
	}

	// Every rookie pose is its own member: a format word, a palette and one image.
	_rookie.clear();
	for (int frame = 0; frame < kRA2PSXLevel2FrameCount; ++frame) {
		const Common::String path = Common::String::format("%s/anim%02d",
				kRA2PSXLevel2Parts[part].anims, frame);
		Common::Array<byte> anim;
		Common::Array<RA2PSXTexture> decoded;
		if (!archive.getMember(path, anim) || anim.size() < 8)
			return false;
		const uint16 height = READ_LE_UINT16(anim.data() + 2) & 0xff;
		if (!loadRA2PSXSpriteAnimation(anim, height ? height : 256, decoded) || decoded.empty())
			return false;
		_rookie.push_back(decoded[0]);
	}

	Common::Array<byte> hud;
	_hud.clear();
	if (archive.getMember("trpTex", hud))
		_hud.append(hud);

	_part = part;
	_frame = 0;
	_delay = 0;
	_out = false;
	_moving = false;
	const RA2PSXLevel2PartInfo &part_ = kRA2PSXLevel2Parts[part];
	_scroll[0][0] = _scrollTarget[0][0] = part_.backScroll << 16;
	_scroll[1][0] = _scrollTarget[1][0] = part_.paraScroll << 16;
	_scroll[0][1] = _scrollTarget[0][1] = 0;
	_scroll[1][1] = _scrollTarget[1][1] = 0;
	for (int layer = 0; layer < 2; ++layer) {
		for (int axis = 0; axis < 2; ++axis)
			_scrollStep[layer][axis] = 0;
	}
	return true;
}

void RA2PSXLevel2Scene::setScrollTarget(int backX, int backY, int paraX, int paraY) {
	_scrollTarget[0][0] = backX;
	_scrollTarget[0][1] = backY;
	_scrollTarget[1][0] = paraX;
	_scrollTarget[1][1] = paraY;
	for (int layer = 0; layer < 2; ++layer) {
		for (int axis = 0; axis < 2; ++axis)
			_scrollStep[layer][axis] =
					(_scrollTarget[layer][axis] - _scroll[layer][axis]) / kRA2PSXLevel2ScrollSteps;
	}
}

void RA2PSXLevel2Scene::toggleCover() {
	if (_moving)
		return;
	_moving = true;
	if (_out) {
		// Ducking back puts the walls where they started.
		setScrollTarget(info().backScroll << 16, 0, info().paraScroll << 16, 0);
	} else {
		setScrollTarget(0, 0, 0, 0);
	}
}

int RA2PSXLevel2Scene::aimFrame(int aimX, int aimY) const {
	const RA2PSXLevel2PartInfo &part = info();
	int column = 1;
	while (column <= kRA2PSXLevel2AimColumns && part.aimColumns[column - 1] < aimX)
		++column;
	int row = 0;
	while (row < kRA2PSXLevel2AimRows && part.aimRows[row] < aimY)
		++row;
	return CLIP<int>(column * kRA2PSXLevel2AimColumns + row,
			(int)kRA2PSXLevel2CoverFrames, (int)kRA2PSXLevel2FrameCount - 1);
}

void RA2PSXLevel2Scene::update(int aimX, int aimY) {
	for (int layer = 0; layer < 2; ++layer) {
		for (int axis = 0; axis < 2; ++axis) {
			int &value = _scroll[layer][axis];
			const int target = _scrollTarget[layer][axis];
			const int step = _scrollStep[layer][axis];
			if (value == target || !step)
				continue;
			value += step;
			if ((step < 0 && value <= target) || (step > 0 && value >= target))
				value = target;
		}
	}

	if (_moving) {
		if (--_delay >= 0)
			return;
		_delay = kRA2PSXLevel2RookieDelay;
		// Leaning out walks 0 to 4, ducking back walks it down again.
		if (!_out) {
			if (_frame < kRA2PSXLevel2CoverFrames - 1) {
				++_frame;
			} else {
				_moving = false;
				_out = true;
				_frame = aimFrame(aimX, aimY);
			}
		} else {
			if (_frame >= kRA2PSXLevel2CoverFrames)
				_frame = kRA2PSXLevel2CoverFrames - 1;
			if (_frame > 0) {
				--_frame;
			} else {
				_moving = false;
				_out = false;
			}
		}
		return;
	}

	if (_out)
		_frame = aimFrame(aimX, aimY);
}

void RA2PSXLevel2Scene::drawLayer(Graphics::Surface &surface, const char *name,
		int x, int y) const {
	if (!_textures.has(name))
		return;
	_textures.draw(surface, name, x, y, Common::Rect(0, 0, 256, 256));
}

void RA2PSXLevel2Scene::drawFrame(Graphics::Surface &surface, const RA2PSXTexture &frame,
		int x, int y) const {
	for (int row = 0; row < frame.height; ++row) {
		const int destY = y + row;
		if (destY < 0 || destY >= surface.h)
			continue;
		for (int column = 0; column < frame.width; ++column) {
			const int destX = x + column;
			if (destX < 0 || destX >= surface.w)
				continue;
			const uint32 pixel = frame.pixels[row * frame.width + column];
			if (!(pixel & 0x01000000))
				continue;
			surface.setPixel(destX, destY, surface.format.RGBToColor(
					(pixel >> 16) & 0xff, (pixel >> 8) & 0xff, pixel & 0xff));
		}
	}
}

void RA2PSXLevel2Scene::draw(Graphics::Surface &surface, int aimX, int aimY) const {
	const RA2PSXLevel2PartInfo &part = info();
	const int left = (surface.w - 320) / 2;
	const int top = (surface.h - 240) / 2 + kRA2PSXLevel2SceneTop;
	const int backX = _scroll[0][0] >> 16;
	const int paraX = _scroll[1][0] >> 16;
	surface.fillRect(Common::Rect(surface.w, surface.h), 0);

	drawLayer(surface, "BACK_L", left - backX, top);
	drawLayer(surface, "BACK_R", left + 256 - backX, top);
	drawLayer(surface, "PARA_L", left - paraX, top);
	if (part.hasMiddleStrip)
		drawLayer(surface, "PARA_M", left + 0x36 - paraX, top);
	drawLayer(surface, "PARA_R", left + 0x36 - paraX, top);

	if ((uint)_frame < _rookie.size()) {
		const RA2PSXLevel2Pose &pose = part.poses[_frame];
		drawFrame(surface, _rookie[_frame], left + pose.x + part.rookieOffsetX,
				(surface.h - 240) / 2 + pose.y + part.rookieOffsetY);
	}

	if (outOfCover() && _hud.has("CROSS"))
		_hud.draw(surface, "CROSS", left + aimX - 4,
				(surface.h - 240) / 2 + aimY - 3, Common::Rect(0, 0, 8, 7));
}

// The PSX release composites its waves from sprite sheets instead of playing a SMUSH
// segment, but the level's shape is the same, so it rides runRebel2Level2 as well.
class Rebel2PSX::Level2Handler : public Rebel2Level2Handler {
public:
	Level2Handler(Rebel2PSX &psx, const RA2PSXArchive &archive) :
		_psx(psx), _archive(archive), _phaseState(0) {
	}

	bool shouldQuit() const override { return _psx._vm->shouldQuit(); }

	bool playOpening() override {
		return _psx.playVideo("S1/L02_INTR.STR", 1, false);
	}

	void beginAttempt() override {}

	void beginPhase(int phase, bool) override {
		_phaseState = 0;
		if (!_scene.load(_archive, phase - 1))
			warning("Rebel Assault II: could not load level 2 part %d", phase);
	}

	int16 waveBudget(int phase) override {
		// The per difficulty budget table is not read out of the PSX build yet.
		return (int16)(4 + phase);
	}

	bool playBackgroundWave(int phase) override { return showScene(phase); }
	bool playWave(int phase, uint16) override { return showScene(phase); }

	WaveCredit creditWave(int16, int16 *, int16) override {
		// Nothing decodes r1L2P* or play* yet, so every phase reports itself finished
		// rather than looping on waves that cannot be built.
		WaveCredit credit;
		credit.stop = true;
		_phaseState = 0x0e;
		return credit;
	}

	bool playPhaseEnd(int phase) override {
		static const char *const cutscenes[] = {
			"S1/L02_CUT1.STR", "S1/L02_CUT2.STR"
		};
		return _psx.playVideo(cutscenes[phase - 1], 1, false);
	}

	uint16 phaseState() const override { return _phaseState; }
	bool playerDead() const override { return false; }
	void accumulateKills() override {}
	void accumulateMisses() override {}

	bool handleDeath(int, Result &result) override {
		result = kQuit;
		return false;
	}

	void playComplete(int) override {
		_psx.playVideo("S1/L02_CUT3.STR", 1, false);
		_psx.playVideo("S1/L02_EXTR.STR", 1, false);
	}

private:
	// Runs the part. The rookie leans out and ducks back, the crosshair picks one of the
	// twenty five aim poses, and the walls slide between the two cover positions.
	bool showScene(int phase) {
		const RA2PSXLevel2PartInfo &part = kRA2PSXLevel2Parts[phase - 1];
		const bool cursorWasVisible = CursorMan.isVisible();
		CursorMan.showMouse(false);

		int aimX = (part.aimLeft + part.aimRight) / 2;
		int aimY = (part.aimTop + part.aimBottom) / 2;
		bool left = false, right = false, up = false, down = false;
		bool running = true;
		const int viewX = ((int)_psx._vm->_screenWidth - 320) / 2;
		const int viewY = ((int)_psx._vm->_screenHeight - 240) / 2;

		while (running && !shouldQuit()) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				const bool pressed = event.type == Common::EVENT_KEYDOWN;
				if (pressed || event.type == Common::EVENT_KEYUP) {
					switch (event.kbd.keycode) {
					case Common::KEYCODE_ESCAPE:
						if (pressed)
							running = false;
						break;
					case Common::KEYCODE_LEFT:
					case Common::KEYCODE_a:
						left = pressed;
						break;
					case Common::KEYCODE_RIGHT:
					case Common::KEYCODE_d:
						right = pressed;
						break;
					case Common::KEYCODE_UP:
					case Common::KEYCODE_w:
						up = pressed;
						break;
					case Common::KEYCODE_DOWN:
					case Common::KEYCODE_s:
						down = pressed;
						break;
					case Common::KEYCODE_SPACE:
					case Common::KEYCODE_RETURN:
						if (pressed && !event.kbdRepeat)
							_scene.toggleCover();
						break;
					default:
						break;
					}
				} else if (event.type == Common::EVENT_MOUSEMOVE) {
					aimX = event.mouse.x - viewX;
					aimY = event.mouse.y - viewY;
				} else if (event.type == Common::EVENT_RBUTTONDOWN) {
					_scene.toggleCover();
				} else if (event.type == Common::EVENT_LBUTTONDOWN) {
					if (!_scene.outOfCover())
						_scene.toggleCover();
				} else if (event.type == Common::EVENT_QUIT ||
						event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
					_psx._vm->quitGame();
				}
			}

			// Aiming only moves while the rookie is actually out of cover.
			if (_scene.outOfCover()) {
				aimX += (right ? 3 : 0) - (left ? 3 : 0);
				aimY += (down ? 3 : 0) - (up ? 3 : 0);
			}
			aimX = CLIP<int>(aimX, part.aimLeft, part.aimRight);
			aimY = CLIP<int>(aimY, part.aimTop, part.aimBottom);
			_scene.update(aimX, aimY);

			Graphics::Surface output;
			output.create(_psx._vm->_screenWidth, _psx._vm->_screenHeight,
					g_system->getScreenFormat());
			_scene.draw(output, aimX, aimY);
			g_system->copyRectToScreen(output.getPixels(), output.pitch, 0, 0,
					output.w, output.h);
			output.free();
			g_system->updateScreen();
			g_system->delayMillis(20);
		}

		CursorMan.showMouse(cursorWasVisible);
		return !shouldQuit();
	}

	Rebel2PSX &_psx;
	const RA2PSXArchive &_archive;
	RA2PSXLevel2Scene _scene;
	uint16 _phaseState;
};

Common::Error Rebel2PSX::runLevel2() {
	Common::SeekableReadStream *stream = openResource(2);
	if (!stream)
		return Common::Error(Common::kReadingFailed,
				_("Could not open the PlayStation Level 2 resources"));
	RA2PSXArchive archive;
	const bool loaded = archive.load(*stream);
	delete stream;
	if (!loaded)
		return Common::Error(Common::kReadingFailed,
				_("Could not read the PlayStation Level 2 resources"));

	Level2Handler handler(*this, archive);
	if (runRebel2Level2(handler, _vm->_rnd) == Rebel2Level2Handler::kError)
		return Common::Error(Common::kReadingFailed,
				_("Could not play the PlayStation Level 2 videos"));
	return Common::kNoError;
}

} // End of namespace Scumm
