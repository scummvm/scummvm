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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef WAGE_WAGE_H
#define WAGE_WAGE_H

#include "engines/engine.h"
#include "audio/mixer.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/macresman.h"
#include "common/random.h"

#include "wage/debugger.h"

struct ADGameDescription;

namespace Wage {

class Console;
class Chr;
class Designed;
class Dialog;
class Gui;
class Obj;
class Scene;
class World;

typedef Common::Array<Obj *> ObjArray;
typedef Common::Array<Chr *> ChrArray;
typedef Common::List<Obj *> ObjList;
typedef Common::List<Chr *> ChrList;

#define STORAGESCENE "STORAGE@"

enum OperandType {
	OBJ = 0,
	CHR = 1,
	SCENE = 2,
	NUMBER = 3,
	STRING = 4,
	CLICK_INPUT = 5,
	TEXT_INPUT = 6,
	UNKNOWN = 100
};

enum Directions {
	NORTH = 0,
	SOUTH = 1,
	EAST = 2,
	WEST = 3
};

// our engine debug levels
enum {
	kWageDebugExample = 1 << 0,
	kWageDebugExample2 = 1 << 1
	// next new level must be 1 << 2 (4)
	// the current limitation is 32 debug levels (1 << 31 is the last one)
};

Common::Rect *readRect(Common::SeekableReadStream *in);
const char *getIndefiniteArticle(const Common::String &word);
const char *prependGenderSpecificPronoun(int gender);
const char *getGenderSpecificPronoun(int gender, bool capitalize);
bool isStorageScene(const Common::String &name);

class WageEngine : public Engine {
	friend class Dialog;
public:
	WageEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~WageEngine() override;

	bool hasFeature(EngineFeature f) const override;

	Common::Error run() override;

	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;

	const char *getGameFile() const;
	void processTurn(Common::String *textInput, Designed *clickInput);
	void regen();

	const char *getTargetName() { return _targetName.c_str(); }

private:
	bool loadWorld(Common::MacResManager *resMan);
	void performInitialSetup();
	void wearObjs(Chr *chr);
	void processTurnInternal(Common::String *textInput, Designed *clickInput);
	void performCombatAction(Chr *npc, Chr *player);
	int getValidMoveDirections(Chr *npc);
	void performAttack(Chr *attacker, Chr *victim, Obj *weapon);
	void performMagic(Chr *attacker, Chr *victim, Obj *magicalObject);
	void performMove(Chr *chr, int validMoves);
	void performOffer(Chr *attacker, Chr *victim);
	void performTake(Chr *npc, Obj *obj);
	void decrementUses(Obj *obj);
	bool attackHit(Chr *attacker, Chr *victim, Obj *weapon, int targetIndex);
	void performHealingMagic(Chr *chr, Obj *magicalObject);

	void doClose();
	void updateSoundTimerForScene(Scene *scene, bool firstTime);

public:
	void takeObj(Obj *obj);

	bool handleMoveCommand(Directions dir, const char *dirName);
	bool handleLookCommand();
	Common::String *getGroundItemsList(Scene *scene);
	void appendObjNames(Common::String &str, const ObjArray &objs);
	bool handleInventoryCommand();
	bool handleStatusCommand();
	bool handleRestCommand();
	bool handleAcceptCommand();

	bool handleTakeCommand(const char *target);
	bool handleDropCommand(const char *target);
	bool handleAimCommand(const char *target);
	bool handleWearCommand(const char *target);
	bool handleOfferCommand(const char *target);

	void wearObj(Obj *o, int pos);

	bool tryAttack(const Obj *weapon, const Common::String &input);
	bool handleAttack(Obj *weapon);

	void printPlayerCondition(Chr *player);
	const char *getPercentMessage(double percent);

public:
	Common::RandomSource *_rnd;

	Gui *_gui;
	World *_world;

	Scene *_lastScene;
	int _loopCount;
	int _turn;
	Chr *_monster;
	Chr *_running;
	Obj *_offer;
	int _aim;
	int _opponentAim;
	bool _temporarilyHidden;
	bool _isGameOver;
	bool _commandWasQuick;

	bool _shouldQuit;

	Common::String _inputText;

	void playSound(Common::String soundName);
	void setMenu(Common::String soundName);
	void appendText(const char *str);
	void gameOver();
	bool saveDialog();
	Obj *getOffer();
	Chr *getMonster();
	void processEvents();
	Scene *getSceneByName(Common::String &location);
	void onMove(Designed *what, Designed *from, Designed *to);
	void encounter(Chr *player, Chr *chr);
	void redrawScene();
	void saveGame();

	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool scummVMSaveLoadDialog(bool isSave);

private:
	int getSceneIndex(Scene *scene) const;
	Obj *getObjByOffset(int offset, int objBaseOffset) const;
	Chr *getChrById(int resId) const;
	Chr *getChrByOffset(int offset, int chrBaseOffset) const;
	Scene *getSceneById(int id) const;
	Scene *getSceneByOffset(int offset) const;
	int saveGame(const Common::String &fileName, const Common::String &descriptionString);
	int loadGame(int slotId);

private:
	const ADGameDescription *_gameDescription;

	Common::MacResManager *_resManager;

	Audio::SoundHandle _soundHandle;
};

} // End of namespace Wage

#endif
