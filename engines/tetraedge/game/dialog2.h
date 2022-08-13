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

#ifndef TETRAEDGE_GAME_DIALOG2_H
#define TETRAEDGE_GAME_DIALOG2_H

#include "tetraedge/te/te_timer.h"
#include "tetraedge/te/te_music.h"
#include "tetraedge/te/te_lua_gui.h"

namespace Tetraedge {

class Dialog2 : public TeLayout {
public:
	Dialog2();

	class DialogData {
		bool operator=(const DialogData &other);
	};

	bool isDialogPlaying();
	void launchNextDialog();
	void load();
	void loadFromBackup(); // seems to do nothing useful? just iterates the children..
	bool onAnimationDownFinished();
	bool onAnimationUpFinished();
	bool onMinimumTimeTimer();
	bool onSkipButton();
	bool onSoundFinished();

	void pushDialog(const Common::String &param_1, const Common::String &param_2, const Common::String &param_3, int param_4);
	void pushDialog(const Common::String &param_1, const Common::String &param_2, const Common::String &param_3,
					const Common::String &param_4, const Common::String &param_5, float param_6);
	//void saveToBackup(TiXmlNode *node)
	void startDownAnimation();
	void unload();

	TeLuaGUI &gui() { return _gui; }

	Common::String prevSceneName() { return _prevSceneName; };

private:
	TeTimer _minimumTimeTimer;

	Common::String _prevSceneName;
	Common::String _animDownFinishedResultString;

	TeLuaGUI _gui;
	TeMusic _music;

	TeSignal1Param<const Common::String &> _onAnimationDownFinishedSignal;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_DIALOG2_H
