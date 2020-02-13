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

#ifndef BLADERUNNER_KIA_SECTION_CLUES_H
#define BLADERUNNER_KIA_SECTION_CLUES_H

#include "bladerunner/ui/kia_section_base.h"

#include "common/array.h"
#include "common/str.h"

namespace BladeRunner {

class ActorClues;
class UIContainer;
class UIImagePicker;
class UIScrollBox;

class KIASectionClues : public KIASectionBase {
	static const int kClueCount = 288;

	struct Line {
		Common::String crimeName;
		int lineData;
		int flags;
	};

	UIContainer        *_uiContainer;
	UIImagePicker      *_buttons;
	UIScrollBox        *_cluesScrollBox;
	UIScrollBox        *_filterScrollBox;

	bool                _isOpen;
	bool                _debugIntangible;
	int                 _debugNop;
	ActorClues         *_clues;

	int                 _assetTypeFilterCount;
	int                 _crimeFilterCount;
	int                 _filterCount;
	Common::Array<bool> _filters;

	int                 _mouseX;
	int                 _mouseY;

public:
	KIASectionClues(BladeRunnerEngine *vm, ActorClues *clues);
	~KIASectionClues() override;

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

private:
	static void scrollBoxCallback(void *callbackData, void *source, int lineData, int mouseButton);
	static void mouseUpCallback(int buttonId, void *callbackData);

	void onButtonPressed(int buttonId) override;

	void enableAllFilters();
	void disableAllFilters();

	void populateFilters();
	void populateClues();

	int getClueFilterTypeTextId(int);
	int getClueFilterCrimeId(int);
	int getLineIdForAssetType(int assetType);
	int getLineIdForCrimeId(int crimeId);
};

} // End of namespace BladeRunner

#endif
