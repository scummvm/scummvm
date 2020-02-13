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

#ifndef CHEWY_CONSOLE_H
#define CHEWY_CONSOLE_H

#include "gui/debugger.h"

namespace Chewy {

class ChewyEngine;

class Console : public GUI::Debugger {
public:
	Console(ChewyEngine *vm);
	~Console(void) override;

private:
	ChewyEngine *_vm;

	bool Cmd_Dump(int argc, const char **argv);
	bool Cmd_DumpBg(int argc, const char **argv);
	bool Cmd_DrawImage(int argc, const char **argv);
	bool Cmd_DrawSprite(int argc, const char **argv);
	bool Cmd_PlaySound(int argc, const char **argv);
	bool Cmd_PlaySpeech(int argc, const char **argv);
	bool Cmd_PlayMusic(int argc, const char **argv);
	bool Cmd_PlayVideo(int argc, const char **argv);
	bool Cmd_VideoInfo(int argc, const char **argv);
	bool Cmd_ErrorMessage(int argc, const char **argv);
	bool Cmd_Dialog(int argc, const char **argv);
	bool Cmd_Text(int argc, const char **argv);
	bool Cmd_Scene(int argc, const char **argv);
};

} // End of namespace Chewy
#endif
