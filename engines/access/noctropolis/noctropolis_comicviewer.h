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

#include "common/rect.h"
#include "access/noctropolis/noctropolis_game.h"
#include "access/resources.h"
#include "access/polygon.h"

namespace Access {

namespace Noctropolis {

struct ComicBox {
	byte style;
	byte textColor;
	int16 x, y;
	const char *msgEn;
	const char *msgFr;
	const char *msgEs;
	const char *msgDe;
};

struct ComicBlock {
	const struct Polygon *polygon;
	int32 soundFileIndex, soundResIndex;
	const struct ComicBox *boxes;
	int32 numBoxes;
};

struct ComicPage {
	Common::Path filename;
	int32 musicFileIndex, musicResIndex;
	bool musicRepeat;
	int unk;
	const struct ComicBlock *blocks;
	int32 numBlocks;
};

class ComicResource : public Resource {
public:
	ComicResource(const ComicPage *pages[], int npages);
	uint16 getCount() const { return _pages.size(); }
	const ComicPage *getPage(int index) const { return _pages[index]; }

protected:
	Common::Array<const ComicPage *> _pages;
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

	void run(const ComicResource *comic);

protected:
	NoctropolisEngine *_vm;
	ComicResource *_comic;
	SpriteResource *_bubbleSprites;
	int _currPage;
	PageResult runPage(const ComicPage *page);
	bool drawBubble(const ComicBox &bubble);
};

} // end namespace Noctropolis

} // end namespace Access

#endif // ACCESS_NOCTROPOLIS_NOCTROPOLIS_COMICVIEWER_H
