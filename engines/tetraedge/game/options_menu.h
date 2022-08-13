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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TETRAEDGE_GAME_OPTIONS_MENU_H
#define TETRAEDGE_GAME_OPTIONS_MENU_H

#include "tetraedge/te/te_lua_gui.h"
#include "tetraedge/te/te_music.h"

namespace Tetraedge {

class OptionsMenu : public TeLuaGUI {
public:
	OptionsMenu();

	void enter() override;
	void leave() override;

	bool onCloseTuto();
	bool onCreditsButton();
	bool onDialogVolumeMinusButton();
	bool onDialogVolumePlusButton();
	bool onMusicVolumeMinusButton();
	bool onMusicVolumePlusButton();
	bool onPrivacyPolicyButton();
	bool onQuitButton();
	bool onSFXVolumeMinusButton();
	bool onSFXVolumePlusButton();
	bool onSupportButton();
	bool onTermsOfServiceButton();
	bool onVideoVolumeMinusButton();
	bool onVideoVolumePlusButton();
	bool onVisibleTuto();
	bool onVisibleTutoNextPage();

	void updateDialogVolumeJauge();
	void updateMusicVolumeJauge();
	void updateSFXVolumeJauge();
	void updateVideoVolumeJauge();

private:

	//  TODO: work out virtual thing here TeLuaGUI _gui2;

	TeMusic _music1;
	TeMusic _music2;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_OPTIONS_MENU_H
