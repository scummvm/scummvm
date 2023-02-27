/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef TETRAEDGE_TE_TE_VARIANT_H
#define TETRAEDGE_TE_TE_VARIANT_H

#include "common/str.h"
#include "common/types.h"

namespace Tetraedge {

class TeVariant {
public:
	TeVariant();
	TeVariant(bool val);
	TeVariant(double val);
	TeVariant(const Common::String &val);
	TeVariant(const char *val);
	TeVariant(const TeVariant &other);

	enum VariantType {
		TypeNone,
		TypeBoolean,
		TypeInt32,
		TypeUInt32,
		TypeInt64,
		TypeUInt64,
		TypeFloat32,
		TypeFloat64,
		TypeString
	};

	VariantType type() const { return _type; }

	bool toBoolean(bool *success = nullptr) const;
	float toFloat32(bool *success = nullptr) const;
	double toFloat64(bool *success = nullptr) const;
	int32 toSigned32(bool *success = nullptr) const;
	int64 toSigned64(bool *success = nullptr) const;
	Common::String toString(bool *success = nullptr) const;
	uint32 toUnsigned32(bool *success = nullptr) const;
	uint64 toUnsigned64(bool *success = nullptr) const;

	/// Dump a string representation for debugging
	Common::String dumpStr() const;

private:
	uint64 _data;
	VariantType _type;
	const Common::String _strVal;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_VARIANT_H
