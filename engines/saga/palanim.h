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

// Palette animation module header file

#ifndef SAGA_PALANIM_H
#define SAGA_PALANIM_H

namespace Saga {

#define PALANIM_CYCLETIME 100

struct PalanimEntry {
	uint16 cycle;
	ByteArray palIndex;
	Common::Array<Color> colors;
};

class PalAnim {
 public:
	PalAnim(SagaEngine *vm);

	void loadPalAnim(const ByteArray &resourceData);
	void cycleStart();
	void cycleStep(int vectortime);
	void clear();

 private:
	SagaEngine *_vm;

	Common::Array<PalanimEntry> _entries;
};

} // End of namespace Saga

#endif
