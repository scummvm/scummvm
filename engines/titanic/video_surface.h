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

#ifndef TITANIC_VIDEO_SURFACE_H
#define TITANIC_VIDEO_SURFACE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "titanic/font.h"
#include "titanic/direct_draw.h"
#include "titanic/core/list.h"
#include "titanic/core/resource_key.h"

namespace Titanic {

class CScreenManager;

class CVideoSurface : public ListItem {
private:
	static int _videoSurfaceCounter;
protected:
	CScreenManager *_screenManager;
	CResourceKey _resourceKey;
	DirectDrawSurface *_surface;
	int _field2C;
	int _field34;
	bool _field38;
	int _field3C;
	int _field40;
	int _field44;
	int _field48;
	int _videoSurfaceNum;
	int _field50;
	int _accessCtr;
public:
	CVideoSurface(CScreenManager *screenManager);

	void setSurface(CScreenManager *screenManager, DirectDrawSurface *surface);

	virtual void proc8(const CResourceKey &key) = 0;

	virtual void proc43() = 0;
};

class OSVideoSurface : public CVideoSurface {
public:
	OSVideoSurface(CScreenManager *screenManager, DirectDrawSurface *surface);
	OSVideoSurface(CScreenManager *screenManager, const CResourceKey &key, bool flag = false);

	virtual void proc8(const CResourceKey &key);

	virtual void proc43();
};

} // End of namespace Titanic

#endif /* TITANIC_VIDEO_SURFACE_H */
