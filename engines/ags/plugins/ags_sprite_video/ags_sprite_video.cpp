/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_sprite_video/ags_sprite_video.h"
#include "common/debug.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteVideo {

const char *AGSSpriteVideo::AGS_GetPluginName() {
	return "AGS SpriteVideo Plugin";
}

int LoopsPerSecond;
char video_filename[200];

struct D3D : public IAGSScriptManagedObject {
public:
	int Dispose(void *address, bool force) override {
		delete this;
		return true;
	}
	const char *GetType() override {
		return "D3D";
	};
	int Serialize(void *address, char *buffer, int bufsize) override {
		return 0;
	}
};

struct D3DVideo : public IAGSScriptManagedObject {
public:
	int Dispose(void *address, bool force) override {
		delete this;
		return true;
	}
	const char *GetType() override {
		return "D3DVideo";
	};
	int Serialize(void *address, char *buffer, int bufsize) override {
		return 0;
	}
};

void AGSSpriteVideo::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(D3D::SetLoopsPerSecond^1, AGSSpriteVideo::SetLoopsPerSecond);
	SCRIPT_METHOD(D3D::OpenVideo^1, AGSSpriteVideo::OpenVideo);
	SCRIPT_METHOD(D3D::OpenSprite, AGSSpriteVideo::OpenSprite);
	SCRIPT_METHOD(D3D::OpenSpriteFile, AGSSpriteVideo::OpenSpriteFile);

	SCRIPT_METHOD(D3D_Video::get_scaling, AGSSpriteVideo::get_scaling);
	SCRIPT_METHOD(D3D_Video::set_scaling, AGSSpriteVideo::set_scaling);
	SCRIPT_METHOD(D3D_Video::get_relativeTo, AGSSpriteVideo::get_relativeTo);
	SCRIPT_METHOD(D3D_Video::set_relativeTo, AGSSpriteVideo::set_relativeTo);
	SCRIPT_METHOD(D3D_Video::get_isLooping, AGSSpriteVideo::get_isLooping);
	SCRIPT_METHOD(D3D_Video::set_isLooping, AGSSpriteVideo::set_isLooping);
	SCRIPT_METHOD(D3D_Video::SetAnchor^2, AGSSpriteVideo::SetAnchor);
	SCRIPT_METHOD(D3D_Video::Autoplay^0, AGSSpriteVideo::Autoplay);
	SCRIPT_METHOD(D3D_Video::IsAutoplaying, AGSSpriteVideo::IsAutoplaying);
	SCRIPT_METHOD(D3D_Video::StopAutoplay, AGSSpriteVideo::StopAutoplay);
}

void AGSSpriteVideo::SetLoopsPerSecond(ScriptMethodParams &params) {
	PARAMS1(int, loops);

	debug(0, "AGSSpriteVideo: STUB - D3D SetLoopsPerSecond: %d", loops);
	LoopsPerSecond = loops;
}

void AGSSpriteVideo::OpenVideo(ScriptMethodParams &params) {
	PARAMS1(char *, filename);

	debug(0, "AGSSpriteVideo: STUB - D3D OpenVideo: %s", filename);
	D3DVideo *video = new D3DVideo();
	_engine->RegisterManagedObject(video, video);
	strncpy(video_filename, filename, sizeof(video_filename) - 1);
	LoopsPerSecond = 40;

	params._result = video;
}

void AGSSpriteVideo::OpenSprite(ScriptMethodParams &params) {
	// PARAMS1(int, graphic);

	debug(0, "AGSSpriteVideo: STUB - D3D OpenSprite");
	params._result = 0;
}

void AGSSpriteVideo::OpenSpriteFile(ScriptMethodParams &params) {
	// PARAMS2(char *, filename, int, filtering);

	debug(0, "AGSSpriteVideo: STUB - D3D OpenSpriteFile");
	params._result = 0;
}

void AGSSpriteVideo::get_scaling(ScriptMethodParams &params) {
	debug(0, "AGSSpriteVideo: STUB - D3DVideo get_scaling");
	params._result = 1;
}

void AGSSpriteVideo::set_scaling(ScriptMethodParams &params) {
	// PARAMS1(float, scaling);

	debug(0, "AGSSpriteVideo: STUB - D3DVideo set_scaling");
}

void AGSSpriteVideo::get_relativeTo(ScriptMethodParams &params) {
	debug(0, "AGSSpriteVideo: STUB - D3DVideo get_relativeTo");
	params._result = 1;
}

void AGSSpriteVideo::set_relativeTo(ScriptMethodParams &params) {
	// PARAMS1(int, relative_to);

	debug(0, "AGSSpriteVideo: STUB - D3DVideo set_relativeTo");
}

void AGSSpriteVideo::get_isLooping(ScriptMethodParams &params) {
	debug(0, "AGSSpriteVideo: STUB - D3DVideo get_isLooping");
	params._result = false;
}

void AGSSpriteVideo::set_isLooping(ScriptMethodParams &params) {
	// PARAMS1(bool, looping);

	debug(0, "AGSSpriteVideo: STUB - D3DVideo set_isLooping");
}

void AGSSpriteVideo::SetAnchor(ScriptMethodParams &params) {
	// PARAMS2(float, x, float, y);

	debug(0, "AGSSpriteVideo: STUB - D3DVideo SetAnchor");
}

void AGSSpriteVideo::Autoplay(ScriptMethodParams &params) {

	debug(0, "AGSSpriteVideo: STUB - D3DVideo Autoplay");
	warning("Current video: %s", video_filename);
	warning("Video playback is not yet implemented");
}

void AGSSpriteVideo::IsAutoplaying(ScriptMethodParams &params) {
	debug(0, "AGSSpriteVideo: STUB - D3D IsAutoPlaying");
	params._result = false;
}

void AGSSpriteVideo::StopAutoplay(ScriptMethodParams &params) {
	debug(0, "AGSSpriteVideo: STUB - D3D StopAutoplay");
}

} // namespace AGSSpriteVideo
} // namespace Plugins
} // namespace AGS3
