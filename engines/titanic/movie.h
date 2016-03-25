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

#ifndef TITANIC_MOVIE_H
#define TITANIC_MOVIE_H

#include "video/avi_decoder.h"
#include "titanic/core/list.h"
#include "titanic/core/resource_key.h"

namespace Titanic {

class CVideoSurface;

class CMovie : public ListItem {
public:
	virtual void proc8(int v1, CVideoSurface *surface) = 0;
	virtual void proc9() = 0;
	virtual void proc10() = 0;
	virtual void proc11() = 0;
	virtual void proc12() = 0;
	virtual void stop() = 0;
	virtual void proc14() = 0;
	virtual void setFrame(uint frameNumber) = 0;
	virtual void proc16() = 0;
	virtual void proc17() = 0;
	virtual void proc18() = 0;
	virtual void proc19() = 0;
	virtual void proc20() = 0;
	virtual void *proc21() = 0;
};

class OSMovie : public CMovie {
private:
	Video::AVIDecoder _aviDecoder;
	CVideoSurface *_videoSurface;
public:
	OSMovie(const CResourceKey &name, CVideoSurface *surface);

	virtual void proc8(int v1, CVideoSurface *surface);
	virtual void proc9();
	virtual void proc10();
	virtual void proc11();
	virtual void proc12();
	virtual void stop();
	virtual void proc14();

	/**
	 * Set the current frame number
	 */
	virtual void setFrame(uint frameNumber);
	
	virtual void proc16();
	virtual void proc17();
	virtual void proc18();
	virtual void proc19();
	virtual void proc20();
	virtual void *proc21();

	bool isInGlobalList() const;
};

class CGlobalMovies : public List<CMovie> {
public:
};

} // End of namespace Titanic

#endif /* TITANIC_MOVIE_H */
