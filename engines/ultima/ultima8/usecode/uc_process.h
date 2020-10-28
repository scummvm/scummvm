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
#ifndef ULTIMA8_USECODE_UCPROCESS_H
#define ULTIMA8_USECODE_UCPROCESS_H

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/usecode/uc_stack.h"

namespace Ultima {
namespace Ultima8 {

class Usecode;

// probably won't inherit from Process directly in the future
class UCProcess : public Process {
	friend class UCMachine;
	friend class Kernel;
public:
	UCProcess();
	UCProcess(uint16 classid, uint16 offset, uint32 this_ptr = 0,
	          int thissize = 0, const uint8 *args = 0, int argsize = 0);
	~UCProcess() override;

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	void terminate() override;

	void freeOnTerminate(uint16 index, int type);

	void setReturnValue(uint32 retval) {
		_temp32 = retval;
	}

	//! dump some info about this process to pout
	void dumpInfo() const override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

protected:
	void load(uint16 classid, uint16 offset, uint32 this_ptr = 0,
	          int thissize = 0, const uint8 *args = 0, int argsize = 0);
	void call(uint16 classid, uint16 offset);
	bool ret();

	// stack base pointer
	uint16 _bp;

	Usecode *_usecode;

	uint16 _classId;
	uint16 _ip;

	uint32 _temp32;

	// data stack
	UCStack _stack;

	// "Free Me" list
	Std::list<Std::pair<uint16, int> > _freeOnTerminate;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
