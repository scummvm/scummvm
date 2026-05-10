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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_UI_NOTEBOOKPOPUP_H
#define NANCY_UI_NOTEBOOKPOPUP_H

#include "engines/nancy/renderobject.h"

namespace Nancy {

struct NancyInput;
struct UINB;

namespace UI {

// Nancy 10+ notebook popup. Driven by the UINB chunk: overlay image + two
// tab buttons.
class NotebookPopup : public RenderObject {
public:
	NotebookPopup();
	~NotebookPopup() override = default;

	void init() override;
	void registerGraphics() override;
	void handleInput(NancyInput &input);

	bool isOpen() const { return _isOpen; }
	void open();
	void close();
	void toggle() { if (_isOpen) close(); else open(); }

private:
	static const uint kNumTabs = 2;

	void drawBackground();
	void drawTabs();

	const UINB *_uinbData;

	Graphics::ManagedSurface _overlayImage; // popup background image

	bool _isOpen;
	int _activeTab; // 0..1, matching UINB::tabs index
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NOTEBOOKPOPUP_H
