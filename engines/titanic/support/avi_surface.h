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

#ifndef TITANIC_AVI_SURFACE_H
#define TITANIC_AVI_SURFACE_H

#include "video/video_decoder.h"
#include "titanic/core/resource_key.h"
#include "titanic/support/movie_range_info.h"

namespace Titanic {

class CSoundManager;

class AVISurface {
private:
	int _field4;
	int _field8;
	int _fieldC;
	int _field10;
	int _frame;
	CMovieRangeInfoList _movieRangeInfo;
	int _field28;
	int _field2C;
	int _field30;
	int _field34;
	int _field38;
	CSoundManager *_soundManager;
	// TODO: Lots more fields
public:
	AVISurface(const CResourceKey &key);

	
};

} // End of namespace Titanic

#endif /* TITANIC_AVI_SURFACE_H */
