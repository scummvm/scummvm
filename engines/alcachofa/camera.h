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

#ifndef ALCACHOFA_CAMERA_H
#define ALCACHOFA_CAMERA_H

#include "alcachofa/common.h"
#include "math/matrix4.h"

namespace Alcachofa {

class Graphic;
class WalkingCharacter;
class Process;
struct Task;

static constexpr const int16_t kBaseScale = 300;
static constexpr const float kInvBaseScale = 1.0f / kBaseScale;

class Camera {
public:
	static Camera *create();
	virtual ~Camera();
	virtual Math::Angle rotation() const = 0;
	virtual float scale() const = 0;

	virtual void preUpdate() = 0;
	virtual void update() = 0;
	virtual void setRoomBounds(Graphic &background) = 0;
	virtual void setFollow(WalkingCharacter *target) = 0;
	virtual void onChangedRoom(bool resetCamera) = 0;
	virtual void onTriggeredDoor(WalkingCharacter *target) = 0;
	virtual void onTriggeredDoor(Common::Point fixedPosition) = 0;
	virtual void onScriptChangedCharacter(MainCharacterKind kind) = 0;
	virtual void onUserChangedCharacter() = 0;
	virtual void onOpenMenu() = 0;
	virtual void onCloseMenu() = 0;
	virtual void syncGame(Common::Serializer &s) = 0;

	Math::Vector3d transform2Dto3D(Math::Vector3d v) const;
	Math::Vector3d transform3Dto2D(Math::Vector3d v) const;
	Common::Point transform3Dto2D(Common::Point p) const;

protected:
	Math::Vector3d setAppliedCenter(Math::Vector3d center);
	void setupMatricesAround(Math::Vector3d center);

	float _roomScale = 1.0f;
	Math::Vector2d
		_roomMin = Math::Vector2d(-10000, -10000),
		_roomMax = Math::Vector2d(10000, 10000);
	Math::Vector3d _appliedCenter;
	Math::Matrix4
		_mat3Dto2D,
		_mat2Dto3D;
};

class CameraV1 : public Camera {
public:
	Math::Angle rotation() const override;
	float scale() const override;

	void preUpdate() override;
	void update() override;
	void setRoomBounds(Graphic &background) override;
	void setFollow(WalkingCharacter *target) override;
	void onChangedRoom(bool resetCamera) override;
	void onTriggeredDoor(WalkingCharacter *target) override;
	void onTriggeredDoor(Common::Point fixedPosition) override;
	void onScriptChangedCharacter(MainCharacterKind kind) override;
	void onUserChangedCharacter() override;
	void onOpenMenu() override;
	void onCloseMenu() override;
	void syncGame(Common::Serializer &s) override;
	void lerpOrSet(Common::Point target, int32 mode);

	Task *disguise(Process &process, int32 duration);

protected:
	friend struct CamV1DisguiseTask;
	void updateLerping(Math::Vector3d &newCenter, float deltaTime, float speed);

	WalkingCharacter *_followTarget = nullptr;
	Math::Vector3d _target;
	bool _isLerping = false;
	float _lerpSpeed = 0.0f;
	uint32 _lastUpdateTime = 0;
};

// V2 is so similar that most can be reused from V1
class CameraV2 final : public CameraV1 {
public:
	void update() override;
	void setRoomBounds(Graphic &background) override;
	void setFollow(WalkingCharacter *target) override;
};

class CameraV3 final : public Camera {
public:
	Math::Angle rotation() const override;
	float scale() const override;

	void preUpdate() override;
	void update() override;
	void setRoomBounds(Graphic &background) override;
	void setFollow(WalkingCharacter *target) override;
	void setFollow(WalkingCharacter *target, bool catchup);
	void onChangedRoom(bool resetCamera) override;
	void onTriggeredDoor(WalkingCharacter *target) override;
	void onTriggeredDoor(Common::Point fixedPosition) override;
	void onScriptChangedCharacter(MainCharacterKind kind) override;
	void onUserChangedCharacter() override;
	void onOpenMenu() override;
	void onCloseMenu() override;
	void syncGame(Common::Serializer &s) override;

	Task *lerpPos(Process &process,
		Math::Vector2d targetPos,
		int32 duration, EasingType easingType);
	Task *lerpPos(Process &process,
		Math::Vector3d targetPos,
		int32 duration, EasingType easingType);
	Task *lerpPosZ(Process &process,
		float targetPosZ,
		int32 duration, EasingType easingType);
	Task *lerpScale(Process &process,
		float targetScale,
		int32 duration, EasingType easingType);
	Task *lerpRotation(Process &process,
		float targetRotation,
		int32 duration, EasingType easingType);
	Task *lerpPosScale(Process &process,
		Math::Vector3d targetPos, float targetScale,
		int32 duration, EasingType moveEasingType, EasingType scaleEasingType);
	Task *waitToStop(Process &process);
	Task *shake(Process &process, Math::Vector2d amplitude, Math::Vector2d frequency, int32 duration);

private:
	friend struct CamLerpTask;
	friend struct CamLerpPosTask;
	friend struct CamLerpScaleTask;
	friend struct CamLerpPosScaleTask;
	friend struct CamLerpRotationTask;
	friend struct CamShakeTask;
	friend struct CamWaitToStopTask;
	friend struct CamSetInactiveAttributeTask;

	void resetRotationAndScale();
	void setPosition(Math::Vector2d v);
	void setPosition(Math::Vector3d v);
	void backup(uint slot);
	void restore(uint slot);
	void updateFollowing(float deltaTime);

	struct State {
		Math::Vector3d _usedCenter = Math::Vector3d(512, 384, 0);
		float
			_scale = 1.0f,
			_speed = 0.0f,
			_maxSpeedFactor = 230.0f;
		Math::Angle _rotation;
		bool _isBraking = false;
		bool _isFollowingTarget = false;

		void syncGame(Common::Serializer &s);
	};

	static constexpr uint kStateBackupCount = 2;
	State _cur, _backups[kStateBackupCount];
	WalkingCharacter *_followTarget = nullptr;
	uint32 _lastUpdateTime = 0;
	bool _isChanging = false,
		_catchUp = false;
	Math::Vector2d _shake;
};

}

#endif // ALCACHOFA_CAMERA_H
