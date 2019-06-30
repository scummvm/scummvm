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

#ifndef BLADERUNNER_CHAPTERS_H
#define BLADERUNNER_CHAPTERS_H

namespace BladeRunner {

class BladeRunnerEngine;

class Chapters {
	BladeRunnerEngine *_vm;

	int  _chapter;
	int  _resourceIds[6];
	bool _hasOpenResources;

public:
	Chapters(BladeRunnerEngine *vm) {
		_vm = vm;
		_chapter = 0;

		_resourceIds[0] = 1;
		_resourceIds[1] = 1;
		_resourceIds[2] = 2;
		_resourceIds[3] = 2;
		_resourceIds[4] = 3;
		_resourceIds[5] = 4;

		_hasOpenResources = false;
	}

	bool enterChapter(int chapter);
	void closeResources();

	bool hasOpenResources() { return _hasOpenResources; }
	int  currentResourceId() { return _chapter ? _resourceIds[_chapter] : -1; }
};

} // End of namespace BladeRunner

#endif
