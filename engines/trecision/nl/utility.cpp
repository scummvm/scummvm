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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "engines/engine.h"

#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/trecision.h"

namespace Trecision {

void SScriptFrame::sendFrame() {
	doEvent(_class, _event, MP_DEFAULT, _u16Param1, _u16Param2, _u8Param, _u32Param);
}

/* -----------------21/01/98 10.22-------------------
 * 				DoSys
 * --------------------------------------------------*/
void DoSys(uint16 curObj) {
	switch (curObj) {
	case o00QUIT:
		if (QuitGame())
			doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;

	case o00EXIT:
		if (g_vm->_oldRoom == rSYS)
			break;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[o00EXIT]._goRoom, 0, 0, 0);
		break;

	case o00SAVE:
		if (g_vm->_oldRoom == rSYS)
			break;
		g_vm->_curRoom = g_vm->_obj[o00EXIT]._goRoom;
		if (!DataSave()) {
			g_vm->showInventoryName(NO_OBJECTS, false);
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, g_vm->_mouseX, g_vm->_mouseY, 0, 0);
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_obj[o00EXIT]._goRoom, 0, 0, 0);
		}
		g_vm->_curRoom = rSYS;
		break;

	case o00LOAD:
		if (!DataLoad()) {
			g_vm->showInventoryName(NO_OBJECTS, false);
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, g_vm->_mouseX, g_vm->_mouseY, 0, 0);
		}
		break;

	case o00SPEECHON:
		if (ConfMan.getBool("subtitles")) {
			g_vm->_obj[o00SPEECHON]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[o00SPEECHOFF]._mode |= OBJMODE_OBJSTATUS;
			ConfMan.setBool("speech_mute", true);
			g_vm->_curObj = o00SPEECHOFF;
			RegenRoom();
			ShowObjName(g_vm->_curObj, true);
		}
		break;

	case o00SPEECHOFF:
		g_vm->_obj[o00SPEECHOFF]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[o00SPEECHON]._mode |= OBJMODE_OBJSTATUS;
		ConfMan.setBool("speech_mute", false);
		g_vm->_curObj = o00SPEECHON;
		RegenRoom();
		ShowObjName(g_vm->_curObj, true);
		break;

	case o00TEXTON:
		if (!ConfMan.getBool("speech_mute")) {
			g_vm->_obj[o00TEXTON]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[o00TEXTOFF]._mode |= OBJMODE_OBJSTATUS;
			ConfMan.setBool("subtitles", false);
			g_vm->_curObj = o00TEXTOFF;
			RegenRoom();
			ShowObjName(g_vm->_curObj, true);
		}
		break;

	case o00TEXTOFF:
		g_vm->_obj[o00TEXTOFF]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[o00TEXTON]._mode |= OBJMODE_OBJSTATUS;
		ConfMan.setBool("subtitles", true);
		g_vm->_curObj = o00TEXTON;
		RegenRoom();
		ShowObjName(g_vm->_curObj, true);
		break;

	case o00SPEECH1D:
	case o00SPEECH2D:
	case o00SPEECH3D:
	case o00SPEECH4D:
	case o00SPEECH5D:
	case o00SPEECH6D:
	case o00MUSIC1D:
	case o00MUSIC2D:
	case o00MUSIC3D:
	case o00MUSIC4D:
	case o00MUSIC5D:
	case o00MUSIC6D:
	case o00SOUND1D:
	case o00SOUND2D:
	case o00SOUND3D:
	case o00SOUND4D:
	case o00SOUND5D:
	case o00SOUND6D:
		g_vm->_obj[curObj]._mode &= ~OBJMODE_OBJSTATUS;
		if ((curObj != o00SPEECH6D) && (curObj != o00MUSIC6D) && (curObj != o00SOUND6D))
			g_vm->_obj[curObj + 1]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[curObj - 1]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[curObj - 2]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		if (curObj < o00MUSIC1D)
			ConfMan.setInt("speech_volume", ((curObj - 2 - o00SPEECH1D) / 2) * 51);
		else if (curObj > o00MUSIC6D)
			ConfMan.setInt("sfx_volume", ((curObj - 2 - o00SOUND1D) / 2) * 51);
		else
			ConfMan.setInt("music_volume", ((curObj - 2 - o00MUSIC1D) / 2) * 51);
		break;

	case o00SPEECH1U:
	case o00SPEECH2U:
	case o00SPEECH3U:
	case o00SPEECH4U:
	case o00SPEECH5U:
	case o00MUSIC1U:
	case o00MUSIC2U:
	case o00MUSIC3U:
	case o00MUSIC4U:
	case o00MUSIC5U:
	case o00SOUND1U:
	case o00SOUND2U:
	case o00SOUND3U:
	case o00SOUND4U:
	case o00SOUND5U:
		g_vm->_obj[curObj]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[curObj - 1]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[curObj + 1]._mode |= OBJMODE_OBJSTATUS;
		if ((curObj != o00SPEECH5U) && (curObj != o00MUSIC5U) && (curObj != o00SOUND5U))
			g_vm->_obj[curObj + 2]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		if (curObj < o00MUSIC1D)
			ConfMan.setInt("speech_volume", ((curObj + 1 - o00SPEECH1D) / 2) * 51);
		else if (curObj > o00MUSIC6D)
			ConfMan.setInt("sfx_volume", ((curObj + 1 - o00SOUND1D) / 2) * 51);
		else
			ConfMan.setInt("music_volume", ((curObj + 1 - o00MUSIC1D) / 2) * 51);
		break;
	}

	g_engine->syncSoundSettings();
	ConfMan.flushToDisk();
}

} // End of namespace Trecision
