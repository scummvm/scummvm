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

#ifndef ACCESS_NOCTROPOLIS_NOCTROPOLIS_COMICVIEWER_H
#define ACCESS_NOCTROPOLIS_NOCTROPOLIS_COMICVIEWER_H

#include "access/noctropolis/noctropolis_game.h"
#include "access/resources.h"
#include "access/polygon.h"

namespace Access {

namespace Noctropolis {

struct ComicPageBubble {
    byte style;
    byte textColor;
    int16 x, y;
    Common::String text;
};

class PolygonResource : public Resource {
public:
    int findPolygonAt(int16 x, int16 y) const;
    void free();
protected:
    PolygonArray _polygons;
    void internalLoad(Common::SeekableReadStream &source, uint32 size);
};

struct ComicPageHotspot {
    PolygonResource polygons;
    int32 soundFileIndex, soundResIndex;
    Common::Array<ComicPageBubble> bubbles;
};

struct ComicPage {
    Common::Path filename;
    int32 musicFileIndex, musicResIndex;
    Common::Array<ComicPageHotspot> hotspots;
};

class ComicResource : public Resource {
public:
    uint16 getCount() const { return _pages.size(); }
    ComicPage *getPage(int index) const { return _pages[index]; }
    void free();
protected:
    Common::Array<ComicPage*> _pages;
    ComicPageHotspot loadHotspot(Common::SeekableReadStream &source);
    void internalLoad(Common::SeekableReadStream &source, uint32 size);
};


enum PageResult {
	kPageResultNone,
	kPageResultExit,
	kPageResultNextPage,
	kPageResultPrevPage
};

class ComicViewer {
public:
	ComicViewer(NoctropolisEngine *vm);
	~ComicViewer();
	
	void run(ComicResource *comic);
	
protected:
	NoctropolisEngine *_vm;
	ComicResource *_comic;
	SpriteResource *_bubbleSprites;
	int _currPage;
	PageResult runPage(ComicPage *page);
	void drawBubble(const ComicPageBubble &bubble);
};

} // end namespace Noctropolis

} // end namespace Access

#endif // ACCESS_NOCTROPOLIS_NOCTROPOLIS_COMICVIEWER_H
