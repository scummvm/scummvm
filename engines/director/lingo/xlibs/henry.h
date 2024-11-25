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

#ifndef DIRECTOR_LINGO_XLIBS_HENRY_H
#define DIRECTOR_LINGO_XLIBS_HENRY_H

namespace Director {

class HenryXObject : public Object<HenryXObject> {
public:
	HenryXObject(ObjectType objType);
};

namespace HenryXObj {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_name(int nargs);
void m_userHitMouse(int nargs);
void m_startNewPosition(int nargs);
void m_startNewPositionSubView(int nargs);
void m_committNewPosition(int nargs);
void m_doLeftTurn(int nargs);
void m_doRightTurn(int nargs);
void m_checkCursor(int nargs);
void m_getRollOverActions(int nargs);
void m_getEntryActions(int nargs);
void m_peekEntryActions(int nargs);
void m_getExitActions(int nargs);
void m_setSavedPosition(int nargs);
void m_getStateVariable(int nargs);
void m_setStateVariable(int nargs);
void m_getStringVariable(int nargs);
void m_setStringVariable(int nargs);
void m_startTimer(int nargs);
void m_abortTimer(int nargs);
void m_checkTimers(int nargs);
void m_getTimerStatus(int nargs);
void m_pickUpItem(int nargs);
void m_dropItem(int nargs);
void m_dumpItem(int nargs);
void m_returnItem(int nargs);
void m_ingestItem(int nargs);
void m_putItemInContainer(int nargs);
void m_putItemAtLocation(int nargs);
void m_getCurrentPosition(int nargs);
void m_getPlayerProperties(int nargs);
void m_saveGame(int nargs);
void m_loadGame(int nargs);
void m_showCursor(int nargs);
void m_releaseCursor(int nargs);
void m_getRoomList(int nargs);
void m_getItemList(int nargs);
void m_getCurrentItem(int nargs);
void m_getCurrentContainer(int nargs);
void m_getCurrentInteraction(int nargs);
void m_getItemLocation(int nargs);
void m_getContainerContents(int nargs);
void m_pauseGame(int nargs);
void m_resumeGame(int nargs);

} // End of namespace HenryXObj

} // End of namespace Director

#endif
