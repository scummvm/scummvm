/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/v/voyagerxsound.h"
#include "director/lingo/xlibs/x/xsoundxfcn.h"

/**************************************************
 *
 * USED IN:
 * puppetmotel
 *
 **************************************************/

namespace Director {

const char *const XSoundXFCN::xlibName = "XSound";
const XlibFileDesc XSoundXFCN::fileNames[] = {
	{ "XSound",	"puppetmotel" },
	{ "_XSound",	"puppetmotel" },
	{ nullptr,	nullptr },
};

static const BuiltinProto builtins[] = {
	{ "XSound", XSoundXFCN::m_XSound, -1, 0, 400, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void XSoundXFCN::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);

	if (!g_lingo->_openXLibsState.contains("XSound")) {
		VoyagerXSoundXObject *xobj = new VoyagerXSoundXObject(type);
		g_lingo->_openXLibsState.setVal("XSound", xobj);
	}
}

void XSoundXFCN::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void XSoundXFCN::m_XSound(int nargs) {
	g_lingo->printSTUBWithArglist("XSoundXFCN::m_XSound", nargs);
	VoyagerXSoundXObject *xobj = (VoyagerXSoundXObject *)g_lingo->_openXLibsState.getVal("XSound");

	if (nargs == 0) {
		warning("XSoundXFCN: need at least one arg");
		g_lingo->push(Datum());
		return;
	}
	Datum cmd = g_lingo->peek(nargs-1);
	if (cmd.type != STRING) {
		warning("XSoundXFCN: arg 1 needs to be string");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum());
		return;
	}
	Common::String cmdName = cmd.asString();

	if (cmdName == "open") {
		ARGNUMCHECK(3);
		Datum monostereo = g_lingo->pop();
		Datum numchan = g_lingo->pop();
		g_lingo->pop();
		g_lingo->push(xobj->open(numchan.asInt(), monostereo.asInt()));
	} else if (cmdName == "close") {
		ARGNUMCHECK(1);
		g_lingo->pop();
		xobj->close();
		g_lingo->push(Datum(0));
	} else if (cmdName == "status") {
		ARGNUMCHECK(2);
		Datum chan = g_lingo->pop();
		g_lingo->pop();
		g_lingo->push(xobj->status(chan.asInt()));
	} else if (cmdName == "playfile") {
		if (nargs < 3) {
			warning("XSoundXFCN::playfile: expected at least 3 args");
			g_lingo->dropStack(nargs);
			g_lingo->push(0);
			return;
		} else if (nargs > 5) {
			g_lingo->dropStack(nargs - 5);
			nargs = 5;
		}
		Datum tend(-1);
		if (nargs == 5) {
			tend = g_lingo->pop();
			nargs--;
		}
		Datum tstart(-1);
		if (nargs == 4) {
			tstart = g_lingo->pop();
			nargs--;
		}
		Common::String path = g_lingo->pop().asString();
		Datum chan = g_lingo->pop();
		g_lingo->pop();
		int result = xobj->playfile(chan.asInt(), path, tstart.asInt(), tend.asInt());
		g_lingo->push(result);
	} else if (cmdName == "stop") {
		ARGNUMCHECK(2);
		Datum chan = g_lingo->pop();
		g_lingo->pop();
		xobj->stop(chan.asInt());
		g_lingo->push(1);
	} else if (cmdName == "volume") {
		ARGNUMCHECK(3);
		Datum vol = g_lingo->pop();
		Datum chan = g_lingo->pop();
		g_lingo->pop();
		xobj->volume(chan.asInt(), vol.asInt());
		g_lingo->push(1);
	} else if (cmdName == "leftRightVol") {
		ARGNUMCHECK(4);
		Datum rvol = g_lingo->pop();
		Datum lvol = g_lingo->pop();
		Datum chan = g_lingo->pop();
		g_lingo->pop();
		xobj->leftrightvol(chan.asInt(), (uint8)lvol.asInt(), (uint8)rvol.asInt());
		g_lingo->push(1);
	} else if (cmdName == "fade") {
		if (nargs < 3) {
			warning("XSoundXFCN::fade: expected at least 2 args");
			g_lingo->dropStack(nargs);
			g_lingo->push(Datum());
			return;
		}
		if (nargs > 5) {
			warning("VoyagerXSoundXObj: dropping %d extra args", nargs - 5);
			g_lingo->dropStack(nargs - 5);
			nargs = 5;
		}
		bool autoStop = false;
		int duration = 0;
		if (nargs == 5) {
			autoStop = (bool)g_lingo->pop().asInt();
			nargs--;
		}
		if (nargs == 4) {
			duration = g_lingo->pop().asInt();
			nargs--;
		}
		int endVol = g_lingo->pop().asInt();
		int chan = g_lingo->pop().asInt();
		g_lingo->pop();

		g_lingo->push(Datum(xobj->fade(chan, endVol, duration, autoStop)));
	} else if (cmdName == "frequency") {
		ARGNUMCHECK(3);
		Datum percent = g_lingo->pop();
		Datum chan = g_lingo->pop();
		g_lingo->pop();
		xobj->frequency(chan.asInt(), percent.asInt());
		g_lingo->push(1);
	} else if (cmdName == "pan") {
		ARGNUMCHECK(3);
		Datum percent = g_lingo->pop();
		Datum chan = g_lingo->pop();
		g_lingo->pop();
		xobj->pan(chan.asInt(), percent.asInt());
		g_lingo->push(1);
	} else {
		warning("XSoundXFCN: unknown command %s", cmdName.c_str());
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum());
		return;
	}
}

}
