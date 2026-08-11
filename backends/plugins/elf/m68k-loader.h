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

#ifndef BACKENDS_PLUGINS_M68K_LOADER_H
#define BACKENDS_PLUGINS_M68K_LOADER_H

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(M68K_TARGET)

#include "backends/plugins/elf/elf-loader.h"

class M68KDLObject : public DLObject {
protected:
	bool relocate(Elf32_Off offset, Elf32_Word size, byte *relSegment) override;
	bool relocateRels(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) override;
};

#endif /* defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(M68K_TARGET) */

#endif /* BACKENDS_PLUGINS_M68K_LOADER_H */
