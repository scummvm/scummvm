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

#ifndef RIPPER_COMBAT_MECHINI_H
#define RIPPER_COMBAT_MECHINI_H

#include "ripper/combat/combat.h"

namespace Ripper {

class MechiniEncounter : public CombatEncounter {
public:
	explicit MechiniEncounter(RipperEngine *engine);
};

} // End of namespace Ripper

#endif // RIPPER_COMBAT_MECHINI_H
