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

#ifndef BLADERUNNER_KIA_SECTION_SUSPECTS_H
#define BLADERUNNER_KIA_SECTION_SUSPECTS_H

#include "bladerunner/ui/kia_section_base.h"

#include "common/array.h"

namespace BladeRunner {

class ActorClues;
class BladeRunnerEngine;
class Shape;
class UICheckBox;
class UIContainer;
class UIImagePicker;
class UIScrollBox;

class KIASectionSuspects : public KIASectionBase {
	// _vm->_gameInfo->getClueCount()
	static const int kClueCount = 288;

	struct AcquiredClue {
		int clueId;
		int actorId;
	};

	bool           _isOpen;

	UIContainer     *_uiContainer;
	UIImagePicker   *_buttons;
	UIScrollBox     *_cluesScrollBox;
	UIScrollBox     *_crimesScrollBox;
	UICheckBox      *_whereaboutsCheckBox;
	UICheckBox      *_MOCheckBox;
	UICheckBox      *_replicantCheckBox;
	UICheckBox      *_nonReplicantCheckBox;
	UICheckBox      *_othersCheckBox;

	bool _whereaboutsFilter;
	bool _MOFilter;
	bool _replicantFilter;
	bool _nonReplicantFilter;
	bool _othersFilter;

	ActorClues    *_clues;

	int            _acquiredClueCount;
	AcquiredClue   _acquiredClues[kClueCount];

	int                 _suspectSelected;
	int                 _suspectsFoundCount;
	Common::Array<bool> _suspectsFound;
	Common::Array<bool> _suspectsWithIdentity;

	int   _mouseX;
	int   _mouseY;

	int    _suspectPhotoShapeId;
	int    _suspectPhotoNotUsed;
	Shape *_suspectPhotoShape;

public:
	int                 _crimeSelected;

public:
	KIASectionSuspects(BladeRunnerEngine *vm, ActorClues *clues);
	~KIASectionSuspects();

	void reset();

	void open() override;
	void close() override;

	void draw(Graphics::Surface &surface) override;

	void handleMouseMove(int mouseX, int mouseY) override;
	void handleMouseDown(bool mainButton) override;
	void handleMouseUp(bool mainButton) override;
	void handleMouseScroll(int direction) override;

	void saveToLog();
	void loadFromLog();

	void selectSuspect(int suspectId);

private:
	static void scrollBoxCallback(void *callbackData, void *source, int lineData, int mouseButton);
	static void checkBoxCallback(void *callbackData, void *source);
	static void mouseUpCallback(int buttonId, void *callbackData);

	void onButtonPressed(int buttonId);

	void populateAcquiredClues();
	void populateSuspects();
	void populateCrimes();
	void populateVisibleClues();
	void updateSuspectPhoto();

	void nextSuspect();
	void prevSuspect();

	void enableAllFilters();
	void disableAllFilters();
};

} // End of namespace BladeRunner

#endif
