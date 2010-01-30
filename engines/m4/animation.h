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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef M4_ANIMATION_H
#define M4_ANIMATION_H

#include "m4/m4.h"
#include "m4/graphics.h"
#include "m4/assets.h"

namespace M4 {

struct AnimationFrame {
    uint16 animFrameIndex;
    byte u;
    byte seriesIndex;
    uint16 seriesFrameIndex;
    uint16 x, y;
    byte v, w;
};

class Animation {
    public:
		Animation(MadsM4Engine *vm);
        ~Animation();

        void load(const char *filename);
		void loadFullScreen(const char *filename);
        void start();
        bool updateAnim();
        void stop();

    private:
		bool _playing;
		MadsM4Engine *_vm;
        int _seriesCount;
        int _frameCount;
        int _frameEntryCount;
        AnimationFrame *_frameEntries;
        Common::String *_spriteSeriesNames;
        SpriteAsset *_spriteSeries;
        int _curFrame, _curFrameEntry;
};

} // End of namespace M4

#endif
