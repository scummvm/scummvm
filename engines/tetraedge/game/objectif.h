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

#ifndef TETRAEDGE_GAME_OBJECTIF_H
#define TETRAEDGE_GAME_OBJECTIF_H

#include "tetraedge/te/te_vector2f32.h"
#include "tetraedge/te/te_lua_gui.h"

namespace Tetraedge {

class Objectif {
public:
	struct Task {
		Common::String _headTask;
		Common::String _subTask;
		bool _taskFlag;
	};

	Objectif();

	void createChildLayout(TeLayout *layout, Common::String const &taskId, bool isSubTask);
	void enter();
	bool hideBouton();
	bool isMouseIn(const TeVector2s32 &mousept);
	bool isVisibleObjectif();
	void deleteObjectif(Common::String const &head, Common::String const &sub);
	void leave();
	void load();
	bool onHelpButtonValidated();
	void pushObjectif(Common::String const &head, Common::String const &sub);
	void reattachLayout(TeLayout *layout);
	void removeChildren();
	// void save()
	void setVisibleButtonHelp(bool visible);
	void setVisibleObjectif(bool visible);
	void unload();
	void update();

	TeLuaGUI &gui1() { return _gui1; }

private:
	TeLuaGUI _gui1;
	TeLuaGUI _gui2;
	Common::Array<Task> _tasks;
	bool _helpButtonVisible;

	static bool _layoutsDirty;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_OBJECTIF_H
