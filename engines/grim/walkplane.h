/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_WALKPLANE_H
#define GRIM_WALKPLANE_H

#include "common/str.h"

#include "graphics/vector3d.h"
#include "graphics/line3d.h"

namespace Common {
	class MemoryReadStream;
}

namespace Grim {

class SaveGame;
class TextSplitter;

class Sector {
public:
	enum SectorType {
		NoneType = 0,
		WalkType = 0x1000,
		FunnelType = 0x1100,
		CameraType = 0x2000,
		SpecialType = 0x4000,
		HotType = 0x8000
	};

	Sector() : _vertices(NULL) {}
	Sector(const Sector &other);
	virtual ~Sector();

	void saveState(SaveGame *savedState) const;
	bool restoreState(SaveGame *savedState);

	void load(TextSplitter &ts);
	void loadBinary(Common::MemoryReadStream *ms);
	void setVisible(bool visible);

	const char *getName() const { return _name.c_str(); }
	int getSectorId() const { return _id; }
	SectorType getType() const { return _type; } // FIXME: Implement type de-masking
	bool isVisible() const { return _visible; }
	bool isPointInSector(Graphics::Vector3d point) const;
	Common::List<Graphics::Line3d> getBridgesTo(Sector *sector) const;

	Graphics::Vector3d getProjectionToPlane(Graphics::Vector3d point) const;
	Graphics::Vector3d getProjectionToPuckVector(Graphics::Vector3d v) const;

	Graphics::Vector3d getClosestPoint(Graphics::Vector3d point) const;

	// Interface to trace a ray to its exit from the polygon
	struct ExitInfo {
		Graphics::Vector3d exitPoint;
		float angleWithEdge;
		Graphics::Vector3d edgeDir;
	};
	void getExitInfo(Graphics::Vector3d start, Graphics::Vector3d dir, struct ExitInfo *result);

	int getNumVertices() { return _numVertices; }
	Graphics::Vector3d *getVertices() { return _vertices; }
	Graphics::Vector3d getNormal() { return _normal; }

	Sector &operator=(const Sector &other);
	bool operator==(const Sector &other) const;

private:
	int _numVertices, _id;

	Common::String _name;
	SectorType _type;
	bool _visible;
	Graphics::Vector3d *_vertices;
	float _height;

	Graphics::Vector3d _normal;
};

} // end of namespace Grim

#endif
