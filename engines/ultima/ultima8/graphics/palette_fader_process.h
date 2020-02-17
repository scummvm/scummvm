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

#ifndef ULTIMA8_GRAPHICS_PALETTEFADERPROCESS_H
#define ULTIMA8_GRAPHICS_PALETTEFADERPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class PaletteFaderProcess : public Process {
	int                         _priority;
	int32                       _counter;
	int32                       _maxCounter;
	int16                       _oldMatrix[12]; // Fixed point -4.11
	int16                       _newMatrix[12];
public:
	static PaletteFaderProcess  *_fader;

	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE()
	PaletteFaderProcess();
	PaletteFaderProcess(PalTransforms trans, int priority, int frames);
	PaletteFaderProcess(uint32 rgba, bool from, int priority, int frames, bool current);
	PaletteFaderProcess(int16 from[12], int16 to[12], int priority, int frames);
	~PaletteFaderProcess(void) override;

	void run() override;

	INTRINSIC(I_fadeToPaletteTransform);
	INTRINSIC(I_fadeToBlack);
	INTRINSIC(I_fadeFromWhite);
	INTRINSIC(I_fadeToWhite);
	INTRINSIC(I_fadeFromBlack);
	INTRINSIC(I_lightningBolt);

	bool loadData(IDataSource *ids, uint32 version);
protected:
	void saveData(ODataSource *ods) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
