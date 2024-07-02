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

#ifndef QDENGINE_XLIBS_UTIL_XTL_SAFE_CAST_H
#define QDENGINE_XLIBS_UTIL_XTL_SAFE_CAST_H

namespace QDEngine {

#ifndef _FINAL_VERSION_

template <class DestinationType, class SourceType>
inline DestinationType safe_cast(SourceType *source) {
	DestinationType p = dynamic_cast<DestinationType>(source);
	xassert((p || !source) && "Incorrect cast");
	return p;
}

template <class DestinationType, class SourceType>
inline DestinationType safe_cast_ref(SourceType &source) {
	return dynamic_cast<DestinationType>(source);
}

#else

#define safe_cast static_cast
#define safe_cast_ref static_cast

#endif

}

#endif // QDENGINE_XLIBS_UTIL_XTL_SAFE_CAST_H
