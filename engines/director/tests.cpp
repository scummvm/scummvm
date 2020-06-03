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

#include "common/config-manager.h"
#include "common/system.h"

#include "common/macresman.h"

#include "graphics/fonts/macfont.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"

#include "engines/util.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/score.h"

namespace Director {

//////////////////////
// Graphics tests
//////////////////////
void DirectorEngine::testFontScaling() {
	int x = 10;
	int y = 10;
	int w = 640;
	int h = 480;

	initGraphics(w, h);
	setPalette(-1);

	Graphics::ManagedSurface surface;

	surface.create(w, h);
	surface.clear(255);

	Graphics::MacFont origFont(Graphics::kMacFontNewYork, 18);

	const Graphics::MacFONTFont *font1 = (const Graphics::MacFONTFont *)_wm->_fontMan->getFont(origFont);

	Graphics::MacFONTFont::testBlit(font1, &surface, 0, x, y + 200, 500);

	Graphics::MacFont bigFont(Graphics::kMacFontNewYork, 15);

	font1 = (const Graphics::MacFONTFont *)_wm->_fontMan->getFont(bigFont);

	Graphics::MacFONTFont::testBlit(font1, &surface, 0, x, y + 50 + 200, 500);

	const char *text = "d";

	for (int i = 9; i <= 20; i++) {
		Graphics::MacFont macFont(Graphics::kMacFontNewYork, i);

		const Graphics::Font *font = _wm->_fontMan->getFont(macFont);

		int width = font->getStringWidth(text);

		Common::Rect bbox = font->getBoundingBox(text, x, y, w);
		surface.frameRect(bbox, 15);

		font->drawString(&surface, text, x, y, width, 0);

		x += width + 1;
	}

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			int y1 = 80 + i * 7;
			int x1 = 80 + j * 7;

			for (x = x1; x < x1 + 6; x++)
				for (y = y1; y < y1 + 6; y++)
					*((byte *)surface.getBasePtr(x, y)) = transformColor(i * 16 + j);
		}
	}

	g_system->copyRectToScreen(surface.getPixels(), surface.pitch, 0, 0, w, h); // testing fonts

	Common::Event event;

	while (true) {
		if (g_system->getEventManager()->pollEvent(event))
			if (event.type == Common::EVENT_QUIT)
				break;

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void DirectorEngine::testFonts() {
	Common::String fontName("Helvetica");

	Common::MacResManager *fontFile = new Common::MacResManager();
	if (!fontFile->open(fontName))
		error("testFonts(): Could not open %s as a resource fork", fontName.c_str());

	Common::MacResIDArray fonds = fontFile->getResIDArray(MKTAG('F','O','N','D'));
	if (fonds.size() > 0) {
		for (Common::Array<uint16>::iterator iterator = fonds.begin(); iterator != fonds.end(); ++iterator) {
			Common::SeekableReadStream *stream = fontFile->getResource(MKTAG('F', 'O', 'N', 'D'), *iterator);
			Common::String name = fontFile->getResName(MKTAG('F', 'O', 'N', 'D'), *iterator);

			debug("Font: %s", name.c_str());

			Graphics::MacFontFamily font;
			font.load(*stream);
		}
	}

	delete fontFile;
}

//////////////////////
// Movie iteration
//////////////////////
Common::HashMap<Common::String, Score *> *DirectorEngine::scanMovies(const Common::String &folder) {
	Common::FSNode directory(folder);
	Common::FSList movies;
	const char *sharedMMMname;

	if (getPlatform() == Common::kPlatformWindows)
		sharedMMMname = "SHARDCST.MMM";
	else
		sharedMMMname = "Shared Cast";


	Common::HashMap<Common::String, Score *> *nameMap = new Common::HashMap<Common::String, Score *>();
	if (!directory.getChildren(movies, Common::FSNode::kListFilesOnly))
		return nameMap;

	if (!movies.empty()) {
		for (Common::FSList::const_iterator i = movies.begin(); i != movies.end(); ++i) {
			debugC(2, kDebugLoading, "File: %s", i->getName().c_str());

			if (Common::matchString(i->getName().c_str(), sharedMMMname, true)) {
				_sharedCastFile = i->getName();

				debugC(2, kDebugLoading, "Shared cast detected: %s", i->getName().c_str());
				continue;
			}

			Archive *arc = createArchive();

			warning("name: %s", i->getName().c_str());
			arc->openFile(i->getName());
			Score *sc = new Score(this);
			sc->setArchive(arc);
			nameMap->setVal(sc->getMacName(), sc);

			debugC(2, kDebugLoading, "Movie name: \"%s\"", sc->getMacName().c_str());
		}
	}

	return nameMap;
}

void DirectorEngine::enqueueAllMovies() {
	Common::FSNode dir(ConfMan.get("path"));
	Common::FSList files;
	if (!dir.getChildren(files, Common::FSNode::kListFilesOnly)) {
		warning("DirectorEngine::enqueueAllMovies(): Failed inquiring file list");
		return;
	}

	for (Common::FSList::const_iterator file = files.begin(); file != files.end(); ++file)
		_movieQueue.push_back((*file).getName());

	Common::sort(_movieQueue.begin(), _movieQueue.end());

	debug(1, "=========> Enqueued %d movies", _movieQueue.size());
}

MovieReference DirectorEngine::getNextMovieFromQueue() {
	MovieReference res;

	if (_movieQueue.empty())
		return res;

	res.movie = _movieQueue.front();

	debug(0, "=======================================");
	debug(0, "=========> Next movie is %s", res.movie.c_str());
	debug(0, "=======================================");

	_movieQueue.remove_at(0);

	return res;
}

} // End of namespace Director
