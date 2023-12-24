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

#ifndef TETRAEDGE_GAME_NOTIFIER_H
#define TETRAEDGE_GAME_NOTIFIER_H

#include "common/str.h"

namespace Tetraedge {

class Notifier {
public:
	Notifier();

	void launchNextnotifier();
	void load();
	bool onFadeInFinished();
	bool onFadeOutFinished();
	bool onVisibleFinished();

	void push(const Common::String &name, const Common::Path &imgpath);
	void unload();

	TeLuaGUI &gui() { return _gui; }

private:
	struct notifierData {
		Common::String _name;
		Common::Path _imgpath;
	};
	Common::Array<notifierData> _notifierDataArray;
	TeLuaGUI _gui;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_NOTIFIER_H
