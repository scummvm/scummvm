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

#include "common/scummsys.h"

#ifdef USE_ELF_LOADER

/**
 * These functions are a hack to workaround a GCC limitation
 * At every function entry, GCC adds a check on the stack size
 * If the stack is too small, the functions _rt_stkovf_split_small or _rt_stkovf_split_big are called
 * This call is done using a PC relative 24 bits address but we want to link back to main executable functions
 * and this is not possible using this relocation type.
 * So we create wrapper functions which will just jump to the main function using a 32 bits relocation.
 * The wrapping is done by ld thanks to its --wrap argument
 */

__asm__ (
	".global __wrap___rt_stkovf_split_small\n"
	".type __wrap___rt_stkovf_split_small, %function\n"
	"__wrap___rt_stkovf_split_small:\n"
	"LDR	pc, .Lsmall\n"
	".Lsmall:\n"
	".word	__real___rt_stkovf_split_small\n"
);

__asm__ (
	".global __wrap___rt_stkovf_split_big\n"
	".type __wrap___rt_stkovf_split_big, %function\n"
	"__wrap___rt_stkovf_split_big:\n"
	"LDR	pc, .Lbig\n"
	".Lbig:\n"
	".word	__real___rt_stkovf_split_big\n"
);

#endif
