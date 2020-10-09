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

#ifndef MYST_SCRIPTS_MENU_H
#define MYST_SCRIPTS_MENU_H

#include "mohawk/myst_scripts.h"

#include "common/scummsys.h"
#include "common/util.h"

#include "graphics/font.h"

namespace Mohawk {

class MystAreaVideo;
struct MystScriptEntry;

namespace MystStacks {

#define DECLARE_OPCODE(x) void x(uint16 var, const ArgumentsArray &args)

class Menu : public MystScriptParser {
public:
	explicit Menu(MohawkEngine_Myst *vm);
	~Menu() override;

	void setInGame(bool inGame) {
		_inGame = inGame;
	}

	void setCanSave(bool canSave) {
		_canSave = canSave;
	}

	void disablePersistentScripts() override;
	void runPersistentScripts() override;

private:
	void setupOpcodes();
	uint16 getVar(uint16 var) override;

	DECLARE_OPCODE(o_playIntroMovies);
	DECLARE_OPCODE(o_menuItemEnter);
	DECLARE_OPCODE(o_menuItemLeave);
	DECLARE_OPCODE(o_menuResume);
	DECLARE_OPCODE(o_menuLoad);
	DECLARE_OPCODE(o_menuSave);
	DECLARE_OPCODE(o_menuNew);
	DECLARE_OPCODE(o_menuOptions);
	DECLARE_OPCODE(o_menuQuit);
	DECLARE_OPCODE(o_menuInit);
	DECLARE_OPCODE(o_menuExit);

	bool _inGame;
	bool _canSave;
	bool _menuItemHovered[6];
	bool _wasCursorVisible;

	bool _introMoviesRunning;
	int _introStep;
	void introMovies_run();

	bool showConfirmationDialog(const Common::U32String &message, const Common::U32String &confirmButton, const Common::U32String &cancelButton);

	void drawButtonImages(const Common::U32String &text, MystAreaImageSwitch *area, Graphics::TextAlign align, uint16 highlightedIndex, uint16 disabledIndex) const;
	void replaceButtonSubImageWithText(const Common::U32String &text, const Graphics::TextAlign &align, MystAreaImageSwitch *area,
	                                   uint16 subimageIndex, const Common::Rect &backgroundRect, int16 deltaY,
	                                   uint8 r, uint8 g, uint8 b) const;
	const char **getButtonCaptions() const;
	void resetButtons();

	PauseToken _pauseToken;

};

} // End of namespace MystStacks
} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
