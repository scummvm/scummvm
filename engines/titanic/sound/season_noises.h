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

#ifndef TITANIC_SEASON_NOISES_H
#define TITANIC_SEASON_NOISES_H

#include "titanic/sound/view_auto_sound_player.h"

namespace Titanic {

class CSeasonNoises : public CViewAutoSoundPlayer {
	DECLARE_MESSAGE_MAP;
	bool ChangeSeasonMsg(CChangeSeasonMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool ActMsg(CActMsg *msg);
	bool LoadSuccessMsg(CLoadSuccessMsg *msg);
private:
	Season _seasonNumber;
	CString _springName;
	CString _summerName;
	CString _autumnName;
	CString _winterName;
public:
	CLASSDEF;
	CSeasonNoises();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};

} // End of namespace Titanic

#endif /* TITANIC_SEASON_NOISES_H */
