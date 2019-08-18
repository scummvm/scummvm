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

#ifndef BLADERUNNER_KIA_SECTION_POGO_H
#define BLADERUNNER_KIA_SECTION_POGO_H

#include "bladerunner/color.h"
#include "bladerunner/ui/kia_section_base.h"

namespace BladeRunner {

class KIASectionPogo : public KIASectionBase {
	static const int      kStringCount = 158;
	static const int      kLineCount = 22;
	static const char    *kStrings[];
	static const Color256 kTextColors[];

	const char *_strings[kStringCount];
	int         _stringIndex;

	const char *_lineTexts[kLineCount];
	int         _lineTimeouts[kLineCount];
	int         _lineOffsets[kLineCount];

	uint32      _timeLast;

public:
	KIASectionPogo(BladeRunnerEngine *vm);

	void open() override;

	void draw(Graphics::Surface &surface) override;
};

} // End of namespace BladeRunner

#endif
