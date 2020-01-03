/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MOHAWK_CURSORS_H
#define MOHAWK_CURSORS_H

#include "common/scummsys.h"

namespace Common {
class MacResManager;
class WinResources;
class SeekableReadStream;
class String;
}

namespace Graphics {
struct WinCursorGroup;
}

#include "mohawk/resource.h"

namespace Mohawk {

enum {
	kRivenOpenHandCursor = 2003,
	kRivenClosedHandCursor = 2004,
	kRivenMainCursor = 3000,
	kRivenPelletCursor = 5000,
	kRivenHideCursor = 9000
};

class MohawkArchive;
class MohawkEngine;

class CursorManager {
public:
	CursorManager() {}
	virtual ~CursorManager() {}

	virtual void showCursor();
	virtual void hideCursor();
	virtual void setCursor(uint16 id);
	virtual void setCursor(const Common::String &name) {}
	virtual void setDefaultCursor();
	virtual bool hasSource() const { return false; }

protected:
	// Set a Mac CURS/crsr cursor to the screen
	void setMacCursor(Common::SeekableReadStream *stream);
};

// The default Mohawk cursor manager
// Uses standard tCUR resources
class DefaultCursorManager : public CursorManager {
public:
	explicit DefaultCursorManager(MohawkEngine *vm, uint32 tag = ID_TCUR) : _vm(vm), _tag(tag) {}
	~DefaultCursorManager() override {}

	void setCursor(uint16 id) override;
	bool hasSource() const override { return true; }

private:
	MohawkEngine *_vm;
	uint32 _tag;
};

#ifdef ENABLE_MYST

// 803-805 are animated, one large bmp which is in chunks - these are NEVER USED
// Other cursors (200, 300, 400, 500, 600, 700) are not the same in each stack
enum {
	kDefaultMystCursor = 100,				// The default hand
	kWhitePageCursor = 800,					// Holding a white page
	kRedPageCursor = 801,					// Holding a red page
	kBluePageCursor = 802,					// Holding a blue page
	// kDroppingWhitePageAnimCursor = 803,
	// kDroppingRedPageAnimCursor = 804,
	// kDroppingBluePageAnimCursor = 805,
	kNewMatchCursor = 900,					// Match that has not yet been lit
	kLitMatchCursor = 901,					// Match that's burning
	kDeadMatchCursor = 902,					// Match that's been extinguished
	kKeyCursor = 903, 						// Key in Lighthouse in Stoneship
	kRotateClockwiseCursor = 904, 			// Rotate gear clockwise (boiler on Myst)
	kRotateCounterClockwiseCursor = 905,	// Rotate gear counter clockwise (boiler on Myst)
	kMystZipModeCursor = 999				// Zip Mode cursor
};

class MohawkEngine_Myst;

// The cursor manager for Myst
// Uses WDIB + CLRC resources
class MystCursorManager : public CursorManager {
public:
	explicit MystCursorManager(MohawkEngine_Myst *vm);
	~MystCursorManager() override;

	void showCursor() override;
	void hideCursor() override;
	void setCursor(uint16 id) override;
	void setDefaultCursor() override;
	bool hasSource() const override { return true; }

private:
	MohawkEngine_Myst *_vm;
};

#endif // ENABLE_MYST

// The cursor manager for Mac applications
class MacCursorManager : public CursorManager {
public:
	explicit MacCursorManager(const Common::String &appName);
	~MacCursorManager() override;

	void setCursor(uint16 id) override;
	bool hasSource() const override { return _resFork != nullptr; }

private:
	Common::MacResManager *_resFork;
};

// The cursor manager for Living Books v2+ games
// Handles custom generated cursors in addition to tCUR resources
class LivingBooksCursorManager_v2 : public CursorManager {
public:
	LivingBooksCursorManager_v2();
	~LivingBooksCursorManager_v2() override;

	void setCursor(uint16 id) override;
	void setCursor(const Common::String &name) override;
	bool hasSource() const override { return _sysArchive != nullptr; }

private:
	MohawkArchive *_sysArchive;
};

// The cursor manager for Windows EXE's
class WinCursorManager : public CursorManager {
public:
	~WinCursorManager() override;

	void setCursor(uint16 id) override;
	bool hasSource() const override { return !_cursors.empty(); }

protected:
	void loadCursors(Common::WinResources *exe);

private:
	struct CursorItem {
		uint16 id;
		Graphics::WinCursorGroup *cursorGroup;
	};

	Common::Array<CursorItem> _cursors;
};

// The cursor manager for NE EXE's
class NECursorManager : public WinCursorManager {
public:
	explicit NECursorManager(const Common::String &appName);
};

// The cursor manager for PE EXE's
class PECursorManager : public WinCursorManager {
public:
	explicit PECursorManager(const Common::String &appName);
};

} // End of namespace Mohawk

#endif
