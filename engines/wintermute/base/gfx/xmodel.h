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
#include "engines/wintermute/base/gfx/xmath.h"
#include "engines/wintermute/base/gfx/3deffect.h"
#include "engines/wintermute/base/gfx/3deffect_params.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/video/video_theora_player.h"
#include "engines/wintermute/utils/utils.h"

namespace Wintermute {

class AnimationChannel;
class AnimationSet;
class FrameNode;
class Material;
class ShadowVolume;
class XFileData;
class Effect3D;
class Effect3DParams;

#define X_NUM_ANIMATION_CHANNELS 10

class XModel : public BaseObject {
private:
	class XModelMatSprite {
	public:
		char *_matName;
		char *_effectFile;
		BaseSprite *_sprite;
		VideoTheoraPlayer *_theora;
		Effect3D *_effect;
		Effect3DParams *_effectParams;

		XModelMatSprite() {
			_matName = nullptr;
			_sprite = nullptr;
			_theora = nullptr;
			_effect = nullptr;
			_effectFile = nullptr;
			_effectParams = nullptr;
		}

		XModelMatSprite(const char *matName, BaseSprite *sprite) {
			_theora = nullptr;
			_matName = nullptr;
			_effect = nullptr;
			BaseUtils::setString(&_matName, matName);
			_sprite = sprite;
			_effectFile = nullptr;
			_effectParams = nullptr;
		}

		XModelMatSprite(const char *matName, VideoTheoraPlayer *theora) {
			_sprite = nullptr;
			_matName = nullptr;
			_effect = nullptr;
			BaseUtils::setString(&_matName, matName);
			_theora = theora;
			_effectFile = nullptr;
			_effectParams = nullptr;
		}

		XModelMatSprite(const char *matName, Effect3D *effect) {
			_sprite = nullptr;
			_matName = nullptr;
			_theora = nullptr;
			BaseUtils::setString(&_matName, matName);
			_effect = effect;
			_effectFile = nullptr;
			_effectParams = new Effect3DParams();
		}

		~XModelMatSprite() {
			delete[] _matName;
			_matName = nullptr;
			delete _effectFile;
			_effectFile = nullptr;
			delete _sprite;
			_sprite = nullptr;
			delete _theora;
			_theora = nullptr;
			delete _effect;
			_effect = nullptr;
			delete _effectParams;
			_effectParams = nullptr;
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
			_theora = nullptr;
			delete _sprite;
			_sprite = nullptr;
			_theora = theora;

			return true;
		}

		bool setEffect(Effect3D *effect) {
			delete _effect;
			_effect = effect;

			if (!_effectParams)
				_effectParams = new Effect3DParams();
			else
				_effectParams->clear();

			return true;
		}

		bool persist(BasePersistenceManager *persistMgr) {
			persistMgr->transferCharPtr(TMEMBER(_matName));
			persistMgr->transferPtr(TMEMBER(_sprite));

			persistMgr->transferPtr(TMEMBER(_theora));

			if (persistMgr->getIsSaving()) {
				char *effectFileName = nullptr;
				if (_effect)
					BaseUtils::setString(&effectFileName, _effect->getFileName());
				else
					effectFileName = nullptr;

				persistMgr->transferCharPtr(TMEMBER(effectFileName));
				delete[] effectFileName;
			} else {
				persistMgr->transferCharPtr(TMEMBER(_effectFile));
			}

			if (persistMgr->getIsSaving()) {
				bool hasParams = _effectParams != nullptr;
				persistMgr->transferBool(TMEMBER(hasParams));

				if (hasParams)
					_effectParams->persist(persistMgr);
			} else {
				bool hasParams;
				persistMgr->transferBool(TMEMBER(hasParams));

				if (hasParams) {
					_effectParams = new Effect3DParams();
					_effectParams->persist(persistMgr);
				} else
					_effectParams = nullptr;
			}

			return true;
		}
	};

public:

	const static int kDefaultTicksPerSecond = 4800;

	DECLARE_PERSISTENT(XModel, BaseObject)

	XModel(BaseGame *inGame, BaseObject *owner);
	virtual ~XModel();

	XModel *_parentModel{};

	bool loadFromFile(const Common::String &filename, XModel *parentModel = nullptr);
	bool mergeFromFile(const Common::String &filename);

	bool loadAnimationSet(const Common::String &filename, XFileData *xobj);
	bool loadAnimation(const Common::String &filename, XFileData *xobj, AnimationSet *parentAnimSet = nullptr);

	bool update() override;
	bool render();
	bool renderFlatShadowModel();
	bool reset();

	bool updateShadowVol(ShadowVolume *shadow, DXMatrix *modelMat, DXVector3 *light, float extrusionDepth);

	bool playAnim(int channel, const Common::String &anim, uint32 transitionTime = 0, bool forceReset = false, uint32 stopTransitionTime = 0);
	bool isAnimPending(char *animName = nullptr);
	bool isAnimPending(int channel, const char *animName = nullptr);

	bool isTransparentAt(int x, int y);

	static bool loadName(BaseNamedObject *obj, XFileData *data);
	static bool loadName(Common::String &targetStr, XFileData *data);

	Rect32 _boundingRect;
	BaseObject *_owner{};

	bool parseAnim(byte *buffer);
	bool parseEvent(AnimationSet *anim, byte *buffer);
	AnimationSet *getAnimationSetByName(const Common::String &name);

	bool stopAnim(int channel, uint32 transitionTime);
	bool stopAnim(uint32 transitionTime);

	DXMatrix *getBoneMatrix(const char *boneName);
	FrameNode *getRootFrame();

	bool setMaterialSprite(const char *materialName, const char *spriteFilename);
	bool setMaterialTheora(const char *materialName, const char *theoraFilename);
	bool setMaterialEffect(const char *materialName, const char *effectFilename);
	bool removeMaterialEffect(const char *materialName);
	bool setMaterialEffectParam(const char *materialName, const char *paramName, ScValue *val);
	bool setMaterialEffectParam(const char *materialName, const char *paramName, DXVector4 val);
	bool initializeSimple();

	bool invalidateDeviceObjects() override;
	bool restoreDeviceObjects() override;

	bool unloadAnimation(const char *animName);

	uint32 _ticksPerSecond{};

	BaseArray<AnimationSet *> _animationSets;

private:
	void cleanup(bool complete = true);
	bool findBones(bool animOnly = false, XModel *parentModel = nullptr);

	void updateBoundingRect();
	void static inline updateRect(Rect32 *rc, DXVector3 *vec);
	DXViewport _drawingViewport{};
	DXMatrix _lastWorldMat;
	DXMatrix _lastViewMat;
	DXMatrix _lastProjMat;
	int32 _lastOffsetX{};
	int32 _lastOffsetY{};

	DXVector3 _BBoxStart;
	DXVector3 _BBoxEnd;

protected:
	BaseArray<const char*> _mergedModels;
	AnimationChannel *_channels[X_NUM_ANIMATION_CHANNELS]{};

	FrameNode *_rootFrame{};

	BaseArray<XModelMatSprite *> _matSprites;
};

} // namespace Wintermute

#endif
