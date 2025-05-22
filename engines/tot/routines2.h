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
#ifndef TOT_ROUTINES2_H
#define TOT_ROUTINES2_H

#include "tot/anims.h"
#include "tot/graphics.h"
#include "tot/mouse.h"
#include "tot/playanim.h"
#include "tot/sound.h"
#include "tot/util.h"

namespace Tot {

enum TRAJECTORIES_OP {
	// Subtracts the animation dimensions to the trajectory coordinates to adjust movement
	SET_WITH_ANIM = false,
	// Adds the animation dimensions to the trajectory coordinates to restore
	RESTORE = true
};

void loadScreenMemory();
void loadAnimation(Common::String animacion);
void assignText();
void updateAltScreen(byte altScreenNumber);
void verifyCopyProtection();
void verifyCopyProtection2();
void cargatele();
void loadScreen();
void loadCharAnimation();
void freeScreenObjects();
void freeAnimation();
void freeInventory();
void loadItem(uint coordx, uint coordy, uint picSize, int32 pic, uint prof);
void updateInventory(byte index);
void readBitmap(int32 posbm, byte *puntbm, uint tambm, uint errorbm);
void updateItem(uint filePos);
void readItemRegister(Common::SeekableReadStream *stream, uint objPos, InvItemRegister &thisRegObj);
void readItemRegister(uint objPos);
void saveItemRegister(InvItemRegister object, Common::SeekableWriteStream *stream);
void saveTemporaryGame();
void drawLookAtItem(RoomObjectListEntry obj);
void putIcon(uint posX, uint posY, uint iconNumber);
void drawBackpack();
void inventory(byte direction, byte limit);
void mask();
void drawMenu(byte menuNumber);
void generateDiploma(Common::String &photoName);
void credits();
void checkMouseGrid();
void introduction();
void firstIntroduction();
void readAlphaGraph(Common::String &dato, int long_, int posx, int posy, byte colorbarra);
void readAlphaGraphSmall(Common::String &data, int long_, int posx, int posy, byte colorbarra, byte textColor);
void hipercadena(Common::String cadenatextnueva, uint xhcnueva, uint yhcnueva, byte anchohc, byte colortextohc, byte colorsombrahc);
void buttonBorder(uint x1, uint y1, uint x2, uint y2, byte color1, byte color2, byte color3, byte color4, byte color5, uint xtexto, Common::String outputGameName);
void copyProtection();
void initialLogo();
void initialMenu(bool fade);
void exitToDOS();
void soundControls();
void sacrificeScene();
void ending();
void loadBat();
void loadDevil();
void assembleCompleteBackground(byte *image, uint coordx, uint coordy);
void assembleScreen(bool scroll = false);
void disableSecondAnimation();
void setRoomTrajectories(int alto, int ancho, TRAJECTORIES_OP op, bool fixGrids = true);
void loadAnimationForDirection(Common::SeekableReadStream *stream, int direction);
void clearGame();

// Debug methods
void drawMouseGrid(RoomFileRegister *screen);
void drawScreenGrid(RoomFileRegister *screen);
void drawCharacterPosition();
void printPos(int x, int y, int screenPosX, int screenPosY, const char *label);
void drawPos(uint x, uint y, byte color);
void drawGrid();
void drawRect(byte color, int x, int y, int x2, int y2);
void drawX(int x, int y, byte color);
void drawLine(int x, int y, int x2, int y2, byte color);
} // End of namespace Tot

#endif
