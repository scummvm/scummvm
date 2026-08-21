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
#ifndef SLUDGE_STATUSBA_H
#define SLUDGE_STATUSBA_H

namespace Sludge {

struct StatusBar {
	Common::String text;
	StatusBar *next;
};

struct StatusStuff {
	StatusBar *firstStatusBar;
	uint16 alignStatus;
	int litStatus;
	int statusX, statusY;
	int statusR, statusG, statusB;
	int statusLR, statusLG, statusLB;
};

class StatusBarManager {
public:
	StatusBarManager(SludgeEngine *sludge);

	void init();
	void set(Common::String &txt);
	void clear();
	void addStatusBar();
	void killLastStatus();
	void statusBarColour(byte r, byte g, byte b);
	void statusBarLitColour(byte r, byte g, byte b);
	void setLitStatus(int i);
	const Common::String statusBarText();
	void positionStatus(int, int);
	void draw();

	// Load and save
	bool loadStatusBars(Common::SeekableReadStream *stream);
	void saveStatusBars(Common::WriteStream *stream);

	// For freezing
	void restoreBarStuff(StatusStuff *here);
	StatusStuff *copyStatusBarStuff(StatusStuff *here);

	void setAlignStatus(uint16 val) { _nowStatus->alignStatus = val; }

private:
	SpritePalette _verbLinePalette;
	SpritePalette _litVerbLinePalette;

	StatusStuff _mainStatus;
	StatusStuff *_nowStatus;

	SludgeEngine *_sludge;
};

} // End of namespace Sludge

#endif
