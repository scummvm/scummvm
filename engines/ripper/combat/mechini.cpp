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

#include "ripper/combat/mechini.h"

namespace Ripper {

namespace {

static const CombatEncounterDefinition kMechiniDefinition = {
	"mechini",
	"mechini%u.ini",
	"mechwav0.wav",
	"mechwav1.wav",
	"mechwav3.wav",
	"mechwav2.wav",
	{ "mechexp0.pl", "mechexp1.pl" },
	"crshr",
	0x1a5
};

} // End of anonymous namespace

MechiniEncounter::MechiniEncounter(RipperEngine *engine) :
		CombatEncounter(engine, kMechiniDefinition) {
}

} // End of namespace Ripper
