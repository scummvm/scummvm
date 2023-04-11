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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_INPUT_TYPES_H
#define HPL_INPUT_TYPES_H

namespace hpl {

//-------------------------------------------------

enum eInputDeviceType {
	eInputDeviceType_Keyboard,
	eInputDeviceType_Mouse,
	eInputDeviceType_Gamepad,
	eInputDeviceType_LastEnum
};

//-------------------------------------------------

enum eMButton {
	eMButton_Left,
	eMButton_Middle,
	eMButton_Right,
	eMButton_WheelUp,
	eMButton_WheelDown,
	eMButton_6,
	eMButton_7,
	eMButton_8,
	eMButton_9,
	eMButton_LastEnum
};

} // namespace hpl

#endif // HPL_INPUT_TYPES_H
