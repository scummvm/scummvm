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

#include "common/serializer.h"

#include "tetraedge/te/te_timer.h"
#include "tetraedge/te/te_music.h"
#include "tetraedge/te/te_lua_gui.h"

namespace Tetraedge {

class Dialog2 : public TeLayout {
public:
	Dialog2();

	struct DialogData {
		Common::String _name;
		Common::String _stringVal;
		Common::Path _sound;
		Common::String _charname;
		Common::String _animfile;
		float _animBlend;
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

	void pushDialog(const Common::String &name, const Common::String &textVal, const Common::String &sound, int param_4);
	void pushDialog(const Common::String &name, const Common::String &textVal, const Common::String &sound,
					const Common::String &charName, const Common::String &animFile, float animBlend);
	//void saveToBackup(TiXmlNode *node)
	void startDownAnimation();
	void unload();

	TeLuaGUI &gui() { return _gui; }
	TeSignal1Param<const Common::String &> &onAnimationDownFinishedSignal() { return _onAnimationDownFinishedSignal; }

private:
	Common::Array<DialogData> _dialogs;

	TeTimer _minimumTimeTimer;

	TeLuaGUI _gui;
	TeMusic _music;

	DialogData _currentDialogData;

	TeSignal1Param<const Common::String &> _onAnimationDownFinishedSignal;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_DIALOG2_H
