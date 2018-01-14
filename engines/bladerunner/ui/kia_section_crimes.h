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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_KIA_SECTION_CRIME_H
#define BLADERUNNER_KIA_SECTION_CRIME_H

#include "bladerunner/shape.h"
#include "bladerunner/ui/kia_section_base.h"

namespace BladeRunner {

class ActorClues;
class BladeRunnerEngine;
class UIContainer;
class UIImagePicker;
class UIScrollBox;

class KIASectionCrimes : public KIASectionBase {
	struct Clue
	{
		int clueId;
		int actorId;
	};

protected:

	UIContainer *_uiContainer;
	// void (__cdecl **scrollboxCallback)(int, UIScrollBoxStruc *, int, int);
	int _isOpen;
	int *_list;
	UIImagePicker *_uiImagePicker;

	UIScrollBox *_cluesScrollbox;
	ActorClues *_clues;
	int _clueInfoCount;
	Clue _clueInfo[1000];
	int _selectedCrimeId;
	int _crimesFoundCount;
	int *_crimeIsKnown;
	int _selectedSuspectId;
	int _suspectsFoundCount;
	int *_suspectIsKnown;
	int *_suspectHasIdentity;
	int _mouseX;
	int _mouseY;
	int _suspectPhotoShapeId;
	int _field_15;
	Shape _shape;

public:
	KIASectionCrimes(BladeRunnerEngine *vm);
	~KIASectionCrimes();

	void saveToLog() {}
	void loadFromLog() {}

protected:

};

} // End of namespace BladeRunner

#endif
