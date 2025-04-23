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

#ifndef LASTEXPRESS_HELPERS_H
#define LASTEXPRESS_HELPERS_H

//////////////////////////////////////////////////////////////////////////
// Misc helpers
//////////////////////////////////////////////////////////////////////////

#define LOW_BYTE(w)           ((unsigned char)(((unsigned long)(w)) & 0xff))

// Misc
#define getArchiveMember(name) _engine->getResourceManager()->getFileStream(name)
#define rnd(value) _engine->getRandom().getRandomNumber(value - 1)

// Engine subclasses
#define getLogic() _engine->getGameLogic()
#define getMenuOld() _engine->getGameMenu()

// Logic
#define getActionOld() getLogic()->getGameAction()
#define getBeetle() getLogic()->getGameBeetle()
#define getFight() getLogic()->getGameFight()
#define getEntities() getLogic()->getGameEntities()
#define getSaveLoad() getLogic()->getGameSaveLoad()
#define isNightOld() getLogic()->getGameState()->isNightTime()

#define getCharacter(c) (_engine->_characters->characters[c])
#define getCharacterCurrentParams(c) (getCharacter(c).callParams[getCharacter(c).currentCall].parameters)
#define getCharacterParams(c, n) (getCharacter(c).callParams[n].parameters)

// State
#define getState() getLogic()->getGameState()->getGameState()
#define HELPERgetEvent(id) getState()->events[id]
#define getFlags() getLogic()->getGameState()->getGameFlags()
#define getInventory() getLogic()->getGameState()->getGameInventory()
#define getObjects() getLogic()->getGameState()->getGameObjects()
#define getProgress() getState()->progress
#define getSavePoints() getLogic()->getGameState()->getGameSavePoints()
#define getGlobalTimer() getLogic()->getGameState()->getTimer()
#define setGlobalTimer(timer) getLogic()->getGameState()->setTimer(timer)
#define setCoords(coords) getLogic()->getGameState()->setCoordinates(coords)
#define getCoords() getLogic()->getGameState()->getCoordinates()
#define getFrameCount() _engine->getFrameCounter()

// Scenes
#define getScenes() _engine->getSceneManager()

// Sound
#define getSound() _engine->getSoundManagerOld()
#define getSoundQueue() _engine->getSoundManagerOld()->getQueue()

// Others
#define getEntityData(entity) getEntities()->getData(entity)

//////////////////////////////////////////////////////////////////////////
// Graphics
//////////////////////////////////////////////////////////////////////////

// Sequences
#define loadSequence(name) Sequence::load(name, getArchiveMember(name))
#define loadSequence1(name, field30) Sequence::load(name, getArchiveMember(name), field30)

#define clearBg(type) _engine->getGraphicsManagerOld()->clear(type)
#define showScene(index, type) _engine->getGraphicsManagerOld()->draw(getScenes()->get(index), type);

#define askForRedraw() _engine->getGraphicsManagerOld()->change()
#define redrawScreen() do { _engine->getGraphicsManagerOld()->update(); _engine->_system->updateScreen(); } while (false)

// Used to delete character sequences
#define SAFE_DELETE(_p) do { if (_p) delete (_p); (_p) = nullptr; } while (false)
#define SAFE_FREE(_p)   do { if (_p) free   (_p); (_p) = nullptr; } while (false)

//////////////////////////////////////////////////////////////////////////
// Output
//////////////////////////////////////////////////////////////////////////
extern const char *g_actionNames[];
extern const char *g_directionNames[];
extern const char *g_entityNames[];

#define ACTION_NAME(action) (action > 18 ? Common::String::format("%d", action).c_str() : g_actionNames[action])
#define DIRECTION_NAME(direction) (direction >= 6 ? "INVALID" : g_directionNames[direction])
#define ENTITY_NAME(index) (index >= 40 ? "INVALID" : g_entityNames[index])


#endif // LASTEXPRESS_HELPERS_H
