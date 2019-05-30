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

#ifndef CRYOMNI3D_CRYOMNI3D_H
#define CRYOMNI3D_CRYOMNI3D_H

#include "audio/mixer.h"

#include "common/array.h"
#include "common/keyboard.h"
#include "common/queue.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "engines/engine.h"

#include "font_manager.h"
#include "objects.h"
#include "sprites.h"

class OSystem;

namespace Common {
struct Point;
class SeekableReadStream;
}

namespace Image {
class ImageDecoder;
}

/**
 * This is the namespace of the Cryo Omni3D engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Versailles
 * - ...
 */
namespace CryOmni3D {

enum CryOmni3DGameType {
	GType_VERSAILLES
};

struct CryOmni3DGameDescription;

// Engine Debug Flags
enum {
	kDebugFile     = (1 << 0),
	kDebugVariable = (1 << 1),
	kDebugSaveLoad = (1 << 2)
};

enum DragStatus {
	kDragStatus_NoDrag = 0,
	kDragStatus_Pressed,
	kDragStatus_Finished,
	kDragStatus_Dragging
};

class CryOmni3DEngine : public ::Engine {
protected:
	virtual Common::Error run();

public:
	CryOmni3DEngine(OSystem *syst, const CryOmni3DGameDescription *gamedesc);
	virtual ~CryOmni3DEngine();

	// Detection related functions
	const CryOmni3DGameDescription *_gameDescription;
	const char *getGameId() const;
	uint32 getFeatures() const;
	const char *getAppName() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	uint8 getGameType() const;
	Common::Language getLanguage() const;

	bool hasFeature(EngineFeature f) const;

private:
	void pauseEngineIntern(bool);

public:
	Image::ImageDecoder *loadHLZ(const Common::String &filename);

	void fillSurface(byte color);
	void setCursor(const Graphics::Cursor &cursor) const;
	void setCursor(uint cursorId) const;
	typedef void (CryOmni3DEngine::*HNMCallback)(uint frameNum);
	void playHNM(const Common::String &filename,
	             Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType,
	             HNMCallback beforeDraw = nullptr, HNMCallback afterDraw = nullptr);
	void displayHLZ(const Common::String &filename);

	bool pollEvents();
	Common::Point getMousePos();
	void setMousePos(const Common::Point &point);
	uint getCurrentMouseButton();
	Common::KeyState getNextKey();
	bool checkKeysPressed();
	bool checkKeysPressed(uint numKeys, ...);
	void clearKeys() { _keysPressed.clear(); }
	void waitMouseRelease();
	void setAutoRepeatClick(uint millis);
	DragStatus getDragStatus() { return _dragStatus; }

	Common::String prepareFileName(const Common::String &baseName, const char *extension) const {
		const char *const extensions[] = { extension, nullptr };
		return prepareFileName(baseName, extensions);
	}
	virtual Common::String prepareFileName(const Common::String &baseName,
	                                       const char *const *extensions) const;

	virtual bool displayToolbar(const Graphics::Surface *original) = 0;
	virtual bool hasPlaceDocumentation() = 0;
	virtual bool displayPlaceDocumentation() = 0;
	virtual uint displayOptions() = 0;
	virtual bool shouldAbort() { return g_engine->shouldQuit(); }

	virtual void makeTranslucent(Graphics::Surface &dst, const Graphics::Surface &src) const = 0;
	virtual void setupPalette(const byte *colors, uint start, uint num) = 0;

protected:
	void copySubPalette(byte *dst, const byte *src, uint start, uint num);
	void setPalette(const byte *colors, uint start, uint num);
	void lockPalette(uint startRW, uint endRW) { _lockPaletteStartRW = startRW; _lockPaletteEndRW = endRW; }
	void unlockPalette() { _lockPaletteStartRW = 0; _lockPaletteEndRW = 255; }
	void fadeOutPalette();
	void fadeInPalette(const byte *colors);
	void setBlackPalette();

protected:
	FontManager _fontManager;
	Sprites _sprites;
	Objects _objects;
	Inventory _inventory;

	Common::Queue<Common::KeyState> _keysPressed;

	DragStatus _dragStatus;
	Common::Point _dragStart;
	uint _autoRepeatNextEvent;

private:
	uint _lockPaletteStartRW;
	uint _lockPaletteEndRW;
};

} // End of namespace CryOmni3D

#endif
