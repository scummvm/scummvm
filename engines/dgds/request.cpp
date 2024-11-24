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

static const byte DragonButtonColors[] = {
	0x73, 0xF0, 0x7B, 0xDF, 0x5F, 0x5F, 0x7E, 0x27, 0x16, 0x73, 0x27, 0x16, 0xDF
};

static const byte DragonSliderColors[] = {
	0x7B, 0x4D, 0xF4, 0x54, 0xDF, 0x74, 0x58
};

static const byte DragonHeaderTxtColor = 0;
static const byte DragonHeaderTopColor = 0;
static const byte DragonHeaderBottomColor = 15;
static const byte DragonFallbackColors[] = {
	0x7, 0x7, 0x8, 0x7, 0x0, 0xF, 0x7, 0xC,
	0x4, 0x0, 0xF, 0xF, 0xC, 0x4, 0x7, 0xF,
	0x8, 0x7, 0x0, 0x7, 0x7
};

static const byte ChinaBackgroundColor = 23;
static const byte ChinaHeaderTxtColor = 25;
static const byte ChinaHeaderTopColor = 16;
static const byte ChinaHeaderBottomColor = 20;
static const byte ChinaButtonColorsOn[] = {
	0x10, 0x11, 0x10, 0x10, 0x10, 0x06, 0x14, 0x1A,
};
static const byte ChinaButtonColorsOff[] = {
	0x10, 0x14, 0x06, 0x18, 0x10, 0x11, 0x14, 0x13,
};

static const byte ChinaSliderColors[] = {
    0x1A, 0x15, 0x10, 0x06, 0x18
};

static const byte WillyBackgroundColor = 16;
static const byte WillyButtonColor = 20;

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
			gptr->_fontNo = vals[8];
			gptr->_col1 = vals[9];
			gptr->_col2 = vals[10];
			gptr->_col3 = vals[11];
			gptr->_parentX = data._rect.x;
			gptr->_parentY = data._rect.y;
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

		switch (gadgetType) {
		case kGadgetText: {
			uint16 i1 = str->readUint16LE();
			uint16 i2 = str->readUint16LE();
			if (gptr) {
				TextAreaGadget *g1 = static_cast<TextAreaGadget *>(gptr.get());
				g1->_textGadget_i1 = i1;
				g1->_bufLen = i2;
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
				g8->_xStep = i1;
				g8->_yStep = i2;
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

	// Slight HACK - only Willy Beamish has a different number for the main menu
	if (chunkNum == kMenuMainBeamish)
		chunkNum = kMenuMain;

	// Note: The original has some logic about loading single request blocks
	// here, is only ever called with "num" of -1 (load all),
	// so maybe just skip it?
	if (num != -1)
		error("Request::parseGADChunk: Implement handling of num other than -1");

	data._fileNum = chunkNum;
	data._rect.x = str->readUint16LE();
	data._rect.y = str->readUint16LE();
	data._rect.width = str->readUint16LE();
	data._rect.height = str->readUint16LE();
	data._col1 = str->readUint16LE();
	data._col2 = str->readUint16LE();
	data._flags = str->readUint16LE();

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

	uint16 numFillAreas = str->readUint16LE();
	data._fillAreaList.resize(numFillAreas);
	for (int i = 0; i < numFillAreas; i++) {
		RequestFillArea &dst = data._fillAreaList[i];
		dst._x = str->readUint16LE();
		dst._y = str->readUint16LE();
		dst._width = str->readUint16LE();
		dst._height = str->readUint16LE();
		dst._col1 = str->readUint16LE();
		dst._col2 = str->readUint16LE();
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
			error("GAD chunk before any REQ chunks in Request file %s", _filename.c_str());
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

void Gadget::draw(Graphics::ManagedSurface *dst) const {}

Common::Point Gadget::topLeft() const {
	return Common::Point(_x + _parentX, _y + _parentY);
}

Common::Point Gadget::midPoint() const {
	return topLeft() + Common::Point(_width / 2, _height / 2);
}

bool Gadget::containsPoint(const Common::Point &pt) {
	Common::Point tl = topLeft();
	Common::Rect gadgetRect(tl, _width, _height - 1);
	return gadgetRect.contains(pt);
}

void Gadget::setVisible(bool visible) {
	if (visible)
		_flags3 &= ~0x40;
	else
		_flags3 |= 0x40;
}

byte ButtonGadget::drawDragonBg(Graphics::ManagedSurface *dst, bool enabled) const {
	Common::Point pt = topLeft();
	int16 x = pt.x;
	int16 y = pt.y;

	int16 right = x + _width;
	int16 x2 = right - 1;
	int16 bottom = (y + _height) - 1;

	byte fill = DragonButtonColors[0];
	dst->drawLine(x, y, x2, y, fill);
	dst->drawLine(x + 2, y + 2, right - 3, y + 2, fill);
	dst->drawLine(x + 1, bottom - 2, x + 1, bottom - 2, fill);
	dst->drawLine(right - 2, bottom - 2, right - 2, bottom - 2, fill);
	dst->drawLine(x + 1, bottom - 1, right - 2, bottom - 1, fill);

	fill = DragonButtonColors[1];
	dst->drawLine(x, y + 1, x, bottom, fill);
	dst->drawLine(x2, y + 1, x2, bottom, fill);
	dst->drawLine(x + 2, y + 3, x + 2, bottom - 2, fill);
	dst->drawLine(right - 3, y + 3, right - 3, bottom - 2, fill);
	dst->drawLine(x + 3,bottom - 2, right - 4, bottom - 2, fill);

	fill = DragonButtonColors[2];
	dst->drawLine(x + 1, y + 2, x + 1, bottom - 3, fill);
	dst->drawLine(right - 2, y + 2, right - 2, bottom - 3, fill);
	dst->drawLine(x + 1, bottom, right - 2, bottom, DragonButtonColors[3]);
	dst->drawLine(x + 1, y + 1, right - 2, y + 1, DragonButtonColors[4]);

	int colOffset;
	if (enabled) {
		colOffset = 5;
	}  else {
		colOffset = 9;
	}

	dst->drawLine(x + 3, y + 3, right - 4, y + 3, DragonButtonColors[colOffset + 1]);

	// TODO: This is done with a different call in the game.. is there some reason for that?
	dst->fillRect(Common::Rect(x + 3, y + 4, x + 3 + _width - 6, y + 4 + _height - 8), DragonButtonColors[colOffset + 2]);

	dst->drawLine(x + 3, bottom - 3, right - 4, bottom - 3, DragonButtonColors[colOffset + 3]);

	return DragonButtonColors[colOffset];
}

byte ButtonGadget::drawChinaBg(Graphics::ManagedSurface *dst, bool enabled) const {
	Common::Point pt = topLeft();
	int16 x = pt.x;
	int16 x2 = x + _width - 1;
	int16 y = pt.y;
	int16 y2 = y + _height - 1;
	int cnum = 0;
	const byte *colors = (enabled ? ChinaButtonColorsOn : ChinaButtonColorsOff);

	for (int i = 0; i < 7; i++) {
		byte drawCol = colors[cnum];
		cnum++;
		if (i < 3) {
			dst->drawLine(x + i + 1, y + i, x2 - i, y + i, drawCol);
			dst->drawLine(x2 - i, y + i + 1, x2 - i, y2 - i, drawCol);
		} else if (i < 4) {
			int16 rheight, rwidth;
			if (_height + -6 < 3)
				rheight = 2;
			else
				rheight = _height + -6;

			if (_width + -6 < 3)
				rwidth = 2;
			else
				rwidth = _width + -6;

			dst->fillRect(Common::Rect(Common::Point(x + 3, y + 3), rwidth, rheight), drawCol);
		} else {
			int16 x2_ = 6 - i;
			dst->drawLine(x + x2_, y + x2_, x + x2_, y2 - x2_, drawCol);
			dst->drawLine(x + x2_, y2 - x2_, (x2 - x2_) + -1, y2 - x2_, drawCol);
		}
	}
	return colors[7];
}

byte ButtonGadget::drawWillyBg(Graphics::ManagedSurface *dst, bool enabled) const {
	Common::Point pt = topLeft();
	uint16 cornerFrame = enabled ? 8 : 16;
	RequestData::drawCorners(dst, cornerFrame, pt.x, pt.y, _width, _height);
	int16 fillHeight = enabled ? _height - 8 : _height - 6;
	dst->fillRect(Common::Rect(Common::Point(pt.x + 8, pt.y + 3), _width - 16, fillHeight), WillyButtonColor);
	return 0;
}

void ButtonGadget::draw(Graphics::ManagedSurface *dst) const {
	DgdsGameId gameId = DgdsEngine::getInstance()->getGameId();
	bool enabled = !(_flags3 & 9);

	byte textCol;
	if (gameId == GID_DRAGON)
		textCol = drawDragonBg(dst, enabled);
	else if (gameId == GID_HOC)
		textCol = drawChinaBg(dst, enabled);
	else
		textCol = drawWillyBg(dst, enabled);

	Common::Point pt = topLeft();
	int16 x = pt.x;
	int16 y = pt.y;

	if (!_buttonName.empty()) {
		const DgdsFont *font = RequestData::getMenuFont();
		Common::String name = _buttonName;
		int fontHeight = font->getFontHeight();
		//bool twoline;
		int yoffset;
		uint32 linebreak = name.find('&');

		Common::String line1, line2;
		if (linebreak != Common::String::npos) {
			//twoline = true;
			name.setChar(' ', linebreak);
			yoffset = _height + 1 - fontHeight * 2;
			line1 = _buttonName.substr(0, linebreak);
			line2 = _buttonName.substr(linebreak + 1);
		} else {
			//twoline = false;
			yoffset = _height - fontHeight;
			line1 = _buttonName;
		}

		yoffset = y + yoffset / 2;
		if (gameId == GID_WILLY)
			yoffset--;

		int lineWidth = font->getStringWidth(line1);
		font->drawString(dst, line1, x + (_width - lineWidth) / 2 + 1, yoffset + 2, lineWidth, textCol);

		if (linebreak != Common::String::npos) {
			lineWidth = font->getStringWidth(line2);
			font->drawString(dst, line2, x + (_width - lineWidth) / 2 + 1, yoffset + fontHeight, lineWidth, textCol);
		}
	} else if (gameId == GID_WILLY && _height == 32) {
		drawWillyBmpButtons(dst);
	}
}

enum WillyMenuIcons {
	kWillyArrowRight = 33,
	kWillyCD = 34,
	kWillyRewind = 35,
	kWillyStop = 36,
	kWillyQuestionMark = 37,
	kWillyBam = 38,
	kWillyMusicNote = 39,
	kWillyHelpText = 40,
	kWillySwoosh = 42,
	kWillyPlay = 43,
	kWillyNoBam = 44,
	kWillyNoMusicNote = 45,
};

void ButtonGadget::drawCenteredBmpIcon(Graphics::ManagedSurface *dst, int16 cornerNum) const {
	const Image *uiCorners = RequestData::getCorners();
	const Common::Rect screen(SCREEN_WIDTH, SCREEN_HEIGHT);
	const Common::Point tl = topLeft();
	int16 x = tl.x + (_width - uiCorners->width(cornerNum)) / 2;
	int16 y = tl.y + (_height - uiCorners->height(cornerNum)) / 2;
	uiCorners->drawBitmap(cornerNum, x, y, screen, *dst);
}

void ButtonGadget::drawWillyLoadSaveIcon(Graphics::ManagedSurface *dst, bool isLoad) const {
	const Image *uiCorners = RequestData::getCorners();
	const Common::Rect screen(SCREEN_WIDTH, SCREEN_HEIGHT);
	const Common::Point tl = topLeft();
	int16 cdIconWidth = uiCorners->width(kWillyCD);
	int16 arrowIconWidth = uiCorners->width(kWillyArrowRight);
	int16 x = tl.x + (_width - cdIconWidth - arrowIconWidth) / 2;
	int16 cdy = tl.y + (_height - uiCorners->height(kWillyCD)) / 2;
	int16 arrowy = tl.y + (_height - uiCorners->height(kWillyArrowRight)) / 2;
	uiCorners->drawBitmap(kWillyCD, x, cdy, screen, *dst);
	uiCorners->drawBitmap(kWillyArrowRight, x + cdIconWidth, arrowy, screen, *dst, isLoad ? kImageFlipNone : kImageFlipH);
}

void ButtonGadget::drawWillyBmpButtons(Graphics::ManagedSurface *dst) const {

	switch (_gadgetNo) {
	case 110: // SAVE (CD icon, arrow flipped)
		drawWillyLoadSaveIcon(dst, false);
		break;
	case 111: // LOAD (CD icon, arrow)
		drawWillyLoadSaveIcon(dst, true);
		break;
	case 112: // RESTART (rewind icon)
		drawCenteredBmpIcon(dst, kWillyRewind);
		break;
	case 113: // QUIT (stop icon)
		drawCenteredBmpIcon(dst, kWillyStop);
		break;
	case 114: // HELP (questionmark icon)
		drawCenteredBmpIcon(dst, kWillyQuestionMark);
		break;
	case 115: // SFX (bam icon)
		drawCenteredBmpIcon(dst, kWillyBam);
		break;
	case 116: // MUSIC (musical note icon)
		drawCenteredBmpIcon(dst, kWillyMusicNote);
		break;
	case 120: // HELP (questionmark and text icon)
		drawCenteredBmpIcon(dst, kWillyHelpText);
		break;
	case 118: // CREDITS (swoosh icon)
		drawCenteredBmpIcon(dst, kWillySwoosh);
		break;
	case 119: // PLAY (play icon)
		drawCenteredBmpIcon(dst, kWillyPlay);
		break;
	default:
		break;
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
	return Common::String::format("TextArea<%s, %d buflen %d>", base.c_str(), _textGadget_i1, _bufLen);
}

void TextAreaGadget::draw(Graphics::ManagedSurface *dst) const {
	const DgdsFont *font = RequestData::getMenuFont();
	font->drawString(dst, _buttonName, _x + _parentX, _y + _parentY, 0, 0);
}

Common::String SliderGadget::dump() const {
	const Common::String base = Gadget::dump();
	return Common::String::format("Slider<%s, %d %d %d %d>", base.c_str(), _gadget2_i1, _gadget2_i2, _gadget2_i3, _gadget2_i4);
}

// Slider labels and title are hard-coded in game, not part of data files.
static const char *_sliderTitleForGadget(uint16 num, Common::Language language) {
	if (language == Common::EN_ANY) {
		switch (num) {
		case 0x7B:	return "DIFFICULTY";
		case 0x7D:	return "TEXT SPEED";
		case 0x83:	return "DETAIL LEVEL";
		case 0x98:	return "MOUSE SPEED";
		case 0x9C:	return "BUTTON THRESHOLD";
		default:	return "SLIDER";
		}
	} else if (language == Common::DE_DEU) {
		switch (num) {
		case 0x7B:	return "SCHWIERIGKEITSGRAD";
		case 0x7D:	return "TEXT-VERWEILDAUER";
		case 0x83:	return "DETAILS";
		case 0x98:	return "GESCHWINDIGKEIT";
		case 0x9C:	return "TASTENEMPFINDLICHKEIT";
		default:	return "REGLER";
		}
	} else {
		error("Unsupported language %d", language);
	}
}

static const char *_sliderLabelsForGadget(uint16 num, Common::Language language) {
	if (language == Common::EN_ANY) {
		switch (num) {
		case 0x7B:	return "EASY         HARD";
		case 0x7D:	return "SLOW         FAST";
		case 0x83:	return "LOW        HIGH";
		case 0x98:	return "SLOW         FAST";
		case 0x9C:	return "LONG         SHORT";
		default:	return "MIN         MAX";
		}
	} else if (language == Common::DE_DEU) {
		return "-             +";
	} else {
		error("Unsupported language %d", language);
	}
}

static const int SLIDER_HANDLE_FRAME = 28;

void SliderGadget::drawDragonBg(Graphics::ManagedSurface *dst) const {
	int16 x = _x + _parentX;
	int16 y = _y + _parentY;
	int16 x2 = x + _width;
	int16 y2 = (y + _height) - 1;

	int16 y1 = y - 1;
	dst->drawLine(x - 2, y - 1, x - 2, y2, DragonSliderColors[0]);
	dst->drawLine(x - 1, y1, x - 1, y2, DragonSliderColors[1]);
	dst->drawLine(x, y2, x2 - 1, y2, DragonSliderColors[1]);
	dst->drawLine(x, y1, x2, y1, DragonSliderColors[2]);
	dst->drawLine(x2, y1, x2, y2, DragonSliderColors[2]);
	dst->drawLine(x2 + 1, y1, x2 + 1, y2, DragonSliderColors[3]);
	dst->drawLine(x, y, x2 - 1, y, DragonSliderColors[4]);
	dst->drawLine(x2 - 1, y + 1, x2 - 1, (y + _height) - 2, DragonSliderColors[4]);
	// This is not exactly what happens in the original, but gets the same result
	Common::Rect fillrect = Common::Rect(x, y + 1, x + _width - 1, y + _height - 1);
	dst->fillRect(fillrect, DragonSliderColors[5]);
	fillrect.grow(-1);
	dst->fillRect(fillrect, DragonSliderColors[6]);
}

void SliderGadget::drawChinaBg(Graphics::ManagedSurface *dst) const {
	int16 x = _x + _parentX;
	int16 y = _y + _parentY;
	int16 y2 = y + 8;
	int16 x2 = x + _width + 2;

	// left 1st
    dst->drawLine(x - 2, y - 1, x - 2, y2 + 1, ChinaSliderColors[0]);
    // left 2nd
    dst->drawLine(x - 1, y, x - 1, y2, ChinaSliderColors[1]);
    // left 3rd
	dst->drawLine(x, y + 1, x, y2 - 1, ChinaSliderColors[2]);

	// top 1st
	dst->drawLine(x - 1, y - 1, x2, y - 1, ChinaSliderColors[2]);
	// top 2nd
	dst->drawLine(x, y, x2 - 1, y, ChinaSliderColors[0]);
	// top 3rd
	dst->drawLine(x + 1, y + 1, x2 - 2, y + 1, ChinaSliderColors[3]);

	// right 1st
	dst->drawLine(x2 - 1, y + 1, x2 - 1, y2 - 1, ChinaSliderColors[3]);
	// right 2nd
	dst->drawLine(x2, y, x2, y2, ChinaSliderColors[0]);
	// right 3rd
	dst->drawLine(x2 + 1, y - 1, x2 + 1, y2 + 1, ChinaSliderColors[2]);

	// bottom 1st
	dst->drawLine(x + 1, y2 - 1, x2 - 2, y2 - 1, ChinaSliderColors[2]);
	// bottom 2nd
	dst->drawLine(x, y2, x2 - 1, y2, ChinaSliderColors[1]);
	// bottom 3rd
	dst->drawLine(x - 1, y2 + 1, x2, y2 + 1, ChinaSliderColors[0]);

	Common::Rect fillrect = Common::Rect(x + 1, y + 2, x2 - 1, y2 - 1);
	dst->fillRect(fillrect, ChinaSliderColors[4]);
}


void SliderGadget::draw(Graphics::ManagedSurface *dst) const {
	const DgdsFont *font = RequestData::getMenuFont();
	Common::Language language = DgdsEngine::getInstance()->getGameLang();

	int16 x = _x + _parentX;
	int16 y = _y + _parentY;

	int16 titley = (y - font->getFontHeight()) + 1;
	const char *title = _sliderTitleForGadget(_gadgetNo, language);
	const char *labels = _sliderLabelsForGadget(_gadgetNo, language);
	int16 titleWidth = font->getStringWidth(title);
	DgdsGameId gameId = DgdsEngine::getInstance()->getGameId();

	byte textCol = (gameId == GID_DRAGON) ? 0 : 0x13;
	int16 labelYOff = (gameId == GID_DRAGON) ? 7 : 11;

	font->drawString(dst, title, x + (_width - titleWidth) / 2, titley, titleWidth, textCol);
	int16 labelWidth = font->getStringWidth(labels);
	font->drawString(dst, labels, x + (_width - labelWidth) / 2, y + labelYOff, labelWidth, textCol);

	if (gameId == GID_DRAGON)
		drawDragonBg(dst);
	else
		drawChinaBg(dst);

	// Draw the slider control in the right spot
	const Image *uiCorners = RequestData::getCorners();
	uiCorners->drawBitmap(SLIDER_HANDLE_FRAME, x + _handleX, y, Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), *dst);
}

SliderGadget::SliderGadget() : _lock(false), _steps(0), _gadget2_i1(0),
	_gadget2_i2(0), _gadget2_i3(0), _gadget2_i4(0), _handleX(0) {
}

int16 SliderGadget::getHandleWidth() const {
	const Image *uiCorners = RequestData::getCorners();
	int16 handleWidth = uiCorners->width(SLIDER_HANDLE_FRAME);
	return handleWidth - 2;
}

int16 SliderGadget::getUsableWidth() const {
	return _width + 4 - getHandleWidth();
}

void SliderGadget::onDrag(const Common::Point &mousePt) {
	const Common::Point topLeftPt = topLeft();
	const Common::Point relMouse = mousePt - topLeftPt;
	// match middle of handle to mouse point
	int16 handleWidth = getHandleWidth();
	_handleX = CLIP(relMouse.x - handleWidth / 2, 0, (int)getUsableWidth());
}

int16 SliderGadget::onDragFinish(const Common::Point &mousePt) {
	onDrag(mousePt);
	int16 newVal = getValue();
	if (_lock)
		setValue(newVal);
	return newVal;
}

int16 SliderGadget::getValue() {
	int16 stepSize = getUsableWidth() / (_steps - 1);
	// Find the closest step point to the left end of the handle
	int16 closestStep = (_handleX + stepSize / 2) / stepSize;
	return CLIP(closestStep, (int16)0, _steps);
}

void SliderGadget::setValue(int16 val) {
	// if val is steps-1, slider x should be at..
	int16 usableWidth = getUsableWidth();
	if (val == _steps - 1)
		_handleX = usableWidth;
	else
		_handleX = (usableWidth * val) / (_steps - 1);
}

int16 SliderGadget::onClick(const Common::Point &mousePt) {
	const Common::Point topLeftPt = topLeft();
	const Common::Point relMouse = mousePt - topLeftPt;

	// A click should move the slider to the next step in the direction of the click.
	int16 handleMiddle = _handleX + getHandleWidth() / 2;
	// round up step size to ensure we move far enough..

	int16 val = getValue();
	int16 newVal = val;
	if (relMouse.x > handleMiddle)
		newVal++;
	else
		newVal--;

	debug(1, "clicked on slider %d, move val from %d -> %d", _gadgetNo, val, newVal);

	newVal = CLIP((int)newVal, 0, _steps - 1);
	setValue(newVal);
	return newVal;
}

Common::String ImageGadget::dump() const {
	const Common::String base = Gadget::dump();
	return Common::String::format("Image<%s, xStep %d yStep %d>", base.c_str(), _xStep, _yStep);
}

static void _drawFrame(Graphics::ManagedSurface *dst, int16 x, int16 y, int16 w, int16 h, byte col1, byte col2) {
	const int xmax = x + w - 1;
	const int ymax = y + h - 1;
	bool filled = true;
	if (filled) {
		for (int yy = y; yy < ymax; yy++) {
			for (int xx = x; xx < xmax; xx = xx + 1) {
				dst->setPixel(xx, yy, (byte)dst->getPixel(xx, yy) ^ col2);
			}
		}
	}
	for (int yy = y; yy <= ymax; yy++) {
		dst->setPixel(x, yy, (byte)dst->getPixel(x, yy) ^ col1);
		dst->setPixel(xmax, yy, (byte)dst->getPixel(xmax, yy) ^ col1);
	}
	for (int xx = x; xx < xmax; xx++) {
		dst->setPixel(xx, y, (byte)dst->getPixel(xx, y) ^ col1);
		dst->setPixel(xx, ymax, (byte)dst->getPixel(xx, ymax) ^ col1);
	}
}

void ImageGadget::draw(Graphics::ManagedSurface *dst) const {
	int xstep = _xStep;
	int ystep = _yStep;

	if (!xstep || !ystep)
		return;

	int xoff = _x + _parentX;
	int yoff = _y + _parentY;
	Common::Rect drawRect(Common::Point(xoff, yoff), _width, _height);
	dst->fillRect(drawRect, _col1);
	// Note: not quite the same as the original logic here, but gets the same result.
	_drawFrame(dst, xoff, yoff, _width, _height, _sval1I, _sval1I);

	// NOTE: This only done in inventory in originals
	if (DgdsEngine::getInstance()->getGameId() == GID_DRAGON)
		RequestData::drawCorners(dst, 19, xoff - 2, yoff - 2, _width + 4, _height + 4);
	else
		RequestData::drawCorners(dst, 19, xoff - 4, yoff - 4, _width + 8, _height + 8);
}

Common::String RequestData::dump() const {
	Common::String ret = Common::String::format("RequestData<file %d %s c1 %d c2 %d flg %d\n",
								_fileNum, _rect.dump("").c_str(), _col1, _col2, _flags);
	for (const auto &t : _textItemList)
		ret += Common::String::format("    TextItem<'%s' pos (%d,%d) %d %d>\n", t._txt.c_str(),
								t._x, t._y, t._vals[0], t._vals[1]);
	for (const auto &f : _fillAreaList)
		ret += Common::String::format("    FillArea<x %d y %d w %d h %d c1 %d c2 %d>\n", f._x, f._y,
								f._width, f._height, f._col1, f._col2);
	for (const auto &g : _gadgets)
		ret += Common::String::format("    %s\n", g->dump().c_str());
	ret += ">";

	return ret;
}

void RequestData::drawBg(Graphics::ManagedSurface *dst) const {
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

	bool isDragon = DgdsEngine::getInstance()->getGameId() == GID_DRAGON;
	if (slidery && isDragon)
		drawBackgroundWithSliderArea(dst, slidery, header);
	else
		drawBackgroundNoSliders(dst, header);
}

void RequestData::drawInvType(Graphics::ManagedSurface *dst) {
	if (_flags & 0x40)
		return;

	drawBackgroundNoSliders(dst, "");
	for (const auto &fillArea : _fillAreaList) {
		Common::Rect r(Common::Point(_rect.x + fillArea._x, _rect.y + fillArea._y), fillArea._width, fillArea._height);
		dst->fillRect(r, fillArea._col1);
	}

	for (const auto &textItem : _textItemList) {
		if (!textItem._txt.empty())
			error("TODO: RequestData::drawInvType: Implement support for drawing text item.");
	}

	for (auto &gadget : _gadgets)
		gadget->_flags3 |= 0x100;

	for (auto &gadget : _gadgets) {
		if (!(gadget->_flags3 & 0x40)) {
			gadget->draw(dst);
		}
	}

	_flags |= 4;
}

/*static*/
const DgdsFont *RequestData::getMenuFont() {
	return DgdsEngine::getInstance()->getFontMan()->getFont(FontManager::kGameFont);
}

/*static*/
const Image *RequestData::getCorners() {
	return DgdsEngine::getInstance()->getUICorners().get();
}

/*static*/
void RequestData::drawCorners(Graphics::ManagedSurface *dst, uint16 startNum, uint16 x, uint16 y, uint16 width, uint16 height) {
	const Image *uiCorners = RequestData::getCorners();
	assert(uiCorners->loadedFrameCount() > startNum + 7);
	const Common::Array<Common::SharedPtr<Graphics::ManagedSurface>> &cframes = uiCorners->getFrames();
	const Common::SharedPtr<Graphics::ManagedSurface> *corners = cframes.data() + startNum;

	//
	// Corners images are:
	//  0  1  2  3  4  5  6  7
	// TL  T TR  L  R BL  B BR
	//

	//
	// The T, L, R, and B blocks are repeated as many times as is needed to make the
	// size of menu/button we need.
	//
	// The last repeated one is bottom/right-aligned so that it doesn't overlap with
	// the corner (this is visible eg in Willy Beamish menu buttons)
	//

	for (int xoff = x + corners[0]->w; xoff < (x + width) - corners[2]->w; xoff += corners[1]->w) {
		int xdraw = MIN(xoff, (x + width) - corners[2]->w - corners[1]->w);
		dst->transBlitFrom(*corners[1], Common::Point(xdraw, y));
	}

	for (int xoff = x + corners[5]->w; xoff < (x + width) - corners[7]->w; xoff += corners[6]->w) {
		int xdraw = MIN(xoff, (x + width) - corners[7]->w - corners[6]->w);
		dst->transBlitFrom(*corners[6], Common::Point(xdraw, (y + height) - corners[6]->h));
	}

	for (int yoff = y + corners[0]->h; yoff < (y + height) - corners[5]->h; yoff += corners[3]->h) {
		int ydraw = MIN(yoff, (y + height) - corners[5]->h - corners[3]->h);
		dst->transBlitFrom(*corners[3], Common::Point(x, ydraw));
	}

	for (int yoff = y + corners[2]->h; yoff < (y + height) - corners[7]->h; yoff += corners[4]->h) {
		int ydraw = MIN(yoff, (y + height) - corners[7]->h - corners[4]->h);
		dst->transBlitFrom(*corners[4], Common::Point((x + width) - corners[4]->w, ydraw));
	}

	dst->transBlitFrom(*corners[0], Common::Point(x, y));
	dst->transBlitFrom(*corners[2], Common::Point((x + width) - corners[2]->w, y));
	dst->transBlitFrom(*corners[5], Common::Point(x, (y + height) - corners[5]->h));
	dst->transBlitFrom(*corners[7], Common::Point((x + width) - corners[7]->w, (y + height) - corners[7]->h));
}

/*static*/
void RequestData::drawHeader(Graphics::ManagedSurface *dst, int16 x, int16 y, int16 width, int16 yoffset, const Common::String &header, byte fontCol, bool drawBox, byte boxTopColor, byte boxBottomColor) {
	if (!header.empty()) {
		const DgdsFont *font = getMenuFont();
		int hwidth = font->getStringWidth(header);
		int hheight = font->getFontHeight();
		int hleft = x + (width - hwidth) / 2;
		int hright = hleft + hwidth + 3;
		int htop = y + yoffset;
		int hbottom = htop + hheight;

		font->drawString(dst, header, hleft + 1, htop + 2, hwidth, fontCol);
		if (drawBox) {
			dst->drawLine(hleft - 3, htop, hright, htop, boxTopColor);
			dst->drawLine(hright, htop + 1, hright, hbottom, boxTopColor);
			dst->drawLine(hleft - 3, htop + 1, hleft - 3, hbottom, boxBottomColor);
			dst->drawLine(hleft - 2, hbottom, hleft + hwidth + 2, hbottom, boxBottomColor);
		}
	}
}

void RequestData::drawBackgroundWithSliderArea(Graphics::ManagedSurface *dst, int16 sliderHeight, const Common::String &header) const {
	int16 x = _rect.x;
	int16 y = _rect.y;
	int16 width = _rect.width;
	int16 height = _rect.height;
	uint16 sliderBgHeight = sliderHeight + 18;
	fillBackground(dst, x, y, width, sliderBgHeight, 0);
	fillBackground(dst, x + 8, y + sliderBgHeight, width - 16, height - sliderBgHeight, 8 - sliderBgHeight);
	fillBackground(dst, x + 9, y + 8, width - 18, sliderHeight + 2, 8);
	fillBackground(dst, x + 17, y + 8 + sliderHeight + 2, width - 34, height - sliderBgHeight, 32 - sliderBgHeight);

	const Image *uiCorners = RequestData::getCorners();
	assert(uiCorners->loadedFrameCount() >= 11);
	const Common::Array<Common::SharedPtr<Graphics::ManagedSurface>> &corners = uiCorners->getFrames();

	for (int xoff = x + corners[0]->w; xoff < (x + width) - corners[3]->w; xoff += corners[2]->w) {
		dst->transBlitFrom(*corners[2], Common::Point(xoff, y));
	}
	for (int xoff = x + 8 + corners[6]->w; xoff < (x + 8 + width - 16) - corners[8]->w; xoff += corners[7]->w) {
		dst->transBlitFrom(*corners[7], Common::Point(xoff, (y + height) - corners[7]->h));
	}
	for (int yoff = y + corners[3]->h; yoff < (y + sliderBgHeight) - corners[10]->h; yoff += corners[5]->h) {
		dst->transBlitFrom(*corners[5], Common::Point((x + width) - corners[5]->w, yoff));
	}
	for (int yoff = y + corners[1]->h; yoff < (y + sliderBgHeight) - corners[9]->h; yoff += corners[4]->h) {
		dst->transBlitFrom(*corners[4], Common::Point(x, yoff));
	}
	for (int yoff = y + sliderBgHeight; yoff < (y + height) - corners[6]->h; yoff += corners[4]->h) {
		dst->transBlitFrom(*corners[4], Common::Point(x + 8, yoff));
	}
	for (int yoff = y + sliderBgHeight; yoff < (y + height) - corners[8]->h; yoff += corners[5]->h) {
		dst->transBlitFrom(*corners[5], Common::Point((x + 8 + width - 16) - corners[5]->w, yoff));
	}
	dst->transBlitFrom(*corners[1], Common::Point(x, y));
	dst->transBlitFrom(*corners[3], Common::Point((x + width) - corners[3]->w, y));
	dst->transBlitFrom(*corners[6], Common::Point(x + 8, (y + height) - corners[6]->h));
	dst->transBlitFrom(*corners[8], Common::Point((x + width - 8) - corners[8]->w, (y + height) - corners[8]->h));
	dst->transBlitFrom(*corners[9], Common::Point(x, (y + sliderBgHeight) - corners[9]->h));
	dst->transBlitFrom(*corners[10], Common::Point((x + width) - corners[10]->w, (y + sliderBgHeight) - corners[10]->h));

	if (DgdsEngine::getInstance()->getGameId() == GID_DRAGON)
		drawHeader(dst, x, y, width, 9, header, DragonHeaderTxtColor, true, DragonHeaderTopColor, DragonHeaderBottomColor);
	else
		drawHeader(dst, x, y + 4, width, 9, header, ChinaHeaderTxtColor, true, ChinaHeaderTopColor, ChinaHeaderBottomColor);
}


void RequestData::drawBackgroundNoSliders(Graphics::ManagedSurface *dst, const Common::String &header) const {
	DgdsGameId gameId = DgdsEngine::getInstance()->getGameId();

	if (_rect.width == 0 || _rect.height == 0) {
		warning("drawBackgroundNoSliders: empty rect");
		return;
	}

	if (gameId != GID_WILLY)
		fillBackground(dst, _rect.x, _rect.y, _rect.width, _rect.height, 0);
	else
		fillBackground(dst, _rect.x + 5, _rect.y + 5, _rect.width - 10, _rect.height - 10, 0);
	uint16 cornerOffset = (gameId == GID_DRAGON ? 11 : (gameId == GID_HOC ? 1 : 0));
	drawCorners(dst, cornerOffset, _rect.x, _rect.y, _rect.width, _rect.height);
	if (gameId == GID_DRAGON)
		drawHeader(dst, _rect.x, _rect.y, _rect.width, 4, header, DragonHeaderTxtColor, true, DragonHeaderTopColor, DragonHeaderBottomColor);
	else
		drawHeader(dst, _rect.x, _rect.y + 4, _rect.width, 4, header, ChinaHeaderTxtColor, true, ChinaHeaderTopColor, ChinaHeaderBottomColor);
}

/*static*/
void RequestData::fillBackground(Graphics::ManagedSurface *dst, uint16 x, uint16 y, uint16 width, uint16 height, int16 startoffset) {
	DgdsEngine *engine = DgdsEngine::getInstance();

	if (engine->getGameId() == GID_DRAGON && engine->getDetailLevel() == kDgdsDetailHigh && !engine->isEGA()) {
		Graphics::Surface area = dst->getSubArea(Common::Rect(Common::Point(x, y), width, height));
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
		byte bgCol = DragonFallbackColors[0];
		if (engine->getGameId() == GID_HOC)
			bgCol = ChinaBackgroundColor;
		else if (engine->getGameId() == GID_WILLY)
			bgCol = WillyBackgroundColor;
		dst->fillRect(Common::Rect(Common::Point(x, y), width, height), bgCol);
	}
}

Gadget *RequestData::findGadgetByNumWithFlags3Not0x40(int16 num) {
	for (auto &gadget : _gadgets) {
		if (gadget->_gadgetNo == num && (gadget->_flags3 & 0x40) == 0)
			return gadget.get();
	}
	return nullptr;
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
