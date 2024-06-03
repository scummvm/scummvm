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

#include "common/keyboard.h"

#include "ultima/shared/std/string.h"
#include "ultima/ultima8/gumps/computer_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/gfx/fonts/rendered_text.h"
#include "ultima/ultima8/gfx/fonts/font.h"
#include "ultima/ultima8/gfx/fonts/font_manager.h"
#include "ultima/ultima8/gfx/fonts/shape_font.h"
#include "ultima/ultima8/usecode/uc_machine.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ComputerGump)

static const int COMPUTER_FONT = 6;
static const int COMPUTER_GUMP_SHAPE = 30;
static const int COMPUTER_GUMP_SOUND = 0x33;
static const int MAX_LINE_LEN = 19;

static const int TEXT_XOFF = 41;
static const int TEXT_YOFF = 38;

ComputerGump::ComputerGump()
	: ModalGump(), _curTextLine(0), _charOff(0), _nextCharTick(0), _paused(false), _curDisplayLine(0), _tick(0) {
	for (int i = 0; i < ARRAYSIZE(_renderedLines); i++) {
		_renderedLines[i] = nullptr;
	}
}

ComputerGump::ComputerGump(const Std::string &msg) :
	ModalGump(0, 0, 100, 100), _curTextLine(0), _curDisplayLine(0),
	_charOff(0), _nextCharTick(0), _paused(false), _tick(0) {
	for (int i = 0; i < ARRAYSIZE(_renderedLines); i++) {
		_renderedLines[i] = nullptr;
	}

	// Split the string on ^ or flow to 20 char lines.
	debug("M '%s'", msg.c_str());
	uint32 start = 0;
	uint32 end = 0;
	for (uint32 i = 0; i < msg.size(); i++) {
		if (msg[i] == '^') {
			_textLines.push_back(msg.substr(start, end - start));
			debug("^ %d %d %d '%s'", i, start, end, _textLines.back().c_str());
			end = i + 1;
			start = i + 1;
			continue;
		}
		end++;
		if (end >= msg.size())
			break;
		if (end - start >= MAX_LINE_LEN) {
			while (end > start && msg[end] != ' ')
				end--;
			_textLines.push_back(msg.substr(start, end - start));
			debug("L %d %d %d '%s'", i, start, end, _textLines.back().c_str());
			i = end;
			end = i + 1;
			start = i + 1;
		}
	}
	if (start < msg.size())
		_textLines.push_back(msg.substr(start));
}

ComputerGump::~ComputerGump(void) {
	for (int i = 0; i < ARRAYSIZE(_renderedLines); i++) {
		if (_renderedLines[i])
			delete _renderedLines[i];
	}
}

void ComputerGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	Shape *shape = GameData::get_instance()->getGumps()->
							getShape(COMPUTER_GUMP_SHAPE);
	if (!shape) {
		error("Couldn't load shape for computer");
		return;
	}

	const ShapeFrame *topFrame = shape->getFrame(0);
	const ShapeFrame *botFrame = shape->getFrame(1);
	if (!topFrame || !botFrame) {
		error("Couldn't load shape frames for computer");
		return;
	}

	_dims.left = 0;
	_dims.top = 0;
	_dims.setWidth(topFrame->_width);
	_dims.setHeight(topFrame->_height + botFrame->_height);

	Gump *topGump = new Gump(0, 0, topFrame->_width, topFrame->_height);
	topGump->SetShape(shape, 0);
	topGump->InitGump(this, false);
	Gump *botGump = new Gump(0, topFrame->_height, botFrame->_width, botFrame->_height);
	botGump->SetShape(shape, 1);
	botGump->InitGump(this, false);

}

void ComputerGump::run() {
	ModalGump::run();

	_tick++;
	if (_paused || _tick < _nextCharTick)
		return;

	bool playsound = nextChar();

	AudioProcess *audio = AudioProcess::get_instance();
	if (playsound && audio) {
		if (audio->isSFXPlaying(COMPUTER_GUMP_SOUND))
			audio->stopSFX(COMPUTER_GUMP_SOUND, 0);
		audio->playSFX(COMPUTER_GUMP_SOUND, 0x80, 0, 1);
	}
}


bool ComputerGump::nextChar() {
	Font *computerfont = FontManager::get_instance()->getGameFont(COMPUTER_FONT, true);

	if (_charOff >= _textLines[_curTextLine].size()) {
		_curTextLine++;
		_curDisplayLine++;
		_charOff = 0;

		if (_curTextLine >= _textLines.size()) {
			_paused = true;
			return false;
		}
	}

	_nextCharTick = _tick + 2;

	Common::String display;
	if (_curDisplayLine == ARRAYSIZE(_renderedLines) - 1) {
		display = "<MORE>";
		_paused = true;
	} else {
		const Common::String &curline = _textLines[_curTextLine];
		if (_charOff < curline.size() && curline[_charOff] == '*') {
			_nextCharTick += 10;
			_charOff++;
			return false;
		}
		_charOff++;
		for (uint32 i = 0; i < _charOff && i < curline.size(); i++) {
			char next = curline[i];
			if (next == '*')
				display += ' ';
			else
				display += next;
		}
	}

	// Render the new line
	unsigned int remaining;
	RenderedText *rendered = computerfont->renderText(display, remaining);

	if (_renderedLines[_curDisplayLine] != nullptr) {
		delete _renderedLines[_curDisplayLine];
	}
	_renderedLines[_curDisplayLine] = rendered;

	return true;
}


void ComputerGump::Paint(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	ModalGump::Paint(surf, lerp_factor, scaled);
	for (int i = 0; i < ARRAYSIZE(_renderedLines); i++) {
		if (_renderedLines[i] != nullptr)
			_renderedLines[i]->draw(surf, _x + TEXT_XOFF, _y + TEXT_YOFF + i * 9);
	}
}

void ComputerGump::nextScreen() {
	_nextCharTick = 0;
	_charOff = 0;
	_paused = false;
	_curTextLine++;
	_curDisplayLine = 0;

	for (int i = 0; i < ARRAYSIZE(_renderedLines); i++) {
		if (_renderedLines[i] != nullptr) {
			delete _renderedLines[i];
			_renderedLines[i] = nullptr;
		}
	}

	if (_curTextLine >= _textLines.size())
		Close();
}

Gump *ComputerGump::onMouseDown(int button, int32 mx, int32 my) {
	if (_paused) {
		nextScreen();
	} else {
		// Not super efficient but it does the job.
		while (!_paused)
			nextChar();
	}
	return this;
}

bool ComputerGump::OnKeyDown(int key, int mod) {
	if (key == Common::KEYCODE_ESCAPE) {
		_paused = true;
		Close();
	}

	if (_paused) {
		nextScreen();
	} else {
		// Not super efficient but it does the job.
		while (!_paused)
			nextChar();
	}

	return true;
}

uint32 ComputerGump::I_readComputer(const uint8 *args, unsigned int /*argsize*/) {
	ARG_STRING(str);

	Gump *gump = new ComputerGump(str);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);

	return 0;
}

void ComputerGump::saveData(Common::WriteStream *ws) {
	warning("Trying to save ModalGump");
}

bool ComputerGump::loadData(Common::ReadStream *rs, uint32 version) {
	warning("Trying to load ModalGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
