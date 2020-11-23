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

#ifndef AGS_STD_INITIALIZER_LIST_H
#define AGS_STD_INITIALIZER_LIST_H

namespace AGS3 {
namespace std {

// CLASS TEMPLATE initializer_list
template <class _Elem>
class initializer_list {
public:
	using value_type = _Elem;
	using reference = const _Elem &;
	using const_reference = const _Elem &;
	using size_type = size_t;

	using iterator = const _Elem *;
	using const_iterator = const _Elem *;

	constexpr initializer_list() : _First(nullptr), _Last(nullptr) {
	}

	constexpr initializer_list(const _Elem *_First_arg, const _Elem *_Last_arg) noexcept
		: _First(_First_arg), _Last(_Last_arg) {
	}

	constexpr const _Elem *begin() const {
		return _First;
	}

	constexpr const _Elem *end() const {
		return _Last;
	}

	constexpr size_t size() const {
		return static_cast<size_t>(_Last - _First);
	}

private:
	const _Elem *_First;
	const _Elem *_Last;
};

} // namespace std
} // namespace AGS3

#endif
