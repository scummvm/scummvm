/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef	_CONTROL_S
#define	_CONTROL_S

namespace Sword2 {

class Sword2Engine;

class Gui {
private:
	int _musicVolume[17];
	int _soundVolume[15];

public:
	Sword2Engine *_vm;

	int _baseSlot;
	uint8 _currentGraphicsLevel;

	bool _subtitles;
	bool _stereoReversed;
	bool _pointerTextSelected;

	Gui(Sword2Engine *vm);

	uint32 restoreControl(void);
	void saveControl(void);
	bool startControl(void);
	void quitControl(void);
	void restartControl(void);
	void optionControl(void);
	void readOptionSettings(void);
	void writeOptionSettings(void);
	void updateGraphicsLevel(int newLevel);
};

} // End of namespace Sword2

#endif
