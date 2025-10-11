
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

#ifndef GAMOS_ARRAY2D_H
#define GAMOS_ARRAY2D_H

#include "common/array.h"
#include "common/rect.h"

template <class T>
class Array2D : protected Common::Array<T>
{
private:
    typedef Common::Array<T> __base;
public:
    typedef uint size_type; /*!< Size type of the array. */

    using Common::Array<T>::data;
    using Common::Array<T>::operator[];
    using Common::Array<T>::size;
    using Common::Array<T>::begin;
    using Common::Array<T>::end;
    using Common::Array<T>::empty;

public:


    Array2D() = default;
    Array2D( Array2D && ) = default;
    Array2D( const Array2D & ) = default;

    Array2D& operator=( const Array2D & ) = default;
    Array2D& operator=( Array2D && ) = default;

    Array2D(size_type w, size_type h) {
        resize(w, h);
    }

    Array2D(const Common::Point &sz) {
        resize(sz);
    }

    Array2D<T>* copy() {
        Array2D<T> *tmp = new Array2D<T>;
        *tmp = *this;
        return tmp;
    }

    void resize(const Common::Point &sz) {
        _w = sz.x;
        _h = sz.y;

        __base::resize(_w * _h);
    }

    void resize(size_type w, size_type h) {
        _w = w;
        _h = h;

        __base::resize(_w * _h);
    }

    T& at(size_type x, size_type y) {
        return __base::operator[](x + y * _w);
    }

    T& at(const Common::Point &p) {
        return __base::operator[](p.x + p.y * _w);
    }

    T& at(size_type n) {
        return __base::operator[](n);
    }

    T& operator()(size_type x, size_type y) {
        return at(x + y * _w);
    }

    T& operator()(const Common::Point &p) {
        return at(p.x + p.y * _w);
    }

    T* getLinePtr(size_type y) {
        return &(at(y * _w));
    }

    const T* getLinePtr(size_type y) const {
        return &(at(y * _w));
    }

    Common::Point sizes() const {
        return Common::Point(_w, _h);
    }

    size_type getIndex(size_type x, size_type y) const {
        size_t n = x + y * _w;
        if (n < size())
            return n;
        return -1;
    }

    Common::Point indexToCoords(size_type id) const {
        if (id >= 0 && id < size())
            return Common::Point(id % _w, id / _w);
        return Common::Point(-1, -1);
    }

    size_type width() const {
        return _w;
    }

    size_type height() const {
        return _h;
    }

    void clear() {
        _w = 0;
        _h = 0;
        Common::Array<T>::clear();
    }

    bool isNotNull() const {
        return _w > 0 && _h > 0;
    }

    bool isNull() const {
        return _w == 0 || _h == 0;
    }

    bool isOk() const {
        return _w > 0 && _h > 0 && (size() == _w * _h);
    }

protected:
    size_type _w = 0;
    size_type _h = 0;
};


#endif
