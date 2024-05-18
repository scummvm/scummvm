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

#include "objects.h"
#include "rooms.h"
#include "stream-helper.h"

using namespace Common;

namespace Alcachofa {

MenuButton::MenuButton(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, _actionId(stream.readSint32LE())
	, _graphicNormal(stream)
	, _graphicHovered(stream)
	, _graphicClicked(stream)
	, _graphicDisabled(stream) {
}

InternetMenuButton::InternetMenuButton(Room *room, ReadStream &stream)
	: MenuButton(room, stream) {
}

OptionsMenuButton::OptionsMenuButton(Room *room, ReadStream &stream)
	: MenuButton(room, stream) {
}

MainMenuButton::MainMenuButton(Room *room, ReadStream &stream)
	: MenuButton(room, stream) {
}

PushButton::PushButton(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, _alwaysVisible(readBool(stream))
	, _graphic1(stream)
	, _graphic2(stream)
	, _actionId(stream.readSint32LE()) {
}

EditBox::EditBox(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, i1(stream.readSint32LE())
	, p1(Shape(stream).firstPoint())
	, _labelId(readVarString(stream))
	, b1(readBool(stream))
	, i3(stream.readSint32LE())
	, i4(stream.readSint32LE())
	, i5(stream.readSint32LE())
	, _fontId(stream.readSint32LE()) {
}

CheckBox::CheckBox(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, b1(readBool(stream))
	, _graph1(stream)
	, _graph2(stream)
	, _graph3(stream)
	, _graph4(stream)
	, _valueId(stream.readSint32LE()) {
}

CheckBoxAutoAdjustNoise::CheckBoxAutoAdjustNoise(Room *room, ReadStream &stream)
	: CheckBox(room, stream) {
	stream.readByte(); // unused and ignored byte
}

SlideButton::SlideButton(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, i1(stream.readSint32LE())
	, p1(Shape(stream).firstPoint())
	, p2(Shape(stream).firstPoint())
	, _graph1(stream)
	, _graph2(stream)
	, _graph3(stream) {
}

IRCWindow::IRCWindow(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _p1(Shape(stream).firstPoint())
	, _p2(Shape(stream).firstPoint()) {
}

MessageBox::MessageBox(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _graph1(stream)
	, _graph2(stream)
	, _graph3(stream)
	, _graph4(stream)
	, _graph5(stream) {
	_graph1.start(true);
	_graph2.start(true);
	_graph3.start(true);
	_graph4.start(true);
	_graph5.start(true);
}

VoiceMeter::VoiceMeter(Room *room, ReadStream &stream)
	: GraphicObject(room, stream) {
	stream.readByte(); // unused and ignored byte
}

}
