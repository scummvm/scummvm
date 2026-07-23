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
 */

#ifndef SCUMM_INSANE_REBEL2_SHARED_H
#define SCUMM_INSANE_REBEL2_SHARED_H

#include "common/keyboard.h"

namespace Scumm {

enum Rebel2MenuCommand {
	kRebel2MenuCommandNone,
	kRebel2MenuCommandUp,
	kRebel2MenuCommandDown,
	kRebel2MenuCommandAccept,
	kRebel2MenuCommandCancel
};

enum {
	kRebel2MenuResultCancel = -2,
	kRebel2MenuResultNone = -1
};

inline Rebel2MenuCommand getRebel2MenuCommand(const Common::KeyState &key) {
	switch (key.keycode) {
	case Common::KEYCODE_UP:
		return kRebel2MenuCommandUp;
	case Common::KEYCODE_DOWN:
		return kRebel2MenuCommandDown;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		return kRebel2MenuCommandAccept;
	case Common::KEYCODE_ESCAPE:
		return kRebel2MenuCommandCancel;
	default:
		return kRebel2MenuCommandNone;
	}
}

inline int applyRebel2MenuCommand(Rebel2MenuCommand command, int itemCount,
		int &selection) {
	if (itemCount <= 0)
		return kRebel2MenuResultNone;

	switch (command) {
	case kRebel2MenuCommandUp:
		if (--selection < 0)
			selection = itemCount - 1;
		break;
	case kRebel2MenuCommandDown:
		if (++selection >= itemCount)
			selection = 0;
		break;
	case kRebel2MenuCommandAccept:
		return selection >= 0 && selection < itemCount ? selection : kRebel2MenuResultNone;
	case kRebel2MenuCommandCancel:
		return kRebel2MenuResultCancel;
	default:
		break;
	}
	return kRebel2MenuResultNone;
}

inline bool updateRebel2Fire(bool pressed, bool wasPressed,
		bool rapidFire, bool autoFire, int16 &rapidFireCounter) {
	const bool pressedEdge = pressed && !wasPressed;
	if (pressedEdge)
		rapidFireCounter = 0;

	bool rapidFireShot = false;
	if (rapidFire) {
		rapidFireShot = pressed && rapidFireCounter % 5 == 0;
		rapidFireCounter = (rapidFireCounter + 1) % 5;
	}
	return pressedEdge || rapidFireShot || autoFire;
}

} // End of namespace Scumm

#endif
