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

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

namespace Bolt {

int16 XpLib::switchDisplayMode(int16 mode) {
	return int16();
}

bool XpLib::initDisplay() {
	return false;
}

void XpLib::shutdownDisplay() {
}

bool XpLib::createSurface(XPSurface *surf) {
	return false;
}

void XpLib::freeSurface(XPSurface *surf) {
}

bool XpLib::chooseDisplaySpec(int *outMode, int numSpecs, DisplaySpecs *specs) {
	return false;
}

void XpLib::setCoordSpec(int16 x, int16 y, int16 width, int16 height) {
}

void XpLib::virtualToScreen(int16 *x, int16 *y) {
}

void XpLib::screenToVirtual(int16 *x, int16 *y) {
}

void XpLib::displayPic(XPPicDesc *pic, int16 x, int16 y, int16 page) {
}

bool XpLib::clipAndBlit(XPPicDesc *src, XPSurface *dest, int16 x, int16 y, ClipRect *outClip) {
	return false;
}

void XpLib::addDirtyRect(ClipRect *rect) {
}

void XpLib::setFrameRate(int16 fps) {
}

void XpLib::updateDisplay() {
}

void XpLib::waitForFrameRate() {
}

void XpLib::handlePaletteTransitions() {
}

void XpLib::flushPalette() {
}

void XpLib::overlayComposite() {
}

void XpLib::compositeToScreen() {
}

void XpLib::mergeDirtyRects() {
}

void XpLib::blitDirtyRects(ClipRect *rects, int16 count) {
}

void XpLib::compositeDirtyRects(ClipRect *rects, int16 count) {
}

void XpLib::applyCursorPalette(bool enable) {
}

void XpLib::prepareBackSurface() {
}

void XpLib::setTransparency(bool toggle) {
}

void XpLib::fillDisplay(byte color, int16 page) {
}

} // End of namespace Bolt
