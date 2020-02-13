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

#ifndef BLADERUNNER_KIA_SECTION_LOAD_H
#define BLADERUNNER_KIA_SECTION_LOAD_H

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

class KIASectionLoad : public KIASectionBase {
	UIContainer  *_uiContainer;
	UIScrollBox  *_scrollBox;

	uint32        _timeLast;
	uint32        _timeLeft;

	SaveStateList _saveList;

	int           _hoveredLineId;
	int           _displayingLineId;
	int           _newGameEasyLineId;
	int           _newGameMediumLineId;
	int           _newGameHardLineId;

public:
	KIASectionLoad(BladeRunnerEngine *vm);
	~KIASectionLoad() override;

	void open() override;
	void close() override;

	void draw(Graphics::Surface &surface) override;

	void handleMouseMove(int mouseX, int mouseY) override;
	void handleMouseDown(bool mainButton) override;
	void handleMouseUp(bool mainButton) override;
	void handleMouseScroll(int direction) override;

private:
	static void scrollBoxCallback(void *callbackData, void *source, int lineData, int mouseButton);
};

} // End of namespace BladeRunner

#endif
