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

#ifndef DGDS_REQUEST_H
#define DGDS_REQUEST_H

#include "common/stream.h"
#include "common/array.h"

#include "dgds/parser.h"

namespace Dgds {

class Font;
class Image;

struct TextItem {
	uint16 _x;
	uint16 _y;
	uint16 _vals[2];
	Common::String _txt;
};

// This struct is defined in the code, but seems
// to not be used in any of the games?
struct RequestStruct2 {
	uint16 _vals[6];
};

enum GadgetType {
	kGadgetNone = 0,
	kGadgetText = 1,
	kGadgetSlider = 2,
	kGadgetButton = 4,
	kGadgetImage = 8,
};

// basic gadget is 52 (0x34) bytes
class Gadget {
public:
	virtual ~Gadget() {}

	// NOTE: Most of these names are still guesses
	uint16 _gadgetNo;
	uint16 _x;
	uint16 _y;
	uint16 _width;
	uint16 _height;
	GadgetType _gadgetType;
	uint16 _flags2;
	uint16 _flags3;

	uint16 _sval1Type; // if 1 then use the string, else use the int
	Common::String _sval1S;
	uint16 _sval1I;

	uint16 _sval2Type; // if 1 then use the string, else use the int
	Common::String _sval2S;
	uint16 _sval2I;

	Common::String _buttonName;

	// some other fields..
	uint16 _field14_0x20;
	uint16 _field15_0x22;
	uint16 _field16_0x24;
	uint16 _field17_0x26;

	uint16 _field20_0x28;
	uint16 _field21_0x2a;

	uint16 _parentX;
	uint16 _parentY;

	virtual Common::String dump() const;
	virtual void draw(Graphics::Surface *dst) const;
	virtual void toggle(bool enable) {}
};

// Button gadget has no additional fields, but some behavior differences.
class ButtonGadget : public Gadget {
public:
	void draw(Graphics::Surface *dst) const override;
	void toggle(bool enable) override;
};

// extended gadget type 1 is 62 (0x3e) bytes
class TextAreaGadget : public Gadget {
public:
	uint16 _gadget1_i1;
	uint16 _gadget1_i2;

	Common::String dump() const override;
	void draw(Graphics::Surface *dst) const override;
};

// extended gadget type 2 is 74 (0x4a) bytes
class SliderGadget : public Gadget {
public:
	uint16 _gadget2_i1;
	uint16 _gadget2_i2;
	uint16 _gadget2_i3;
	uint16 _gadget2_i4;

	Common::String dump() const override;
	void draw(Graphics::Surface *dst) const override;
};

// extended gadget type 8 is 68 (0x44) bytes
class ImageGadget : public Gadget {
public:
	uint16 _gadget8_i1;
	uint16 _gadget8_i2;

	Common::String dump() const override;
	void draw(Graphics::Surface *dst) const override;
};

class RequestData {
public:
	uint16 _fileNum;
	uint16 _x;
	uint16 _y;
	uint16 _width;
	uint16 _height;
	uint16 _vals[3];
	Common::Array<TextItem> _textItemList;
	Common::Array<RequestStruct2> _struct2List;
	Common::Array<Common::SharedPtr<Gadget>> _gadgets;

	Common::String dump() const;
	void draw(Graphics::Surface *dst) const;

	static const Font *getMenuFont();
	static const Image *getCorner(int cornerNum);

	static void fillBackground(Graphics::Surface *dst, uint16 x, uint16 y, uint16 width, uint16 height, int16 startoffset);
	static void drawCorners(Graphics::Surface *dst, uint16 startNum, uint16 x, uint16 y, uint16 width, uint16 height);
	static void drawHeader(Graphics::Surface *dst, int16 x, int16 y, int16 width, int16 yoffset, const Common::String &header);

private:

	void drawBackgroundNoSliders(Graphics::Surface *dst, const Common::String &header) const;
	void drawBackgroundWithSliderArea(Graphics::Surface *dst, int16 sliderHeight, const Common::String &header) const;

};

// A REQ file contains a sequence of REQ and GAD block pairs.
class REQFileData : public ParserData {
public:
	Common::Array<RequestData> _requests;

	Common::String dump() const;
};

/**
 * A "Request" is described by a REQ file.  Requests are things like menus,
 * inventory items, etc.
 *
 * Request files include REQ and GAD (Gadget) chunks.
 */
class RequestParser : public DgdsParser {
public:
	RequestParser(ResourceManager *resman, Decompressor *decompressor);
	virtual ~RequestParser() {}

	bool handleChunk(DgdsChunkReader &chunk, ParserData *data) override;

protected:
	bool parseREQChunk(RequestData &data, DgdsChunkReader &chunk, int num);
	bool parseGADChunk(RequestData &data, DgdsChunkReader &chunk, int num);
};


} // End of namespace Dgds

#endif // DGDS_REQUEST_H
