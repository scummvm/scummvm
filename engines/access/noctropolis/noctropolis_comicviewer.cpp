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

#include "access/noctropolis/noctropolis_comicviewer.h"

namespace Access {

namespace Noctropolis {


ComicResource::ComicResource(const ComicPage *pages[], int npages) {
	for (int i = 0; i < npages; i++)
		_pages.push_back(pages[i]);
}

////////////////

ComicViewer::ComicViewer(NoctropolisEngine *vm) : _vm(vm) {
}

ComicViewer::~ComicViewer() {
}

void ComicViewer::run(const ComicResource *comic) {
	PageResult result = kPageResultNone;
	_currPage = 0;

	debug("ComicViewer::run() getCount() = %d", comic->getCount());

	_vm->_screen->savePalette();

	while (result != kPageResultExit) {
		result = runPage(comic->getPage(_currPage));
		if (result == kPageResultNextPage && _currPage + 1 < comic->getCount()) {
			_currPage++;
		} else if (result == kPageResultPrevPage && _currPage > 0) {
			_currPage--;
		}
	}

	// Fade out and then restore the screen
	// First copy the comic palette or fade out will be wrong
	_vm->_screen->copyRawPalToTempPal();
	_vm->_screen->fadeOut();
	// Now restore the original screen and pal.
	_vm->_screen->clearScreen();
	_vm->_screen->restorePalette();
	_vm->_screen->setPalette();
	_vm->_screen->copyRawPalToTempPal();
	_vm->copyBF2Vid();
}

PageResult ComicViewer::runPage(const ComicPage *page) {
	PageResult result = kPageResultNone;

	_vm->_files->loadScreen(Common::Path(page->filename));

	Resource *bubbleData = _vm->_files->loadRawFile("COMDATA/comic.ap");
	_bubbleSprites = new SpriteResource(_vm, bubbleData);
	delete bubbleData;

	bool playedSound = false;

	while (result == kPageResultNone) {
		_vm->_events->pollEvents();
		_vm->_events->delayUntilNextFrame();
		Common::CustomEventType action = kActionNone;
		_vm->_events->getAction(action);

		if (action == kActionMoveLeft) {
			result = kPageResultPrevPage;
			continue;
		} else if (action == kActionMoveRight) {
			result = kPageResultNextPage;
			continue;
		}

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
		for (int i = 0; i < page->numBlocks; i++) {
			const ComicBlock &hotspot = page->blocks[i];
			if (hotspot.polygon->pointInside(_vm->_events->_mousePos.x, _vm->_events->_mousePos.y)) {
				hotspotIndex = i;
				break;
			}
		}

		if (hotspotIndex != -1) {
			_vm->_events->setCursor(CURSOR_CROSSHAIRS);
			if (_vm->_events->_leftButton) {
				_vm->_events->debounceLeft();
				const ComicBlock &hotspot = page->blocks[hotspotIndex];

				if (hotspot.soundFileIndex >= 0) {
					_vm->_sound->loadSoundTable(1, hotspot.soundFileIndex, hotspot.soundResIndex);
					_vm->_sound->playSound(1);
					playedSound = true;
				}
				// TODO: Play hotspot sound
				for (int bubbleIndex = 0; bubbleIndex < hotspot.numBoxes; bubbleIndex++) {
					const ComicBox &bubble = hotspot.boxes[bubbleIndex];
					// Slightly lazy, just save/restore whole screen when
					// drawing the comic bubble rather than trying to
					// calculate the size here.
					_vm->_screen->saveBlock(Common::Rect(640, 400));
					if (!drawBubble(bubble))
						continue;
					_vm->_events->setCursor(CURSOR_NONE);
					_vm->_events->waitKeyActionMouse();
					_vm->_events->setCursor(CURSOR_CROSSHAIRS);
					_vm->_screen->restoreBlock();
				}
			}
		} else {
			_vm->_events->setCursor(CURSOR_ARROW);
		}

		if (_vm->_events->_rightButton || action == kActionSkip) {
			result = kPageResultExit;
			_vm->_events->debounceRight();
		}
	}

	if (playedSound) {
		_vm->_sound->freeSound(1);
	}

	delete _bubbleSprites;

	return result;

}

static void drawString(const char *str, const Font *font, Screen *screen, int x, int y) {
	Common::Point pt(x, y);
	bool lastLine = false;
	Common::String remainder = str;
	while (!lastLine) {
		Common::String line;
		int width = 640;
		lastLine = font->getLine(remainder, 640, line, width, Font::kWidthInPixels);
		font->drawString(screen, line, pt);
		pt.y += font->stringHeight(line);
	}
}

bool ComicViewer::drawBubble(const ComicBox &bubble) {
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
		font = _vm->_fonts.getFont(5);
	} else {
		font = _vm->_fonts.getFont(6);
	}

	const char *text;
	switch (_vm->getLanguage()) {
	case Common::EN_ANY: text = bubble.msgEn; break;
	case Common::FR_FRA: text = bubble.msgFr; break;
	case Common::ES_ESP: text = bubble.msgEs; break;
	case Common::DE_DEU: text = bubble.msgDe; break;
	default: error("Unsupported language in drawBubble");
	}

	// Some translations have more bubbles to fit the text.
	if (text == nullptr)
		return false;

	textWidth = font->stringWidth(text);
	textHeight = font->stringHeight(text);

	if (bubble.style == 0) {
		_vm->_screen->fillRect(Common::Rect(bubble.x - 4, bubble.y - 4, bubble.x + 4 + textWidth, bubble.y + 4 + textHeight), 243);
		_vm->_screen->frameRect(Common::Rect(bubble.x - 4, bubble.y - 4, bubble.x + 4 + textWidth, bubble.y + 4 + textHeight), 244);
		// TODO: is this color the one to set?
		Font::_fontColors[1] = bubble.textColor;
		drawString(text, font, _vm->_screen, bubble.x, bubble.y);
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
		Font::_fontColors[1] = textColor;
		drawString(text, font, _vm->_screen, bubble.x + 7, bubble.y + 10);
	}

	return true;
}

} // end namespace Noctropolis

} // end namespace Access
