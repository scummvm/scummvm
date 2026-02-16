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

void XpLib::blit() {

}

void XpLib::maskBlit() {
}

void XpLib::getPalette() {
}

void XpLib::setPalette() {
}

void XpLib::startCycle() {
}

void XpLib::stopCycle() {
}

void XpLib::setScreenBrightness() {
}

bool XpLib::chooseDisplaySpec(int *outMode, int numSpecs, DisplaySpecs *specs) {
	return false;
}

void XpLib::setCoordSpec(int32 x, int32 y, int32 width, int32 height) {
}

void XpLib::displayPic() {
}

void XpLib::updateDisplay() {
}

void XpLib::setFrameRate() {
}

void XpLib::setTransparency(bool toggle) {
}

void XpLib::fillDisplay() {
}

} // End of namespace Bolt
