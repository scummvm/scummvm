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

#ifndef PETKA_PANEL_H
#define PETKA_PANEL_H

#include "petka/interfaces/interface.h"

namespace Petka {

class InterfacePanel : public SubInterface {
public:
	InterfacePanel();

	void start(int id) override;

	void onLeftButtonDown(Common::Point p) override;
	void onRightButtonDown(Common::Point p) override;
	void onMouseMove(Common::Point p) override;

	int getHeroSpeed();
	bool showSubtitles() const { return _subtitles; }

private:
	void readSettings();
	void applySettings();

	void updateSliders();
	void updateSubtitles();

private:
	bool _subtitles;
	int _speechFrame;
	int _musicFrame;
	int _sfxFrame;
	int _speedFrame;
	Common::Point _objectPoints[24];
};

} // End of namespace Petka

#endif
