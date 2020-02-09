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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ILLUSIONS_FONTRESOURCE_H
#define ILLUSIONS_FONTRESOURCE_H

#include "illusions/graphics.h"
#include "illusions/resourcesystem.h"

namespace Illusions {

class IllusionsEngine;

class FontResourceLoader : public BaseResourceLoader {
public:
	FontResourceLoader(IllusionsEngine *vm) : _vm(vm) {}
	~FontResourceLoader() override {}
	void load(Resource *resource) override;
	bool isFlag(int flag) override;
protected:
	IllusionsEngine *_vm;
};

struct CharInfo {
	int16 _width;
	int16 _field_2;
	byte *_pixels;
	void load(byte *dataStart, Common::SeekableReadStream &stream);
};

struct CharRange {
	uint16 _firstChar;
	uint16 _lastChar;
	CharInfo *_charInfos;
	void load(byte *dataStart, Common::SeekableReadStream &stream);
	CharInfo *getCharInfo(uint16 c);
	bool containsChar(uint16 c);
};

class FontResource {
public:
	FontResource();
	~FontResource();
	void load(Resource *resource);
	CharInfo *getCharInfo(uint16 c);
	int16 getColorIndex() const { return _colorIndex; }
	int16 getCharHeight() const { return _charHeight; }
	int16 getLineIncr() const { return _lineIncr; }
	const Common::Rect calculateRectForText(uint16 *text, uint textLength);
public:
	uint32 _totalSize;
	int16 _charHeight;
	int16 _defaultChar;
	int16 _colorIndex;
	int16 _lineIncr;
	int16 _widthC;
	uint _charRangesCount;
	CharRange *_charRanges;
	CharRange *getCharRange(uint16 c);
};

class FontInstance : public ResourceInstance {
public:
	FontInstance(IllusionsEngine *vm);
	void load(Resource *resource) override;
	void unload() override;
public:
	IllusionsEngine *_vm;
	FontResource *_fontResource;
	uint32 _resId;
};

} // End of namespace Illusions

#endif // ILLUSIONS_FONTRESOURCE_H
