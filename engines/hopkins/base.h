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

// Hopkins FBI underwater-base shooter (the original Windows WBASE module).

#ifndef HOPKINS_BASE_H
#define HOPKINS_BASE_H

#include "hopkins/base_data.h"
#include "hopkins/base_engine.h"

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

class BaseRenderer;
class HopkinsEngine;

enum BaseRunStatus {
	kBaseRunCompleted,
	kBaseRunFallback,
	kBaseRunQuit
};

struct BaseRunResult {
	BaseRunStatus status;
	int roomId;

	BaseRunResult(BaseRunStatus runStatus, int resultRoom = 0) :
			status(runStatus), roomId(resultRoom) {}
};

class BaseGame {
public:
	explicit BaseGame(HopkinsEngine *vm);
	~BaseGame();

	static bool hasRequiredResources(Common::String *missingResources = nullptr);

	/** Run entry 194..199 and distinguish gameplay, fallback and host-exit results. */
	BaseRunResult run(int entryId);

private:
	struct GraphicsStateBackup {
		bool breakout;
		bool mouseVisible;
		bool disableInventory;
		int lineNbr;
		int lineNbr2;
		int minX;
		int minY;
		int maxX;
		int maxY;
		byte paletteBuffer[256 * 2];
		byte palette[800];
		byte oldPalette[800];
	};

	class SessionGuard;

	static const BaseEntryPoint *entryPoints(uint &count);
	static const BaseEntryPoint *findEntryPoint(int entryId);

	bool initializePresentation(GraphicsStateBackup &backup);
	void restorePresentation(const GraphicsStateBackup &backup);
	void initializeAudio();
	void releaseAudio();
	void switchKeymaps(bool entering);
	void pollInput();
	void handleAction(uint32 action, bool pressed);
	void openMainMenu();
	void refreshPresentation();
	void processSoundEvents();
	void renderFrame();
	void setEngineActive(bool active);

	HopkinsEngine *_vm;
	BaseData _data;
	BaseEngine *_engine;
	BaseRenderer *_renderer;
	Common::Array<byte> _framebuffer;
	BaseInputState _input;
	const BaseEntryPoint *_entry;
	int _result;
	bool _audioLoaded[6];
	bool _keymapsSwitched;
	bool _defaultKeymapWasEnabled;
	bool _shortcutKeymapWasEnabled;
	bool _baseKeymapWasEnabled;
	bool _inputSuspended;
	bool _mainMenuRequested;
	bool _presentationRefreshRequested;
	bool _timingResetRequested;
	bool _quitRequested;
};

} // End of namespace Hopkins

#endif // HOPKINS_BASE_H
