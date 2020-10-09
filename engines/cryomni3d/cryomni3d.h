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

#include "graphics/cursorman.h"

#include "cryomni3d/font_manager.h"
#include "cryomni3d/objects.h"
#include "cryomni3d/sprites.h"
#include "cryomni3d/detection.h"

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

class DATSeekableStream;

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
	Common::Error run() override;

public:
	CryOmni3DEngine(OSystem *syst, const CryOmni3DGameDescription *gamedesc);
	~CryOmni3DEngine() override;

	// Detection related functions
	const CryOmni3DGameDescription *_gameDescription;
	const char *getGameId() const;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	uint8 getGameType() const;
	Common::Language getLanguage() const;

	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override { return _canLoadSave; }
	bool canSaveGameStateCurrently() override { return _canLoadSave; }

	void setCanLoadSave(bool canLoadSave) { _canLoadSave = canLoadSave; }
	static const uint kSaveDescriptionLen = 20;
private:
	void pauseEngineIntern(bool) override;

public:
	Image::ImageDecoder *loadHLZ(const Common::String &filename);

	void fillSurface(byte color);
	/* We use CursorMan because it avoids problems with cursors in GMM */
	void setCursor(const Graphics::Cursor &cursor) const;
	void setCursor(uint cursorId) const;
	bool showMouse(bool visible) { return CursorMan.showMouse(visible); }
	typedef void (CryOmni3DEngine::*HNMCallback)(uint frameNum);
	void playHNM(const Common::String &filename,
	             Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType,
	             HNMCallback beforeDraw = nullptr, HNMCallback afterDraw = nullptr);
	bool displayHLZ(const Common::String &filename, uint32 timeout = uint(-1));

	bool pollEvents();
	Common::Point getMousePos();
	void setMousePos(const Common::Point &point);
	uint getCurrentMouseButton() { return _lastMouseButton; }
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
	DATSeekableStream *getStaticData(uint32 gameId, uint16 version) const;

	void copySubPalette(byte *dst, const byte *src, uint start, uint num);
	void setPalette(const byte *colors, uint start, uint num);
	void lockPalette(uint startRW, uint endRW) { _lockPaletteStartRW = startRW; _lockPaletteEndRW = endRW; }
	void unlockPalette() { _lockPaletteStartRW = 0; _lockPaletteEndRW = 255; }
	void fadeOutPalette();
	void fadeInPalette(const byte *colors);
	void setBlackPalette();

	void setHNMClipping(const Common::Rect &clip) { _hnmClipping = clip; _hnmHasClip = true; }
	void unsetHNMClipping() { _hnmHasClip = false; }

protected:
	bool _canLoadSave;

	FontManager _fontManager;
	Sprites _sprites;
	Objects _objects;
	Inventory _inventory;

	Common::Queue<Common::KeyState> _keysPressed;

	DragStatus _dragStatus;
	Common::Point _dragStart;
	uint _lastMouseButton;
	uint _autoRepeatNextEvent;

private:
	uint _lockPaletteStartRW;
	uint _lockPaletteEndRW;

	Common::Rect _hnmClipping;
	bool _hnmHasClip;
};

} // End of namespace CryOmni3D

#endif
