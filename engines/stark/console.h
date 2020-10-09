/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "gui/debugger.h"

namespace Stark {

namespace Resources {
class Anim;
class Object;
class Script;
}

class ArchiveVisitor;

class Console : public GUI::Debugger {
public:
	Console();
	virtual ~Console();

private:
	bool Cmd_DumpArchive(int argc, const char **argv);
	bool Cmd_DumpRoot(int argc, const char **argv);
	bool Cmd_DumpStatic(int argc, const char **argv);
	bool Cmd_DumpGlobal(int argc, const char **argv);
	bool Cmd_DumpKnowledge(int argc, const char **argv);
	bool Cmd_DumpLevel(int argc, const char **argv);
	bool Cmd_DumpLocation(int argc, const char **argv);
	bool Cmd_ForceScript(int argc, const char **argv);
	bool Cmd_DecompileScript(int argc, const char **argv);
	bool Cmd_TestDecompiler(int argc, const char** argv);
	bool Cmd_ListInventoryItems(int argc, const char **argv);
	bool Cmd_EnableInventoryItem(int argc, const char **argv);
	bool Cmd_ListLocations(int argc, const char** argv);
	bool Cmd_ListScripts(int argc, const char** argv);
	bool Cmd_EnableScript(int argc, const char** argv);
	bool Cmd_ListAnimations(int argc, const char **argv);
	bool Cmd_ForceAnimation(int argc, const char **argv);
	bool Cmd_Location(int argc, const char **argv);
	bool Cmd_Chapter(int argc, const char **argv);
	bool Cmd_ChangeLocation(int argc, const char **argv);
	bool Cmd_ChangeChapter(int argc, const char **argv);
	bool Cmd_ChangeKnowledge(int argc, const char **argv);
	bool Cmd_ExtractAllTextures(int argc, const char **argv);

	Common::Array<Resources::Anim *> listAllLocationAnimations() const;
	Common::Array<Resources::Script *> listAllLocationScripts() const;

	void walkAllArchives(ArchiveVisitor *visitor);
};

} // End of namespace Stark

#endif // CONSOLE_H_
