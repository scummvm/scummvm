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

#include "ripper/combat/atkini.h"

namespace Ripper {

namespace {

static const CombatEncounterDefinition kAtkiniDefinition = {
	"atkini",
	"atkini%u.ini",
	"atkini1.ini",
	"weap2c1.wav",
	{ nullptr, nullptr },
	"mechwav1.wav",
	"mechwav3.wav",
	"mechwav2.wav",
	{ "atkexp0.pl", "atkexp1.pl" },
	"atkcr",
	0x1ba,
	true
};

} // End of anonymous namespace

AtkiniEncounter::AtkiniEncounter(RipperEngine *engine) :
		CombatEncounter(engine, kAtkiniDefinition) {
}

} // End of namespace Ripper
