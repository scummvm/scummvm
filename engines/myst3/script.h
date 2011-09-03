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

#ifndef SCRIPT_H_
#define SCRIPT_H_

#include "common/array.h"

namespace Myst3 {

class Myst3Engine;
struct Opcode;

class Script {
public:
	Script(Myst3Engine *vm);
	virtual ~Script();

	void run(Common::Array<Opcode> *script);

private:
	void runOp(Opcode *op);

	Myst3Engine *_vm;
};

} /* namespace Myst3 */
#endif /* SCRIPT_H_ */
