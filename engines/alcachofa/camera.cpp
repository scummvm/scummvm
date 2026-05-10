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

#include "alcachofa/camera.h"
#include "alcachofa/alcachofa.h"
#include "alcachofa/graphics.h"

#include "common/system.h"
#include "math/vector4d.h"

using namespace Common;
using namespace Math;

namespace Alcachofa {

//
// Base camera
//
Camera *Camera::create() {
	if (g_engine->isV1())
		return new CameraV1();
	else if (g_engine->isV2())
		return new CameraV2();
	else if (g_engine->isV3())
		return new CameraV3();
	else
		error("Camera is not implemented for this engine version");
}

Camera::~Camera() {}

static Matrix4 scale2DMatrix(float scale) {
	Matrix4 m;
	m(0, 0) = scale;
	m(1, 1) = scale;
	return m;
}

void Camera::setupMatricesAround(Vector3d center) {
	Matrix4 matTemp;
	matTemp.buildAroundZ(rotation());
	_mat3Dto2D.setToIdentity();
	_mat3Dto2D.translate(-center);
	_mat3Dto2D = matTemp * _mat3Dto2D;
	_mat3Dto2D = scale2DMatrix(scale()) * _mat3Dto2D;

	_mat2Dto3D.setToIdentity();
	_mat2Dto3D.translate(center);
	matTemp.buildAroundZ(-rotation());
	matTemp = matTemp * scale2DMatrix(1 / scale());
	_mat2Dto3D = _mat2Dto3D * matTemp;
}

void minmax(Vector3d &min, Vector3d &max, Vector3d val) {
	min.set(
		MIN(min.x(), val.x()),
		MIN(min.y(), val.y()),
		MIN(min.z(), val.z()));
	max.set(
		MAX(max.x(), val.x()),
		MAX(max.y(), val.y()),
		MAX(max.z(), val.z()));
}

Vector3d Camera::setAppliedCenter(Vector3d center) {
	setupMatricesAround(center);
	if (g_engine->game().shouldClipCamera()) {
		const float screenW = g_system->getWidth(), screenH = g_system->getHeight();
		Vector3d min, max;
		min = max = transform2Dto3D(Vector3d(0, 0, _roomScale));
		minmax(min, max, transform2Dto3D(Vector3d(screenW, 0, _roomScale)));
		minmax(min, max, transform2Dto3D(Vector3d(screenW, screenH, _roomScale)));
		minmax(min, max, transform2Dto3D(Vector3d(0, screenH, _roomScale)));
		center.x() += MAX(0.0f, _roomMin.getX() - min.x());
		center.y() += MAX(0.0f, _roomMin.getY() - min.y());
		center.x() -= MAX(0.0f, max.x() - _roomMax.getX());
		center.y() -= MAX(0.0f, max.y() - _roomMax.getY());
		setupMatricesAround(center);
	}
	return _appliedCenter = center;
}

Vector3d Camera::transform2Dto3D(Vector3d v2d) const {
	// if this looks like normal 3D math to *someone* please contact.
	Vector4d vh;
	vh.w() = 1.0f;
	vh.z() = v2d.z() - _appliedCenter.z();
	vh.y() = (v2d.y() - g_system->getHeight() * 0.5f) * vh.z() * kInvBaseScale;
	vh.x() = (v2d.x() - g_system->getWidth() * 0.5f) * vh.z() * kInvBaseScale;
	vh = _mat2Dto3D * vh;
	return Vector3d(vh.x(), vh.y(), 0.0f);
}

Vector3d Camera::transform3Dto2D(Vector3d v3d) const {
	// I swear there is a better way than this. This is stupid. But it is original.
	float depthScale = v3d.z() * kInvBaseScale;
	Vector4d vh;
	vh.x() = v3d.x() * depthScale + (1 - depthScale) * g_system->getWidth() * 0.5f;
	vh.y() = v3d.y() * depthScale + (1 - depthScale) * g_system->getHeight() * 0.5f;
	vh.z() = v3d.z();
	vh.w() = 1.0f;
	vh = _mat3Dto2D * vh;
	return Vector3d(
		g_system->getWidth() * 0.5f + vh.x() * kBaseScale / vh.z(),
		g_system->getHeight() * 0.5f + vh.y() * kBaseScale / vh.z(),
		scale() * kBaseScale / vh.z());
}

Point Camera::transform3Dto2D(Point p3d) const {
	auto v2d = transform3Dto2D({ (float)p3d.x, (float)p3d.y, kBaseScale });
	return { (int16)v2d.x(), (int16)v2d.y() };
}

static void syncMatrix(Serializer &s, Matrix4 &m) {
	float *data = m.getData();
	for (int i = 0; i < 16; i++)
		s.syncAsFloatLE(data[i]);
}

static void syncVector(Serializer &s, Vector3d &v) {
	s.syncAsFloatLE(v.x());
	s.syncAsFloatLE(v.y());
	s.syncAsFloatLE(v.z());
}

static void syncFollowTarget(Serializer &s, WalkingCharacter *&followTarget) {
	// originally the follow object is also searched for before changing the room
	// so that would practically mean only the main characters could be reasonably found
	// instead we fall back to global search
	String name;
	if (followTarget != nullptr)
		name = followTarget->name();
	s.syncString(name);
	if (s.isLoading()) {
		if (name.empty())
			followTarget = nullptr;
		else {
			followTarget = dynamic_cast<WalkingCharacter *>(g_engine->world().getObjectByName(name.c_str()));
			if (followTarget == nullptr)
				followTarget = dynamic_cast<WalkingCharacter *>(g_engine->world().getObjectByNameFromAnyRoom(name.c_str()));
			if (followTarget == nullptr)
				warning("Camera follow target from savestate was not found: %s", name.c_str());
		}
	}
}

//
// CameraV1
//

Angle CameraV1::rotation() const {
	return {};
}

float CameraV1::scale() const {
	return 1.0f;
}

void CameraV1::preUpdate() {
}

void CameraV1::update() {
	auto deltaTime = (g_engine->getMillis() - _lastUpdateTime) / 1000.0f;
	auto newCenter = _appliedCenter;

	if (_followTarget != nullptr) {
		// this threshold is responsible for the jitter while following
		const auto threshold = _isLerping ? 100 : 200;
		_target = as3D(_followTarget->position());
		auto deltaPos = _target - newCenter;
		_lastUpdateTime = g_engine->getMillis();

		_isLerping = false;
		if (fabsf(deltaPos.x()) > threshold) {
			newCenter.x() += copysignf(350.0f, deltaPos.x()) * deltaTime;
			_isLerping = true;
		}
		if (fabsf(deltaPos.y()) > threshold) {
			newCenter.y() += copysignf(350.0f, deltaPos.y()) * deltaTime;
			_isLerping = true;
		}
	} else if (_isLerping) {
		updateLerping(newCenter, deltaTime, _lerpSpeed);
	}

	setAppliedCenter(newCenter);
}

void CameraV1::updateLerping(Vector3d &newCenter, float deltaTime, float speed) {
	auto distance = newCenter.getDistanceTo(_target);
	auto move = deltaTime * speed;
	_lastUpdateTime = g_engine->getMillis();

	if (move < distance)
		newCenter += (_target - newCenter) / distance * move;
	else {
		newCenter = _target;
		_isLerping = false;
	}
}

void CameraV2::update() {
	auto deltaTime = (g_engine->getMillis() - _lastUpdateTime) / 1000.0f;
	auto newCenter = _appliedCenter;

	if (_followTarget != nullptr) {
		_target = as3D(_followTarget->position());
		auto delta = _target - _appliedCenter;
		_isLerping |= MAX(fabsf(delta.x()), fabsf(delta.y())) > 35.0f;

		if (_isLerping) {
			// The original code contains this formula to attenuate camera speed
			// However timing experiments show that this is somehow negated or overwritten
			//float scaleFactor = _followTarget->graphic()->depthScale();
			//if (scaleFactor < 19660 / 65535.0f)
			//	scaleFactor = (13107 / 65535.0f) + scaleFactor / 3;
			updateLerping(newCenter, deltaTime, _lerpSpeed);
		} else
			_lastUpdateTime = g_engine->getMillis();
	} else if (_isLerping)
		updateLerping(newCenter, deltaTime, _lerpSpeed);

	setAppliedCenter(newCenter);
}

void CameraV1::setRoomBounds(Graphic &background) {
	auto bgSize = background.animation().imageSize(0);
	Point screenSize(g_system->getWidth(), g_system->getHeight());
	_roomMin = as2D(background.topLeft() + screenSize / 2);
	_roomMax = _roomMin + as2D(bgSize - screenSize);
	_roomScale = 0;
}

void CameraV2::setRoomBounds(Graphic &background) {
	Point bgSize = background.animation().imageSize(0);
	float scaleFactor = background.scale() / (float)kBaseScale;
	Point screenSize(g_system->getWidth(), g_system->getHeight());
	_roomMin = as2D(background.topLeft() + screenSize / 2) * scaleFactor;
	_roomMax = _roomMin + as2D(bgSize - screenSize) * scaleFactor;
	_roomScale = 0;
}

void CameraV1::setFollow(WalkingCharacter *target) {
	_lastUpdateTime = g_engine->getMillis();
	_followTarget = target;
	_isLerping = false;
	if (target != nullptr)
		setAppliedCenter(as3D(target->position()));
}

void CameraV2::setFollow(WalkingCharacter *target) {
	CameraV1::setFollow(target);
	_lerpSpeed = 230.0f;
}

void CameraV1::onChangedRoom(bool resetCamera) {
	// nothing to do in V1
}

void CameraV1::onTriggeredDoor(WalkingCharacter *target) {
	setFollow(target);
}

void CameraV1::onTriggeredDoor(Common::Point fixedPosition) {
	// should probably never be called
	debug(1, "Set camera to fixed position in V1: %d, %d", fixedPosition.x, fixedPosition.y);
}

void CameraV1::onScriptChangedCharacter(MainCharacterKind kind) {
	if (kind != MainCharacterKind::None)
		setFollow(g_engine->player().activeCharacter());
}

void CameraV1::onUserChangedCharacter() {
	setFollow(g_engine->player().activeCharacter());
}

void CameraV1::onOpenMenu() {
	// we rely on room bounds clipping to set the camera position
	// interaction locks prevent opening menus during lerps, follows are fine due to clipping 
}

void CameraV1::onCloseMenu() {
}

void CameraV1::syncGame(Serializer &s) {
	syncVector(s, _appliedCenter);
	syncMatrix(s, _mat3Dto2D);
	syncMatrix(s, _mat2Dto3D);
	syncFollowTarget(s, _followTarget);
	syncVector(s, _target);
	s.syncAsByte(_isLerping);
	s.syncAsFloatLE(_lerpSpeed);
	s.syncAsUint32LE(_lastUpdateTime);
} 

void CameraV1::lerpOrSet(Point target, int32 mode) {
	_target = as3D(target);
	_lastUpdateTime = g_engine->getMillis();
	_followTarget = nullptr;
	_isLerping = true;

	if (mode == 1) {
		// snap to target
		_isLerping = false;
		_appliedCenter = _target;
	} else if (mode <= 0) {
		// fixed speed, overshoot target
		_target.x() += copysignf(100, _appliedCenter.x() - _target.x());
		_target.y() += copysignf(100, _appliedCenter.y() - _target.y());
		_lerpSpeed = 350.0f;
	} else {
		// dynamic speed
		_lerpSpeed = MAX(1.0f, _target.getDistanceTo(_appliedCenter) / mode);
	}
}

// The original name for this task is "disfraza" which I can only translate as "disguise"
// It is a slightly bouncing vertical camera movement with fixed distance

struct CamV1DisguiseTask final : public Task {
	CamV1DisguiseTask(Process &process, int32 durationMs)
		: Task(process)
		, _camera(g_engine->cameraV1())
		, _durationMs(durationMs) {}

	CamV1DisguiseTask(Process &process, Serializer &s)
		: Task(process)
		, _camera(g_engine->cameraV1()) {
		CamV1DisguiseTask::syncGame(s);
	}

	TaskReturn run() override {
		if (_startTime == 0) {
			_startPosition = _camera._appliedCenter;
			_startTime = g_engine->getMillis();
		}
		if (_durationMs <= 0 || g_engine->getMillis() - _startTime >= (uint32)_durationMs)
			return TaskReturn::finish(0);

		Vector3d newPosition = _startPosition;
		uint32 t = (g_engine->getMillis() - _startTime) * 5;
		if (t <= 50)
			newPosition.y() += t;
		else if (t <= 150)
			newPosition.y() += 100 - t;
		else if (t >= 200)
			newPosition.y() += t - 200;
		_camera._appliedCenter = newPosition;
		_camera.setFollow(nullptr);

		return TaskReturn::yield();
	}

	void debugPrint() override {
		g_engine->getDebugger()->debugPrintf("\"Disguise\" camera for %dms", _durationMs);
	}

	void syncGame(Serializer &s) override {
		Task::syncGame(s);
		s.syncAsSint32LE(_durationMs);
		s.syncAsUint32LE(_startTime);
		syncVector(s, _startPosition);
	}

	const char *taskName() const override;

private:
	CameraV1 &_camera;
	int32 _durationMs = 0;
	uint32 _startTime = 0;
	Vector3d _startPosition;
};
DECLARE_TASK(CamV1DisguiseTask)

Task *CameraV1::disguise(Process &process, int32 duration) {
	return new CamV1DisguiseTask(process, duration);
}

//
// CameraV3
//

Angle CameraV3::rotation() const {
	return _cur._rotation;
}

float CameraV3::scale() const {
	return _cur._scale;
}

void CameraV3::preUpdate() {
	_shake = {};
}

void CameraV3::setRoomBounds(Graphic &background) {
	auto bgSize = background.animation().imageSize(0);
	/* The fallback fixes a bug where if the background image is invalid the original engine
		* would not update the background size. This would be around 1024,768 due to
		* previous rooms in the bug instances I found.
		*/
	if (bgSize == Point(0, 0))
		bgSize = Point(1024, 768);

	const auto bgScale = background.scale();
	float scaleFactor = 1 - bgScale * kInvBaseScale;
	_roomMin = Vector2d(
		g_system->getWidth() / 2 * scaleFactor,
		g_system->getHeight() / 2 * scaleFactor);
	_roomMax = _roomMin + Vector2d(
		bgSize.x * bgScale * kInvBaseScale,
		bgSize.y * bgScale * kInvBaseScale);
	_roomScale = bgScale;
}

void CameraV3::setFollow(WalkingCharacter *target) {
	setFollow(target, false);
}

void CameraV3::setFollow(WalkingCharacter *target, bool catchUp) {
	_cur._isFollowingTarget = target != nullptr;
	_followTarget = target;
	_lastUpdateTime = g_engine->getMillis();
	_catchUp = catchUp;
	if (target == nullptr)
		_isChanging = false;
}

void CameraV3::onChangedRoom(bool resetCamera) {
	if (resetCamera)
		resetRotationAndScale();
	if (_followTarget != nullptr)
		setFollow(_followTarget, true);
}

void CameraV3::onTriggeredDoor(WalkingCharacter *target) {
	setFollow(target, true);
}

void CameraV3::onTriggeredDoor(Point fixedPosition) {
	setPosition(as2D(fixedPosition));
}

void CameraV3::onScriptChangedCharacter(MainCharacterKind kind) {
	resetRotationAndScale();
	if (kind != MainCharacterKind::None)
		setFollow(g_engine->player().activeCharacter());
	backup(0);
}

void CameraV3::onUserChangedCharacter() {
	setFollow(g_engine->player().activeCharacter());
	restore(0);
}

void CameraV3::onOpenMenu() {
	backup(1);
	setPosition(Math::Vector3d(
		g_system->getWidth() / 2.0f, g_system->getHeight() / 2.0f, 0.0f));
}

void CameraV3::onCloseMenu() {
	restore(1);
}

void CameraV3::resetRotationAndScale() {
	_cur._scale = 1;
	_cur._rotation = 0;
	_cur._usedCenter.z() = 0;
}

void CameraV3::setPosition(Vector2d v) {
	setPosition({ v.getX(), v.getY(), _cur._usedCenter.z() });
}

void CameraV3::setPosition(Vector3d v) {
	_cur._usedCenter = v;
	setFollow(nullptr);
}

void CameraV3::backup(uint slot) {
	assert(slot < kStateBackupCount);
	_backups[slot] = _cur;
}

void CameraV3::restore(uint slot) {
	assert(slot < kStateBackupCount);
	auto backupState = _backups[slot];
	_backups[slot] = _cur;
	_cur = backupState;
}

void CameraV3::update() {
	// original would be some smoothing of delta times, let's not.
	uint32 now = g_engine->getMillis();
	float deltaTime = (now - _lastUpdateTime) / 1000.0f;
	deltaTime = MAX(0.001f, MIN(0.5f, deltaTime));
	_lastUpdateTime = now;

	if (_catchUp) {
		for (int i = 0; i < 4; i++)
			updateFollowing(50.0f);
		_catchUp = false;
	} else
		updateFollowing(deltaTime);
	setAppliedCenter(_cur._usedCenter + Vector3d(_shake.getX(), _shake.getY(), 0.0f));
}

void CameraV3::updateFollowing(float deltaTime) {
	if (!_cur._isFollowingTarget || _followTarget == nullptr)
		return;
	const float resolutionFactor = g_system->getWidth() * 0.00125f;
	const float acceleration = 460 * resolutionFactor;
	const float baseDeadZoneSize = 25 * resolutionFactor;
	const float minSpeed = 20 * resolutionFactor;
	const float maxSpeed = this->_cur._maxSpeedFactor * resolutionFactor;
	const float depthScale = _followTarget->graphic()->depthScale();
	const auto characterPolygon = _followTarget->shape()->at(0);
	const float halfHeight = ABS(characterPolygon._points[0].y - characterPolygon._points[2].y) / 2.0f;

	Vector3d targetCenter = setAppliedCenter({
		_shake.getX() + _followTarget->position().x,
		_shake.getY() + _followTarget->position().y - depthScale * 85,
		_cur._usedCenter.z() });
	targetCenter.y() -= halfHeight;
	float distanceToTarget = as2D(_cur._usedCenter - targetCenter).getMagnitude();
	float moveDistance = _followTarget->stepSizeFactor() * _cur._speed * deltaTime;

	float deadZoneSize = baseDeadZoneSize / _cur._scale;
	if (_followTarget->isWalking() && depthScale > 0.8f)
		deadZoneSize = (baseDeadZoneSize + (depthScale - 0.8f) * 200) / _cur._scale;
	bool isFarAway = false;
	if (ABS(targetCenter.x() - _cur._usedCenter.x()) > deadZoneSize ||
		ABS(targetCenter.y() - _cur._usedCenter.y()) > deadZoneSize) {
		isFarAway = true;
		_cur._isBraking = false;
		_isChanging = true;
	}

	if (_cur._isBraking) {
		_cur._speed -= acceleration * 0.9f * deltaTime;
		_cur._speed = MAX(_cur._speed, minSpeed);
	}
	if (_isChanging && !_cur._isBraking) {
		_cur._speed += acceleration * deltaTime;
		_cur._speed = MIN(_cur._speed, maxSpeed);
		if (!isFarAway)
			_cur._isBraking = true;
	}
	if (_isChanging) {
		if (distanceToTarget <= moveDistance) {
			_cur._usedCenter = targetCenter;
			_isChanging = false;
			_cur._isBraking = false;
		} else {
			Vector3d deltaCenter = targetCenter - _cur._usedCenter;
			deltaCenter.z() = 0.0f;
			_cur._usedCenter += deltaCenter * moveDistance / distanceToTarget;
		}
	}
}

void CameraV3::State::syncGame(Serializer &s) {
	syncVector(s, _usedCenter);
	s.syncAsFloatLE(_scale);
	s.syncAsFloatLE(_speed);
	s.syncAsFloatLE(_maxSpeedFactor);
	float rotationDegs = _rotation.getDegrees();
	s.syncAsFloatLE(rotationDegs);
	_rotation.setDegrees(rotationDegs);
	s.syncAsByte(_isBraking);
	s.syncAsByte(_isFollowingTarget);
}

void CameraV3::syncGame(Serializer &s) {
	syncMatrix(s, _mat3Dto2D);
	syncMatrix(s, _mat2Dto3D);
	syncVector(s, _appliedCenter);
	s.syncAsUint32LE(_lastUpdateTime);
	s.syncAsByte(_isChanging);
	_cur.syncGame(s);
	for (uint i = 0; i < kStateBackupCount; i++)
		_backups[i].syncGame(s);

	syncFollowTarget(s, _followTarget);
}

struct CamLerpTask : public Task {
	CamLerpTask(Process &process, uint32 duration = 0, EasingType easingType = EasingType::Linear)
		: Task(process)
		, _camera(g_engine->cameraV3())
		, _duration(duration)
		, _easingType(easingType) {}

	TaskReturn run() override {
		TASK_BEGIN;
		_startTime = g_engine->getMillis();
		while (g_engine->getMillis() - _startTime < _duration) {
			update(ease((g_engine->getMillis() - _startTime) / (float)_duration, _easingType));
			_camera._isChanging = true;
			TASK_YIELD(1);
		}
		update(1.0f);
		TASK_END;
	}

	void debugPrint() override {
		uint32 remaining = g_engine->getMillis() - _startTime <= _duration
			? _duration - (g_engine->getMillis() - _startTime)
			: 0;
		g_engine->console().debugPrintf("%s camera with %ums remaining\n", taskName(), remaining);
	}

	void syncGame(Serializer &s) override {
		Task::syncGame(s);
		s.syncAsUint32LE(_startTime);
		s.syncAsUint32LE(_duration);
		syncEnum(s, _easingType);
	}

protected:
	virtual void update(float t) = 0;

	CameraV3 &_camera;
	uint32 _startTime = 0, _duration;
	EasingType _easingType;
};

struct CamLerpPosTask final : public CamLerpTask {
	CamLerpPosTask(Process &process, Vector3d targetPos, int32 duration, EasingType easingType)
		: CamLerpTask(process, duration, easingType)
		, _fromPos(_camera._appliedCenter)
		, _deltaPos(targetPos - _camera._appliedCenter) {}

	CamLerpPosTask(Process &process, Serializer &s)
		: CamLerpTask(process) {
		syncGame(s);
	}

	void syncGame(Serializer &s) override {
		CamLerpTask::syncGame(s);
		syncVector(s, _fromPos);
		syncVector(s, _deltaPos);
	}

	const char *taskName() const override;

protected:
	void update(float t) override {
		_camera.setPosition(_fromPos + _deltaPos * t);
	}

	Vector3d _fromPos, _deltaPos;
};
DECLARE_TASK(CamLerpPosTask)

struct CamLerpScaleTask final : public CamLerpTask {
	CamLerpScaleTask(Process &process, float targetScale, int32 duration, EasingType easingType)
		: CamLerpTask(process, duration, easingType)
		, _fromScale(_camera._cur._scale)
		, _deltaScale(targetScale - _camera._cur._scale) {}

	CamLerpScaleTask(Process &process, Serializer &s)
		: CamLerpTask(process) {
		syncGame(s);
	}

	void syncGame(Serializer &s) override {
		CamLerpTask::syncGame(s);
		s.syncAsFloatLE(_fromScale);
		s.syncAsFloatLE(_deltaScale);
	}

	const char *taskName() const override;

protected:
	void update(float t) override {
		_camera._cur._scale = _fromScale + _deltaScale * t;
	}

	float _fromScale = 0, _deltaScale = 0;
};
DECLARE_TASK(CamLerpScaleTask)

struct CamLerpPosScaleTask final : public CamLerpTask {
	CamLerpPosScaleTask(Process &process,
						Vector3d targetPos, float targetScale,
						int32 duration,
						EasingType moveEasingType, EasingType scaleEasingType)
		: CamLerpTask(process, duration, EasingType::Linear) // linear as we need different ones per component
		, _fromPos(_camera._appliedCenter)
		, _deltaPos(targetPos - _camera._appliedCenter)
		, _fromScale(_camera._cur._scale)
		, _deltaScale(targetScale - _camera._cur._scale)
		, _moveEasingType(moveEasingType)
		, _scaleEasingType(scaleEasingType) {}

	CamLerpPosScaleTask(Process &process, Serializer &s)
		: CamLerpTask(process) {
		syncGame(s);
	}

	void syncGame(Serializer &s) override {
		CamLerpTask::syncGame(s);
		syncVector(s, _fromPos);
		syncVector(s, _deltaPos);
		s.syncAsFloatLE(_fromScale);
		s.syncAsFloatLE(_deltaScale);
		syncEnum(s, _moveEasingType);
		syncEnum(s, _scaleEasingType);
	}

	const char *taskName() const override;

protected:
	void update(float t) override {
		_camera.setPosition(_fromPos + _deltaPos * ease(t, _moveEasingType));
		_camera._cur._scale = _fromScale + _deltaScale * ease(t, _scaleEasingType);
	}

	Vector3d _fromPos, _deltaPos;
	float _fromScale = 0, _deltaScale = 0;
	EasingType _moveEasingType = {}, _scaleEasingType = {};
};
DECLARE_TASK(CamLerpPosScaleTask)

struct CamLerpRotationTask final : public CamLerpTask {
	CamLerpRotationTask(Process &process, float targetRotation, int32 duration, EasingType easingType)
		: CamLerpTask(process, duration, easingType)
		, _fromRotation(_camera._cur._rotation.getDegrees())
		, _deltaRotation(targetRotation - _camera._cur._rotation.getDegrees()) {}

	CamLerpRotationTask(Process &process, Serializer &s)
		: CamLerpTask(process) {
		syncGame(s);
	}

	void syncGame(Serializer &s) override {
		CamLerpTask::syncGame(s);
		s.syncAsFloatLE(_fromRotation);
		s.syncAsFloatLE(_deltaRotation);
	}

	const char *taskName() const override;

protected:
	void update(float t) override {
		_camera._cur._rotation = Angle(_fromRotation + _deltaRotation * t);
	}

	float _fromRotation = 0, _deltaRotation = 0;
};
DECLARE_TASK(CamLerpRotationTask)

static void syncVector(Serializer &s, Vector2d &v) {
	float *data = v.getData();
	s.syncAsFloatLE(data[0]);
	s.syncAsFloatLE(data[1]);
}

struct CamShakeTask final : public CamLerpTask {
	CamShakeTask(Process &process, Vector2d amplitude, Vector2d frequency, int32 duration)
		: CamLerpTask(process, duration, EasingType::Linear)
		, _amplitude(amplitude)
		, _frequency(frequency) {}

	CamShakeTask(Process &process, Serializer &s)
		: CamLerpTask(process) {
		syncGame(s);
	}

	void syncGame(Serializer &s) override {
		CamLerpTask::syncGame(s);
		syncVector(s, _amplitude);
		syncVector(s, _frequency);
	}

	const char *taskName() const override;

protected:
	void update(float t) override {
		const Vector2d phase = _frequency * t * (float)M_PI * 2.0f;
		const float amplTimeFactor = 1.0f / expf(t * 5.0f); // a curve starting at 1, depreciating towards 0 
		_camera._shake = {
			sinf(phase.getX()) * _amplitude.getX() * amplTimeFactor,
			sinf(phase.getY()) * _amplitude.getY() * amplTimeFactor
		};
	}

	Vector2d _amplitude, _frequency;
};
DECLARE_TASK(CamShakeTask)

struct CamWaitToStopTask final : public Task {
	CamWaitToStopTask(Process &process)
		: Task(process)
		, _camera(g_engine->cameraV3()) {}

	CamWaitToStopTask(Process &process, Serializer &s)
		: Task(process)
		, _camera(g_engine->cameraV3()) {
		syncGame(s);
	}

	TaskReturn run() override {
		return _camera._isChanging
			? TaskReturn::yield()
			: TaskReturn::finish(1);
	}

	void debugPrint() override {
		g_engine->console().debugPrintf("Wait for camera to stop moving\n");
	}

	const char *taskName() const override;

private:
	CameraV3 &_camera;
};
DECLARE_TASK(CamWaitToStopTask)

struct CamSetInactiveAttributeTask final : public Task {
	enum Attribute {
		kPosZ,
		kScale,
		kRotation
	};

	CamSetInactiveAttributeTask(Process &process, Attribute attribute, float value, int32 delay)
		: Task(process)
		, _camera(g_engine->cameraV3())
		, _attribute(attribute)
		, _value(value)
		, _delay(delay) {}

	CamSetInactiveAttributeTask(Process &process, Serializer &s)
		: Task(process)
		, _camera(g_engine->cameraV3()) {
		syncGame(s);
	}

	TaskReturn run() override {
		if (_delay > 0) {
			uint32 delay = (uint32)_delay;
			_delay = 0;
			return TaskReturn::waitFor(new DelayTask(process(), delay));
		}

		auto &state = _camera._backups[0];
		switch (_attribute) {
		case kPosZ:
			state._usedCenter.z() = _value;
			break;
		case kScale:
			state._scale = _value;
			break;
		case kRotation:
			state._rotation = _value;
			break;
		default:
			g_engine->game().unknownCamSetInactiveAttribute((int)_attribute);
			break;
		}
		return TaskReturn::finish(0);
	}

	void debugPrint() override {
		const char *attributeName;
		switch (_attribute) {
		case kPosZ:
			attributeName = "PosZ";
			break;
		case kScale:
			attributeName = "Scale";
			break;
		case kRotation:
			attributeName = "Rotation";
			break;
		default:
			attributeName = "<unknown>";
			break;
		}
		g_engine->console().debugPrintf("Set inactive camera %s to %f after %dms\n", attributeName, _value, _delay);
	}

	void syncGame(Serializer &s) override {
		Task::syncGame(s);
		syncEnum(s, _attribute);
		s.syncAsFloatLE(_value);
		s.syncAsSint32LE(_delay);
	}

	const char *taskName() const override;

private:
	CameraV3 &_camera;
	Attribute _attribute = {};
	float _value = 0;
	int32 _delay = 0;
};
DECLARE_TASK(CamSetInactiveAttributeTask)

Task *CameraV3::lerpPos(Process &process,
					  Vector2d targetPos,
					  int32 duration, EasingType easingType) {
	if (!process.isActiveForPlayer()) {
		return new DelayTask(process, duration); // lerpPos does not handle inactive players
	}
	Vector3d targetPos3d(targetPos.getX(), targetPos.getY(), _appliedCenter.z());
	return new CamLerpPosTask(process, targetPos3d, duration, easingType);
}

Task *CameraV3::lerpPos(Process &process,
					  Vector3d targetPos,
					  int32 duration, EasingType easingType) {
	if (!process.isActiveForPlayer()) {
		return new DelayTask(process, duration); // lerpPos does not handle inactive players
	}
	setFollow(nullptr); // 3D position lerping is the only task that resets following
	return new CamLerpPosTask(process, targetPos, duration, easingType);
}

Task *CameraV3::lerpPosZ(Process &process,
					   float targetPosZ,
					   int32 duration, EasingType easingType) {
	if (!process.isActiveForPlayer()) {
		return new CamSetInactiveAttributeTask(process, CamSetInactiveAttributeTask::kPosZ, targetPosZ, duration);
	}
	Vector3d targetPos(_appliedCenter.x(), _appliedCenter.y(), targetPosZ);
	return new CamLerpPosTask(process, targetPos, duration, easingType);
}

Task *CameraV3::lerpScale(Process &process,
						float targetScale,
						int32 duration, EasingType easingType) {
	if (!process.isActiveForPlayer()) {
		return new CamSetInactiveAttributeTask(process, CamSetInactiveAttributeTask::kScale, targetScale, duration);
	}
	return new CamLerpScaleTask(process, targetScale, duration, easingType);
}

Task *CameraV3::lerpRotation(Process &process,
						   float targetRotation,
						   int32 duration, EasingType easingType) {
	if (!process.isActiveForPlayer()) {
		return new CamSetInactiveAttributeTask(process, CamSetInactiveAttributeTask::kRotation, targetRotation, duration);
	}
	return new CamLerpRotationTask(process, targetRotation, duration, easingType);
}

Task *CameraV3::lerpPosScale(Process &process,
						   Vector3d targetPos, float targetScale,
						   int32 duration,
						   EasingType moveEasingType, EasingType scaleEasingType) {
	if (!process.isActiveForPlayer()) {
		return new CamSetInactiveAttributeTask(process, CamSetInactiveAttributeTask::kScale, targetScale, duration);
	}
	return new CamLerpPosScaleTask(process, targetPos, targetScale, duration, moveEasingType, scaleEasingType);
}

Task *CameraV3::waitToStop(Process &process) {
	return new CamWaitToStopTask(process);
}

Task *CameraV3::shake(Process &process, Math::Vector2d amplitude, Math::Vector2d frequency, int32 duration) {
	if (!process.isActiveForPlayer()) {
		return new DelayTask(process, (uint32)duration);
	}
	return new CamShakeTask(process, amplitude, frequency, duration);
}

} // namespace Alcachofa
