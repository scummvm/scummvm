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

/* OPL implementation for OPL2LPT and OPL3LPT through libieee1284.
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

static const uint8 OPL2LPTRegisterSelect[] = {
	(C1284_NSELECTIN | C1284_NSTROBE | C1284_NINIT) ^ C1284_INVERTED,
	(C1284_NSELECTIN | C1284_NSTROBE) ^ C1284_INVERTED,
	(C1284_NSELECTIN | C1284_NSTROBE | C1284_NINIT) ^ C1284_INVERTED
};

static const uint8 OPL3LPTRegisterSelect[] = {
	(C1284_NSTROBE | C1284_NINIT) ^ C1284_INVERTED,
	C1284_NSTROBE ^ C1284_INVERTED,
	(C1284_NSTROBE | C1284_NINIT) ^ C1284_INVERTED
};

static const uint8 OPL2LPTRegisterWrite[] = {
	(C1284_NSELECTIN | C1284_NINIT) ^ C1284_INVERTED,
	C1284_NSELECTIN ^ C1284_INVERTED,
	(C1284_NSELECTIN | C1284_NINIT) ^ C1284_INVERTED
};

namespace OPL {
namespace OPL2LPT {

class OPL : public ::OPL::RealOPL {
private:
	struct parport *_pport;
	Config::OplType _type;
	int index;

public:
	explicit OPL(Config::OplType type);
	~OPL();

	bool init();
	void reset();

	void write(int a, int v);
	byte read(int a);

	void writeReg(int r, int v);
};

OPL::OPL(Config::OplType type) : _pport(nullptr), _type(type), index(0) {
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
	if (_type == Config::kOpl3) {
		for (int i = 0; i < 256; i++) {
			writeReg(i + 256, 0);
		}
	}
	index = 0;
}

void OPL::write(int port, int val) {
	if (port & 1) {
		writeReg(index, val);
	} else {
		switch (_type) {
		case Config::kOpl2:
			index = val & 0xff;
			break;
		case Config::kOpl3:
			index = (val & 0xff) | ((port << 7) & 0x100);
			break;
		default:
			warning("OPL2LPT: unsupported OPL mode %d", _type);
			break;
		}
	}
}

byte OPL::read(int port) {
	// No read support for the OPL2LPT
	return 0;
}

void OPL::writeReg(int r, int v) {
	if (_type == Config::kOpl3) {
		r &= 0x1ff;
	} else {
		r &= 0xff;
	}
	v &= 0xff;

	ieee1284_write_data(_pport, r & 0xff);
	if (r < 0x100) {
		ieee1284_write_control(_pport, OPL2LPTRegisterSelect[0]);
		ieee1284_write_control(_pport, OPL2LPTRegisterSelect[1]);
		ieee1284_write_control(_pport, OPL2LPTRegisterSelect[2]);
	} else {
		ieee1284_write_control(_pport, OPL3LPTRegisterSelect[0]);
		ieee1284_write_control(_pport, OPL3LPTRegisterSelect[1]);
		ieee1284_write_control(_pport, OPL3LPTRegisterSelect[2]);
	}
	usleep(4);		// 3.3 us

	ieee1284_write_data(_pport, v);
	ieee1284_write_control(_pport, OPL2LPTRegisterWrite[0]);
	ieee1284_write_control(_pport, OPL2LPTRegisterWrite[1]);
	ieee1284_write_control(_pport, OPL2LPTRegisterWrite[2]);
	usleep(23);
}

OPL *create(Config::OplType type) {
	return new OPL(type);
}

} // End of namespace OPL2LPT
} // End of namespace OPL
