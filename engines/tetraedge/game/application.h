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

#ifndef TETRAEDGE_GAME_APPLICATION_H
#define TETRAEDGE_GAME_APPLICATION_H

#include "common/str.h"
#include "common/ptr.h"

#include "tetraedge/game/bonus_menu.h"
#include "tetraedge/game/credits.h"
#include "tetraedge/game/global_bonus_menu.h"
#include "tetraedge/game/main_menu.h"
#include "tetraedge/game/options_menu.h"
#include "tetraedge/game/loc_file.h"
#include "tetraedge/game/owner_error_menu.h"
#include "tetraedge/game/splash_screens.h"
#include "tetraedge/game/in_game_scene.h"
#include "tetraedge/game/upsell_screen.h"

#include "tetraedge/te/te_visual_fade.h"
#include "tetraedge/te/te_music.h"
#include "tetraedge/te/te_xml_gui.h"
#include "tetraedge/te/te_font3.h"

namespace Common {
struct Event;
}

namespace Tetraedge {

class Application {
public:
	Application();
	~Application();

	void create();
	void destroy();

	void startGame(bool newGame, int difficulty);
	void resume();
	bool run();
	void suspend();
	void showNoCellIcon(bool show);
	void showLoadingIcon(bool show);
	void saveCorrupted(const Common::String &fname);

	void performRender();
	//void preloadTextrue(); does nothing

	void fade();
	void blackFade();
	void captureFade();
	bool isFading();

	bool isLockCursor();
	bool isLockPad();
	void lockCursor(bool lock);
	void lockCursorFromAction(bool lock);

	void loadOptions(const Common::String &fname);
	void saveOptions(const Common::String &fname);

	void getSavegameThumbnail(Graphics::Surface &thumb);

	Common::String getHelpText(const Common::String &key);

	BonusMenu &bonusMenu() { return _bonusMenu; }
	GlobalBonusMenu &globalBonusMenu() { return _globalBonusMenu; }
	MainMenu &mainMenu() { return _mainMenu; }
	OptionsMenu &optionsMenu() { return _optionsMenu; }
	TeMusic &music() { return _music; }
	Credits &credits() { return _credits; }
	UpsellScreen &upsellScreen() { return _upsellScreen; }
	TeVisualFade &visualFade() { return _visFade; }
	TeSpriteLayout &appSpriteLayout() { return _appSpriteLayout; }
	TeSpriteLayout &mouseCursorLayout() { return _mouseCursorLayout; }
	const Common::String getVersionString() const { return _versionString; }
	TeLayout &getMainWindow() { return _mainWindow; }
	void setTutoActivated(bool val) { _tutoActivated = val; }
	TeCamera *mainWindowCamera() { return _mainWindowCamera.get(); }
	Common::Array<Common::String> &unrecalAnims() { return _unrecalAnims; }
	int &difficulty() { return _difficulty; }
	bool &tutoActivated() { return _tutoActivated; }

	void setFinishedGame(bool val) { _finishedGame = val; }
	void setFinishedFremium(bool val) { _finishedFremium = val; }
	const Common::String &firstWarpPath() { return _firstWarpPath; }
	const Common::String &firstZone() { return _firstZone; }
	const Common::String &firstScene() { return _firstScene; }
	TeLayout &frontLayout() { return _frontLayout; };
	TeLayout &frontOrientationLayout() { return _frontOrientationLayout; }
	TeLayout &backLayout() { return _backLayout; }
	LocFile &loc() { return _loc; }
	bool ratioStretched() const { return _ratioStretched; }

private:
	void drawBack();
	void drawFront();

	const char *inAppUnlockFullVersionID();

	bool onBlackFadeAnimationFinished();
	bool onMainWindowSizeChanged();
	bool onMousePositionChanged(const Common::Point &p);

	bool _finishedGame;
	bool _finishedFremium;

	TeVisualFade _visFade;
	TeMusic _music;
	TeSpriteLayout _appSpriteLayout;
	TeSpriteLayout _mouseCursorLayout;
	TeSpriteLayout _autoSaveIcon1;
	TeSpriteLayout _autoSaveIcon2;

	TeButtonLayout _lockCursorButton;
	TeButtonLayout _lockCursorFromActionButton;

	TeLayout _frontLayout;
	TeLayout _frontOrientationLayout;
	TeLayout _backLayout;

	LocFile _loc;

	Common::String _applicationTitle;
	Common::String _versionString;
	Common::String _firstWarpPath;
	Common::String _firstZone;
	Common::String _firstScene;

	Common::Array<Common::String> _unrecalAnims;

	TeCurveAnim2<Te3DObject2, TeColor> _autoSaveIconAnim1;
	TeCurveAnim2<Te3DObject2, TeColor> _autoSaveIconAnim2;

	Common::SharedPtr<TeCamera> _mainWindowCamera; // TODO: should be part of TeMainWindow.
	TeLayout _mainWindow; // TODO: should be a specialised class.

	GlobalBonusMenu _globalBonusMenu;
	BonusMenu _bonusMenu;
	MainMenu _mainMenu;
	OptionsMenu _optionsMenu;
	Credits _credits;
	OwnerErrorMenu _ownerErrorMenu;
	SplashScreens _splashScreens;
	UpsellScreen _upsellScreen;

	TeIntrusivePtr<TeFont3> _fontComic;
	TeIntrusivePtr<TeFont3> _fontArgh;
	TeIntrusivePtr<TeFont3> _fontArial;
	TeIntrusivePtr<TeFont3> _fontChaucer;
	TeIntrusivePtr<TeFont3> _fontColaborate;
	TeIntrusivePtr<TeFont3> _fontProDisplay;

	bool _captureFade;
	bool _created;
	bool _tutoActivated;
	bool _drawShadows;
	bool _ratioStretched;

	int _difficulty;

	TeXmlGui _helpGui;
	static bool _dontUpdateWhenApplicationPaused;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_APPLICATION_H
