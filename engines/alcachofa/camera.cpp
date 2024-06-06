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

#include "camera.h"
#include "script.h"
#include "alcachofa.h"

#include "common/system.h"
#include "math/vector4d.h"

using namespace Common;
using namespace Math;

namespace Alcachofa {

void Camera::resetRotationAndScale() {
	_scale = 1;
	_rotation = 0;
	_usedCenter.z() = 0;
}

void Camera::setRoomBounds(Point bgSize, int16 bgScale) {
	float scaleFactor = 1 - bgScale * kInvBaseScale;
	_roomMin = Vector2d(
		g_system->getWidth() / 2 * scaleFactor,
		g_system->getHeight() / 2 * scaleFactor);
	_roomMax = _roomMin + Vector2d(
		bgSize.x * bgScale * kInvBaseScale,
		bgSize.y * bgScale * kInvBaseScale);
	_roomScale = bgScale;
}

void Camera::setFollow(WalkingCharacter *target, bool catchUp) {
	_followTarget = target;
	_lastUpdateTime = g_system->getMillis();
	_catchUp = catchUp;
	if (target == nullptr)
		_isChanging = false;
}

void Camera::setPosition(Vector2d v) {
	setPosition({ v.getX(), v.getY(), _usedCenter.z() });
}

void Camera::setPosition(Vector3d v) {
	_usedCenter = v;
	setFollow(nullptr);
}

static Matrix4 scaleMatrix(float scale) {
	Matrix4 m;
	m(0, 0) = scale;
	m(1, 1) = scale;
	m(2, 2) = scale;
	return m;
}

void Camera::setupMatricesAround(Vector3d center) {
	Matrix4 matTemp;
	matTemp.buildAroundZ(_rotation);
	_mat3Dto2D.setToIdentity();
	_mat3Dto2D.translate(-center);
	_mat3Dto2D = matTemp * _mat3Dto2D;
	_mat3Dto2D = _mat3Dto2D * scaleMatrix(_scale);

	_mat2Dto3D.setToIdentity();
	_mat2Dto3D.translate(center);
	matTemp.buildAroundZ(-_rotation);
	matTemp = scaleMatrix(1 / _scale) * matTemp;
	_mat2Dto3D = matTemp * _mat2Dto3D;
}

void minmax(Vector3d &min, Vector3d &max, Vector3d val)
{
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
	if (g_engine->script().variable("EncuadrarCamara") || true) {
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
	vh.z() = v2d.z() - _usedCenter.z();
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
		_scale * kBaseScale / vh.z());
}

void Camera::update() {
	// original would be some smoothing of delta times, let's not.
	uint32 now = g_system->getMillis();
	float deltaTime = (now - _lastUpdateTime) / 1000.0f;
	deltaTime = MAX(0.001f, MIN(0.5f, deltaTime));
	_lastUpdateTime = now;

	if (_catchUp && _followTarget != nullptr) {
		for (int i = 0; i < 4; i++)
			updateFollowing(50.0f);
	}
	else
		updateFollowing(deltaTime);
	setAppliedCenter(_usedCenter + Vector3d(_shake.getX(), _shake.getY(), 0.0f));
}

void Camera::updateFollowing(float deltaTime) {
	if (_followTarget == nullptr)
		return;
	const float resolutionFactor = g_system->getWidth() * 0.00125f;
	const float acceleration = 460 * resolutionFactor;
	const float baseDeadZoneSize = 25 * resolutionFactor;
	const float minSpeed = 20 * resolutionFactor;
	const float maxSpeed = this->_maxSpeedFactor * resolutionFactor;
	const float depthScale = _followTarget->graphic()->depthScale();
	const auto characterPolygon = _followTarget->shape()->at(0);
	const float halfHeight = ABS(characterPolygon._points[0].y - characterPolygon._points[2].y) / 2.0f;

	Vector3d targetCenter = setAppliedCenter({
		_shake.getX() + _followTarget->position().x,
		_shake.getY() + _followTarget->position().y - depthScale * 85,
		_usedCenter.z()});
	targetCenter.y() -= halfHeight;
	float distanceToTarget = as2D(_usedCenter - targetCenter).getMagnitude();
	float moveDistance = _followTarget->stepSizeFactor() * _speed * deltaTime;

	float deadZoneSize = baseDeadZoneSize / _scale;
	if (_followTarget->isWalking() && depthScale > 0.8f)
		deadZoneSize = (baseDeadZoneSize + (depthScale - 0.8f) * 200) / _scale;
	bool isFarAway = false;
	if (ABS(targetCenter.x() - _usedCenter.x()) > deadZoneSize ||
		ABS(targetCenter.y() - _usedCenter.y()) > deadZoneSize) {
		isFarAway = true;
		_isBraking = false;
		_isChanging = true;
	}

	if (_isBraking) {
		_speed -= acceleration * 0.9f * deltaTime;
		_speed = MAX(_speed, minSpeed);
	}
	if (_isChanging && !_isBraking) {
		_speed += acceleration * deltaTime;
		_speed = MIN(_speed, maxSpeed);
		if (!isFarAway)
			_isBraking = true;
	}
	if (_isChanging) {
		if (distanceToTarget <= moveDistance) {
			_usedCenter = targetCenter;
			_isChanging = false;
			_isBraking = false;
		}
		else {
			Vector3d deltaCenter = targetCenter - _usedCenter;
			deltaCenter.z() = 0.0f;
			_usedCenter += deltaCenter * moveDistance / distanceToTarget;
		}
	}
}

struct CamLerpTask : public Task {
	CamLerpTask(Process &process, uint32 duration, EasingType easingType)
		: Task(process)
		, _camera(g_engine->camera())
		, _duration(duration)
		, _easingType(easingType) {}

	virtual TaskReturn run() override {
		TASK_BEGIN;
		_startTime = g_system->getMillis();
		while (g_system->getMillis() - _startTime < _duration) {
			update(ease((g_system->getMillis() - _startTime) / (float)_duration, _easingType));
			_camera._isChanging = true;
			TASK_YIELD;
		}
		update(1.0f);
		TASK_END;
	}

	virtual void debugPrint() override {
		uint32 remaining = g_system->getMillis() - _startTime <= _duration
			? _duration - (g_system->getMillis() - _startTime)
			: 0;
		g_engine->console().debugPrintf("Lerp camera with %ums remaining\n", remaining);
	}

protected:
	virtual void update(float t) = 0;

	Camera &_camera;
	uint32 _startTime = 0, _duration;
	EasingType _easingType;
};

struct CamLerpPosTask final : public CamLerpTask {
	CamLerpPosTask(Process &process, Vector3d targetPos, int32 duration, EasingType easingType)
		: CamLerpTask(process, duration, easingType)
		, _fromPos(_camera._appliedCenter)
		, _deltaPos(targetPos - _camera._appliedCenter) {}

protected:
	virtual void update(float t) override {
		_camera.setPosition(_fromPos + _deltaPos * t);
	}

	Vector3d _fromPos, _deltaPos;
};

struct CamLerpScaleTask final : public CamLerpTask {
	CamLerpScaleTask(Process &process, float targetScale, int32 duration, EasingType easingType)
		: CamLerpTask(process, duration, easingType)
		, _fromScale(_camera._scale)
		, _deltaScale(targetScale - _camera._scale) {}

protected:
	virtual void update(float t) override {
		_camera._scale = _fromScale + _deltaScale * t;
	}

	float _fromScale, _deltaScale;
};

struct CamLerpPosScaleTask final : public CamLerpTask {
	CamLerpPosScaleTask(Process &process,
		Vector3d targetPos, float targetScale,
		int32 duration, EasingType moveEasingType, EasingType scaleEasingType)
		: CamLerpTask(process, duration, EasingType::Linear) // linear as we need different ones per component
		, _fromPos(_camera._appliedCenter)
		, _deltaPos(targetPos - _camera._appliedCenter)
		, _fromScale(_camera._scale)
		, _deltaScale(targetScale - _camera._scale)
		, _moveEasingType(moveEasingType)
		, _scaleEasingType(scaleEasingType) {}

protected:
	virtual void update(float t) override {
		_camera.setPosition(_fromPos + _deltaPos * ease(t, _moveEasingType));
		_camera._scale = _fromScale + _deltaScale * ease(t, _scaleEasingType);
	}

	Vector3d _fromPos, _deltaPos;
	float _fromScale, _deltaScale;
	EasingType _moveEasingType, _scaleEasingType;
};

struct CamLerpRotationTask final : public CamLerpTask {
	CamLerpRotationTask(Process &process, float targetRotation, int32 duration, EasingType easingType)
		: CamLerpTask(process, duration, easingType)
		, _fromRotation(_camera._rotation.getDegrees())
		, _deltaRotation(targetRotation - _camera._rotation.getDegrees()) {}

protected:
	virtual void update(float t) override {
		_camera._rotation = Angle(_fromRotation + _deltaRotation * t);
	}

	float _fromRotation, _deltaRotation;
};

struct CamWaitToStopTask final : public Task {
	CamWaitToStopTask(Process &process)
		: Task(process)
		, _camera(g_engine->camera()) {}

	virtual TaskReturn run() override {
		return _camera._isChanging
			? TaskReturn::yield()
			: TaskReturn::finish(1);
	}

	virtual void debugPrint() override {
		g_engine->console().debugPrintf("Wait for camera to stop moving\n");
	}

private:
	Camera &_camera;
};

Task *Camera::lerpPos(Process &process,
	Vector2d targetPos,
	int32 duration, EasingType easingType) {
	if (!process.isActiveForPlayer()) {
		warning("stub: non-active camera lerp script invoked");
		return new DelayTask(process, duration);
	}
	Vector3d targetPos3d(targetPos.getX(), targetPos.getY(), _appliedCenter.z());
	return new CamLerpPosTask(process, targetPos3d, duration, easingType);
}

Task *Camera::lerpPos(Process &process,
	Vector3d targetPos,
	int32 duration, EasingType easingType) {
	if (!process.isActiveForPlayer()) {
		warning("stub: non-active camera lerp script invoked");
		return new DelayTask(process, duration);
	}
	setFollow(nullptr); // 3D position lerping is the only task that resets following
	return new CamLerpPosTask(process, targetPos, duration, easingType);
}

Task *Camera::lerpPosZ(Process &process,
	float targetPosZ,
	int32 duration, EasingType easingType) {
	if (!process.isActiveForPlayer()) {
		warning("stub: non-active camera lerp script invoked");
		return new DelayTask(process, duration);
	}
	Vector3d targetPos(_appliedCenter.x(), _appliedCenter.y(), targetPosZ);
	return new CamLerpPosTask(process, targetPos, duration, easingType);
}

Task *Camera::lerpScale(Process &process,
	float targetScale,
	int32 duration, EasingType easingType) {
	if (!process.isActiveForPlayer()) {
		warning("stub: non-active camera lerp script invoked");
		return new DelayTask(process, duration);
	}
	return new CamLerpScaleTask(process, targetScale, duration, easingType);
}

Task *Camera::lerpRotation(Process &process,
	float targetRotation,
	int32 duration, EasingType easingType) {
	if (!process.isActiveForPlayer()) {
		warning("stub: non-active camera lerp script invoked");
		return new DelayTask(process, duration);
	}
	return new CamLerpRotationTask(process, targetRotation, duration, easingType);
}

Task *Camera::lerpPosScale(Process &process,
	Vector3d targetPos, float targetScale,
	int32 duration, EasingType moveEasingType, EasingType scaleEasingType) {
	if (!process.isActiveForPlayer()) {
		warning("stub: non-active camera lerp script invoked");
		return new DelayTask(process, duration);
	}
	return new CamLerpPosScaleTask(process, targetPos, targetScale, duration, moveEasingType, scaleEasingType);
}

Task *Camera::waitToStop(Process &process) {
	return new CamWaitToStopTask(process);
}

}
