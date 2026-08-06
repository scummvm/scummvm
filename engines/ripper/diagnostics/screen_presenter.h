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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_DIAGNOSTICS_SCREEN_PRESENTER_H
#define RIPPER_DIAGNOSTICS_SCREEN_PRESENTER_H

namespace Ripper {

class ScreenPresenter {
public:
	virtual ~ScreenPresenter() {}
	virtual void presentScreen() = 0;
};

void registerScreenPresenter(ScreenPresenter *presenter);
void unregisterScreenPresenter(ScreenPresenter *presenter);
void presentScreen();

} // End of namespace Ripper

#endif // RIPPER_DIAGNOSTICS_SCREEN_PRESENTER_H
