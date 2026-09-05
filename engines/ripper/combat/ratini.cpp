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

#include "ripper/combat/ratini.h"

namespace Ripper {

namespace {

static const CombatEncounterDefinition kRatiniDefinition = {
	"ratini",
	"ratini%u.ini",
	"ratini1.ini",
	"",
	{ "loop3.wav", "loop4.wav" },
	"",
	"mechwav3.wav",
	"mechwav2.wav",
	{ "ratexp0.pl", "ratexp1.pl" },
	"ratcr",
	0x1ba,
	false
};

} // End of anonymous namespace

RatiniEncounter::RatiniEncounter(RipperEngine *engine) :
		CombatEncounter(engine, kRatiniDefinition) {
}

} // End of namespace Ripper
