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

#ifndef ACCESS_DEBUGGER_H
#define ACCESS_DEBUGGER_H

#include "common/scummsys.h"
#include "gui/debugger.h"
#include "access/amazon/amazon_resources.h"
#include "access/martian/martian_resources.h"

namespace Access {

class AccessEngine;

class Debugger : public GUI::Debugger {
protected:
	AccessEngine *_vm;
	Common::String _playMovieFile;

	bool Cmd_LoadScene(int argc, const char **argv);
	bool Cmd_Cheat(int argc, const char **argv);
	bool Cmd_PlayMovie(int argc, const char **argv);
public:
	static Debugger *init(AccessEngine *vm);
	void postEnter() override;
public:
	Debugger(AccessEngine *vm);
	~Debugger() override;
};

namespace Amazon {

class AmazonDebugger : public Debugger {
protected:
	bool Cmd_StartChapter(int argc, const char **argv);
public:
	AmazonDebugger(AccessEngine *vm);
	~AmazonDebugger() override {}
};

} // End of namespace Amazon

} // End of namespace Access

#endif	/* ACCESS_DEBUGGER_H */
