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

#include "director/director.h"
#include "director/movie.h"
#include "director/castmember/shape.h"
#include "director/lingo/lingo-the.h"

namespace Director {

ShapeCastMember::ShapeCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastShape;

	byte unk1;

	_ink = kInkTypeCopy;

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "ShapeCastMember::ShapeCastMember(): Shape data");
		stream.hexdump(stream.size());
	}

	if (version < kFileVer400) {
		unk1 = stream.readByte();
		_shapeType = static_cast<ShapeType>(stream.readByte());
		_initialRect = Movie::readRect(stream);
		_pattern = stream.readUint16BE();
		// Normalize D2 and D3 colors from -128 ... 127 to 0 ... 255.
		_fgCol = g_director->transformColor((128 + stream.readByte()) & 0xff);
		_bgCol = g_director->transformColor((128 + stream.readByte()) & 0xff);
		_fillType = stream.readByte();
		_ink = static_cast<InkType>(_fillType & 0x3f);
		_lineThickness = stream.readByte();
		_lineDirection = stream.readByte();
	} else if (version >= kFileVer400 && version < kFileVer600) {
		unk1 = stream.readByte();
		_shapeType = static_cast<ShapeType>(stream.readByte());
		_initialRect = Movie::readRect(stream);
		_pattern = stream.readUint16BE();
		_fgCol = g_director->transformColor((uint8)stream.readByte());
		_bgCol = g_director->transformColor((uint8)stream.readByte());
		_fillType = stream.readByte();
		_ink = static_cast<InkType>(_fillType & 0x3f);
		_lineThickness = stream.readByte();
		_lineDirection = stream.readByte();
	} else {
		warning("STUB: ShapeCastMember::ShapeCastMember(): not yet implemented");
		unk1 = 0;
		_shapeType = kShapeRectangle;
		_pattern = 0;
		_fgCol = _bgCol = 0;
		_fillType = 0;
		_lineThickness = 1;
		_lineDirection = 0;
	}
	_modified = false;

	debugC(3, kDebugLoading, "ShapeCastMember: unk1: %x type: %d pat: %d fg: %d bg: %d fill: %d thick: %d dir: %d",
		unk1, _shapeType, _pattern, _fgCol, _bgCol, _fillType, _lineThickness, _lineDirection);

	if (debugChannelSet(3, kDebugLoading))
		_initialRect.debugPrint(0, "ShapeCastMember: rect:");
}

ShapeCastMember::ShapeCastMember(Cast *cast, uint16 castId, ShapeCastMember &source)
	: CastMember(cast, castId) {
	_type = kCastShape;
	_loaded = source._loaded;

	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	_children = source._children;

	_shapeType = source._shapeType;
	_pattern = source._pattern;
	_fillType = source._fillType;
	_lineThickness = source._lineThickness;
	_lineDirection = source._lineDirection;
	_ink = source._ink;
}

void ShapeCastMember::setBackColor(uint32 bgCol) {
	_bgCol = bgCol;
	_modified = true;
}

void ShapeCastMember::setForeColor(uint32 fgCol) {
	_fgCol = fgCol;
	_modified = true;
}

bool ShapeCastMember::hasField(int field) {
	switch (field) {
	case kTheFilled:
	case kTheLineSize:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum ShapeCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheFilled:
		d = Datum((bool)_fillType);
		break;
	case kTheLineSize:
		d = Datum(_lineThickness);
		break;
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

bool ShapeCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheFilled:
		_fillType = d.asInt() ? 1 : 0;
		return true;
	case kTheLineSize:
		_lineThickness = d.asInt();
		return true;
	default:
		break;
	}

	return CastMember::setField(field, d);
}


Common::String ShapeCastMember::formatInfo() {
	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, foreColor: %d, backColor: %d, shapeType: %d, pattern: %d, fillType: %d, lineThickness: %d, lineDirection: %d, ink: %d",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		getForeColor(), getBackColor(),
		_shapeType, _pattern, _fillType,
		_lineThickness, _lineDirection, _ink
	);
}

}
