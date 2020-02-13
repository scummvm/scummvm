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

#ifndef MOHAWK_CONSOLE_H
#define MOHAWK_CONSOLE_H

#include "gui/debugger.h"

namespace Mohawk {

class MohawkEngine_LivingBooks;

#ifdef ENABLE_MYST

class MohawkEngine_Myst;

class MystConsole : public GUI::Debugger {
public:
	explicit MystConsole(MohawkEngine_Myst *vm);
	~MystConsole() override;

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
	bool Cmd_Cache(int argc, const char **argv);
	bool Cmd_Resources(int argc, const char **argv);
	bool Cmd_QuickTest(int argc, const char **argv);
};

#endif

#ifdef ENABLE_RIVEN

class MohawkEngine_Riven;

class RivenConsole : public GUI::Debugger {
public:
	explicit RivenConsole(MohawkEngine_Riven *vm);
	~RivenConsole() override;

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
	bool Cmd_Hotspots(int argc, const char **argv);
	bool Cmd_ZipMode(int argc, const char **argv);
	bool Cmd_DumpCard(int argc, const char **argv);
	bool Cmd_DumpStack(int argc, const char **argv);
	bool Cmd_DumpScript(int argc, const char **argv);
	bool Cmd_ListZipCards(int argc, const char **argv);
	bool Cmd_GetRMAP(int argc, const char **argv);
	bool Cmd_Combos(int argc, const char **argv);
	bool Cmd_SliderState(int argc, const char **argv);
	bool Cmd_QuickTest(int argc, const char **argv);
};

#endif

class LivingBooksConsole : public GUI::Debugger {
public:
	explicit LivingBooksConsole(MohawkEngine_LivingBooks *vm);
	~LivingBooksConsole() override;

private:
	MohawkEngine_LivingBooks *_vm;

	bool Cmd_PlaySound(int argc, const char **argv);
	bool Cmd_StopSound(int argc, const char **argv);
	bool Cmd_DrawImage(int argc, const char **argv);
	bool Cmd_ChangePage(int argc, const char **argv);
	bool Cmd_ChangeCursor(int argc, const char **argv);
};

#ifdef ENABLE_CSTIME

class MohawkEngine_CSTime;

class CSTimeConsole : public GUI::Debugger {
public:
	CSTimeConsole(MohawkEngine_CSTime *vm);
	~CSTimeConsole(void) override;

private:
	MohawkEngine_CSTime *_vm;

	bool Cmd_PlaySound(int argc, const char **argv);
	bool Cmd_StopSound(int argc, const char **argv);
	bool Cmd_DrawImage(int argc, const char **argv);
	bool Cmd_DrawSubimage(int argc, const char **argv);
	bool Cmd_ChangeCase(int argc, const char **argv);
	bool Cmd_ChangeScene(int argc, const char **argv);
	bool Cmd_CaseVariable(int argc, const char **argv);
	bool Cmd_InvItem(int argc, const char **argv);
};

#endif

} // End of namespace Mohawk

#endif
