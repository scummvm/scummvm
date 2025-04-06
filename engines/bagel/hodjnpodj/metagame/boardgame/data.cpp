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

#include "bagel/hodjnpodj/metagame/boardgame/data.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

CMap::CMap() {
	m_bPositionDetermined = false;
	m_bSprite = false;
	m_bPalette = false;
	m_bOverlay = false;
	m_bMasked = false;
	m_bMetaGame = false;
	m_bRelocatable = false;
	m_bPositionSpecified = false;
	m_bSpecialPaint = false;
}

CGtlData::CGtlData() {
	TimeDate *td[2] = { &m_stAcceptClickActive, &m_stLDownTime };

	for (TimeDate *d : td) {
		d->tm_sec = 0;
		d->tm_min = 0;
		d->tm_hour = 0;
		d->tm_mday = 0;
		d->tm_mon = 0;
		d->tm_year = 0;
		d->tm_wday = 0;
	}
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
