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

#ifndef MADSV2_ANIMVIEW_H
#define MADSV2_ANIMVIEW_H

#include "audio/audiostream.h"
#include "mads/madsv2/core/anim.h"
#include "mads/madsv2/core/cycle.h"

namespace MADS {
namespace MADSV2 {
namespace AnimView {

// Variables shared with other AnimView namespace files
extern Audio::AudioStream *speechStream;
extern int speechFlags;
extern int current_error_code;
extern int currentFrame, minFrame, maxFrame;
extern bool foundSeries;
extern int seriesMinFrame, seriesMaxFrame;
extern bool timerFlag1;
extern bool peelFlag;
extern int runCtr1;
extern int runFx;
extern long timer1, timer2;;
extern AnimPtr current_anim;
extern AnimInterPtr current_anim_inter;
extern int speechIndex;
extern int speechLoops;
extern int runVal6, runVal7, runVal8;
extern bool loadFontFlag;
extern int imageFrame;
extern CycleList anim_cycle_list;
extern bool has_cycles;
extern int currentViewX, currentViewY;


// Main animview function
extern void animview_main(const char *resName);

} // namespace AnimView
} // namespace MADSV2
} // namespace MADS

#endif
