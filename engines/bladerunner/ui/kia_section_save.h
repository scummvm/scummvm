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

#ifndef BLADERUNNER_KIA_SECTION_SAVE_H
#define BLADERUNNER_KIA_SECTION_SAVE_H

#include "bladerunner/ui/kia_section_base.h"

#include "common/scummsys.h"
#include "common/str.h"

#include "engines/savestate.h"

namespace Graphics {
struct Surface;
}

namespace BladeRunner {

class UIContainer;
class UIScrollBox;
class UIInputBox;
class UIImagePicker;

class KIASectionSave : public KIASectionBase {
	enum State {
		kStateNormal    = 0,
		kStateOverwrite = 1,
		kStateDelete    = 2
	};

	UIContainer   *_uiContainer;
	UIScrollBox   *_scrollBox;
	UIInputBox    *_inputBox;
	UIImagePicker *_buttons;

	uint32        _timeLast;
	uint32        _timeLeft;

	SaveStateList _saveList;

	State         _state;

	int           _mouseX;
	int           _mouseY;

	int           _hoveredLineId;
	int           _displayingLineId;
	int           _selectedLineId;
	int           _newSaveLineId;

public:
	KIASectionSave(BladeRunnerEngine *vm);
	~KIASectionSave() override;

	void open() override;
	void close() override;

	void draw(Graphics::Surface &surface) override;

	void handleKeyUp(const Common::KeyState &kbd) override;
	void handleKeyDown(const Common::KeyState &kbd) override;
	void handleMouseMove(int mouseX, int mouseY) override;
	void handleMouseDown(bool mainButton) override;
	void handleMouseUp(bool mainButton) override;
	void handleMouseScroll(int direction) override;

private:
	static void scrollBoxCallback(void *callbackData, void *source, int lineData, int mouseButton);
	static void inputBoxCallback(void *callbackData, void *source);

	static void onButtonPressed(int buttonId, void *callbackData);

	void changeState(State state);
	void save();
	void deleteSave();
};

} // End of namespace BladeRunner

#endif
