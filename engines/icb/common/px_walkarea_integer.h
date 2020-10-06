/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef WALKAREA_H
#define WALKAREA_H

#include "engines/icb/common/px_rcutypes.h"

namespace ICB {

#define INTEGER_WALKAREA_API_SCHEMA 1

typedef struct __point { // 3D integer coordinate representation
	__point(void) : x(0), y(0), z(0) { ; }
	__point(int X, int Y, int Z) : x(X), y(Y), z(Z) { ; }

	int32 x;
	int32 y;
	int32 z;
} __point;

typedef struct {
	char name[32];         // Name of the walkarea
	char cameraCluster[8]; // Hashed cameraName value

	// Bounding box dimensions
	int32 x; // Top-left corner x coordinate (Revolution space)
	int32 y; // Top-left corner y coordinate (Revolution space)
	int32 z; // Top-left corner z coordinate (Revolution space)
	int32 w; // Width
	int32 h; // Height

	// THE AREA DEFINITION (All in Revolution space)
	uint32 noPoints;   // Number of verteces\knots in 2D spline
	__point points[1]; // The points themselves (spline is always closed)

	char cameraName[1]; // Name of associated camera (DWORD aligned)
} __aWalkArea;

class INTEGER_WalkAreaFile {
public:
	uint32 schema; // The format version
	char ID[4];    // ID "WGA"

	// Class methods

	INTEGER_WalkAreaFile() { ; }
	~INTEGER_WalkAreaFile() { ; }

	uint32 GetSchema(void) const { return schema; }
	uint32 GetNoAreas(void) const { return noAreas; }

	// Get pointer to a specific WalkArea
	inline const __aWalkArea *GetWalkArea(uint32 number) const;
	inline uint32 GetNoPoints(uint32 number) const;
	inline int32 GetBox_X(uint32 number) const;
	inline int32 GetBox_Y(uint32 number) const;
	inline int32 GetBox_Z(uint32 number) const;
	inline int32 GetBox_W(uint32 number) const;
	inline int32 GetBox_H(uint32 number) const;

	bool8 GetAreaName(uint32 number, const char *&name) const;
	bool8 GetCluster(uint32 number, const char *&cluster) const;
	bool8 GetPoint(uint32 area, uint32 number, __point &point) const;
	bool8 GetCameraName(uint32 number, const char *&name) const;

      private:
	uint32 noAreas;
	uint32 offsetTable[1];
};

inline const __aWalkArea *INTEGER_WalkAreaFile::GetWalkArea(uint32 number) const { return ((const __aWalkArea *)(((const char *)this) + offsetTable[number])); }

inline uint32 INTEGER_WalkAreaFile::GetNoPoints(uint32 number) const { return (GetWalkArea(number)->noPoints); }

inline int32 INTEGER_WalkAreaFile::GetBox_X(uint32 number) const { return (GetWalkArea(number)->x); }

inline int32 INTEGER_WalkAreaFile::GetBox_Y(uint32 number) const { return (GetWalkArea(number)->y); }

inline int32 INTEGER_WalkAreaFile::GetBox_Z(uint32 number) const { return (GetWalkArea(number)->z); }

inline int32 INTEGER_WalkAreaFile::GetBox_W(uint32 number) const { return (GetWalkArea(number)->w); }

inline int32 INTEGER_WalkAreaFile::GetBox_H(uint32 number) const { return (GetWalkArea(number)->h); }

inline bool8 INTEGER_WalkAreaFile::GetAreaName(uint32 number, const char *&name) const {
	if (number >= noAreas)
		return FALSE8;

	name = GetWalkArea(number)->name;

	return TRUE8;
}

inline bool8 INTEGER_WalkAreaFile::GetCluster(uint32 number, const char *&cluster) const {
	if (number >= noAreas)
		return FALSE8;

	cluster = GetWalkArea(number)->cameraCluster;

	return TRUE8;
}

inline bool8 INTEGER_WalkAreaFile::GetPoint(uint32 area, uint32 number, __point &point) const {
	if (area >= noAreas)
		return FALSE8;

	point.x = GetWalkArea(area)->points[number].x;
	point.y = GetWalkArea(area)->points[number].y;
	point.z = GetWalkArea(area)->points[number].z;

	return TRUE8;
}

inline bool8 INTEGER_WalkAreaFile::GetCameraName(uint32 number, const char *&name) const {
	if (number >= noAreas)
		return FALSE8;

	// Get the address of the start of the cameraName (by asking for a point that isn't there
	name = (const char *)&GetWalkArea(number)->points[GetNoPoints(number)];

	return TRUE8;
}

} // End of namespace ICB

#endif
