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

#ifndef TITANIC_SURFACE_FADER_H
#define TITANIC_SURFACE_FADER_H

#include "common/scummsys.h"

namespace Titanic {

class CVideoSurface;
class CScreenManager;
class CSurfaceArea;

class CSurfaceFader {
private:
	byte *_dataP;
	bool _fadeIn;
private:
	/**
	 * Create a faded version of the source surface for the new step
	 */
	void step(CSurfaceArea &srcSurface, CSurfaceArea &destSurface);

	/**
	 * Sets up an internal surface to match the size of the specified one
	 */
	bool setupSurface(CScreenManager *screenManager, CVideoSurface *srcSurface);
public:
	int _index;
	int _count;
	CVideoSurface *_videoSurface;
public:
	CSurfaceFader();
	~CSurfaceFader();

	/**
	 * Reset fading back to the start
	 */
	void reset();

	/**
	 * Creates a faded version of the passed source surface, based on a percentage
	 * visibility specified by _index of _count
	 */
	CVideoSurface *draw(CScreenManager *screenManager, CVideoSurface *srcSurface);

	/**
	 * Sets whether a fade in (versus a fade out) should be done
	 */
	void setFadeIn(bool fadeIn);

	/**
	 * Returns true if a fade is in progress
	 */
	bool isActive() const { return _index >= 0 && _index < _count; }
};

} // End of namespace Titanic

#endif /* TITANIC_SURFACE_SHADER_H */
