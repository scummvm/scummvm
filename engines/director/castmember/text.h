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

#ifndef DIRECTOR_CASTMEMBER_TEXT_H
#define DIRECTOR_CASTMEMBER_TEXT_H

#include "director/castmember/castmember.h"

namespace Director {

class TextCastMember : public CastMember {
public:
	TextCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version, uint8 flags1 = 0, bool asButton = false);
	TextCastMember(Cast *cast, uint16 castId, TextCastMember &source);
	void setColors(uint32 *fgcolor, uint32 *bgcolor) override;

	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	bool isEditable() override { return _editable; }
	void setEditable(bool editable) override { _editable = editable; }
	void updateFromWidget(Graphics::MacWidget *widget) override;
	Graphics::TextAlign getAlignment();

	uint32 getBackColor() override { return _bgcolor; }
	void setBackColor(uint32 bgCol) override;
	uint32 getForeColor() override { return _fgcolor; }
	void setForeColor(uint32 fgCol) override;

	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	bool hasChunkField(int field);
	Datum getChunkField(int field, int start, int end);
	bool setChunkField(int field, int start, int end, const Datum &value);

	int getTextHeight();

	int getTextSize();
	void setTextSize(int textSize);

	Common::String formatInfo() override;

	void load() override;
	void unload() override;

	SizeType _borderSize;
	SizeType _gutterSize;
	SizeType _boxShadow;
	uint16 _maxHeight;
	uint16 _textHeight;

	uint32 _fontId;
	uint16 _fontSize;
	TextType _textType;
	TextAlignType _textAlign;
	SizeType _textShadow;
	uint16 _scroll;
	byte _textSlant;
	byte _textFlags;
	uint16 _bgpalinfo1, _bgpalinfo2, _bgpalinfo3;
	uint16 _fgpalinfo1, _fgpalinfo2, _fgpalinfo3;
	ButtonType _buttonType;
	bool _editable;
	int _lineSpacing;

	Common::U32String _ftext;
	Common::U32String _ptext;
	Common::String _rtext;
	void importStxt(const Stxt *stxt);
	void importRTE(byte *text);

	Common::U32String getText();
	Common::String getRawText();
	void setRawText(const Common::String &text);

private:
	Graphics::MacWidget *createWindowOrWidget(Common::Rect &bbox, Channel *channel, Common::Rect dims, Graphics::MacFont *macFont);

	uint32 _bgcolor;
	uint32 _fgcolor;
};

class RTECastMember : public TextCastMember {
public:
	RTECastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	RTECastMember(Cast *cast, uint16 castId, RTECastMember &source);

	void loadChunks();
	void load() override;
};

} // End of namespace Director

#endif
