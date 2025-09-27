#include "access/noctropolis/noctropolis_comicviewer.h"

namespace Access {

namespace Noctropolis {

int PolygonResource::findPolygonAt(int16 x, int16 y) const {
	for (uint i = 0; i < _polygons.size(); i++) {
		if (_polygons[i].pointInside(x, y))
			return i;
	}
	return -1;
}
      
void PolygonResource::internalLoad(Common::SeekableReadStream &source, uint32 size) {
	int polygonCount = source.readUint32LE();
	while (polygonCount--) {
		Polygon polygon;
		int pointsCount = source.readUint16LE();
		while (pointsCount--) {
			Common::Point point;
			point.x = source.readUint16LE();
			point.y = source.readUint16LE();
			polygon.points.push_back(point);
		}
		_polygons.push_back(polygon);
	}
}
  
void PolygonResource::free() {
	_polygons.clear();
}


ComicViewer::ComicViewer(NoctropolisEngine *vm)
	: _vm(vm)
{
}

ComicViewer::~ComicViewer() {
}

void ComicViewer::run(ComicResource *comic) {

	PageResult result = kPageResultNone;

	_currPage = 0;
	
	debug("ComicViewer::run() getCount() = %d", comic->getCount());

	while (result != kPageResultExit) {
		result = runPage(comic->getPage(_currPage));
		if (result == kPageResultNextPage && _currPage + 1 < comic->getCount()) {
			_currPage++;
		} else if (result == kPageResultPrevPage && _currPage > 0) {
			_currPage--;
		}
	}

}

PageResult ComicViewer::runPage(ComicPage *page) {
	PageResult result = kPageResultNone;

	_vm->_files->loadScreen(Common::Path(page->filename.baseName()));
	_vm->_buffer2.blitFrom(*_vm->_screen);

	Resource *bubbleData = _vm->_files->loadRawFile("comic.ap");
	_bubbleSprites = new SpriteResource(_vm, bubbleData);
	delete bubbleData;

	// TODO: Copy the palette else the fade out will be wrong?
	//_vm->_screen->palPaletteRange(pagePicture->getPalette(), 0, 256);
	//_vm->copySystemPalette();
	//_vm->setMainPalette();

	//vgaScreen->drawScreen(pagePicture, 640, 400);

	while (result == kPageResultNone) {
	
		_vm->_events->pollEvents();
		_vm->_events->delayUntilNextFrame();

		if (_vm->_events->_mousePos.y >= 389) {
			PageResult naviResult = kPageResultNone;
			if (_vm->_events->_mousePos.x < 59)
				naviResult = kPageResultPrevPage;
			else if (_vm->_events->_mousePos.x >= 580)
				naviResult = kPageResultNextPage;
			if (naviResult != kPageResultNone) {
				_vm->_events->setCursor(CURSOR_CROSSHAIRS);
				if (_vm->_events->_leftButton) {
					_vm->_events->debounceLeft();
					result = naviResult;
					break;
				}
				continue;
			}
		}

		int hotspotIndex = -1;
		for (uint i = 0; i < page->hotspots.size(); i++) {
			const ComicPageHotspot &hotspot = page->hotspots[i];
			if (hotspot.polygons.findPolygonAt(_vm->_events->_mousePos.x, _vm->_events->_mousePos.y) != -1) {
				hotspotIndex = i;
				break;
			}
		}

		if (hotspotIndex != -1) {
			_vm->_events->setCursor(CURSOR_CROSSHAIRS);
			if (_vm->_events->_leftButton) {
				_vm->_events->debounceLeft();
				const ComicPageHotspot &hotspot = page->hotspots[hotspotIndex];
				// TODO: Play hotspot sound
				for (uint bubbleIndex = 0; bubbleIndex < hotspot.bubbles.size(); bubbleIndex++) {
					const ComicPageBubble &bubble = hotspot.bubbles[bubbleIndex];
					drawBubble(bubble);
					_vm->_events->waitKeyActionMouse();
					//_vm->drawScreen(pagePicture, 640, 400);
				}
			}
		} else {
			_vm->_events->setCursor(CURSOR_ARROW);
		}
		
		if (_vm->_events->_rightButton) {
			result = kPageResultExit;
			_vm->_events->debounceLeft();
		}

		_vm->_events->delay();

	}

	delete _bubbleSprites;

	return result;

}

void ComicViewer::drawBubble(const ComicPageBubble &bubble) {

	static const struct {
		struct { int16 px, py; } positions[4];
		int16 sprites[3];
	} kBubbleDef[] = {
		{{{-43, -13}, {+159, -9}, {-44, +63}, {+150, +61}}, {3, 6, 0}},
		{{{-48, -15}, {+147, -13}, {-54, +42}, {+139, +44}}, {4, 7, 1}},
		{{{-53, -17}, {+148, -14}, {-58, +27}, {+143, +28}}, {5, 8, 2}}
	};

	const Font *font;
	uint16 textWidth, textHeight;
	uint16 bubbleX = bubble.x;
	uint16 bubbleY = bubble.y;

	// TODO: is this the right font choice?
	if (bubble.style == 0) {
		font = _vm->_fonts._font1;
	} else {
		font = _vm->_fonts._font2;
	}

	textWidth = font->stringWidth(bubble.text);
	textHeight = font->stringHeight(bubble.text);

	if (bubble.style == 0) {
		_vm->_screen->fillRect(Common::Rect(bubble.x - 4, bubble.y - 4, bubble.x + 4 + textWidth, bubble.y + 4 + textHeight), 243);
		_vm->_screen->frameRect(Common::Rect(bubble.x - 4, bubble.y - 4, bubble.x + 4 + textWidth, bubble.y + 4 + textHeight), 244);
		// TODO: is this color the one to set?
		Font::_fontColors[0] = bubble.textColor;
		font->drawString(_vm->_screen, bubble.text, Common::Point(bubble.x, bubble.y));
	} else {

		int spriteIndex1, spriteIndex2, style, defIndex;
		byte color = bubble.textColor;
		byte textColor;

		if (bubble.style < 5) {
			style = bubble.style - 1;
			spriteIndex1 = style + 0;
		} else {
			style = bubble.style - 5;
			spriteIndex1 = style + 8;
		}

		if (textHeight > 40)
			defIndex = 0;
		else if (textHeight > 28)
			defIndex = 1;
		else
			defIndex = 2;
		
		bubbleX += kBubbleDef[defIndex].positions[style].px;
		bubbleY += kBubbleDef[defIndex].positions[style].py;

		if (color == 246) {
			textColor = 255;
			spriteIndex2 = kBubbleDef[defIndex].sprites[0];
			spriteIndex1 += 13;
		} else if (color == 244) {
			textColor = 255;
			spriteIndex2 = kBubbleDef[defIndex].sprites[1];
			spriteIndex1 += 13;
		} else {
			textColor = 246;
			spriteIndex2 = kBubbleDef[defIndex].sprites[2];
			spriteIndex1 += 9;
		}
		
		_vm->_screen->plotImage(_bubbleSprites, spriteIndex2, Common::Point(bubble.x, bubble.y));
		_vm->_screen->plotImage(_bubbleSprites, spriteIndex1, Common::Point(bubbleX, bubbleY));
		Font::_fontColors[0] = textColor;
		font->drawString(_vm->_screen, bubble.text, Common::Point(bubble.x + 7, bubble.y + 10));

	}

}

}

}
