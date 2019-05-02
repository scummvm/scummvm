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

#include "titanic/game/auto_animate.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CAutoAnimate, CBackground)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(InitializeAnimMsg)
END_MESSAGE_MAP()

void CAutoAnimate::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_enabled, indent);
	file->writeNumberLine(_redo, indent);
	file->writeNumberLine(_repeat, indent);
	CBackground::save(file, indent);
}

void CAutoAnimate::load(SimpleFile *file) {
	file->readNumber();
	_enabled = file->readNumber();
	_redo = file->readNumber();
	_repeat = file->readNumber();
	CBackground::load(file);
}

bool CAutoAnimate::EnterViewMsg(CEnterViewMsg *msg) {
	if (_enabled) {
		uint flags = _repeat ? MOVIE_REPEAT : 0;
		if (_startFrame != _endFrame)
			playMovie(_startFrame, _endFrame, flags);
		else
			playMovie(flags);

		if (!_redo)
			_enabled = false;
	}

	return true;
}

bool CAutoAnimate::InitializeAnimMsg(CInitializeAnimMsg *msg) {
	_enabled = true;
	return true;
}

} // End of namespace Titanic
