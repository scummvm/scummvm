/* Copyright (C) 1994-2003 Revolution Software Ltd
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

class Sword2Gui {
public:
	int _baseSlot;
	uint8 _currentGraphicsLevel;

	uint8 _subtitles;
	uint8 _speechSelected;
	uint8 _stereoReversed;
	uint8 _pointerTextSelected;

	Sword2Gui() : _baseSlot(0), _stereoReversed(0),
		_pointerTextSelected(0) {}

	uint32 restoreControl(void);
	void saveControl(void);
	void quitControl(void);
	void restartControl(void);
	void optionControl(void);
	int32 readOptionSettings(void);
	void updateGraphicsLevel(uint8 newLevel);
};

extern Sword2Gui gui;

#endif
