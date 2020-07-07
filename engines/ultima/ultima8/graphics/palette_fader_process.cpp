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
#include "ultima/ultima8/kernel/core_app.h"

namespace Ultima {
namespace Ultima8 {

PaletteFaderProcess *PaletteFaderProcess::_fader = nullptr;

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(PaletteFaderProcess)

PaletteFaderProcess::PaletteFaderProcess() : Process(), _priority(0),
	_counter(0), _maxCounter(0) {
}

PaletteFaderProcess::PaletteFaderProcess(PalTransforms trans,
        int priority_, int frames) : _priority(priority_),
	_counter(frames), _maxCounter(frames) {
	PaletteManager  *pm = PaletteManager::get_instance();
	Palette *pal = pm->getPalette(PaletteManager::Pal_Game);
	for (int i = 0; i < 12; i++) _oldMatrix[i] = pal->_matrix[i];
	pm->getTransformMatrix(_newMatrix, trans);
	pal->_transform = trans;
}

PaletteFaderProcess::PaletteFaderProcess(uint32 col32, bool from,
        int priority, int frames, bool current) : _priority(priority),
	_counter(frames), _maxCounter(frames) {
	PaletteManager  *pm = PaletteManager::get_instance();
	Palette *pal = pm->getPalette(PaletteManager::Pal_Game);
	if (!from) {
		if (current)
			for (int i = 0; i < 12; i++) _oldMatrix[i] = pal->_matrix[i];
		else
			pm->getTransformMatrix(_oldMatrix, pal->_transform);
		pm->getTransformMatrix(_newMatrix, col32);
	} else {
		pm->getTransformMatrix(_oldMatrix, col32);
		if (current)
			for (int i = 0; i < 12; i++) _newMatrix[i] = pal->_matrix[i];
		else
			pm->getTransformMatrix(_newMatrix, pal->_transform);
	}
}

PaletteFaderProcess::PaletteFaderProcess(int16 from[12], int16 to[12],
        int priority_, int frames) : _priority(priority_),
	_counter(frames), _maxCounter(frames) {
	int i;
	for (i = 0; i < 12; i++) _oldMatrix[i] = from[i];
	for (i = 0; i < 12; i++) _newMatrix[i] = to[i];
}

PaletteFaderProcess::~PaletteFaderProcess(void) {
	if (_fader == this)
		_fader = nullptr;
}

void PaletteFaderProcess::run() {
	int16   matrix[12];

	for (int i = 0; i < 12; i++) {
		int32 o = _oldMatrix[i] * _counter;
		int32 n = _newMatrix[i] * (_maxCounter - _counter);
		matrix[i] = static_cast<int16>((o + n) / _maxCounter);
	}

	PaletteManager::get_instance()->transformPalette(
	    PaletteManager::Pal_Game,
	    matrix);

	if (!_counter--) terminate();
}

void PaletteFaderProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_priority));
	ws->writeUint32LE(static_cast<uint32>(_counter));
	ws->writeUint32LE(static_cast<uint32>(_maxCounter));
	unsigned int i;
	for (i = 0; i < 12; ++i)
		ws->writeUint16LE(_oldMatrix[i]);
	for (i = 0; i < 12; ++i)
		ws->writeUint16LE(_newMatrix[i]);
}

bool PaletteFaderProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_priority = static_cast<int>(rs->readUint32LE());
	_counter = static_cast<int>(rs->readUint32LE());
	_maxCounter = static_cast<int>(rs->readUint32LE());

	unsigned int i;
	for (i = 0; i < 12; ++i)
		_oldMatrix[i] = rs->readUint16LE();
	for (i = 0; i < 12; ++i)
		_newMatrix[i] = rs->readUint16LE();

	_fader = this; //static
	return true;
}

uint32 PaletteFaderProcess::I_fadeToPaletteTransform(const uint8 *args,
        unsigned int /*argsize*/) {
	ARG_UINT16(transform);
	ARG_UINT16(priority);

	// If current _fader has higher _priority, we do nothing
	if (_fader && _fader->_priority > priority) return 0;
	else if (_fader) _fader->terminate();

	_fader = new PaletteFaderProcess(static_cast<PalTransforms>(transform),
	                                priority, 45);

	return Kernel::get_instance()->addProcess(_fader);
}

uint32 PaletteFaderProcess::I_fadeToBlack(const uint8 *args,
        unsigned int argsize) {
	if (_fader && _fader->_priority > 0x7FFF) return 0;
	else if (_fader) _fader->terminate();

	int nsteps = (GAME_IS_U8 ? 30 : 40);
	if (argsize > 0) {
		ARG_UINT16(n);
		nsteps = n;
	}

	_fader = new PaletteFaderProcess(0x00000000, false, 0x7FFF, nsteps, true);
	return Kernel::get_instance()->addProcess(_fader);
}

uint32 PaletteFaderProcess::I_fadeFromBlack(const uint8 *args,
        unsigned int argsize) {
	if (_fader && _fader->_priority > 0x7FFF) return 0;
	else if (_fader) _fader->terminate();

	int nsteps = (GAME_IS_U8 ? 30 : 40);
	if (argsize > 0) {
		ARG_UINT16(n);
		nsteps = n;
	}

	_fader = new PaletteFaderProcess(0x00000000, true, 0x7FFF, nsteps, false);
	return Kernel::get_instance()->addProcess(_fader);
}

uint32 PaletteFaderProcess::I_fadeToWhite(const uint8 * /*args*/,
        unsigned int /*argsize*/) {
	if (_fader && _fader->_priority > 0x7FFF) return 0;
	else if (_fader) _fader->terminate();

	_fader = new PaletteFaderProcess(0x00FFFFFF, false, 0x7FFF, 30, true);
	return Kernel::get_instance()->addProcess(_fader);
}

uint32 PaletteFaderProcess::I_fadeFromWhite(const uint8 * /*args*/,
        unsigned int /*argsize*/) {
	if (_fader && _fader->_priority > 0x7FFF) return 0;
	else if (_fader) _fader->terminate();

	_fader = new PaletteFaderProcess(0x00FFFFFF, true, 0x7FFF, 30, false);
	return Kernel::get_instance()->addProcess(_fader);
}

uint32 PaletteFaderProcess::I_lightningBolt(const uint8 * /*args*/,
        unsigned int /*argsize*/) {
	if (_fader && _fader->_priority > -1) return 0;
	else if (_fader) _fader->terminate();

	_fader = new PaletteFaderProcess(0x3FCFCFCF, true, -1, 10, false);
	return Kernel::get_instance()->addProcess(_fader);
}

} // End of namespace Ultima8
} // End of namespace Ultima
