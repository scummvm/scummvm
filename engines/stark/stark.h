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

#ifndef STARK_H
#define STARK_H

#include "engines/engine.h"
#include "common/events.h"
#include "common/str-array.h"
#include "stark/detection.h"

struct ADGameDescription;

namespace Common {
class RandomSource;
}

namespace Graphics {
class FrameLimiter;
}

namespace Stark {

namespace Gfx {
class Driver;
}

class ArchiveLoader;
class Console;
class DialogPlayer;
class Diary;
class FontProvider;
class Global;
class GameInterface;
class Scene;
class StateProvider;
class StaticProvider;
class ResourceProvider;
class UserInterface;
class Settings;
class GameChapter;
class GameMessage;

enum STARKAction {
	kActionNone,
	kActionDiaryMenu,
	kActionSaveGame,
	kActionLoadGame,
	kActionConversationLog,
	kActionAprilsDiary,
	kActionVideoReplay,
	kActionGameSettings,
	kActionSaveScreenshot,
	kActionToggleSubtitles,
	kActionQuitToMenu,
	kActionCycleBackInventory,
	kActionCycleForwardInventory,
	kActionInventory,
	kActionDisplayExits,
	kActionExitGame,
	kActionPause,
	kActionInventoryScrollUp,
	kActionInventoryScrollDown,
	kActionDialogueScrollUp,
	kActionDialogueScrollDown,
	kActionPrevDialogue,
	kActionNextDialogue,
	kActionSelectDialogue,
	kActionSkip
};


class StarkEngine : public Engine {
public:
	StarkEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~StarkEngine() override;

	/** Build a save file name for the specified target and slot */
	static Common::String formatSaveName(const char *target, int slot);

	/** List all the save file names for the specified target */
	static Common::StringArray listSaveNames(const char *target);

	/** Extract the save slot number from the provided save file name */
	static int getSaveNameSlot(const char *target, const Common::String &saveName);

	uint32 getGameFlags() const;

protected:
	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	void pauseEngineIntern(bool pause) override;

private:
	void mainLoop();
	void updateDisplayScene();
	void processEvents();
	void onScreenChanged() const;
	void addModsToSearchPath() const;
	static void checkRecommendedDatafiles();

	Graphics::FrameLimiter *_frameLimiter;
	PauseToken _gamePauseToken;

	const ADGameDescription *_gameDescription;

	// Double click handling
	static const uint _doubleClickDelay = 500; // ms
	uint _lastClickTime;
};

} // End of namespace Stark

#endif // STARK_H
