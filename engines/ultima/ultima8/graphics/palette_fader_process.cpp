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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/palette_fader_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/graphics/palette.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima8 {

#define PALETTEFADER_COUNTER    30

PaletteFaderProcess *PaletteFaderProcess::fader = 0;

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(PaletteFaderProcess, Process)

PaletteFaderProcess::PaletteFaderProcess()
	: Process() {

}

PaletteFaderProcess::PaletteFaderProcess(Pentagram::PalTransforms trans,
        int priority_, int frames) : priority(priority_),
	counter(frames), max_counter(frames) {
	PaletteManager  *pm = PaletteManager::get_instance();
	Pentagram::Palette *pal = pm->getPalette(PaletteManager::Pal_Game);
	for (int i = 0; i < 12; i++) old_matrix[i] = pal->matrix[i];
	pm->getTransformMatrix(new_matrix, trans);
	pal->transform = trans;
}

PaletteFaderProcess::PaletteFaderProcess(uint32 col32, bool from,
        int priority_, int frames, bool current) : priority(priority_),
	counter(frames), max_counter(frames) {
	PaletteManager  *pm = PaletteManager::get_instance();
	Pentagram::Palette *pal = pm->getPalette(PaletteManager::Pal_Game);
	if (!from) {
		if (current)
			for (int i = 0; i < 12; i++) old_matrix[i] = pal->matrix[i];
		else
			pm->getTransformMatrix(old_matrix, pal->transform);
		pm->getTransformMatrix(new_matrix, col32);
	} else {
		pm->getTransformMatrix(old_matrix, col32);
		if (current)
			for (int i = 0; i < 12; i++) new_matrix[i] = pal->matrix[i];
		else
			pm->getTransformMatrix(new_matrix, pal->transform);
	}
}

PaletteFaderProcess::PaletteFaderProcess(int16 from[12], int16 to[12],
        int priority_, int frames) : priority(priority_),
	counter(frames), max_counter(frames) {
	int i;
	for (i = 0; i < 12; i++) old_matrix[i] = from[i];
	for (i = 0; i < 12; i++) new_matrix[i] = to[i];
}

PaletteFaderProcess::~PaletteFaderProcess(void) {
	if (fader == this)
		fader = 0;
}

void PaletteFaderProcess::run() {
	int16   matrix[12];

	for (int i = 0; i < 12; i++) {
		int32 o = old_matrix[i] * counter;
		int32 n = new_matrix[i] * (max_counter - counter);
		matrix[i] = static_cast<int16>((o + n) / max_counter);
	}

	PaletteManager::get_instance()->transformPalette(
	    PaletteManager::Pal_Game,
	    matrix);

	if (!counter--) terminate();
}

void PaletteFaderProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(priority));
	ods->write4(static_cast<uint32>(counter));
	ods->write4(static_cast<uint32>(max_counter));
	unsigned int i;
	for (i = 0; i < 12; ++i)
		ods->write2(old_matrix[i]);
	for (i = 0; i < 12; ++i)
		ods->write2(new_matrix[i]);
}

bool PaletteFaderProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	priority = static_cast<int>(ids->read4());
	counter = static_cast<int>(ids->read4());
	max_counter = static_cast<int>(ids->read4());

	unsigned int i;
	for (i = 0; i < 12; ++i)
		old_matrix[i] = ids->read2();
	for (i = 0; i < 12; ++i)
		new_matrix[i] = ids->read2();

	fader = this; //static
	return true;
}

uint32 PaletteFaderProcess::I_fadeToPaletteTransform(const uint8 *args,
        unsigned int /*argsize*/) {
	ARG_UINT16(transform);
	ARG_UINT16(priority);

	// If current fader has higher priority, we do nothing
	if (fader && fader->priority > priority) return 0;
	else if (fader) fader->terminate();

	fader = new PaletteFaderProcess(static_cast<Pentagram::PalTransforms>(transform),
	                                priority, 45);

	return Kernel::get_instance()->addProcess(fader);
}

uint32 PaletteFaderProcess::I_fadeToBlack(const uint8 * /*args*/,
        unsigned int /*argsize*/) {
	if (fader && fader->priority > 0x7FFF) return 0;
	else if (fader) fader->terminate();

	fader = new PaletteFaderProcess(0x00000000, false, 0x7FFF, 30, true);
	return Kernel::get_instance()->addProcess(fader);
}

uint32 PaletteFaderProcess::I_fadeFromBlack(const uint8 * /*args*/,
        unsigned int /*argsize*/) {
	if (fader && fader->priority > 0x7FFF) return 0;
	else if (fader) fader->terminate();

	fader = new PaletteFaderProcess(0x00000000, true, 0x7FFF, 30, false);
	return Kernel::get_instance()->addProcess(fader);
}

uint32 PaletteFaderProcess::I_fadeToWhite(const uint8 * /*args*/,
        unsigned int /*argsize*/) {
	if (fader && fader->priority > 0x7FFF) return 0;
	else if (fader) fader->terminate();

	fader = new PaletteFaderProcess(0x00FFFFFF, false, 0x7FFF, 30, true);
	return Kernel::get_instance()->addProcess(fader);
}

uint32 PaletteFaderProcess::I_fadeFromWhite(const uint8 * /*args*/,
        unsigned int /*argsize*/) {
	if (fader && fader->priority > 0x7FFF) return 0;
	else if (fader) fader->terminate();

	fader = new PaletteFaderProcess(0x00FFFFFF, true, 0x7FFF, 30, false);
	return Kernel::get_instance()->addProcess(fader);
}

uint32 PaletteFaderProcess::I_lightningBolt(const uint8 * /*args*/,
        unsigned int /*argsize*/) {
	if (fader && fader->priority > -1) return 0;
	else if (fader) fader->terminate();

	fader = new PaletteFaderProcess(0x3FCFCFCF, true, -1, 10, false);
	return Kernel::get_instance()->addProcess(fader);
}

} // End of namespace Ultima8
