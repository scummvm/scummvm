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

#ifndef COMMON_COSINETABLES_H
#define COMMON_COSINETABLES_H

namespace Common {

/**
 * @defgroup common_cosinetables Cosine tables
 * @ingroup common
 *
 * @brief  Functions for working with cosine tables.
 *
 * @{
 */

class CosineTable {
public:
	/**
	 * Construct a cosine table given the number of points.
	 *
	 * @param nPoints Number of distinct radian points that must be in range [16,65536] and be divisible by 4.
	 */
	CosineTable(int nPoints);
	~CosineTable();

	/**
	 * Get a pointer to a table.
	 *
	 * This table contains nPoints/2 entries.
	 * Prefer to use at()
	 * The layout of this table is as follows:
	 * - Entries 0 up to (including) nPoints/4:
	 *           cos(0) till (including) cos(1/2*pi)
	 * - Entries (excluding) nPoints/4 up to nPoints/2:
	 *           (excluding) cos(3/2*pi) till (excluding) cos(2*pi)
	 */
	const float *getTable() { return _tableEOS; }

	/**
	 * Return cos(2*pi * index / nPoints )
	 * Index must be in range [0, nPoints - 1]
	 * Faster than atLegacy.
	 */
	float at(int index) const;

	/**
	 * Returns cos(2*pi * index / nPoints )
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

#endif // COMMON_COSINETABLES_H
