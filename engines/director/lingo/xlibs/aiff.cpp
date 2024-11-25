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

/*************************************
 *
 * USED IN:
 * Gahan Wilson's Ultimate Haunted House
 *
 *************************************/

/*
 * -- copyright 1994 by Byron Priess Multimedia, authored by MayoSmith and Lee
 * aiff
 * I      mNew                --Read Docs to avoid Hard Drive failure
 * X      mDispose            --Disposes of XObject instance
 * S      mName               --Returns the XObject name (Widget)
 * I      mStatus             --Returns an integer status code
 * SI     mError, code        --Returns an error string
 * S      mLastError          --Returns last error string
 * III    mAdd, arg1, arg2    --Returns arg1+arg2
 * SSI    mFirst, str, nchars --Return the first nchars of string str
 * V      mMul, f1, f2        --Returns f1*f2 as floating point
 * X      mGlobals            --Sample code to Read & Modify globals
 * X      mSymbols            --Sample code to work with Symbols
 * X      mSendPerform        --Sample code to show SendPerform call
 * X      mFactory            --Sample code to find Factory objects
 * IS     mDuration, str      --Read Docs to avoid Hard Drive failure
 */

#include "common/macresman.h"
#include "audio/decoders/aiff.h"
#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/aiff.h"


namespace Director {

const char *const AiffXObj::xlibName = "aiff";
const XlibFileDesc AiffXObj::fileNames[] = {
	{ "AIFF",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",   		AiffXObj::m_new,					0,	0,	400 },	// D4
	{ "Duration",   AiffXObj::m_duration,				1,	1,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void AiffXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		AiffXObject::initMethods(xlibMethods);
		AiffXObject *xobj = new AiffXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void AiffXObj::close(ObjectType type) {
	if (type == kXObj) {
		AiffXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


AiffXObject::AiffXObject(ObjectType ObjectType) :Object<AiffXObject>("Aiff") {
	_objType = ObjectType;
}

void AiffXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AiffXObj::new", nargs);
	g_lingo->push(g_lingo->_state->me);
}

void AiffXObj::m_duration(int nargs) {
	g_lingo->printSTUBWithArglist("AiffXObj::m_duration", nargs);
	auto filePath = g_lingo->pop().asString();

	auto aiffStream = Common::MacResManager::openFileOrDataFork(findPath(filePath));
	if (!aiffStream) {
		warning("Failed to open %s", filePath.c_str());
		g_lingo->push(0);
		return;
	}

	auto aiffHeader = Audio::AIFFHeader::readAIFFHeader(aiffStream, DisposeAfterUse::YES);

	int duration = (aiffHeader->getFrameCount() / (float)aiffHeader->getFrameRate()) * 60;

	delete aiffHeader;
	delete aiffStream;
	g_lingo->push(Datum(duration));
}

} // End of namespace Director
