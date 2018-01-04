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
#ifndef SLUDGE_LOADSAVE_H
#define SLUDGE_LOADSAVE_H

namespace Sludge {

struct LoadedFunction;
struct Variable;
struct VariableStack;

bool saveGame(const Common::String &fname);
bool loadGame(const Common::String &fname);

bool saveVariable(Variable *from, Common::WriteStream *stream);
bool loadVariable(Variable *to, Common::SeekableReadStream *stream);

VariableStack *loadStack(Common::SeekableReadStream *stream, VariableStack **last);
bool saveStackRef(StackHandler *vs, Common::WriteStream *stream);
StackHandler *loadStackRef(Common::SeekableReadStream *stream);

LoadedFunction *loadFunction(Common::SeekableReadStream *stream);
void saveFunction(LoadedFunction *fun, Common::WriteStream *stream);

} // End of namespace Sludge

#endif
