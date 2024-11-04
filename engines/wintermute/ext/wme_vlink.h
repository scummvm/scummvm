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

#ifndef WINTERMUTE_VLINK_H
#define WINTERMUTE_VLINK_H

#include "common/str.h"
#include "common/mutex.h"
#include "video/video_decoder.h"
#include "graphics/surface.h"

#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/gfx/base_surface.h"

namespace Wintermute {

class SXVlink : public BaseScriptable {
public:
	DECLARE_PERSISTENT(SXVlink, BaseScriptable)
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
	SXVlink(BaseGame *inGame, ScStack *stack);
	~SXVlink() override;
	
private:
	Common::Mutex _frameMutex;
	Video::VideoDecoder *_videoDecoder{};
	Graphics::Surface _surface;
	int32 _volume;
	bool _videoFinished{};
	bool _updateNeeded{};
	int32 _frame{};
	
	static void timerCallback(void *instance);
	void prepareFrame();
};

} // End of namespace Wintermute

#endif
