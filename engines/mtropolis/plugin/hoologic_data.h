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

#ifndef MTROPOLIS_PLUGIN_HOOLOGIC_DATA_H
#define MTROPOLIS_PLUGIN_HOOLOGIC_DATA_H

#include "mtropolis/data.h"

namespace MTropolis {

namespace Data {

namespace Hoologic {

// Known Hoologic custom modifiers:
// * Bitmap Suite
// - hlBitmapVariable: variable storing a bitmap
// - hlCaptureBitmap: captures a screen image into a bitmap
// - hlPrintBitmap: print a bitmap
// - hlSaveBitmap: saves a bitmap as a PICT/BMP file
// - hlImportBitmap: load a bitmap from a PICT/BMP file
// - hlDisplayBitmap: display a bitmap
// - hlScaleBitmap: scale a bitmap
//
// * Painter Plugin
// - hlPainter: Creates painting effects and interaction by blending several pictures together
//
// * KeyState Plugin
// - hlKeyState: State of keyboard keys, and if modifier keys (CTRL, SHIFT, ...) are used?
//
// * AppleScript Suite
// - ???


struct BitmapVariableModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct CaptureBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2VarRef;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct ImportBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2Bool;
	PlugInTypeTaggedValue unknown3Bool;
	PlugInTypeTaggedValue unknown4VarRef;
	PlugInTypeTaggedValue unknown5VarRef;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct DisplayBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2VarRef;
	PlugInTypeTaggedValue unknown3VarRef;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct ScaleBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2VarRef;
	PlugInTypeTaggedValue unknown3IncomingData;
	PlugInTypeTaggedValue unknown4Int;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct SaveBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2VarRef;
	PlugInTypeTaggedValue unknown3Bool;
	PlugInTypeTaggedValue unknown4Bool;
	PlugInTypeTaggedValue unknown5VarRef;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct PrintBitmapModifier : public PlugInModifierData {
	PlugInTypeTaggedValue unknown1Event;
	PlugInTypeTaggedValue unknown2VarRef;
	PlugInTypeTaggedValue unknown3Bool;
	PlugInTypeTaggedValue unknown4Bool;
	PlugInTypeTaggedValue unknown5Int;
	PlugInTypeTaggedValue unknown6Int;
	PlugInTypeTaggedValue unknown7Null;
	PlugInTypeTaggedValue unknown8Null;
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct PainterModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

struct KeyStateModifier : public PlugInModifierData {
protected:
	DataReadErrorCode load(PlugIn &plugIn, const PlugInModifier &prefix, DataReader &reader) override;
};

} // End of namespace Hoologic

} // End of namespace Data

} // End of namespace MTropolis

#endif
