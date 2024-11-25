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
 * Journey to the Source
 *
 *************************************/

#include "director/director.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/quicktime.h"
#include "video/qt_decoder.h"

namespace Director {

const char *const Quicktime::xlibName = "quicktime";
const XlibFileDesc Quicktime::fileNames[] = {
	{ "QuickTime",  nullptr },
	{ nullptr,	  nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "playStage",	   Quicktime::m_playStage,	  3, 3,  300 },
	// Defined in the xobj itself but never used
	// { "mPlaySmall",	  Quicktime::m_playSmall,	  3, 3,  300 },
	{ nullptr, nullptr, 0, 0, 0 }
};

void Quicktime::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		QuicktimeObject::initMethods(xlibMethods);
		QuicktimeObject *xobj = new QuicktimeObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void Quicktime::close(ObjectType type) {
	if (type == kXObj) {
		QuicktimeObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

QuicktimeObject::QuicktimeObject(ObjectType ObjectType) :Object<QuicktimeObject>("QuickTime") {
	_objType = ObjectType;
}

void Quicktime::m_playStage(int nargs) {
	int top = g_lingo->pop().asInt();
	int left = g_lingo->pop().asInt();
	Common::String movieTitle = g_lingo->pop().asString();

	// Provided file path begins with the volume
	Common::Path filePath = findPath(movieTitle);

	Video::QuickTimeDecoder *video = new Video::QuickTimeDecoder();
	if (!video->loadFile(filePath)) {
		delete video;
		g_lingo->push(Datum());
		return;
	}

	if (!video->isPlaying()) {
		video->setRate(1);
		video->start();
	}

	Graphics::Surface const *frame = nullptr;
	bool keepPlaying = true;
	Common::Event event;
	while (!video->endOfVideo()) {
		if (g_director->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_QUIT:
				g_director->processEventQUIT();
				// fallthrough
			case Common::EVENT_RBUTTONDOWN:
			case Common::EVENT_LBUTTONDOWN:
				keepPlaying = false;
				break;
			default:
				break;
			}
		}
		if (!keepPlaying)
			break;
		if (video->needsUpdate()) {
			frame = video->decodeNextFrame();
			if (frame != nullptr)
				g_system->copyRectToScreen(frame->getPixels(), frame->pitch, left, top, frame->w, frame->h);
		}
		g_system->updateScreen();
		g_director->delayMillis(10);
	}

	if (frame != nullptr)
		// Display the last frame after the video is done
		g_director->getCurrentWindow()->getSurface()->copyRectToSurface(
			frame->getPixels(), frame->pitch, left, top, frame->w, frame->h
		);

	video->close();
	delete video;

	g_lingo->push(Datum());
}

} // End of namespace Director
