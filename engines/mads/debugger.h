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

#ifndef MADS_DEBUGGER_H
#define MADS_DEBUGGER_H

#include "common/scummsys.h"
#include "gui/debugger.h"

namespace MADS {

class MADSEngine;

class Debugger : public GUI::Debugger {
private:
	MADSEngine *_vm;
protected:
	bool Cmd_Mouse(int argc, const char **argv);
	bool Cmd_LoadScene(int argc, const char **argv);
	bool Cmd_ShowHotSpots(int argc, const char **argv);
	bool Cmd_ListHotSpots(int argc, const char **argv);
	bool Cmd_PlaySound(int argc, const char **argv);
	bool Cmd_PlayAudio(int argc, const char **argv);
	bool Cmd_ShowCodes(int argc, const char **argv);
	bool Cmd_DumpFile(int argc, const char **argv);
	bool Cmd_ShowQuote(int argc, const char **argv);
	bool Cmd_ShowVocab(int argc, const char **argv);
	bool Cmd_DumpVocab(int argc, const char **argv);
	bool Cmd_ShowMessage(int argc, const char **argv);
	bool Cmd_ShowItem(int argc, const char **argv);
	bool Cmd_DumpItems(int argc, const char **argv);
	bool Cmd_Item(int argc, const char **argv);
	bool Cmd_PlayAnim(int argc, const char **argv);
	bool Cmd_PlayText(int argc, const char **argv);
	bool Cmd_SetCamera(int argc, const char **argv);
public:
	bool _showMousePos;
public:
	Debugger(MADSEngine *vm);
	~Debugger() override {}
};

} // End of namespace MADS

#endif	/* MADS_DEBUGGER_H */
