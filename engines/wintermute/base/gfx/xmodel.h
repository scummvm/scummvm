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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_XMODEL_H
#define WINTERMUTE_XMODEL_H

#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/video/video_theora_player.h"
#include "engines/wintermute/utils/utils.h"

#include "math/matrix4.h"
#include "math/vector3d.h"
#include "math/vector4d.h"

namespace Wintermute {

class AnimationChannel;
class AnimationSet;
class FrameNode;
class Material;
class ShadowVolume;
class XFileLexer;

struct MaterialReference {
	Common::String _name;
	Material *_material;
};

#define X_NUM_ANIMATION_CHANNELS 10

class XModel : public BaseObject {
private:
	// the D3DX effect stuff is missing here
	// at the moment I am not aware of whether this is used
	// in Alpha Polaris or any other WME game
	// if it is, then this would mean a decent amount of work
	// since we would need to parse and emulate D3DX effects in OpenGL
	class XModelMatSprite {
	public:
		char *_matName;
		BaseSprite *_sprite;
		VideoTheoraPlayer *_theora;

		XModelMatSprite() {
			_matName = nullptr;
			_sprite = nullptr;
			_theora = nullptr;
		}

		XModelMatSprite(const char *matName, BaseSprite *sprite) {
			_theora = nullptr;
			_matName = nullptr;
			BaseUtils::setString(&_matName, matName);
			_sprite = sprite;
		}

		XModelMatSprite(const char *matName, VideoTheoraPlayer *theora) {
			_sprite = nullptr;
			_matName = nullptr;
			BaseUtils::setString(&_matName, matName);
			_theora = theora;
		}

		~XModelMatSprite() {
			delete[] _matName;
			delete _sprite;
			delete _theora;
		}

		bool setSprite(BaseSprite *sprite) {
			delete _theora;
			_theora = nullptr;
			delete _sprite;
			_sprite = sprite;

			return true;
		}

		bool setTheora(VideoTheoraPlayer *theora) {
			delete _theora;
			delete _sprite;
			_sprite = nullptr;
			_theora = theora;

			return true;
		}

		bool persist(BasePersistenceManager *persistMgr) {
			persistMgr->transferCharPtr(TMEMBER(_matName));
			persistMgr->transferPtr(TMEMBER(_sprite));

			persistMgr->transferPtr(TMEMBER(_theora));

			return true;
		}
	};

public:
	// default ticks per second for .X models seems to be 4800
	// not sure if this is truly documented anywhere, though
	// on the other hand, wme chooses the same value,
	// so should be fine for our purposes
	const static int kDefaultTicksPerSecond = 4800;

	DECLARE_PERSISTENT(XModel, BaseObject)

	XModel(BaseGame *inGame, BaseObject *owner);
	virtual ~XModel();

	XModel *_parentModel;

	bool loadFromFile(const Common::String &filename, XModel *parentModel = nullptr);
	bool mergeFromFile(const Common::String &filename);

	bool update() override;
	bool render();
	bool renderFlatShadowModel();
	bool reset();

	bool updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth);

	bool playAnim(int channel, const Common::String &anim, uint32 transitionTime = 0, bool forceReset = false, uint32 stopTransitionTime = 0);
	bool isAnimPending(char *animName = nullptr);
	bool isAnimPending(int channel, const char *animName = nullptr);

	bool isTransparentAt(int x, int y);

	bool loadAnimationSet(XFileLexer &lexer, const Common::String &filename);
	bool loadAnimation(const Common::String &filename, AnimationSet *parentAnimSet);

	Math::Matrix4 _lastWorldMat;
	Rect32 _boundingRect;
	BaseObject *_owner;

	bool parseAnim(byte *buffer);
	bool parseEvent(AnimationSet *anim, byte *buffer);
	AnimationSet *getAnimationSetByName(const Common::String &name);

	bool stopAnim(int channel, uint32 transitionTime);
	bool stopAnim(uint32 transitionTime);

	Math::Matrix4 *getBoneMatrix(const char *boneName);
	FrameNode *getRootFrame();

	bool setMaterialSprite(const char *materialName, const char *spriteFilename);
	bool setMaterialTheora(const char *materialName, const char *theoraFilename);
	bool initializeSimple();

	bool invalidateDeviceObjects() override;
	bool restoreDeviceObjects() override;

	bool unloadAnimation(const char *animName);

	uint32 _ticksPerSecond;

	BaseArray<AnimationSet *> _animationSets;

private:
	void cleanup(bool complete = true);
	bool findBones(bool animOnly = false, XModel *parentModel = nullptr);

	void parseFrameDuringMerge(XFileLexer &lexer, const Common::String &filename);

	void updateBoundingRect();
	void static inline updateRect(Rect32 *rc, int32 x, int32 y);
	Rect32 _drawingViewport;
	Math::Matrix4 _lastViewMat;
	Math::Matrix4 _lastProjMat;
	int32 _lastOffsetX;
	int32 _lastOffsetY;

	Math::Vector3d _BBoxStart;
	Math::Vector3d _BBoxEnd;

	Common::Array<MaterialReference> _materialReferences;

protected:
	BaseArray<const char*> _mergedModels;
	AnimationChannel *_channels[X_NUM_ANIMATION_CHANNELS];

	FrameNode *_rootFrame;

	BaseArray<XModelMatSprite *> _matSprites;
};

} // namespace Wintermute

#endif
