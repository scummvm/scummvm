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

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/rect.h"

#include "graphics/surface.h"

#include "dgds/dgds.h"
#include "dgds/font.h"
#include "dgds/image.h"
#include "dgds/includes.h"
#include "dgds/request.h"
#include "dgds/resource.h"

namespace Dgds {

// TODO: The following colors are from Rise of the Dragon.  Will need to check
// if the same ones are hard-coded in the other games.
static const byte ButtonColors[] = {
	0x73, 0xF0, 0x7B, 0xDF, 0x5F, 0x5F, 0x7E, 0x27, 0x16, 0x73, 0x27, 0x16, 0xDF
};

static const byte SliderColors[] = {
	0x7B, 0x4D, 0xF4, 0x54, 0xDF, 0x74, 0x58
};

static const byte FallbackColors[] = {
	0x7, 0x7, 0x8, 0x7, 0x0, 0xF, 0x7, 0xC,
	0x4, 0x0, 0xF, 0xF, 0xC, 0x4, 0x7, 0xF,
	0x8, 0x7, 0x0, 0x7, 0x7
};

static const byte MenuBackgroundColors[] {
	0x71, 0x71, 0x71, 0x71, 0x71, 0x7B, 0x71, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B,
	0x7B, 0x7B, 0x7B, 0x7B, 0x7A, 0x7B, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A,
	0x7A, 0x7A, 0x7A, 0x46, 0x7A, 0x46, 0x46, 0x46, 0x46, 0x46, 0x46, 0x46,
	0x46, 0x46, 0x58, 0x46, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
	0x58, 0x52, 0x58, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52,
	0x59, 0x52, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x5C,
	0x59, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x0F, 0x5C,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x5C, 0x0F, 0x5C, 0x5C, 0x5C,
	0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x59, 0x5C, 0x59, 0x59, 0x59, 0x59,
	0x59, 0x59, 0x59, 0x59, 0x59, 0x52, 0x59, 0x52, 0x52, 0x52, 0x52, 0x52,
	0x52, 0x52, 0x52, 0x52, 0x58, 0x52, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
	0x58, 0x58, 0x58, 0x46, 0x58, 0x46, 0x46, 0x46, 0x46, 0x46, 0x46, 0x46,
	0x46, 0x46, 0x7A, 0x46, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A,
	0x7A, 0x7B, 0x7A, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B,
	0x71, 0x7B, 0x71, 0x71, 0x71, 0x71, 0x71, 0x71
};


RequestParser::RequestParser(ResourceManager *resman, Decompressor *decompressor) : DgdsParser(resman, decompressor) {
}


bool RequestParser::parseGADChunk(RequestData &data, DgdsChunkReader &chunk, int num) {
	Common::SeekableReadStream *str = chunk.getContent();

	uint16 numGadgets = str->readUint16LE();
	data._gadgets.resize(numGadgets);

	// Note: The original has some logic about loading single gadgets
	// here, is only ever called with "num" of -1 (load all gadgets),
	// so maybe just skip it?
	if (num != -1)
		error("Request::parseGADChunk: Implement handling of num other than -1");

	for (Common::SharedPtr<Gadget> &gptr : data._gadgets) {
		uint16 vals[12];
		for (int i = 0; i < 12; i++)
			vals[i] = str->readUint16LE();

		GadgetType gadgetType = static_cast<GadgetType>(vals[5]);
		if (num == -1 || num == vals[0]) {
			if (gadgetType == kGadgetText)
				gptr.reset(new TextAreaGadget());
			else if (gadgetType == kGadgetSlider)
				gptr.reset(new SliderGadget());
			else if (gadgetType == kGadgetImage)
				gptr.reset(new ImageGadget());
			else if (gadgetType == kGadgetButton)
				gptr.reset(new ButtonGadget());
			else
				gptr.reset(new Gadget());
		}

		if (gptr) {
			gptr->_gadgetNo = vals[0];
			gptr->_x = vals[1];
			gptr->_y = vals[2];
			gptr->_width = vals[3];
			gptr->_height = vals[4];
			gptr->_gadgetType = gadgetType;
			gptr->_flags2 = vals[6];
			gptr->_flags3 = vals[7];
			gptr->_field14_0x20 = vals[8];
			gptr->_field15_0x22 = vals[9];
			gptr->_field15_0x22 = vals[10];
			gptr->_field16_0x24 = vals[11];
			gptr->_parentX = data._x;
			gptr->_parentY = data._y;
		}

		uint16 type1 = str->readUint16LE();
		if (type1 == 1) {
			Common::String s = str->readString();
			if (gptr)
				gptr->_sval1S = s;
		} else {
			uint16 i = str->readUint16LE();
			if (gptr)
				gptr->_sval1I = i;
		}
		if (gptr)
			gptr->_sval1Type = type1;

		uint16 type2 = str->readUint16LE();
		if (type2 == 1) {
			Common::String s = str->readString();
			if (gptr)
				gptr->_sval2S = s;
		} else {
			uint16 i = str->readUint16LE();
			if (gptr)
				gptr->_sval2I = i;
		}
		if (gptr)
			gptr->_sval2Type = type2;

		uint16 val = str->readUint16LE();
		if (gptr) {
			gptr->_field20_0x28 = val;
			gptr->_field21_0x2a = val >> 0xf;
		}

		// TODO: In each of these cases, work out the true offsets to these fields.
		// and if they are shared between gadget types.
		switch (gadgetType) {
		case kGadgetText: {
			uint16 i1 = str->readUint16LE();
			uint16 i2 = str->readUint16LE();
			if (gptr) {
				TextAreaGadget *g1 = static_cast<TextAreaGadget *>(gptr.get());
				// TODO: These fields might actually be shared with other gadget types?
				g1->_gadget1_i1 = i1;
				g1->_gadget1_i2 = i2;
			}
			break;
		}
		case kGadgetSlider: {
			uint16 i1 = str->readUint16LE();
			uint16 i2 = str->readUint16LE();
			uint16 i3 = str->readUint16LE();
			uint16 i4 = str->readUint16LE();
			if (gptr) {
				SliderGadget *g2 = static_cast<SliderGadget *>(gptr.get());
				g2->_gadget2_i1 = i1;
				g2->_gadget2_i2 = i2;
				g2->_gadget2_i3 = i3;
				g2->_gadget2_i4 = i4;
			}
			break;
		}
		case kGadgetButton: {
			Common::String s = str->readString();
			if (gptr)
				gptr->_buttonName = s;
			break;
		}
		case kGadgetImage: {
			uint16 i1 = str->readUint16LE();
			uint16 i2 = str->readUint16LE();
			if (gptr) {
				ImageGadget *g8 = static_cast<ImageGadget *>(gptr.get());
				g8->_gadget8_i1 = i1;
				g8->_gadget8_i2 = i2;
			}
			break;
		}
		default:
			break;
		}
	}
	return str->err();
}


bool RequestParser::parseREQChunk(RequestData &data, DgdsChunkReader &chunk, int num) {
	Common::SeekableReadStream *str = chunk.getContent();

	uint16 chunkNum = str->readUint16LE();

	// Note: The original has some logic about loading single request blocks
	// here, is only ever called with "num" of -1 (load all),
	// so maybe just skip it?
	if (num != -1)
		error("Request::parseGADChunk: Implement handling of num other than -1");

	data._fileNum = chunkNum;
	data._x = str->readUint16LE();
	data._y = str->readUint16LE();
	data._width = str->readUint16LE();
	data._height = str->readUint16LE();
	for (int i = 0; i < 3; i++)
		data._vals[i] = str->readUint16LE();

	uint16 numTextItems = str->readUint16LE();
	data._textItemList.resize(numTextItems);
	for (int i = 0; i < numTextItems; i++) {
		TextItem &dst = data._textItemList[i];
		dst._x = str->readUint16LE();
		dst._y = str->readUint16LE();
		for (int j = 0; j < 2; j++) {
			dst._vals[j] = str->readUint16LE();
		}
		dst._txt = str->readString();
	}

	uint16 numStruct2 = str->readUint16LE();
	data._struct2List.resize(numStruct2);
	for (int i = 0; i < numStruct2; i++) {
		RequestStruct2 &dst = data._struct2List[i];
		for (int j = 0; j < 6; j++) {
			dst._vals[j] = str->readUint16LE();
		}
	}

	return str->err();
}


bool RequestParser::handleChunk(DgdsChunkReader &chunk, ParserData *data) {
	REQFileData &rfdata = *static_cast<REQFileData *>(data);

	// The game supports loading a particular item, but always passes -1?
	int num = -1;

	if (chunk.isContainer()) {
		// TAG: contains tags for the request data, all content
		if (chunk.getId() == ID_TAG)
			chunk.skipContent();

		return false; // continue parsing
	}

	if (chunk.getId() == ID_REQ) {
		rfdata._requests.resize(rfdata._requests.size() + 1);
		parseREQChunk(rfdata._requests.back(), chunk, num);
	} else if (chunk.getId() == ID_GAD) {
		if (rfdata._requests.empty())
			error("GAD chunk before any REQ chunks in Reqeust file %s", _filename.c_str());
		parseGADChunk(rfdata._requests.back(), chunk, num);
	}

	return chunk.getContent()->err();
}


Common::String Gadget::dump() const {
	char buf1[6], buf2[6];
	const char *sval1 = _sval1S.c_str();
	const char *sval2 = _sval2S.c_str();
	if (_sval1Type != 1) {
		sval1 = buf1;
		snprintf(buf1, 6, "%d", _sval1I);
	}
	if (_sval2Type != 1) {
		sval2 = buf2;
		snprintf(buf2, 6, "%d", _sval2I);
	}

	return Common::String::format(
		"%s<num %d pos (%d,%d) sz (%d,%d), typ %d, flgs %04x %04x svals %s, %s, '%s', parent (%d,%d)>",
		_gadgetType == kGadgetButton ? "ButtonGadget" : "Gadget",
		_gadgetNo, _x, _y, _width, _height, _gadgetType, _flags2, _flags3, sval1, sval2,
		_buttonName.c_str(), _parentX, _parentY);
}

void Gadget::draw(Graphics::Surface *dst) const {}

void ButtonGadget::draw(Graphics::Surface *dst) const {
	// TODO: Bounds calculation here might depend on parent.

	int16 x = _x + _parentX;
	int16 y = _y + _parentY;

	int16 right = x + _width;
	int16 x2 = right - 1;
	int16 bottom = (y + _height) - 1;

	byte fill = ButtonColors[0];
	dst->drawLine(x, y, x2, y, fill);
	dst->drawLine(x + 2, y + 2, right - 3, y + 2, fill);
	dst->drawLine(x + 1, bottom - 2, x + 1, bottom - 2, fill);
	dst->drawLine(right - 2, bottom - 2, right - 2, bottom - 2, fill);
	dst->drawLine(x + 1, bottom - 1, right - 2, bottom - 1, fill);

	fill = ButtonColors[1];
	dst->drawLine(x, y + 1, x, bottom, fill);
	dst->drawLine(x2, y + 1, x2, bottom, fill);
	dst->drawLine(x + 2, y + 3, x + 2, bottom - 2, fill);
	dst->drawLine(right - 3, y + 3, right - 3, bottom - 2, fill);
	dst->drawLine(x + 3,bottom - 2, right - 4, bottom - 2, fill);

	fill = ButtonColors[2];
	dst->drawLine(x + 1, y + 2, x + 1, bottom - 3, fill);
	dst->drawLine(right - 2, y + 2, right - 2, bottom - 3, fill);
	dst->drawLine(x + 1, bottom, right - 2, bottom, ButtonColors[3]);
	dst->drawLine(x + 1, y + 1, right - 2, y + 1, ButtonColors[4]);

	bool enabled = !(_flags3 & 9);
	int colOffset;
	if (!enabled) {
		colOffset = 9;
	}  else {
		colOffset = 5;
	}

	dst->drawLine(x + 3, y + 3, right - 4, y + 3, ButtonColors[colOffset + 1]);

	// TODO: This is done with a different call in the game.. is there some reason for that?
	dst->fillRect(Common::Rect(x + 3, y + 4, x + 3 + _width - 6, y + 4 + _height - 8), ButtonColors[colOffset + 2]);

	dst->drawLine(x + 3, bottom - 3, right - 4, bottom - 3, ButtonColors[colOffset + 3]);

	if (!_buttonName.empty()) {
		const Font *font = RequestData::getMenuFont();

		// TODO: Depending on some flags, the game toggles " ON " to " OFF" at the
		// end of the string.
		Common::String name = _buttonName;

		int fontHeight = font->getFontHeight();

		bool twoline;
		int yoffset;
		uint32 linebreak = name.find('&');

		Common::String line1, line2;
		if (linebreak != Common::String::npos) {
			twoline = true;
			name.setChar(' ', linebreak);
			yoffset = _height + 1 - fontHeight * 2;
			line1 = _buttonName.substr(0, linebreak);
			line2 = _buttonName.substr(linebreak + 1);
		} else {
			twoline = false;
			yoffset = _height - fontHeight;
			line1 = _buttonName;
		}

		yoffset = y + yoffset / 2;
		int lineWidth = font->getStringWidth(line1);
		font->drawString(dst, line1, x + (_width - lineWidth) / 2 + 1, yoffset + 2, lineWidth, ButtonColors[colOffset]);

		if (linebreak != Common::String::npos) {
			lineWidth = font->getStringWidth(line2);
			font->drawString(dst, line2, x + (_width - lineWidth) / 2 + 1, yoffset + fontHeight, lineWidth, ButtonColors[colOffset]);
		}
	}

}

void ButtonGadget::toggle(bool enable) {
	if (!enable)
		_flags3 |= 9; // 0x1001
	else
		_flags3 &= 6; // 0x0110
}

Common::String TextAreaGadget::dump() const {
	const Common::String base = Gadget::dump();
	return Common::String::format("TextArea<%s, %d %d>", base.c_str(), _gadget1_i1, _gadget1_i2);
}

void TextAreaGadget::draw(Graphics::Surface *dst) const {
	const Font *font = RequestData::getMenuFont();
	font->drawString(dst, _buttonName, _x + _parentX, _y + _parentY, 0, 0);
}

Common::String SliderGadget::dump() const {
	const Common::String base = Gadget::dump();
	return Common::String::format("Slider<%s, %d %d %d %d>", base.c_str(), _gadget2_i1, _gadget2_i2, _gadget2_i3, _gadget2_i4);
}

// Slider labels and title are hard-coded in game, not part of data files.
static const char *_sliderTitleForGadget(uint16 num) {
	switch (num) {
	case 0x7B:	return "DIFFICULTY";
	case 0x7D:	return "TEXT SPEED";
	case 0x83:	return "DETAIL LEVEL";
	case 0x98:	return "MOUSE SPEED";
	case 0x9C:	return "BUTTON THRESHOLD";
	default:	return "SLIDER";
	}
}

static const char *_sliderLabelsForGadget(uint16 num) {
	switch (num) {
	case 0x7B:	return "EASY         HARD";
	case 0x7D:	return "SLOW         FAST";
	case 0x83:	return "LOW        HIGH";
	case 0x98:	return "SLOW         FAST";
	case 0x9C:	return "LONG         SHORT";
	default:	return "MIN         MAX";
	}
}

void SliderGadget::draw(Graphics::Surface *dst) const {
	const Font *font = RequestData::getMenuFont();

	int16 x = _x + _parentX;
	int16 y = _y + _parentY;
	int16 x2 = x + _width;
	int16 y2 = (y + _height) - 1;

	int16 titley = (y - font->getFontHeight()) + 1;
	const char *title = _sliderTitleForGadget(_gadgetNo);
	const char *labels = _sliderLabelsForGadget(_gadgetNo);
	int16 titleWidth = font->getStringWidth(title);

	font->drawString(dst, title, x + (_width - titleWidth) / 2, titley, titleWidth, 0);
	int16 labelWidth = font->getStringWidth(labels);
	font->drawString(dst, labels, x + (_width - labelWidth) / 2, y + 7, labelWidth, 0);
	int16 y1 = y - 1;
	dst->drawLine(x - 2, y - 1, x - 2, y2, SliderColors[0]);
	dst->drawLine(x - 1, y1, x - 1, y2, SliderColors[1]);
	dst->drawLine(x, y2, x2 - 1, y2, SliderColors[1]);
	dst->drawLine(x, y1, x2, y1, SliderColors[2]);
	dst->drawLine(x2, y1, x2, y2, SliderColors[2]);
	dst->drawLine(x2 + 1, y1, x2 + 1, y2, SliderColors[3]);
	dst->drawLine(x, y, x2 - 1, y, SliderColors[4]);
	dst->drawLine(x2 - 1, y + 1, x2 - 1, (y + _height) - 2, SliderColors[4]);
	// This is not exactly what happens in the original, but gets the same result
	Common::Rect fillrect = Common::Rect(x, y + 1, x + _width - 1, y + _height - 1);
	dst->fillRect(fillrect, SliderColors[5]);
	fillrect.grow(-1);
	dst->fillRect(fillrect, SliderColors[6]);
}

Common::String ImageGadget::dump() const {
	const Common::String base = Gadget::dump();
	return Common::String::format("Image<%s, %d %d>", base.c_str(), _gadget8_i1, _gadget8_i2);
}

void ImageGadget::draw(Graphics::Surface *dst) const {
	error("TODO: Implement ImageGadget::draw");
}

Common::String RequestData::dump() const {
	Common::String ret = Common::String::format("RequestData<file %d pos (%d,%d) size (%d, %d) %d %d %d\n",
								_fileNum, _x, _y, _width, _height, _vals[0], _vals[1], _vals[2]);
	for (const auto &s1 : _textItemList)
		ret += Common::String::format("    TextItem<'%s' pos (%d,%d) %d %d>\n", s1._txt.c_str(),
								s1._x, s1._y, s1._vals[0], s1._vals[1]);
	for (const auto &s2 : _struct2List)
		ret += Common::String::format("    RequestStruct2<%d %d %d %d %d %d>\n", s2._vals[0], s2._vals[1],
								s2._vals[2], s2._vals[3], s2._vals[4], s2._vals[5]);
	for (const auto &g : _gadgets)
		ret += Common::String::format("    %s\n", g->dump().c_str());
	ret += ">";

	return ret;
}

void RequestData::draw(Graphics::Surface *dst) const {
	int slidery = 0;
	for (const auto &gadget : _gadgets) {
		const SliderGadget *slider = dynamic_cast<const SliderGadget *>(gadget.get());
		if (slider) {
			slidery = MAX(slidery, slider->_y + slider->_height);
		}
	}

	Common::String header;
	if (!_textItemList.empty())
		header = _textItemList[0]._txt.substr(1);

	if (slidery)
		drawBackgroundWithSliderArea(dst, slidery, header);
	else
		drawBackgroundNoSliders(dst, header);
}

/*static*/
const Font *RequestData::getMenuFont() {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	return engine->getFontMan()->getFont(FontManager::kGameFont);
}

/*static*/
const Image *RequestData::getCorner(int cornerNum) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	return engine->getUICorners()[cornerNum].get();
}

/*static*/
void RequestData::drawCorners(Graphics::Surface *dst, uint16 startNum, uint16 x, uint16 y, uint16 width, uint16 height) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const Common::Array<Common::SharedPtr<Image>> &allCorners = engine->getUICorners();

	assert(allCorners.size() > startNum + 7);

	const Common::SharedPtr<Image> *corners = allCorners.data() + startNum;
	const Common::Rect screenRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	for (int xoff = x + corners[0]->width(); xoff < (x + width) - corners[2]->width(); xoff += corners[1]->width())
		corners[1]->drawBitmap(xoff, y, screenRect, *dst);

	for (int xoff = x + corners[6]->width(); xoff < (x + width) - corners[7]->width(); xoff += corners[6]->width())
		corners[6]->drawBitmap(xoff, (y + height) - corners[6]->height(), screenRect, *dst);

	for (int yoff = y + corners[0]->height(); yoff < (y + height) - corners[5]->height(); yoff += corners[3]->height()) {
		corners[3]->drawBitmap(x, yoff, screenRect, *dst);
	}

	for (int yoff = y + corners[2]->height(); yoff < (y + height) - corners[7]->height(); yoff += corners[4]->height()) {
		corners[4]->drawBitmap((x + width) - corners[4]->width(), yoff, screenRect, *dst);
	}

	corners[0]->drawBitmap(x, y, screenRect, *dst);
	corners[2]->drawBitmap((x + width) - corners[2]->width(), y, screenRect, *dst);
	corners[5]->drawBitmap(x, (y + height) - corners[5]->height(), screenRect, *dst);
	corners[7]->drawBitmap((x + width) - corners[7]->width(), (y + height) - corners[7]->height(), screenRect, *dst);
}

/*static*/
void RequestData::drawHeader(Graphics::Surface *dst, int16 x, int16 y, int16 width, int16 yoffset, const Common::String &header) {
	if (!header.empty()) {
		const Font *font = getMenuFont();
		int hwidth = font->getStringWidth(header);
		int hheight = font->getFontHeight();
		int hleft = x + (width - hwidth) / 2;
		int hright = hleft + hwidth + 3;
		int htop = y + yoffset;
		int hbottom = htop + hheight;

		font->drawString(dst, header, hleft + 1, htop + 2, hwidth, 0);
		dst->drawLine(hleft - 3, htop, hright, htop, 0);
		dst->drawLine(hright, htop + 1, hright, hbottom, 0);
		dst->drawLine(hleft - 3, htop + 1, hleft - 3, hbottom, 15);
		dst->drawLine(hleft - 2, hbottom, hleft + hwidth, hbottom, 15);
	}
}

void RequestData::drawBackgroundWithSliderArea(Graphics::Surface *dst, int16 sliderHeight, const Common::String &header) const {
	uint16 sliderBgHeight = sliderHeight + 18;
	fillBackground(dst, _x, _y, _width, sliderBgHeight, 0);
	fillBackground(dst, _x + 8, _y + sliderBgHeight, _width - 16, _height - sliderBgHeight, 8 - sliderBgHeight);
	fillBackground(dst, _x + 9, _y + 8, _width - 18, sliderHeight + 2, 8);
	fillBackground(dst, _x + 17, _y + 8 + sliderHeight + 2, _width - 34, _height - sliderBgHeight, 32 - sliderBgHeight);

	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const Common::Array<Common::SharedPtr<Image>> &corners = engine->getUICorners();
	const Common::Rect screenRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	for (int xoff = _x + corners[0]->width(); xoff < (_x + _width) - corners[3]->width(); xoff += corners[2]->width()) {
		corners[2]->drawBitmap(xoff, _y, screenRect, *dst);
	}
	for (int xoff = _x + 8 + corners[6]->width(); xoff < (_x + 8 + _width - 16) - corners[8]->width(); xoff += corners[7]->width()) {
		corners[7]->drawBitmap(xoff, (_y + _height) - corners[7]->height(), screenRect, *dst);
	}
	for (int yoff = _y + corners[3]->height(); yoff < (_y + sliderBgHeight) - corners[10]->height(); yoff += corners[5]->height()) {
		corners[5]->drawBitmap((_x + _width) - corners[5]->width(), yoff, screenRect, *dst);
	}
	for (int yoff = _y + corners[1]->height(); yoff < (_y + sliderBgHeight) - corners[9]->height(); yoff += corners[4]->height()) {
		corners[4]->drawBitmap(_x, yoff, screenRect, *dst);
	}
	for (int yoff = _y + sliderBgHeight; yoff < (_y + _height) - corners[6]->height(); yoff += corners[4]->height()) {
		corners[4]->drawBitmap(_x + 8, yoff, screenRect, *dst);
	}
	for (int yoff = _y + sliderBgHeight; yoff < (_y + _height) - corners[8]->height(); yoff += corners[5]->height()) {
		corners[5]->drawBitmap((_x + 8 + _width - 16) - corners[5]->width(), yoff, screenRect, *dst);
	}
	corners[1]->drawBitmap(_x, _y, screenRect, *dst);
	corners[3]->drawBitmap((_x + _width) - corners[3]->width(), _y, screenRect, *dst);
	corners[6]->drawBitmap(_x + 8, (_y + _height) - corners[6]->height(), screenRect, *dst);
	corners[8]->drawBitmap((_x + _width - 8) - corners[8]->width(), (_y + _height) - corners[8]->height(), screenRect, *dst);
	corners[9]->drawBitmap(_x, (_y + sliderBgHeight) - corners[9]->height(), screenRect, *dst);
	corners[10]->drawBitmap((_x + _width) - corners[10]->width(), (_y + sliderBgHeight) - corners[10]->height(), screenRect, *dst);

	drawHeader(dst, _x, _y, _width, 9, header);
}


void RequestData::drawBackgroundNoSliders(Graphics::Surface *dst, const Common::String &header) const {
	fillBackground(dst, _x, _y, _width, _height, 0);
	drawCorners(dst, 11, _x, _y, _width, _height);
	drawHeader(dst, _x, _y, _width, 4, header);
}

/*static*/
void RequestData::fillBackground(Graphics::Surface *dst, uint16 x, uint16 y, uint16 width, uint16 height, int16 startoffset) {
	bool detailHigh = true;
	if (detailHigh) {
		Graphics::Surface area = dst->getSubArea(Common::Rect(x, y, x + width, y + height));
		while (startoffset < 0)
			startoffset += ARRAYSIZE(MenuBackgroundColors);
		startoffset = startoffset % ARRAYSIZE(MenuBackgroundColors);

		int coloffset = startoffset;
		for (int xoff = 0; xoff < width; xoff++) {
			area.drawLine(xoff, 0, xoff + height, height, MenuBackgroundColors[coloffset]);
			coloffset++;
			if (coloffset >= ARRAYSIZE(MenuBackgroundColors))
				coloffset = 0;
		}
		// TODO: Game positions mouse in middle of menu here?
		coloffset = startoffset;
		for (int yoff = 0; yoff < height; yoff++) {
			area.drawLine(0, yoff, height, yoff + height, MenuBackgroundColors[coloffset]);
			coloffset--;
			if (coloffset < 0)
				coloffset = ARRAYSIZE(MenuBackgroundColors) - 1;
		}
	} else {
		dst->fillRect(Common::Rect(x, y, width, height), FallbackColors[0]);
	}
}


Common::String REQFileData::dump() const {
	Common::String ret("REQFileData<\n");
	for (const auto &req : _requests) {
		ret += req.dump().c_str();
		ret += "\n";
	}
	ret += ">";

	return ret;
}

} // End of namespace Dgds
