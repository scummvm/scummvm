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

#include "common/textconsole.h"
#include "bagel/mfc/joystickapi.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

MMRESULT joySetCapture(HWND hwnd, unsigned int uJoyID,
		unsigned int uPeriod, bool fChanged) {
	return AfxGetApp()->joySetCapture(hwnd, uJoyID, uPeriod, fChanged);
}

MMRESULT joySetThreshold(unsigned int uJoyID, unsigned int uThreshold) {
	return AfxGetApp()->joySetThreshold(uJoyID, uThreshold);
}

MMRESULT joyGetPos(unsigned int uJoyID, LPJOYINFO pji) {
	return AfxGetApp()->joyGetPos(uJoyID, pji);
}

MMRESULT joyReleaseCapture(unsigned int uJoyID) {
	return AfxGetApp()->joyReleaseCapture(uJoyID);
}

} // namespace MFC
} // namespace Bagel
