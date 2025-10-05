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

#ifndef MEDIASTATION_CURSORS_H
#define MEDIASTATION_CURSORS_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/macresman.h"
#include "common/formats/winexe.h"
#include "graphics/wincursor.h"

namespace MediaStation {

// Media Station stores cursors in the executable as named resources.
class CursorManager {
public:
	CursorManager() {}
	virtual ~CursorManager() {}

	virtual void showCursor();
	virtual void hideCursor();

	virtual void setCursor(const Common::String &name) = 0;

protected:
	virtual void setDefaultCursor();
};

class WindowsCursorManager : public CursorManager {
public:
	WindowsCursorManager(const Common::Path &appName);
	~WindowsCursorManager() override;

	virtual void setCursor(const Common::String &name) override;

protected:
	void loadCursors(const Common::Path &appName);

private:
	Common::HashMap<Common::String, Graphics::WinCursorGroup *> _cursors;
};

class MacCursorManager : public CursorManager {
public:
	explicit MacCursorManager(const Common::Path &appName);
	~MacCursorManager() override;

	virtual void setCursor(const Common::String &name) override;

private:
	Common::MacResManager *_resFork;
};

} // End of namespace MediaStation

#endif
