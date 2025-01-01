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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/path.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/splash_screens.h"
#include "tetraedge/te/te_core.h"

namespace Tetraedge {

SplashScreens::SplashScreens() : _splashNo(0), _entered(false) {
	_timer.alarmSignal().add(this, &SplashScreens::onAlarm);
}

void SplashScreens::enter()	{
	if (!_entered) {
		_entered = true;
		_splashNo = 0;
		const char *scriptStr = g_engine->gameIsAmerzone() ? "GUI/PC-MacOSX/Splash0.lua" : "menus/splashes/splash0.lua";
		TetraedgeFSNode node = g_engine->getCore()->findFile(scriptStr);
		if (node.exists()) {
			load(node);
			Application *app = g_engine->getApplication();
			TeLayout *splash = layoutChecked("splash");

			TeLayout *splashImg = dynamic_cast<TeLayout *>(splash->child(0));
			assert(splashImg);
			splashImg->setRatioMode(TeILayout::RATIO_MODE_NONE);
			splashImg->updateSize();

			app->frontLayout().addChild(splash);
			app->performRender();
		}
		onAlarm();
	}
}

bool SplashScreens::onAlarm() {
	Application *app = g_engine->getApplication();
	app->visualFade().init();
	app->captureFade();
	TeLuaGUI::unload();
	const char *scriptStr = g_engine->gameIsAmerzone() ? "GUI/PC-MacOSX/Splash%d.lua" : "menus/splashes/splash%d.lua";
	const Common::Path scriptName(Common::String::format(scriptStr, _splashNo));
	_splashNo++;

	if (ConfMan.getBool("skip_splash")) {
		onQuitSplash();
		return true;
	}

	TetraedgeFSNode node = g_engine->getCore()->findFile(scriptName);
	if (!node.exists()) {
		onQuitSplash();
	} else {
		load(node);

		TeButtonLayout *splash = buttonLayoutChecked("splash");
		splash->onMouseClickValidated().add(this, &SplashScreens::onQuitSplash);

		TeLayout *splashImg = dynamic_cast<TeLayout *>(splash->child(0));
		assert(splashImg);
		splashImg->setRatioMode(TeILayout::RATIO_MODE_NONE);
		splashImg->updateSize();

		app->frontLayout().addChild(splash);

		_timer.start();
		_timer.setAlarmIn(1500000);
	}

	app->fade();
	return true;
}

bool SplashScreens::onQuitSplash() {
	_timer.stop();
	Application *app = g_engine->getApplication();
	app->captureFade();
	TeLuaGUI::unload();
	_entered = false;
	if (!g_engine->getGame()->hasLoadName()) {
		app->mainMenu().enter();
	} else {
		app->startGame(false, 1);
	}
	app->fade();
	return false;
}

} // end namespace Tetraedge
