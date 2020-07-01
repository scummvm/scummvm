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

#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/stage.h"
#include "director/score.h"
#include "director/castmember.h"
#include "director/sprite.h"

namespace Director {

Stage::Stage(int id, bool scrollable, bool resizable, bool editable, Graphics::MacWindowManager *wm)
	: MacWindow(id, scrollable, resizable, editable, wm) {
	_stageColor = 0;
	_puppetTransition = nullptr;
}

bool Stage::render(bool forceRedraw, Graphics::ManagedSurface *blitTo) {
	if (!blitTo)
		blitTo = &_surface;

	if (forceRedraw) {
		blitTo->clear(_stageColor);
		_dirtyRects.clear();
		_dirtyRects.push_back(Common::Rect(_surface.w, _surface.h));
	} else {
		if (_dirtyRects.size() == 0)
			return true;

		mergeDirtyRects();
	}

	for (Common::List<Common::Rect>::iterator i = _dirtyRects.begin(); i != _dirtyRects.end(); i++) {
		const Common::Rect &r = *i;
		blitTo->fillRect(r, _stageColor);

		_dirtyChannels = g_director->getCurrentMovie()->getScore()->getSpriteIntersections(r);
		for (Common::List<Channel *>::iterator j = _dirtyChannels.begin(); j != _dirtyChannels.end(); j++)
			inkBlitFrom(*j, r, blitTo);
	}

	_dirtyRects.clear();
	_contentIsDirty = true;

	return true;
}

void Stage::setStageColor(uint stageColor) {
	if (stageColor != _stageColor) {
		_stageColor = stageColor;
		reset();
	}
}

void Stage::reset() {
	_surface.clear(_stageColor);
	_contentIsDirty = true;
}

void Stage::addDirtyRect(const Common::Rect &r) {
	Common::Rect bounds = r;
	bounds.clip(_dims);

	if (bounds.width() > 0 && bounds.height() > 0)
		_dirtyRects.push_back(bounds);
}

void Stage::mergeDirtyRects() {
	Common::List<Common::Rect>::iterator rOuter, rInner;

	// Process the dirty rect list to find any rects to merge
	for (rOuter = _dirtyRects.begin(); rOuter != _dirtyRects.end(); ++rOuter) {
		rInner = rOuter;
		while (++rInner != _dirtyRects.end()) {

			if ((*rOuter).intersects(*rInner)) {
				// These two rectangles overlap, so merge them
				unionRectangle(*rOuter, *rOuter, *rInner);

				// remove the inner rect from the list
				_dirtyRects.erase(rInner);

				// move back to beginning of list
				rInner = rOuter;
			}
		}
	}
}

bool Stage::unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2) {
	destRect = src1;
	destRect.extend(src2);

	return !destRect.isEmpty();
}

void Stage::inkBlitFrom(Channel *channel, Common::Rect destRect, Graphics::ManagedSurface *blitTo) {
	Common::Rect srcRect = channel->getBbox();
	destRect.clip(srcRect);

	Sprite *sprite = channel->_sprite;
	MacShape *ms = channel->getShape();
	DirectorPlotData pd(_wm, channel->getSurface(), blitTo, sprite->_ink, sprite->_backColor, g_director->getPaletteColorCount());
	pd.destRect = destRect;

	// Shapes do not have surfaces of their own, so draw the shape directly upon
	// stage surface.
	if (ms) {
		if (ms->foreColor == ms->backColor)
			return;

		Common::Rect fillRect((int)srcRect.width(), (int)srcRect.height());
		fillRect.moveTo(srcRect.left, srcRect.top);
		Graphics::MacPlotData plotFill(blitTo, nullptr, &g_director->getPatterns(), sprite->getPattern(), srcRect.left, srcRect.top, 1, ms->backColor);

		Common::Rect strokeRect(MAX((int)srcRect.width() - ms->lineSize, 0), MAX((int)srcRect.height() - ms->lineSize, 0));
		strokeRect.moveTo(srcRect.left, srcRect.top);
		Graphics::MacPlotData plotStroke(blitTo, nullptr, &g_director->getPatterns(), 1, strokeRect.left, strokeRect.top, ms->lineSize, ms->backColor);

		switch (ms->spriteType) {
		case kRectangleSprite:
			pd.macPlot = &plotFill;
			Graphics::drawFilledRect(fillRect, ms->foreColor, inkDrawPixel, &pd);
			// fall through
		case kOutlinedRectangleSprite:
			pd.macPlot = &plotStroke;
			Graphics::drawRect(strokeRect, ms->foreColor, inkDrawPixel, &pd);
			break;
		case kRoundedRectangleSprite:
			pd.macPlot = &plotFill;
			Graphics::drawRoundRect(fillRect, 12, ms->foreColor, true, inkDrawPixel, &pd);
			// fall through
		case kOutlinedRoundedRectangleSprite:
			pd.macPlot = &plotStroke;
			Graphics::drawRoundRect(strokeRect, 12, ms->foreColor, false, inkDrawPixel, &pd);
			break;
		case kOvalSprite:
			pd.macPlot = &plotFill;
			Graphics::drawEllipse(fillRect.left, fillRect.top, fillRect.right, fillRect.bottom, ms->foreColor, true, inkDrawPixel, &plotFill);
			// fall through
		case kOutlinedOvalSprite:
			pd.macPlot = &plotStroke;
			Graphics::drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, false, inkDrawPixel, &plotStroke);
			break;
		case kLineTopBottomSprite:
			pd.macPlot = &plotStroke;
			Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, inkDrawPixel, &plotStroke);
			break;
		case kLineBottomTopSprite:
			pd.macPlot = &plotStroke;
			Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, inkDrawPixel, &plotStroke);
			break;
		default:
			warning("Stage::inkBlitFrom: Expected shape type but got type %d", ms->spriteType);
		}

		delete ms;
		return;
	}

	// Otherwise, we are drawing a cast type that does have a built-in surface, so
	// blit from that.
	// TODO: Work these ink types into inkDrawPixel.
	if (sprite->_ink == kInkTypeMatte) {
		drawMatteSprite(channel, srcRect, destRect, blitTo);
		return;
	} else if (sprite->_ink == kInkTypeReverse) {
		drawReverseSprite(channel, srcRect, destRect, blitTo);
		return;
	}
	// Otherwise, fall through to inkDrawPixel

	pd.srcPoint.y = MAX(abs(srcRect.top - destRect.top), 0);
	for (int i = 0; i < destRect.height(); i++, pd.srcPoint.y++) {
		pd.srcPoint.x = MAX(abs(srcRect.left - destRect.left), 0);

		for (int j = 0; j < destRect.width(); j++, pd.srcPoint.x++)
			inkDrawPixel(destRect.left + j, destRect.top + i, 0, &pd);
	}
}

void Stage::drawMatteSprite(Channel *channel, Common::Rect &srcRect, Common::Rect &destRect, Graphics::ManagedSurface *blitTo) {
	// Like background trans, but all white pixels NOT ENCLOSED by coloured pixels are transparent
	Graphics::Surface tmp;
	tmp.create(destRect.width(), destRect.height(), Graphics::PixelFormat::createFormatCLUT8());
	tmp.copyFrom(channel->getSurface()->rawSurface());

	if (!blitTo->clip(srcRect, destRect))
		return; // Out of screen

	// Searching white color in the corners
	int whiteColor = -1;

	for (int y = 0; y < tmp.h; y++) {
		for (int x = 0; x < tmp.w; x++) {
			byte color = *(byte *)tmp.getBasePtr(x, y);

			if (g_director->getPalette()[color * 3 + 0] == 0xff &&
				g_director->getPalette()[color * 3 + 1] == 0xff &&
				g_director->getPalette()[color * 3 + 2] == 0xff) {
				whiteColor = color;
				break;
			}
		}
	}

	if (whiteColor == -1) {
		debugC(1, kDebugImages, "Score::drawMatteSprite(): No white color for Matte image");

		for (int yy = 0; yy < destRect.height(); yy++) {
		const byte *src = (const byte *)channel->getSurface()->getBasePtr(MAX(abs(srcRect.left - destRect.left), 0), MAX(abs(srcRect.top - destRect.top + yy), 0));
		byte *dst = (byte *)blitTo->getBasePtr(destRect.left, destRect.top + yy);

			for (int xx = 0; xx < destRect.width(); xx++, src++, dst++)
				*dst = *src;
		}
	} else {
		Graphics::FloodFill ff(&tmp, whiteColor, 0, true);

		for (int yy = 0; yy < tmp.h; yy++) {
			ff.addSeed(0, yy);
			ff.addSeed(tmp.w - 1, yy);
		}

		for (int xx = 0; xx < tmp.w; xx++) {
			ff.addSeed(xx, 0);
			ff.addSeed(xx, tmp.h - 1);
		}
		ff.fillMask();

		for (int yy = 0; yy < destRect.height(); yy++) {
			const byte *mask = (const byte *)ff.getMask()->getBasePtr(MAX(abs(srcRect.left - destRect.left), 0), MAX(abs(srcRect.top - destRect.top - yy), 0));
			const byte *src = (const byte *)channel->getSurface()->getBasePtr(MAX(abs(srcRect.left - destRect.left), 0), MAX(abs(srcRect.top - destRect.top - yy), 0));
			byte *dst = (byte *)blitTo->getBasePtr(destRect.left, destRect.top + yy);

			for (int xx = 0; xx < destRect.width(); xx++, src++, dst++, mask++)
				if (*mask == 0)
					*dst = *src;
		}
	}

	tmp.free();
}

void Stage::drawReverseSprite(Channel *channel, Common::Rect &srcRect, Common::Rect &destRect, Graphics::ManagedSurface *blitTo) {
	uint8 skipColor = g_director->getPaletteColorCount() - 1;
	for (int ii = 0; ii < destRect.height(); ii++) {
		const byte *src = (const byte *)channel->getSurface()->getBasePtr(MAX(abs(srcRect.left - destRect.left), 0), MAX(abs(srcRect.top - destRect.top - ii), 0));
		byte *dst = (byte *)blitTo->getBasePtr(destRect.left, destRect.top + ii);
		byte srcColor = *src;

		for (int j = 0; j < destRect.width(); j++, src++, dst++) {
			if (!channel->_sprite->_cast || channel->_sprite->_cast->_type == kCastShape)
				srcColor = 0x0;
			else
				srcColor = *src;
			uint16 targetSprite = g_director->getCurrentMovie()->getScore()->getSpriteIDFromPos(Common::Point(destRect.left + j, destRect.top + ii));
			if ((targetSprite != 0)) {
				// TODO: This entire reverse colour attempt needs a lot more testing on
				// a lot more colour depths.
				if (srcColor != skipColor) {
					if (!g_director->getCurrentMovie()->getScore()->_channels[targetSprite]->_sprite->_cast ||  g_director->getCurrentMovie()->getScore()->_channels[targetSprite]->_sprite->_cast->_type != kCastBitmap) {
						if (*dst == 0 || *dst == 255) {
							*dst = g_director->transformColor(*dst);
						} else if (srcColor == 255 || srcColor == 0) {
							*dst = g_director->transformColor(*dst - 40);
						} else {
							*dst = g_director->transformColor(*src - 40);
						}
					} else {
						if (*dst == 0 && g_director->getVersion() == 3 &&
								g_director->getCurrentMovie()->getScore()->_channels[targetSprite]->_sprite->_cast->_type == kCastBitmap &&
								((BitmapCastMember*)g_director->getCurrentMovie()->getScore()->_channels[targetSprite]->_sprite->_cast)->_bitsPerPixel > 1) {
							*dst = g_director->transformColor(*src - 40);
						} else {
							*dst ^= g_director->transformColor(srcColor);
						}
					}
				}
			} else if (srcColor != skipColor) {
				*dst = g_director->transformColor(srcColor);
			}
		}
	}
}

} // end of namespace Director
