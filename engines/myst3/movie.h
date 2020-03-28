/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef MOVIE_H_
#define MOVIE_H_

#include "engines/myst3/gfx.h"
#include "engines/myst3/node.h"

#include "math/vector3d.h"
#include "video/bink_decoder.h"

namespace Myst3 {

class Myst3Engine;
class Texture;
class Subtitles;

class Movie : public Drawable {
public:
	Movie(Myst3Engine *vm, uint16 id);
	virtual ~Movie();

	virtual void draw() override;
	virtual void drawOverlay() override;

	/** Increase or decrease the movie's pause level by one */
	void pause(bool pause);

	uint16 getId() { return _id; }
	bool isVideoLoaded() {return _bink.isVideoLoaded(); }
	void setPosU(int32 v) { _posU = v; }
	void setPosV(int32 v) { _posV = v; }
	void setForce2d(bool b);
	void setForceOpaque(bool b) { _forceOpaque = b; }
	void setStartFrame(int32 v);
	void setEndFrame(int32 v);
	void setVolume(int32 v) { _volume = v; }

protected:
	Myst3Engine *_vm;

	uint16 _id;
	Subtitles *_subtitles;

	Math::Vector3d _pTopLeft;
	Math::Vector3d _pBottomLeft;
	Math::Vector3d _pBottomRight;
	Math::Vector3d _pTopRight;

	bool _force2d;
	bool _forceOpaque;
	int32 _posU;
	int32 _posV;

	Video::BinkDecoder _bink;
	Texture *_texture;

	int32 _startFrame;
	int32 _endFrame;

	int32 _volume;

	bool _additiveBlending;
	int32 _transparency;

	int32 adjustFrameForRate(int32 frame, bool dataToBink);
	void loadPosition(const ResourceDescription::VideoData &videoData);
	void drawNextFrameToTexture();

	void draw2d();
	void draw3d();
};

class ScriptedMovie : public Movie {
public:
	ScriptedMovie(Myst3Engine *vm, uint16 id);
	virtual ~ScriptedMovie();

	void draw() override;
	void drawOverlay() override;
	virtual void update();

	void setEndFrameVar(uint16 v) { _endFrameVar = v; }
	void setNextFrameReadVar(uint16 v) { _nextFrameReadVar = v; }
	void setNextFrameWriteVar(uint16 v) { _nextFrameWriteVar = v; }
	void setPlayingVar(uint16 v) { _playingVar = v; }
	void setPosUVar(uint16 v) { _posUVar = v; }
	void setPosVVar(uint16 v) { _posVVar = v; }
	void setVolumeVar(uint16 v) { _volumeVar = v; }
	void setStartFrameVar(uint16 v) { _startFrameVar = v; }
	void setCondition(int16 condition) { _condition = condition; }
	void setConditionBit(int16 cb) { _conditionBit = cb; }
	void setDisableWhenComplete(bool upd) { _disableWhenComplete = upd; }
	void setLoop(bool loop) { _loop = loop; }
	void setScriptDriven(bool b) { _scriptDriven = b; }
	void setSoundHeading(uint16 v) { _soundHeading = v; }
	void setSoundAttenuation(uint16 v) { _soundAttenuation = v; }
	void setAdditiveBlending(bool b) { _additiveBlending = b; }
	void setTransparency(int32 v) { _transparency = v; }
	void setTransparencyVar(uint16 v) { _transparencyVar = v; }

protected:
	bool _enabled;
	bool _loop;
	bool _disableWhenComplete;
	bool _scriptDriven;
	bool _isLastFrame;

	int16 _condition;
	uint16 _conditionBit;

	uint16 _startFrameVar;
	uint16 _endFrameVar;
	uint16 _posUVar;
	uint16 _posVVar;
	uint16 _volumeVar;

	uint32 _soundHeading;
	uint32 _soundAttenuation;

	uint16 _nextFrameReadVar;
	uint16 _nextFrameWriteVar;

	uint16 _playingVar;

	uint16 _transparencyVar;

	void updateVolume();
};

class SimpleMovie : public Movie {
public:
	SimpleMovie(Myst3Engine *vm, uint16 id);
	virtual ~SimpleMovie();

	void update();
	bool endOfVideo();

	void playStartupSound();
	void refreshAmbientSounds();

	void setSynchronized(bool b) { _synchronized = b; }

	void play();

private:
	bool _synchronized;
	uint _startEngineTick;
};

// Used by the projectors on J'nanin, see puzzle #14
class ProjectorMovie : public ScriptedMovie {
public:
	ProjectorMovie(Myst3Engine *vm, uint16 id, Graphics::Surface *background);
	virtual ~ProjectorMovie();

	void update();

private:
	Graphics::Surface *_background;
	Graphics::Surface *_frame;

	static const uint kBlurIterations = 30;
	uint8 _blurTableX[kBlurIterations];
	uint8 _blurTableY[kBlurIterations];
};

} // End of namespace Myst3

#endif // MOVIE_H_
