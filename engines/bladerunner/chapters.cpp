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

#include "bladerunner/chapters.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/slice_animations.h"

namespace BladeRunner {

bool Chapters::enterChapter(int chapter) {
	int id = _resourceIds[chapter];

	_vm->_sliceAnimations->openFrames(id);

	if (!_vm->openArchive("A.TLK"))
		return false;

	if (!_vm->openArchive(Common::String::format("VQA%d.MIX", MIN(id, 3))))
		return false;

	if (!_vm->openArchive(Common::String::format("%d.TLK", MIN(id, 3))))
		return false;

	if (!_vm->openArchive(Common::String::format("OUTTAKE%d.MIX", id)))
		return false;

#if BLADERUNNER_DEBUG_GAME
	_vm->openArchive("OUTTAKE1.MIX");
	_vm->openArchive("OUTTAKE2.MIX");
	_vm->openArchive("OUTTAKE3.MIX");
	_vm->openArchive("OUTTAKE4.MIX");
	_vm->openArchive("VQA1.MIX");
	_vm->openArchive("VQA2.MIX");
	_vm->openArchive("VQA3.MIX");
	_vm->openArchive("1.TLK");
	_vm->openArchive("2.TLK");
	_vm->openArchive("3.TLK");
#endif

	_chapter = chapter;
	_hasOpenResources = true;
	return true;
}

void Chapters::closeResources() {
	int id = _resourceIds[_chapter];

	_vm->closeArchive("A.TLK");
	_vm->closeArchive(Common::String::format("VQA%d.MIX", MIN(id, 3)));
	_vm->closeArchive(Common::String::format("%d.TLK", MIN(id, 3)));
	_vm->closeArchive(Common::String::format("OUTTAKE%d.MIX", id));
	_hasOpenResources = false;
}

} // End of namespace BladeRunner
