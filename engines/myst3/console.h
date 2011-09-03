/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "gui/debugger.h"

#include "engines/myst3/myst3.h"

namespace Myst3 {

class Myst3Engine;

class Console : public GUI::Debugger {
public:
	Console(Myst3Engine *vm);
	virtual ~Console();

private:
	Myst3Engine *_vm;

	bool Cmd_Infos(int argc, const char **argv);
	bool Cmd_LookAt(int argc, const char **argv);
};

} /* namespace Myst3 */
#endif /* CONSOLE_H_ */
