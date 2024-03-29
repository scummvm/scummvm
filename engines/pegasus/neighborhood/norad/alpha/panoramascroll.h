/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_PANORAMASCROLL_H
#define PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_PANORAMASCROLL_H

#include "pegasus/neighborhood/norad/alpha/panorama.h"

namespace Pegasus {

class PanoramaScroll : public IdlerAnimation {
public:
	PanoramaScroll(const DisplayElementID);
	~PanoramaScroll() override {}

	void initFromMovieFile(const Common::Path &);
	void initMaskFromPICTFile(const Common::Path &);

	void releasePanorama();

	bool isPanoramaOpen() { return _panorama.isPanoramaOpen(); }
	void getPanoramaBounds(Common::Rect &) const;

	void setBounds(const Common::Rect&) override;

	void draw(const Common::Rect &) override;

protected:
	void timeChanged(const TimeValue) override;

	Panorama _panorama;
	Surface _mask;
	CoordType _totalWidth, _boundsWidth;
};

} // End of namespace Pegasus

#endif
