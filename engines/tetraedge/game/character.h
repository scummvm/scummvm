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

#ifndef TETRAEDGE_GAME_CHARACTER_H
#define TETRAEDGE_GAME_CHARACTER_H

#include "common/array.h"
#include "common/str.h"
#include "common/types.h"
#include "common/ptr.h"
#include "tetraedge/te/te_animation.h"
#include "tetraedge/te/te_model_animation.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_model.h"
#include "tetraedge/te/te_bezier_curve.h"
#include "tetraedge/te/te_free_move_zone.h"
#include "tetraedge/te/te_trs.h"

namespace Tetraedge {

class Character : public TeAnimation {
public:
	Character();
	virtual ~Character();

	struct AnimSettings {
		AnimSettings() : _stepLeft(0), _stepRight(0) {};
		Common::String _file;
		int _stepLeft;
		int _stepRight;
	};

	struct WalkSettings {
		AnimSettings _walkParts[4];

		void clear();
	};

	struct CharacterSettings {
		CharacterSettings() : _walkSpeed(0.0f) {}

		Common::String _name;
		Common::String _modelFileName;
		TeVector3f32 _defaultScale;
		Common::String _idleAnimFileName;
		Common::HashMap<Common::String, WalkSettings> _walkSettings; // keys are "Walk", "Jog", etc
		float _walkSpeed;

		TeVector3f32 _cutSceneCurveDemiPosition;
		Common::String _defaultEyes;	// Note: Engine supports more, but in practice only one ever used.
		Common::String _defaultMouth;	// Note: Engine supports more, but in practice only one ever used.
		Common::String _defaultBody;	// Note: Engine supports more, but in practice only one ever used.

		bool _invertNormals;

		void clear();
	};

	struct AnimCacheElement {
		TeIntrusivePtr<TeModelAnimation> _modelAnim;
		int _size;
	};

	enum WalkPart {
		WalkPart_Start,
		WalkPart_Loop,
		WalkPart_EndD,
		WalkPart_EndG
	};

	struct Callback {
		Common::String _luaFn;
		int _triggerFrame;
		int _lastCheckFrame;
		int _maxCalls;
		float _callsMade;
	};

	void addCallback(const Common::String &s1, const Common::String &s2, float f1, float f2);

	static void animCacheFreeAll();
	static void animCacheFreeOldest();
	static TeIntrusivePtr<TeModelAnimation> animCacheLoad(const Common::Path &path);

	float animLength(const TeModelAnimation &modelanim, int bone, int lastframe);
	float animLengthFromFile(const Common::String &animname, uint32 *pframeCount, uint lastframe = 9999);
	bool blendAnimation(const Common::String &animname, float amount, bool repeat, bool returnToIdle);
	TeVector3f32 correctPosition(const TeVector3f32 &pos);
	float curveOffset();
	void deleteAllCallback();
	void deleteAnim();
	void deleteCallback(const Common::String &str1, const Common::String &str2, float f);
	//static bool deserialize(TiXmlElement *param_1, Walk *param_2);
	void endMove();

	const WalkSettings *getCurrentWalkFiles();
	bool isFramePassed(int frameno);
	bool isWalkEnd();
	int leftStepFrame(enum WalkPart walkpart);
	int rightStepFrame(enum WalkPart walkpart);
	bool loadModel(const Common::String &name, bool unused);
	static bool loadSettings(const Common::String &path);

	bool onBonesUpdate(const Common::String &boneName, TeMatrix4x4 &boneMatrix);
	bool onModelAnimationFinished();
	void permanentUpdate();
	void placeOnCurve(TeIntrusivePtr<TeBezierCurve> &curve);
	//void play() // just called TeAnimation::play();
	void removeAnim();
	void removeFromCurve();
	static Common::String rootBone() { return "Pere"; }

	bool setAnimation(const Common::String &name, bool repeat, bool returnToIdle = false, bool unused = false, int startFrame = -1, int endFrame = 9999);
	void setAnimationSound(const Common::String &name, uint offset);
	void setCurveOffset(float offset);
	void setFreeMoveZone(TeFreeMoveZone *zone);
	bool setShadowVisible(bool visible);
	void setStepSound(const Common::String &stepSound1, const Common::String &stepSound2);
	float speedFromAnim(double amount);
	//void stop(); // just maps to TeAnimation::stop();
	float translationFromAnim(const TeModelAnimation &anim, int bone, int frame);
	TeVector3f32 translationVectorFromAnim(const TeModelAnimation &anim, int bone, int frame);
	TeTRS trsFromAnim(const TeModelAnimation &anim, int bone, int frame);
	void update(double percentval) override;
	void updateAnimFrame();
	void updatePosition(float curveOffset);
	Common::String walkAnim(WalkPart part);
	void walkMode(const Common::String &mode);
	void walkTo(float curveEnd, bool walkFlag);

	TeIntrusivePtr<TeModel> _model;
	TeIntrusivePtr<TeModel> _shadowModel[2];
	TeSignal1Param<const Common::String &> _characterAnimPlayerFinishedSignal;
	TeSignal1Param<const Common::String &> _onCharacterAnimFinishedSignal;

	const CharacterSettings &characterSettings() const { return _characterSettings; }
	Common::String &walkModeStr() { return _walkModeStr; } // writable for loading games.
	const Common::String &curAnimName() const { return _curAnimName; }
	TeFreeMoveZone *freeMoveZone() { return _freeMoveZone; }
	const Common::String &freeMoveZoneName() const { return _freeMoveZoneName; }
	void setFreeMoveZoneName(const Common::String &val) { _freeMoveZoneName = val; }
	bool needsSomeUpdate() const { return _needsSomeUpdate; }
	void setNeedsSomeUpdate(bool val) { _needsSomeUpdate = val; }
	void setCharLookingAt(Character *other) { _charLookingAt = other; }
	const TeVector3f32 &positionCharacter() const { return _positionCharacter; }
	void setPositionCharacter(const TeVector3f32 &val) { _positionCharacter = val; }
	bool positionFlag() const { return _positionFlag; }
	void setPositionFlag(bool val) { _positionFlag = val; }
	void setCurveStartLocation(const TeVector3f32 &val) { _curveStartLocation = val; }
	bool hasAnchor() const { return _hasAnchor; }
	void setHasAnchor(bool val) { _hasAnchor = val; }
	const TeVector2f32 &headRotation() const { return _headRotation; }
	void setHeadRotation(const TeVector2f32 &val) { _headRotation = val; }
	void setLastHeadRotation(const TeVector2f32 &val) { _lastHeadRotation = val; }
	const TeVector3f32 &lastHeadBoneTrans() const { return _lastHeadBoneTrans; }
	Character *charLookingAt() { return _charLookingAt; }
	bool lookingAtTallThing() const { return _lookingAtTallThing; }
	void setLookingAtTallThing(bool val) { _lookingAtTallThing = val; }
	TeIntrusivePtr<TeBezierCurve> curve() { return _curve; }
	void setRecallageY(bool val) { _recallageY = val; }

	static void cleanup();

private:
	float _walkCurveStart;
	float _walkCurveLast;
	float _walkCurveEnd;
	float _walkCurveLen;
	float _walkCurveIncrement;
	float _walkCurveNextLength;
	float _walkedLength;
	int _walkTotalFrames;
	bool _walkToFlag;
	bool _walkEndAnimG;
	TeIntrusivePtr<TeBezierCurve> _curve;
	TeVector3f32 _curveStartLocation;

	TeFreeMoveZone *_freeMoveZone;
	Common::String _freeMoveZoneName;
	Common::String _stepSound1;
	Common::String _stepSound2;
	Common::String _walkModeStr; // Walk or Jog
	Common::String _animSound;

	Character *_charLookingAt;

	uint _animSoundOffset;

	TeIntrusivePtr<TeModelAnimation> _curModelAnim;

	CharacterSettings _characterSettings;

	float _walkStartAnimLen;
	float _walkLoopAnimLen;
	float _walkEndGAnimLen;

	uint32 _walkStartAnimFrameCount;
	uint32 _walkLoopAnimFrameCount;
	uint32 _walkEndGAnimFrameCount;

	int _lastFrame;
	int _lastAnimFrame;
	bool _notWalkAnim;
	bool _returnToIdleAnim;
	bool _callbacksChanged;
	bool _needsSomeUpdate;
	bool _positionFlag;
	bool _lookingAtTallThing;
	bool _hasAnchor;
	bool _recallageY;

	TeVector2f32 _headRotation;
	TeVector2f32 _lastHeadRotation;
	TeVector3f32 _lastHeadBoneTrans;

	TeVector3f32 _positionCharacter;

	// TODO: work out how these are different
	Common::String _setAnimName;
	Common::String _curAnimName;

	Common::HashMap<Common::String, Common::Array<Callback *>> _callbacks;

	// static Common::Array<AnimCacheElement> *_animCache; // Never used?
	// static uint _animCacheSize; // Never used?
	static Common::HashMap<Common::String, TeIntrusivePtr<TeModelAnimation>> *_animCacheMap;
	static Common::HashMap<Common::String, CharacterSettings> *_globalCharacterSettings;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_CHARACTER_H
