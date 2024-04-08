
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

#ifndef BAGEL_BOFLIB_RING_BUFFER_H
#define BAGEL_BOFLIB_RING_BUFFER_H

namespace Bagel {

template<class T>
class CRingBuffer {
private:
	T *m_pStart;
	T *m_pEnd;
	T *m_pCur;

public:
	CRingBuffer(int nNumEntries) {
		m_pStart = (T *)malloc(sizeof(T) * nNumEntries);
		m_pEnd = m_pStart + nNumEntries;
		m_pCur = m_pStart;
	}

	virtual ~CRingBuffer() {
		if (m_pStart != nullptr) {
			free(m_pStart);
			m_pStart = nullptr;
		}
		m_pCur = m_pEnd = nullptr;
	}

	T operator++() {
		if (++m_pCur == m_pEnd)
			m_pCur = m_pStart;

		return *m_pCur;
	}

	T operator++(int) {
		if (++m_pCur == m_pEnd)
			m_pCur = m_pStart;

		return *m_pCur;
	}

	T operator--() {
		if (m_pCur == m_pStart)
			m_pCur = m_pEnd;

		m_pCur--;
		return *m_pCur;
	}

	T operator--(int) {
		if (m_pCur == m_pStart)
			m_pCur = m_pEnd;

		m_pCur--;
		return *m_pCur;
	}

	T operator=(T tNewItem) {
		*m_pCur = tNewItem;

		return *m_pCur;
	}

	bool operator==(T tCompareItem) {
		return *m_pCur == tCompareItem;
	}

	operator T() {
		return *m_pCur;
	}

	T operator[](int i) {
		return *(m_pStart + i);
	}
};

} // namespace Bagel

#endif
