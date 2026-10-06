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

#ifndef NANCY_STATE_DESIGNSELECT_H
#define NANCY_STATE_DESIGNSELECT_H

#include "common/ptr.h"
#include "common/singleton.h"

#include "engines/nancy/state/state.h"
#include "engines/nancy/renderobject.h"
#include "engines/nancy/ui/fullscreenimage.h"
#include "engines/nancy/ui/button.h"

namespace Nancy {

struct LDSN;

namespace State {

// Nancy15 "Design Select": picks the look Nancy's UI wears. Every design is a
// CIF tree of its own (PUI_CRE_Nancy_Default, _Jungle, _Pink_Hibiscus, ...);
// the screen lists whichever ones ship with the game and hands the chosen one
// to Scene::changePlayerCharacterDesign(). Opened from the setup menu.
class DesignSelect : public State, public Common::Singleton<DesignSelect> {
public:
	// State API
	void process() override;
	void onStateEnter(const NancyState::NancyState prevState) override;
	bool onStateExit(const NancyState::NancyState nextState) override;

private:
	// One selectable look, described by the PUIH chunk of its own boot IFF
	struct Design {
		Common::String name;			// CIF tree name, e.g. "PUI_CRE_Nancy_Jungle"
		Common::String themeName;		// label, e.g. "Nancy Classic Look (Default)"
		Common::Path swatchImageName;	// thumbnail shown next to the label
		Common::Rect labelDest;
		Common::Rect swatchDest;
	};

	enum State { kInit, kRun, kStop };

	void init();
	void run();
	void stop();

	void registerGraphics();

	// Finds every design tree belonging to the active character and reads its
	// PUIH chunk for the label and swatch
	void buildDesignList();
	void drawDesigns();

	// Index of the design row under the given point, or -1
	int designAt(const Common::Point &mousePos) const;

	UI::FullScreenImage _background;
	// The design names and swatches, drawn straight onto the backdrop
	RenderObject _list { 6 };
	// The swatches on their own; the names are drawn over a fresh copy of this
	// whenever the selection moves
	Graphics::ManagedSurface _listBase;
	// Sprite sheet the two buttons' artwork is cut from
	Graphics::ManagedSurface _buttonSprites;
	Common::ScopedPtr<UI::Button> _acceptButton;
	Common::ScopedPtr<UI::Button> _cancelButton;

	Common::Array<Design> _designs;
	int _selected = -1;
	bool _accepted = false;

	State _state = kInit;
	const LDSN *_designData = nullptr;
};

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_DESIGNSELECT_H
