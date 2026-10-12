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

#ifndef BCB_H
#define BCB_H

#include "audio/mixer.h"
#include "common/archive.h"
#include "common/array.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/rect.h"
#include "common/str-enc.h"
#include "common/ustr.h"
#include "engines/engine.h"
#include "harold/detection.h"
#include "harold/bcb/ui.h"

namespace Graphics {
class Font;
class FontSJIS;
struct Surface;
}

namespace Video {
class AVIDecoder;
}

namespace BCB {

enum {
	kBCBDebugGeneral = 1
};

class Debugger;

class BCBEngine : public Engine {
	friend class Debugger;

public:
	BCBEngine(OSystem *syst, const HaroldGameDescription *desc);
	~BCBEngine();

	Common::Error run() override;

private:
	bool initializeSearchPaths();
	Common::SeekableReadStream *createMovieReadStream(int movieId) const;
	bool switchActiveMediaDisc(int discNumber);
	void resetGameState();
	bool loadScriptData();
	bool loadByteFile(const char *fileName, Common::Array<byte> &data) const;
	void executeCurrentScriptSequence();
	uint32 executeScriptExpression(uint32 &scriptPc, int &branchTokenSeen);
	bool pushScriptValue(uint32 value);
	uint32 popScriptValue();
	void popScriptOperands(uint32 &leftValue, uint32 &rightValue);
	uint32 dereferenceScriptValue(uint32 value);
	uint32 &scriptOperandDword(uint32 encodedOperand);
	uint32 &stateDword(uint byteOffset);
	uint32 stateDword(uint byteOffset) const;
	void notifyGameClockTicksChanged();
	Graphics::Surface *loadBitmap(const char *fileName);
	const Graphics::Surface *getUiBitmap(const char *fileName);
	Graphics::Surface *loadBitmapFromStream(Common::SeekableReadStream *stream, const char *debugName) const;
	Graphics::Surface *loadDiscBitmap(const char *relativePath) const;
	Graphics::Surface *loadDiscBitmapFromAnyDisc(const char *relativePath) const;
	Common::SeekableReadStream *createProgramReadStream(const char *relativePath) const;
	Common::SeekableReadStream *createDiscReadStream(int discNumber, const char *relativePath) const;
	Common::SeekableReadStream *createActiveDiscReadStream(const char *relativePath) const;
	Common::SeekableReadStream *createActiveDiscHighColorReadStream(const char *relativePath) const;
	void handleLeftButtonDown(const Common::Point &mousePosition);
	bool pollEvents();
	bool pollEvents(bool handleClicks);
	void serviceCursorRefresh(bool screenUpdated = false);
	void delayWithEventPolling(uint32 millis);
	void delayWithEventPolling(uint32 millis, bool handleClicks);
	void mainLoop();

	const HaroldGameDescription *_gameDescription;
	Common::SearchSet _programFiles;
	Common::SearchSet _discFiles[5];
	Common::Array<byte> _scriptData;
	Common::Array<byte> _locationTableData;
	Common::Array<byte> _locationTimingData;
	Common::Array<byte> _allBinData;
	Common::Array<byte> _timedCaptionRecordData;
	Graphics::Surface *_uiBitmap = nullptr;
	Common::String _uiBitmapName;
	Graphics::Surface *_chipBitmap = nullptr;
	uint32 _gameState[0x100] = {};
	uint32 _scriptValueStack[0x100] = {};
	int _scriptStackDepth = 0;
	bool _scriptStopFlag = false;
	int _pendingStillElapsedTimerTicks = 0;
	int _gameOverElapsedTimerTicks = 0;
	int _pendingStillDurationIndex = 0;
	int _pendingClockTickDelta = 0;
	int _activeMediaDiscNumber = 1;
	uint _timedCaptionCursorOffset = 0;
	int _nextTimedCaptionThreshold = 99999;
	int _queuedMoviePlaybackIndex = 0;
	int _previousDisplayStateBeforeSystem = 0;
	int _sceneChoiceTerminatorButtonIndex = 0;
	int _sceneChoiceEnabledButtonLimit = 0;
	Common::U32String _timedCaptionText;
	int _lastLoggedGameClockTicks = -1;
	uint32 _lastCursorRefreshMillis = 0;
	Common::Point _mousePosition;
	bool _sceneChoiceActive = false;
	bool _suppressClickHandling = false;
};

} // End of namespace BCB

#endif // BCB_H
