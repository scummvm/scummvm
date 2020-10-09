#include "agds/textLayout.h"
#include "agds/font.h"
#include "agds/agds.h"
#include "agds/object.h"
#include "common/debug.h"

namespace AGDS {

void TextLayout::paint(AGDSEngine &engine, Graphics::Surface &backbuffer) {
	if (!_valid)
		return;
	Font *font = engine.getFont(_fontId);
	for(uint i = 0; i < _lines.size(); ++i) {
		Line & line = _lines[i];
		font->drawString(&backbuffer, line.text, line.pos.x, line.pos.y, line.size.x, 0);
	}
}

void TextLayout::reset(AGDSEngine &engine) {
	bool valid = _valid;
	_valid = false;
	_lines.clear();

	if (valid) {
		Common::String &var = _npc? _npcNotifyVar: _charNotifyVar;
		if (!var.empty()) {
			engine.setGlobal(var, 0);
		}
		engine.reactivate(_process);
	}
}

void TextLayout::layout(AGDSEngine &engine, const Common::String &process, const Common::String &text, Common::Point pos, int fontId, bool npc) {
	if (text.empty()) {
		_valid = false;
		return;
	}

	_process = process;
	_fontId = fontId;
	_npc = npc;
	Font *font = engine.getFont(fontId);

	_lines.clear();
	int w = 0;

	Common::Point basePos;
	size_t begin = 0;
	while(begin < text.size()) {
		while(begin < text.size() && text[begin]== '\r')
			++begin;
		size_t end = text.find('\n', begin);
		if (end == text.npos)
			end = text.size();

		Common::String line = text.substr(begin, end - begin);
		debug("parsed line: %s", line.c_str());
		begin = end + 1;
		Common::Point size;
		size.x = font->getStringWidth(line);
		size.y = font->getFontHeight();
		_lines.push_back(Line());

		Line & l = _lines.back();
		l.pos = basePos;
		l.text = line;
		l.size = size;

		basePos.y += size.y;
		if (size.x > w)
			w = size.x;
	}

	int dy = -basePos.y / 2;
	for(uint i = 0; i < _lines.size(); ++i) {
		Line & line = _lines[i];
		line.pos.x += pos.x - line.size.x / 2;
		line.pos.y += pos.y + dy;
	}

	_valid = true;

	Common::String &var = _npc? _npcNotifyVar: _charNotifyVar;
	if (!var.empty()) {
		if (!engine.getGlobal(var))
			engine.setGlobal(var, 1);
	}
}

}
