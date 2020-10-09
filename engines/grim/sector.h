/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GRIM_WALKPLANE_H
#define GRIM_WALKPLANE_H

#include "common/str.h"
#include "common/list.h"

#include "math/vector3d.h"
#include "math/line3d.h"

namespace Common {
	class SeekableReadStream;
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

	Sector();
	Sector(const Sector &other);
	virtual ~Sector();

	void saveState(SaveGame *savedState) const;
	bool restoreState(SaveGame *savedState);

	void load(TextSplitter &ts);
	void loadBinary(Common::SeekableReadStream *data);
	void setVisible(bool visible);
	void shrink(float radius);
	void unshrink();

	Common::String getName() const { return _name; }
	int getSectorId() const { return _id; }
	SectorType getType() const { return _type; } // FIXME: Implement type de-masking
	bool isVisible() const { return _visible && !_invalid; }
	bool isPointInSector(const Math::Vector3d &point) const;
	float distanceToPoint(const Math::Vector3d &point) const;
	Common::List<Math::Line3d> getBridgesTo(Sector *sector) const;

	Math::Vector3d getProjectionToPlane(const Math::Vector3d &point) const;
	Math::Vector3d getProjectionToPuckVector(const Math::Vector3d &v) const;

	Math::Vector3d getClosestPoint(const Math::Vector3d &point) const;

	// Interface to trace a ray to its exit from the polygon
	struct ExitInfo {
		Math::Vector3d exitPoint;
		Math::Angle angleWithEdge;
		Math::Vector3d edgeDir;
		int edgeVertex;
	};
	void getExitInfo(const Math::Vector3d &start, const Math::Vector3d &dir, struct ExitInfo *result) const;

	int getNumSortplanes() { return _numSortplanes; }
	int getSortplane(int setup) { return _sortplanes[setup]; }
	int getNumVertices() { return _numVertices; }
	Math::Vector3d *getVertices() const { return _vertices; }
	Math::Vector3d getNormal() const { return _normal; }

	Sector &operator=(const Sector &other);
	bool operator==(const Sector &other) const;

private:
	int _numVertices;
	int _id;
	int _numSortplanes;
	int *_sortplanes;

	Common::String _name;
	SectorType _type;
	bool _visible;
	bool _invalid;
	Math::Vector3d *_vertices;
	Math::Vector3d *_origVertices;
	float _height;
	float _shrinkRadius;

	Math::Vector3d _normal;
};

} // end of namespace Grim

#endif
