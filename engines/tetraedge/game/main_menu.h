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

#ifndef TETRAEDGE_GAME_MAIN_MENU_H
#define TETRAEDGE_GAME_MAIN_MENU_H

#include "common/str.h"
#include "tetraedge/game/confirm.h"
#include "tetraedge/te/te_signal.h"

namespace Tetraedge {

class MainMenu : public TeLuaGUI {
public:
	MainMenu();

	void enter() override;
	void leave() override;

	bool deleteFile(const Common::String &name);
	bool onActivedTuto();
	bool onBFGFreeGamesButtonValidated() { return false; }
	bool onBFGRateIt2ButtonValidated();
	bool onBFGRateItButtonValidated();
	bool onBFGRateItQuitButtonValidated();
	bool onBFGSplashButtonUpdated() { return false; }
	bool onBFGSplashButtonValidated() { return false; }
	bool onBFGTellAFriendButtonValidated() { return false; }
	bool onBFGUnlockGameButtonValidated();
	bool onContinueGameButtonValidated();
	bool onDisabledTuto();
	bool onEnterGameRotateAnimFinished();
	bool onFacebookButtonValidated() { return false; }
	bool onFacebookLogged() { return false; }
	bool onGalleryButtonValidated();
	bool onHowToButtonValidated();
	bool onLoadGameButtonValidated();
	bool onNewGameButtonValidated();
	bool onNewGameConfirmed();
	bool onOptionsButtonValidated();
	bool onQuit();
	bool onQuitButtonValidated();
	bool onUnlockGameButtonValidated();
	bool onWalkThroughButtonValidated() { return false; };

	void refresh();
	void setCenterButtonsVisibility(bool visible);

private:

	void tryDisableButton(const Common::String &btnName);

	Confirm _newGameConfirm;
	Confirm _tutoConfirm;
	Confirm _quitConfirm;

	TeSignal0Param onFacebookLoggedSignal;

	bool _entered;
	bool _confirmingTuto;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_MAIN_MENU_H
