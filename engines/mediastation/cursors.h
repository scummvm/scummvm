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

#include "common/platform.h"
#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/macresman.h"
#include "common/formats/winexe.h"
#include "graphics/wincursor.h"
#include "graphics/maccursor.h"

#include "mediastation/clients.h"
#include "mediastation/datafile.h"

namespace MediaStation {

enum CursorManagerCommandType {
	kCursorManagerInit = 0x0c,
	kCursorManagerNewCursor = 0x15,
	kCursorManagerDisposeCursor = 0x16,
};

enum CursorType {
	kPlatformCursor = 0,
	kResourceCursor = 1,
};

// Media Station stores cursors in the executable as named resources.
class CursorManager : public ParameterClient {
public:
	CursorManager(const Common::Path &appName) : _appName(appName) {}
	virtual ~CursorManager();

	virtual bool attemptToReadFromStream(Chunk &chunk, uint param) override;
	void init(Chunk &chunk);
	void newCursor(Chunk &chunk);
	void disposeCursor(Chunk &chunk);

	void newPlatformCursor(uint16 platformCursorId, uint16 cursorId);
	void newResourceCursor(uint16 cursorId, const Common::String &resourceName);
	// Some engine versions also have newBufferCursor that seems to be unused.

	void showCursor();
	void hideCursor();

	virtual void resetCurrent();
	void registerAsPermanent(uint16 id);
	void setAsPermanent(uint16 id);
	void setAsTemporary(uint16 id);
	void unsetPermanent();
	void unsetTemporary();

protected:
	Common::Path _appName;

	uint16 _baseCursorId = 0;
	uint16 _maxCursorId = 0;
	uint16 _currentCursorId = 0;
	uint16 _permanentCursorId = 0;

	// The original used an array with computed indices, but we use a hashmap for simplicity.
	Common::HashMap<uint16, Graphics::Cursor *> _cursors;

	virtual Graphics::Cursor *loadResourceCursor(const Common::String &name) = 0;
	void setDefaultCursor();
};

class WindowsCursorManager : public CursorManager {
public:
	WindowsCursorManager(const Common::Path &appName);
	~WindowsCursorManager() override;

	virtual Graphics::Cursor *loadResourceCursor(const Common::String &name) override;

private:
	Common::HashMap<Common::String, Graphics::WinCursorGroup *> _cursorGroups;
};

class MacCursorManager : public CursorManager {
public:
	explicit MacCursorManager(const Common::Path &appName);
	~MacCursorManager() override;

	virtual Graphics::Cursor *loadResourceCursor(const Common::String &name) override;

private:
	Common::MacResManager *_resFork;
};

} // End of namespace MediaStation

#endif
