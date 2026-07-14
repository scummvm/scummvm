/* ScummVM - Graphic Adventure Engine */

#include "ripper/dialogue.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/detection.h"
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
	const int y = 348;
	for (uint i = 0; i < _choices.size(); ++i) {
		const int row = y + i * 18;
		for (int yy = row; yy < row + 16 && yy < screen->h; ++yy)
			memset(screen->getBasePtr(160, yy), i == 0 ? 4 : 0, 320);
		int x = 166;
		for (uint c = 0; c < _choices[i].text.size() && x < 475; ++c) {
			const byte ch = (byte)_choices[i].text[c];
			if (ch < _font.firstCharacter || ch >= _font.firstCharacter + _font.glyphs.size())
				continue;
			const BitmapFontGlyph &glyph = _font.glyphs[ch - _font.firstCharacter];
			for (int gy = 0; gy < glyph.height; ++gy) {
				for (int gx = 0; gx < glyph.width; ++gx) {
					if (_font.pixels[glyph.pixelOffset + gy * glyph.width + gx])
						*(byte *)screen->getBasePtr(x + gx, row + gy + 2) = 4;
				}
			}
			x += glyph.width + _font.characterSpacing;
		}
	}
	g_system->unlockScreen();
}

bool DialogueManager::execute(const CompiledScript &script, const ScriptCommand &command) {
	if (command.opcode == 0x16) {
		if (command.arguments.size() < 2 || command.arguments[0].data.empty())
			return false;
		Choice choice;
		for (uint i = 0; i < command.arguments[0].data.size() &&
				command.arguments[0].data[i] != 0; ++i)
			choice.text += (char)command.arguments[0].data[i];
		choice.result = command.arguments[1].value & 0xffff;
		_choices.push_back(choice);
		debugC(1, kDebugScripts,
			"Ripper: dialogue choice appended index=%u result=%u text='%s'",
			_choices.size() - 1, choice.result, choice.text.c_str());
		return true;
	}

	if (command.opcode == 0x0a) {
		_pending = true;
		debugC(1, kDebugScripts,
			"Ripper: dialogue choice list completed script='%s' offset=0x%x "
				"selector=%u choices=%u",
			script.getMemberName().c_str(), command.offset, command.selector, _choices.size());
		for (uint i = 0; i < _choices.size(); ++i)
			debugC(2, kDebugScripts, "Ripper: dialogue choice index=%u result=%u text='%s'",
				i, _choices[i].result, _choices[i].text.c_str());
		return true;
	}

	if (command.opcode == 0x0b) {
		debugC(1, kDebugScripts,
			"Ripper: dialogue selection action script='%s' offset=0x%x selector=%u "
				"arguments=%u choices=%u",
			script.getMemberName().c_str(), command.offset, command.selector,
			command.arguments.size(), _choices.size());
		for (uint i = 0; i < command.arguments.size(); ++i)
			debugC(2, kDebugScripts,
				"Ripper: dialogue selection action argument=%u type=%u value=0x%x bytes=%u",
				i, command.arguments[i].type, command.arguments[i].value,
				command.arguments[i].data.size());
		return true;
	}

	debugC(1, kDebugScripts,
		"Ripper: dialogue command script='%s' offset=0x%x selector=%u arguments=%u",
		script.getMemberName().c_str(), command.offset, command.selector,
		command.arguments.size());
	for (uint i = 0; i < command.arguments.size(); ++i) {
		const ScriptArgument &argument = command.arguments[i];
		Common::String text;
		for (uint j = 0; j < argument.data.size() && argument.data[j] != 0; ++j)
			text += (char)argument.data[j];
		debugC(2, kDebugScripts,
			"Ripper: dialogue argument=%u type=%u value=0x%x bytes=%u text='%s'",
			i, argument.type, argument.value, argument.data.size(), text.c_str());
	}
	// The first slice records the decoded payload. Rendering and modal input
	// will be added once the selector-specific Ghidra handlers are mapped.
	return true;
}

} // End of namespace Ripper
