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

/* OPL implementation for OPL2LPT through libieee1284.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "audio/fmopl.h"

#include <unistd.h>
#include <ieee1284.h>

namespace OPL {
namespace OPL2LPT {

class OPL : public ::OPL::RealOPL {
private:
	struct parport *_pport;
	int index;
	static const uint8 ctrlBytes[];

public:
	OPL();
	~OPL();

	bool init();
	void reset();

	void write(int a, int v);
	byte read(int a);

	void writeReg(int r, int v);
};

const uint8 OPL::ctrlBytes[] = {
	(C1284_NSELECTIN | C1284_NSTROBE | C1284_NINIT) ^ C1284_INVERTED,
	(C1284_NSELECTIN | C1284_NSTROBE) ^ C1284_INVERTED,
	(C1284_NSELECTIN | C1284_NSTROBE | C1284_NINIT) ^ C1284_INVERTED,

	(C1284_NSELECTIN | C1284_NINIT) ^ C1284_INVERTED,
	C1284_NSELECTIN ^ C1284_INVERTED,
	(C1284_NSELECTIN | C1284_NINIT) ^ C1284_INVERTED
};

OPL::OPL() : _pport(nullptr) {
}

OPL::~OPL() {
	if (_pport) {
		stop();
		reset();
		ieee1284_close(_pport);
	}
}

bool OPL::init() {
	struct parport_list parports = {};
	const Common::String parportName = ConfMan.get("opl2lpt_parport");

	// Look for available parallel ports
	if (ieee1284_find_ports(&parports, 0) != E1284_OK) {
		return false;
	}
	for (int i = 0; i < parports.portc; i++) {
		if (parportName == "null" ||
		    parportName == parports.portv[i]->name) {
			int caps = CAP1284_RAW;
			_pport = parports.portv[i];
			if (ieee1284_open(_pport, F1284_EXCL, &caps) != E1284_OK) {
				warning("cannot open parallel port %s", _pport->name);
			}
			if (ieee1284_claim(_pport) != E1284_OK) {
				warning("cannot claim parallel port %s", _pport->name);
				ieee1284_close(_pport);
				continue;
			}
			reset();
			// Safe to free ports here, opened ports are refcounted.
			ieee1284_free_ports(&parports);
			return true;
		}
	}
	_pport = nullptr;
	ieee1284_free_ports(&parports);
	return false;
}

void OPL::reset() {
	for(int i = 0; i < 256; i ++) {
		writeReg(i, 0);
	}
	index = 0;
}

void OPL::write(int port, int val) {
	if (port & 1) {
		writeReg(index, val);
	} else {
		index = val;
	}
}

byte OPL::read(int port) {
	// No read support for the OPL2LPT
	return 0;
}

void OPL::writeReg(int r, int v) {
	r &= 0xff;
	v &= 0xff;
	ieee1284_write_data(_pport, r);
	ieee1284_write_control(_pport, ctrlBytes[0]);
	ieee1284_write_control(_pport, ctrlBytes[1]);
	ieee1284_write_control(_pport, ctrlBytes[2]);
	usleep(4);		// 3.3 us

	ieee1284_write_data(_pport, v);
	ieee1284_write_control(_pport, ctrlBytes[3]);
	ieee1284_write_control(_pport, ctrlBytes[4]);
	ieee1284_write_control(_pport, ctrlBytes[5]);
	usleep(23);
}

OPL *create() {
	return new OPL();
}

} // End of namespace OPL2LPT
} // End of namespace OPL
