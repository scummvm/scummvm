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
#ifndef TOT_ROUTINES_H
#define TOT_ROUTINES_H

#include "common/scummsys.h"

#include "tot/anims.h"
#include "tot/graphics.h"
#include "tot/vars.h"
#include "tot/routines2.h"
#include "tot/util.h"

namespace Tot {

void drawText(uint number);
void displayLoading();
void runaroundRed();
void sprites(bool drawCharacter);
void loadScreenData(uint screenNumber);
RoomFileRegister *readScreenDataFile(Common::SeekableReadStream *screenDataFile);
void saveRoomData(RoomFileRegister *room, Common::SeekableWriteStream *stream);
void calculateTrajectory(uint finalx, uint finaly);
void lookAtObject(byte objectNumber);
void useInventoryObjectWithInventoryObject(uint objectNo1, uint objectNo2);
void pickupScreenObject();
void useScreenObject();
void openScreenObject();
void closeScreenObject();
void action();
void handleAction(byte posinv);
void loadObjects();
void obtainName(Common::String &playerName);
void loadScrollData(uint screenNumber, bool scrollder, uint poshor, int correction);
void loadGame(SavedGame game);
void saveGameToRegister();
void saveLoad();
void calculateRoute(byte zona1, byte zona2, bool extraCorrection = false, bool zonavedada = false);
void goToObject(byte zone1, byte zone2);
void loadTalkAnimations();
void unloadTalkAnimations();
void hypertext(uint numreght, byte colortextoht, byte colorsombraht, uint &numresp, bool banderaconversa);
void wcScene();
void advanceAnimations(bool zonavedada, bool animateMouse);
void updateSecondaryAnimationDepth();
void updateMainCharacterDepth();
void actionLineText(Common::String actionLine);
void initializeObjectFile();
void saveItem(ScreenObject object, Common::SeekableWriteStream *objectDataStream);

TextEntry readVerbRegister(uint numRegister);
TextEntry readVerbRegister();

void readConversationFile(Common::String f);
void printNiches();

} // End of namespace Tot

#endif
