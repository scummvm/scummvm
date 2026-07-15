/* ScummVM - Graphic Adventure Engine */

#include "ripper/dialogue.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/script.h"

namespace Ripper {

bool DialogueManager::initialize(ResourceManager &resources) {
	return resources.loadInterfaceBitmapFont("7pt_font.fnt", _font);
}

void DialogueManager::draw() const {
	if (!_pending || _choices.empty())
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	const int y = 344;
	const int rowHeight = 13;
	for (uint i = 0; i < _choices.size(); ++i) {
		const int row = y + i * rowHeight;
		if (row >= screen->h)
			break;
		const bool selected = i == _selectedChoice;
		for (int yy = row; yy < row + rowHeight && yy < screen->h; ++yy)
			memset(screen->getBasePtr(150, yy), selected ? 253 : 0, 360);
		int x = 166;
		for (uint c = 0; c < _choices[i].text.size() && x < 475; ++c) {
			const byte ch = (byte)_choices[i].text[c];
			if (ch < _font.firstCharacter || ch >= _font.firstCharacter + _font.glyphs.size())
				continue;
			const BitmapFontGlyph &glyph = _font.glyphs[ch - _font.firstCharacter];
			for (int gy = 0; gy < glyph.height; ++gy) {
				for (int gx = 0; gx < glyph.width; ++gx) {
					if (_font.pixels[glyph.pixelOffset + gy * glyph.width + gx])
						*(byte *)screen->getBasePtr(x + gx, row + gy + 2) = selected ? 0 : 4;
				}
			}
			x += glyph.width + _font.characterSpacing;
		}
	}
	g_system->unlockScreen();
}

bool DialogueManager::execute(const CompiledScript &script, const ScriptCommand &command,
		bool includeChoice) {
	if (command.opcode == 0x15 || command.opcode == 0x16) {
		if (command.arguments.size() < 2 || command.arguments[0].data.empty())
			return false;
		Choice choice;
		for (uint i = 0; i < command.arguments[0].data.size() &&
				command.arguments[0].data[i] != 0; ++i)
			choice.text += (char)command.arguments[0].data[i];
		choice.result = command.arguments[1].value & 0xffff;
		if (includeChoice) {
			_choices.push_back(choice);
			debugC(1, kDebugDialogue,
				"Ripper: dialogue choice appended index=%u result=%u text='%s'",
				_choices.size() - 1, choice.result, choice.text.c_str());
		} else {
			debugC(1, kDebugDialogue,
				"Ripper: dialogue choice omitted result=%u reason=response-played text='%s'",
				choice.result, choice.text.c_str());
		}
		return true;
	}

	if (command.opcode == 0x17) {
		_pending = true;
		_selectedChoice = 0;
		debugC(1, kDebugDialogue,
			"Ripper: dialogue chooser activated script='%s' offset=0x%x "
				"selector=%u choices=%u",
			script.getMemberName().c_str(), command.offset, command.selector, _choices.size());
		for (uint i = 0; i < _choices.size(); ++i)
			debugC(2, kDebugDialogue, "Ripper: dialogue choice index=%u result=%u text='%s'",
				i, _choices[i].result, _choices[i].text.c_str());
		return true;
	}

	debugC(1, kDebugDialogue,
		"Ripper: dialogue command script='%s' offset=0x%x selector=%u arguments=%u",
		script.getMemberName().c_str(), command.offset, command.selector,
		command.arguments.size());
	for (uint i = 0; i < command.arguments.size(); ++i) {
		const ScriptArgument &argument = command.arguments[i];
		Common::String text;
		for (uint j = 0; j < argument.data.size() && argument.data[j] != 0; ++j)
			text += (char)argument.data[j];
		debugC(2, kDebugDialogue,
			"Ripper: dialogue argument=%u type=%u value=0x%x bytes=%u text='%s'",
			i, argument.type, argument.value, argument.data.size(), text.c_str());
	}
	return true;
}

bool DialogueManager::service(const MouseState &mouse, uint &result) {
	if (!_pending || _choices.empty())
		return false;
	updateHover(mouse.position);
	const int rowHeight = 13;
	const int row = (mouse.position.y - 344) / rowHeight;
	if ((mouse.pressed & kMouseButtonLeft) == 0 || mouse.position.x < 150 ||
		mouse.position.x >= 510 || mouse.position.y < 344 || row < 0 ||
		(uint)row >= _choices.size())
		return false;
	result = _choices[_selectedChoice].result;
	debugC(1, kDebugDialogue,
		"Ripper: dialogue selected index=%u resultFrame=%u text='%s'",
		_selectedChoice, result, _choices[_selectedChoice].text.c_str());
	_pending = false;
	_choices.clear();
	return true;
}

void DialogueManager::updateHover(const Common::Point &point) {
	if (!_pending || _choices.empty() || point.x < 150 || point.x >= 510 || point.y < 344)
		return;
	const int row = (point.y - 344) / 13;
	if (row < 0 || (uint)row >= _choices.size() || row == (int)_selectedChoice)
		return;
	_selectedChoice = row;
	debugC(2, kDebugDialogue,
		"Ripper: dialogue hover index=%u result=%u text='%s' point=%d,%d",
		_selectedChoice, _choices[_selectedChoice].result,
		_choices[_selectedChoice].text.c_str(), point.x, point.y);
}

} // End of namespace Ripper
