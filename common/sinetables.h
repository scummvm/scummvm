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

#ifndef COMMON_SINETABLES_H
#define COMMON_SINETABLES_H

namespace Common {

/**
 * @defgroup common_sinetables Sine tables
 * @ingroup common
 *
 * @brief API for managing sine tables.
 *
 * @{
 */

class SineTable {
public:
	/**
	 * Construct a sine table given the number of points
	 *
	 * @param nPoints Number of distinct radian points, which must be in range [16,65536] and be divisible by 4
	 */
	SineTable(int nPoints);
	~SineTable();

	/**
	 * Get pointer to table
	 *
	 * This table contains nPoints/2 entries.
	 * Prefer to use at()
	 * The layout of this table is as follows:
	 * - Entries 0 up to (excluding) nPoints/4:
	 *           sin(0) till (excluding) sin(1/2*pi)
	 * - Entries 2_nPoints/4 up to nPoints/2:
	 *           sin(pi) till (excluding) sin(3/2*pi)
	 */
	const float *getTable() { return _tableEOS; }

	/**
	 * Returns sin(2*pi * index / nPoints )
	 * Index must be in range [0, nPoints - 1]
	 * Faster than atLegacy
	 */
	float at(int index) const;

	/**
	 * Returns sin(2*pi * index / nPoints )
	 * Index must be in range [0, nPoints - 1]
	 */
	float atLegacy(int index) const;	

private:
	float *_tableEOS;
	float *_table;
	double _radResolution; // Smallest radian increment
	int _refSize; // _nPoints / 4
	int _nPoints; // range of operator[]
};

/** @} */

} // End of namespace Common

#endif // COMMON_SINETABLES_H
