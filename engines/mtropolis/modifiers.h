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

#ifndef MTROPOLIS_MODIFIERS_H
#define MTROPOLIS_MODIFIERS_H

#include "mtropolis/runtime.h"
#include "mtropolis/data.h"

namespace MTropolis {

struct ModifierLoaderContext;
class MiniscriptProgram;

class BehaviorModifier : public Modifier, public IModifierContainer {
public:
	bool load(ModifierLoaderContext &context, const Data::BehaviorModifier &data);

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;

private:
	Common::Array<Common::SharedPtr<Modifier> > _children;
	Event _enableWhen;
	Event _disableWhen;
};

class MiniscriptModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::MiniscriptModifier &data);

private:
	Event _enableWhen;

	Common::SharedPtr<MiniscriptProgram> _program;
};

struct MessengerSendSpec {
	MessengerSendSpec();
	bool load(const Data::Event &dataEvent, uint32 dataMessageFlags, uint16 dataWith, uint32 dataWithSourceGUID, uint32 dataDestination);

	Event send;
	MessageFlags messageFlags;
	MessageWithType withType;
	uint32 withSourceGUID;
	uint32 destination; // May be a MessageDestination or GUID
};

class MessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::MessengerModifier &data);

private:
	Event _when;
	MessengerSendSpec _sendSpec;
};

class IfMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IfMessengerModifier &data);

private:
	Event _when;
	MessengerSendSpec _sendSpec;

	Common::SharedPtr<MiniscriptProgram> _program;
};

class KeyboardMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::KeyboardMessengerModifier &data);

private:
	Event _send;

	enum KeyCodeType {
		kAny = 0x00,
		kHome = 0x01,
		kEnter = 0x03,
		kEnd = 0x04,
		kHelp = 0x05,
		kBackspace = 0x08,
		kTab = 0x09,
		kPageUp = 0x0b,
		kPageDown = 0x0c,
		kReturn = 0x0d,
		kEscape = 0x1b,
		kArrowLeft = 0x1c,
		kArrowRight = 0x1d,
		kArrowUp = 0x1e,
		kArrowDown = 0x1f,
		kDelete = 0x7f,
		kMacRomanChar = 0xff,
	};

	bool _onDown : 1;
	bool _onUp : 1;
	bool _onRepeat : 1;
	bool _keyModControl : 1;
	bool _keyModCommand : 1;
	bool _keyModOption : 1;
	KeyCodeType _keyCodeType;
	uint8_t _macRomanChar;

	MessengerSendSpec _sendSpec;
};

class GraphicModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::GraphicModifier &data);

	enum InkMode {
		kInkModeCopy = 0x0,
		kInkModeTransparent = 0x1,				// src*dest
		kInkModeGhost = 0x3,					// (1-src)+dest
		kInkModeReverseCopy = 0x4,				// 1-src
		kInkModeReverseGhost = 0x7,				// src+dest
		kInkModeReverseTransparent = 0x9,		// (1-src)*dest
		kInkModeBlend = 0x20,					// (src*bgcolor)+(dest*(1-bgcolor)
		kInkModeBackgroundTransparent = 0x24,	// BG color is transparent
		kInkModeChameleonDark = 0x25,			// src+dest
		kInkModeChameleonLight = 0x27,			// src*dest
		kInkModeBackgroundMatte = 0x224,		// BG color is transparent and non-interactive
		kInkModeInvisible = 0xffff,				// Not drawn, but interactive
	};

	enum Shape {
		kShapeRect = 0x1,
		kShapeRoundedRect = 0x2,
		kShapeOval = 0x3,
		kShapePolygon = 0x9,
		kShapeStar = 0xb,	// 5-point star, horizontal arms are at (top+bottom*2)/3
	};

private:
	Event _applyWhen;
	Event _removeWhen;
	InkMode _inkMode;
	Shape _shape;

	ColorRGB8 _foreColor;
	ColorRGB8 _backColor;
	uint16 _borderSize;
	ColorRGB8 _borderColor;
	uint16 _shadowSize;
	ColorRGB8 _shadowColor;

	Common::Array<Point16> _polyPoints;
};

class BooleanVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data);

private:
	bool _value;
};

class PointVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::PointVariableModifier &data);

private:
	Point16 _value;
};

}	// End of namespace MTropolis

#endif
