/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_MODEL_X_H
#define WINTERMUTE_MODEL_X_H

#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/video/video_theora_player.h"
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

class ModelX : public BaseObject {
private:
	// the D3DX effect stuff is missing here
	// at the moment I am not aware of whether this is used
	// in Alpha Polaris or any other WME game
	// if it is, then this would mean a decent amount of work
	// since we would need to parse and emulate D3DX effects in OpenGL
	class ModelXMatSprite {
	public:
		char *_matName;
		BaseSprite *_sprite;
		VideoTheoraPlayer *_theora;

		ModelXMatSprite() {
			_matName = nullptr;
			_sprite = nullptr;
			_theora = nullptr;
		}

		ModelXMatSprite(const char *matName, BaseSprite *sprite) {
			_theora = nullptr;
			_matName = nullptr;
			uint32 size = strlen(matName);
			_matName = new char[size + 1];
			Common::copy(matName, matName + size, _matName);
			_matName[size] = 0;
			_sprite = sprite;
		}

		ModelXMatSprite(const char *matName, VideoTheoraPlayer *theora) {
			_sprite = nullptr;
			_matName = nullptr;
			uint32 size = strlen(matName);
			_matName = new char[size + 1];
			Common::copy(matName, matName + size, _matName);
			_matName[size] = 0;
			_theora = theora;
		}

		~ModelXMatSprite() {
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

	DECLARE_PERSISTENT(ModelX, BaseObject)

	ModelX(BaseGame *inGame, BaseObject *owner);
	virtual ~ModelX();

	ModelX *_parentModel;

	bool loadFromFile(const Common::String &filename, ModelX *parentModel = nullptr);
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

	virtual bool invalidateDeviceObjects() override;
	virtual bool restoreDeviceObjects() override;

	bool unloadAnimation(const char *animName);

	uint32 _ticksPerSecond;

	BaseArray<AnimationSet *> _animationSets;

private:
	void cleanup(bool complete = true);
	bool findBones(bool animOnly = false, ModelX *parentModel = nullptr);

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

	BaseArray<ModelXMatSprite *> _matSprites;
};

} // namespace Wintermute

#endif
