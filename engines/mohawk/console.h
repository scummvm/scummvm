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
 * $URL$
 * $Id$
 *
 */

#ifndef MOHAWK_CONSOLE_H
#define MOHAWK_CONSOLE_H

#include "gui/debugger.h"

namespace Mohawk {

class MohawkEngine_Myst;
class MohawkEngine_Riven;
class MohawkEngine_LivingBooks;

class MystConsole : public GUI::Debugger {
public:
	MystConsole(MohawkEngine_Myst *vm);
	virtual ~MystConsole(void);
	
protected:
	virtual void preEnter();
	virtual void postEnter();
	
private:
	MohawkEngine_Myst *_vm;
	
	bool Cmd_ChangeCard(int argc, const char **argv);
	bool Cmd_CurCard(int argc, const char **argv);
	bool Cmd_Var(int argc, const char **argv);
	bool Cmd_DrawImage(int argc, const char **argv);
	bool Cmd_DrawRect(int argc, const char **argv);
	bool Cmd_SetResourceEnable(int argc, const char **argv);
	bool Cmd_CurStack(int argc, const char **argv);
	bool Cmd_ChangeStack(int argc, const char **argv);
	bool Cmd_PlaySound(int argc, const char **argv);
	bool Cmd_StopSound(int argc, const char **argv);
	bool Cmd_PlayMovie(int argc, const char **argv);
	bool Cmd_DisableInitOpcodes(int argc, const char **argv);
};

class RivenConsole : public GUI::Debugger {
public:
	RivenConsole(MohawkEngine_Riven *vm);
	virtual ~RivenConsole(void);

protected:
	virtual void preEnter();
	virtual void postEnter();

private:
	MohawkEngine_Riven *_vm;
		
	bool Cmd_ChangeCard(int argc, const char **argv);
	bool Cmd_CurCard(int argc, const char **argv);
	bool Cmd_Var(int argc, const char **argv);
	bool Cmd_PlaySound(int argc, const char **argv);
	bool Cmd_PlaySLST(int argc, const char **argv);
	bool Cmd_StopSound(int argc, const char **argv);
	bool Cmd_CurStack(int argc, const char **argv);
	bool Cmd_ChangeStack(int argc, const char **argv);
	bool Cmd_Restart(int argc, const char **argv);
	bool Cmd_Hotspots(int argc, const char **argv);
	bool Cmd_ZipMode(int argc, const char **argv);
	bool Cmd_RunAllBlocks(int argc, const char **argv);
	bool Cmd_DumpScript(int argc, const char **argv);
	bool Cmd_ListZipCards(int argc, const char **argv);
	bool Cmd_GetRMAP(int argc, const char **argv);
};

class LivingBooksConsole : public GUI::Debugger {
public:
	LivingBooksConsole(MohawkEngine_LivingBooks *vm);
	virtual ~LivingBooksConsole(void);

protected:
	virtual void preEnter();
	virtual void postEnter();

private:
	MohawkEngine_LivingBooks *_vm;

	bool Cmd_PlaySound(int argc, const char **argv);
	bool Cmd_StopSound(int argc, const char **argv);
	bool Cmd_DrawImage(int argc, const char **argv);
};

} // End of namespace Mohawk

#endif
