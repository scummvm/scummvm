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

#include "common/random.h"
#include "common/util.h"

#include "engines/nancy/enginedata.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"

#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/mathtypes.h"
#include "engines/nancy/script/particles.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/types.h"

namespace Nancy {
namespace Script {

// The simulation goes in steps of a fixed length
static const double kStepsPerSecond = 30.0;

// Particles are kept in a ring of this many
static const uint kInitialParticles = 1024;

static double randomFraction() {
	return g_nancy->_randomSource->getRandomNumber(0xFFFFFF) / 16777216.0;
}

static double lerp(double from, double to, double fraction) {
	return from + (to - from) * fraction;
}

static bool readTruth(lua_State *L, int tableIndex, const char *name) {
	lua_getfield(L, tableIndex, name);
	const bool value = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);
	return value;
}

// What happens to the particles, in the order the record runs the stages. A
// stage can give a new particle its values, and do something to the particles
// every step.
class ParticleStage {
public:
	ParticleStage(ParticleRecord &record) : _record(record) {}
	virtual ~ParticleStage() {}

	// A stage that picks its values ahead of time does it for every particle
	// as soon as it is added, and for the new ones when there is room for more
	void onAdded() {
		onGrown(0, _record.getMaxParticles());
	}

	void onGrown(uint first, uint count) {
		if (_isPicked) {
			initParticles(first, count);
		}
	}

	void onSpawn(uint first, uint count) {
		if (!_isPicked) {
			initParticles(first, count);
		}
	}

	virtual void update(uint first, uint count) {}

protected:
	virtual void initParticles(uint first, uint count) {}

	// Scripts set it to "true", so anything but false and nil counts
	void readPicked(lua_State *L, int tableIndex) {
		_isPicked = readTruth(L, tableIndex, "init");
	}

	ParticleRecord &_record;
	bool _isPicked = false;
};

// Every record has these two: the particles get older...
class ParticleLifeStage : public ParticleStage {
public:
	ParticleLifeStage(ParticleRecord &record) : ParticleStage(record) {}

	void update(uint first, uint count) override {
		const double step = 1.0 / kStepsPerSecond / _record.getLife();
		for (uint i = first; i < first + count; ++i) {
			_record.getParticle(i).life += step;
		}
	}

protected:
	void initParticles(uint first, uint count) override {
		for (uint i = first; i < first + count; ++i) {
			_record.getParticle(i).life = 0.0;
		}
	}
};

// ...and the forces on them get worked out anew every step
class ParticleForceClearStage : public ParticleStage {
public:
	ParticleForceClearStage(ParticleRecord &record) : ParticleStage(record) {}

	void update(uint first, uint count) override {
		for (uint i = first; i < first + count; ++i) {
			Particle &particle = _record.getParticle(i);
			particle.acceleration[0] = particle.acceleration[1] = particle.acceleration[2] = 0.0;
		}
	}
};

// Reads a vector field of the stage's table into the given one
static bool readVectorField(lua_State *L, int tableIndex, const char *name, double *out, int size) {
	if (!pushField(L, tableIndex, name)) {
		return false;
	}

	const ScriptVector *vector = testVector(L, -1);
	if (!vector) {
		luaL_error(L, "field %s must be a vector", name);
	}

	for (int i = 0; i < size; ++i) {
		out[i] = vector->v[i];
	}

	lua_pop(L, 1);
	return true;
}

// "velocity": a speed between minSpeed and maxSpeed, in a direction that strays
// from "direction" by up to the two variances, both in radians
class ParticleVelocityStage : public ParticleStage {
public:
	ParticleVelocityStage(ParticleRecord &record, lua_State *L, int tableIndex) : ParticleStage(record) {
		readPicked(L, tableIndex);
		readField(L, tableIndex, "minSpeed", _minSpeed);
		readField(L, tableIndex, "maxSpeed", _maxSpeed);

		double direction[3] = { 0.0, 1.0, 0.0 };
		readVectorField(L, tableIndex, "direction", direction, 3);

		const double length = sqrt(direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2]);
		if (length > 0.0) {
			direction[0] /= length;
			direction[1] /= length;
			direction[2] /= length;
		}

		const double azimuth = atan2(direction[1], direction[0]);
		const double inclination = acos(CLIP(direction[2], -1.0, 1.0));

		double inclinationVariance = 0.0;
		double azimuthVariance = 0.0;
		readField(L, tableIndex, "inclinationVariance", inclinationVariance);
		readField(L, tableIndex, "azimuthVariance", azimuthVariance);

		_minInclination = inclination - inclinationVariance;
		_maxInclination = inclination + inclinationVariance;
		_minAzimuth = azimuth - azimuthVariance;
		_maxAzimuth = azimuth + azimuthVariance;
	}

protected:
	void initParticles(uint first, uint count) override {
		for (uint i = first; i < first + count; ++i) {
			const double speed = lerp(_minSpeed, _maxSpeed, randomFraction()) / kStepsPerSecond;
			const double inclination = lerp(_minInclination, _maxInclination, randomFraction());
			const double azimuth = lerp(_minAzimuth, _maxAzimuth, randomFraction());

			Particle &particle = _record.getParticle(i);
			particle.velocity[0] = cos(azimuth) * speed * sin(inclination);
			particle.velocity[1] = sin(azimuth) * speed * sin(inclination);
			particle.velocity[2] = cos(inclination) * speed;
		}
	}

private:
	double _minSpeed = 0.0;
	double _maxSpeed = 0.0;
	double _minInclination = 0.0;
	double _maxInclination = 0.0;
	double _minAzimuth = 0.0;
	double _maxAzimuth = 0.0;
};

// "angle" and "scale": a value between min and max
class ParticleRangeStage : public ParticleStage {
public:
	ParticleRangeStage(ParticleRecord &record, lua_State *L, int tableIndex, double defaultValue, bool isAngle) :
			ParticleStage(record), _min(defaultValue), _max(defaultValue), _isAngle(isAngle) {
		readPicked(L, tableIndex);
		readField(L, tableIndex, "min", _min);
		readField(L, tableIndex, "max", _max);
	}

protected:
	void initParticles(uint first, uint count) override {
		for (uint i = first; i < first + count; ++i) {
			const double value = lerp(_min, _max, randomFraction());
			if (_isAngle) {
				_record.getParticle(i).angle = value;
			} else {
				_record.getParticle(i).scale = value;
			}
		}
	}

private:
	double _min;
	double _max;
	bool _isAngle;
};

// "color": the "base" color the image gets multiplied with
class ParticleColorStage : public ParticleStage {
public:
	ParticleColorStage(ParticleRecord &record, lua_State *L, int tableIndex) : ParticleStage(record) {
		readPicked(L, tableIndex);
		if (pushField(L, tableIndex, "base")) {
			_color = checkColor(L, -1);
			lua_pop(L, 1);
		}
	}

protected:
	void initParticles(uint first, uint count) override {
		for (uint i = first; i < first + count; ++i) {
			_record.getParticle(i).color = _color;
		}
	}

private:
	uint32 _color = 0xFFFFFFFF;
};

// "sources": the parts of the image to pick from, as a list of rects
class ParticleSourcesStage : public ParticleStage {
public:
	ParticleSourcesStage(ParticleRecord &record, lua_State *L, int tableIndex) : ParticleStage(record) {
		readPicked(L, tableIndex);

		for (int i = 1; ; ++i) {
			lua_rawgeti(L, tableIndex, i);
			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				break;
			}

			_sources.push_back(checkRect(L, -1));
			lua_pop(L, 1);
		}
	}

protected:
	void initParticles(uint first, uint count) override {
		if (_sources.empty()) {
			return;
		}

		const Graphics::ManagedSurface &image = _record.getImage();
		const double width = MAX<int>(image.w, 1);
		const double height = MAX<int>(image.h, 1);

		for (uint i = first; i < first + count; ++i) {
			const ScriptRect &source = _sources[g_nancy->_randomSource->getRandomNumber(_sources.size() - 1)];

			Particle &particle = _record.getParticle(i);
			particle.left = source.left / width;
			particle.right = source.right / width;
			particle.top = source.top / height;
			particle.bottom = source.bottom / height;
			particle.width = source.width();
			particle.height = source.height();
		}
	}

private:
	Common::Array<ScriptRect> _sources;
};

// "positionPerspective": a place inside the view of the camera of the
// "perspective" projection. x and y go from -1 to 1 across the view, and z from 0
// at the near plane to 1 at the far one. More of the particles end up near.
class ParticlePerspectivePositionStage : public ParticleStage {
public:
	ParticlePerspectivePositionStage(ParticleRecord &record, lua_State *L, int tableIndex) : ParticleStage(record) {
		readPicked(L, tableIndex);

		double min[3] = { 0.0, 0.0, 0.0 };
		double max[3] = { 0.0, 0.0, 0.0 };
		readVectorField(L, tableIndex, "min", min, 3);
		readVectorField(L, tableIndex, "max", max, 3);

		double fovy = 0.0;
		double aspectRatio = 0.0;
		if (!readField(L, tableIndex, "fovy", fovy) || !readField(L, tableIndex, "aspectRatio", aspectRatio) ||
				!readField(L, tableIndex, "zN", _zNear) || !readField(L, tableIndex, "zF", _zFar)) {
			luaL_error(L, "Position perspective stage expects numeric fields fovy, aspectRatio, zN, zF");
		}

		const double tanY = tan(fovy * 0.5);
		const double tanX = tan(atan(aspectRatio * tanY));

		_minX = tanX * min[0];
		_maxX = tanX * max[0];
		_minY = tanY * min[1];
		_maxY = tanY * max[1];
		_minZ = min[2];
		_maxZ = max[2];
	}

protected:
	void initParticles(uint first, uint count) override {
		for (uint i = first; i < first + count; ++i) {
			const double z = lerp(_zNear, _zFar, pow(lerp(_minZ, _maxZ, randomFraction()), 0.3));

			Particle &particle = _record.getParticle(i);
			particle.position[2] = z;
			particle.position[0] = lerp(z * _minX, z * _maxX, randomFraction());
			particle.position[1] = lerp(z * _minY, z * _maxY, randomFraction());
		}
	}

private:
	double _minX, _maxX, _minY, _maxY, _minZ, _maxZ;
	double _zNear = 0.0;
	double _zFar = 0.0;
};

// "fade": the particles fade in or out once they are "start" into their life,
// over "duration" of it
class ParticleFadeStage : public ParticleStage {
public:
	ParticleFadeStage(ParticleRecord &record, lua_State *L, int tableIndex) : ParticleStage(record) {
		readField(L, tableIndex, "start", _start);
		readField(L, tableIndex, "duration", _duration);
		_isFadingIn = readTruth(L, tableIndex, "in");
	}

	void update(uint first, uint count) override {
		const double from = _isFadingIn ? 0.0 : 255.0;
		const double change = _isFadingIn ? 255.0 : -255.0;

		// The oldest particles come first, so the ones after one that hasn't
		// started fading haven't either
		for (uint i = first; i < first + count; ++i) {
			Particle &particle = _record.getParticle(i);
			const double fraction = (particle.life - _start) / _duration;
			if (fraction < 0.0) {
				break;
			}

			if (fraction <= 1.0) {
				const uint32 alpha = (uint32)CLIP<int>((int)floor(fraction * change + from + 0.5), 0, 255);
				particle.color = (particle.color & 0xFFFFFF) | (alpha << 24);
			}
		}
	}

private:
	double _start = 0.0;
	double _duration = 1.0;
	bool _isFadingIn = false;
};

// "drag": slows the particles down by a factor every step
class ParticleDragStage : public ParticleStage {
public:
	ParticleDragStage(ParticleRecord &record, lua_State *L, int tableIndex) : ParticleStage(record) {
		readField(L, tableIndex, "factor", _factor);
	}

	void update(uint first, uint count) override {
		for (uint i = first; i < first + count; ++i) {
			Particle &particle = _record.getParticle(i);
			for (int j = 0; j < 3; ++j) {
				particle.velocity[j] *= _factor;
			}
		}
	}

private:
	double _factor = 0.95;
};

// "force": pulls at the particles with a "vector", in units a second squared
class ParticleForceStage : public ParticleStage {
public:
	ParticleForceStage(ParticleRecord &record, lua_State *L, int tableIndex) : ParticleStage(record) {
		if (readVectorField(L, tableIndex, "vector", _force, 3)) {
			for (int i = 0; i < 3; ++i) {
				_force[i] /= kStepsPerSecond * kStepsPerSecond;
			}
		}
	}

	void update(uint first, uint count) override {
		for (uint i = first; i < first + count; ++i) {
			Particle &particle = _record.getParticle(i);
			for (int j = 0; j < 3; ++j) {
				particle.acceleration[j] += _force[j];
			}
		}
	}

private:
	double _force[3] = { 0.0, -1.0 / (kStepsPerSecond * kStepsPerSecond), 0.0 };
};

// "turbulence": every step, every particle gets pushed by "factor" along one of
// the axes, either way, or not at all
class ParticleTurbulenceStage : public ParticleStage {
public:
	ParticleTurbulenceStage(ParticleRecord &record, lua_State *L, int tableIndex) : ParticleStage(record) {
		readField(L, tableIndex, "factor", _factor);
	}

	void update(uint first, uint count) override {
		for (uint i = first; i < first + count; ++i) {
			const uint push = g_nancy->_randomSource->getRandomNumber(7);
			if (push < 6) {
				_record.getParticle(i).acceleration[push / 2] += (push & 1) ? -_factor : _factor;
			}
		}
	}

private:
	double _factor = 0.95;
};

// TODO: The "position" and "script" stages, which no script uses
static ParticleStage *makeStage(ParticleRecord &record, lua_State *L, int tableIndex) {
	Common::String name;
	if (!readField(L, tableIndex, "name", name)) {
		luaL_error(L, "name must be set to a string at each stage");
	}

	if (name == "positionPerspective") {
		return new ParticlePerspectivePositionStage(record, L, tableIndex);
	} else if (name == "velocity") {
		return new ParticleVelocityStage(record, L, tableIndex);
	} else if (name == "angle") {
		return new ParticleRangeStage(record, L, tableIndex, 0.0, true);
	} else if (name == "scale") {
		return new ParticleRangeStage(record, L, tableIndex, 1.0, false);
	} else if (name == "color") {
		return new ParticleColorStage(record, L, tableIndex);
	} else if (name == "sources") {
		return new ParticleSourcesStage(record, L, tableIndex);
	} else if (name == "fade") {
		return new ParticleFadeStage(record, L, tableIndex);
	} else if (name == "drag") {
		return new ParticleDragStage(record, L, tableIndex);
	} else if (name == "force") {
		return new ParticleForceStage(record, L, tableIndex);
	} else if (name == "turbulence") {
		return new ParticleTurbulenceStage(record, L, tableIndex);
	}

	luaL_error(L, "Stage name '%s' does not match a known stage", name.c_str());
	return nullptr;
}

ParticleRecord::~ParticleRecord() {
	for (ParticleStage *stage : _stages) {
		delete stage;
	}

	delete _canvas;
}

void ParticleRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	Common::String imageName;
	if (!readField(L, tableIndex, "ovl", imageName)) {
		luaL_error(L, "Particle AR requires field 'ovl' to be a string");
	}

	if (!g_nancy->_resource->loadImage(Common::Path(imageName), _image)) {
		luaL_error(L, "Could not load particle image %s", imageName.c_str());
	}

	if (!readField(L, tableIndex, "frequency", _frequency)) {
		luaL_error(L, "Particle AR requires field 'frequency' to be a number");
	}

	if (!readField(L, tableIndex, "life", _life)) {
		luaL_error(L, "Particle AR requires field 'life' to be a number");
	}

	if (pushField(L, tableIndex, "perspective")) {
		_perspective = checkMatrix(L, -1);
		lua_pop(L, 1);
	}

	_particles.resize(kInitialParticles);
	addStage(new ParticleLifeStage(*this));
	addStage(new ParticleForceClearStage(*this));

	if (pushField(L, tableIndex, "stages")) {
		const int stagesIndex = absoluteIndex(L, -1);
		for (int i = 1; ; ++i) {
			lua_rawgeti(L, stagesIndex, i);
			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				break;
			}

			luaL_checktype(L, -1, LUA_TTABLE);
			addStage(makeStage(*this, L, absoluteIndex(L, -1)));
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}

	// The particles can be there before the record is
	double fastForward = 0.0;
	readField(L, tableIndex, "fastForward", fastForward);
	if (fastForward > 0.0) {
		simulate(fastForward);
	}
}

void ParticleRecord::addStage(ParticleStage *stage) {
	_stages.push_back(stage);
	stage->onAdded();
}

void ParticleRecord::run(double sceneTime, double elapsedTime) {
	simulate(elapsedTime);
}

// Steps through the given time, 30 steps a second
void ParticleRecord::simulate(double seconds) {
	_stepTime += seconds;
	const int numSteps = (int)(_stepTime * kStepsPerSecond);
	const int newSteps = numSteps - _numSteps;
	_numSteps = numSteps;

	for (int i = 0; i < newSteps; ++i) {
		step();
	}
}

void ParticleRecord::step() {
	const int numSpawned = _numSpawned;
	_spawnTime += 1.0 / kStepsPerSecond;
	_numSpawned = (int)(_spawnTime * _frequency);

	// The particles that lived long enough go
	while (!_batches.empty() && _batches.front().time + _life <= _spawnTime) {
		dropOldestBatch();
	}

	if (_numSpawned > numSpawned) {
		spawn(_numSpawned - numSpawned);
	}

	runStages(_first, _count, false);

	// Forces change the speeds, and speeds the places
	for (uint i = 0; i < _count; ++i) {
		Particle &particle = _particles[(_first + i) % _particles.size()];
		for (int j = 0; j < 3; ++j) {
			particle.velocity[j] += particle.acceleration[j];
			particle.position[j] += particle.velocity[j];
		}
	}
}

// Runs every stage on particles of the ring, which can wrap around: either on
// new particles, or for a step
void ParticleRecord::runStages(uint first, uint count, bool isSpawn) {
	const uint firstPart = MIN<uint>(count, _particles.size() - first);
	for (ParticleStage *stage : _stages) {
		if (isSpawn) {
			stage->onSpawn(first, firstPart);
			if (count > firstPart) {
				stage->onSpawn(0, count - firstPart);
			}
		} else {
			stage->update(first, firstPart);
			if (count > firstPart) {
				stage->update(0, count - firstPart);
			}
		}
	}
}

void ParticleRecord::spawn(uint count) {
	// There are as many particles as the frequency and the life call for
	while (_count + count > _particles.size()) {
		grow();
	}

	const uint first = (_first + _count) % _particles.size();
	runStages(first, count, true);

	Batch batch;
	batch.time = _spawnTime;
	batch.count = count;
	_batches.push_back(batch);
	_count += count;
}

// Doubles the room for particles. The ones that wrapped around to the start of
// the ring move on past its old end, so that they still follow the others.
void ParticleRecord::grow() {
	const uint oldSize = _particles.size();
	_particles.resize(oldSize * 2);
	for (ParticleStage *stage : _stages) {
		stage->onGrown(oldSize, oldSize);
	}

	const uint numWrapped = _first + _count > oldSize ? _first + _count - oldSize : 0;
	for (uint i = 0; i < numWrapped; ++i) {
		_particles[oldSize + i] = _particles[i];
	}
}

void ParticleRecord::dropOldestBatch() {
	const uint count = MIN(_batches.front().count, _count);
	_count -= count;
	_first = (_first + count) % _particles.size();
	_batches.pop_front();
}

void ParticleRecord::onZChanged() {
	if (_canvas) {
		_canvas->setScriptZ(getZ(), getContext());
	}
}

bool ParticleRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "frequency") {
		lua_pushnumber(L, _frequency);
		return true;
	} else if (key == "count") {
		lua_pushinteger(L, _count);
		return true;
	}

	return false;
}

bool ParticleRecord::setField(const Common::String &key) {
	if (key == "frequency") {
		_frequency = luaL_checknumber(getLua(), 3);
		return true;
	}

	return false;
}

// The particles are drawn on a picture as large as the viewport, which is
// started over every frame
void ParticleRecord::updateGraphics() {
	if (!isActive() || _count == 0) {
		if (_canvas) {
			_canvas->setVisible(false);
		}

		return;
	}

	if (!_canvas) {
		const VIEW *viewport = GetEngineData(VIEW);
		assert(viewport);

		const Common::Rect bounds(viewport->screenPosition.width(), viewport->screenPosition.height());

		_canvas = new RecordGraphic();
		_canvas->init();
		_canvas->_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
		_canvas->moveTo(bounds);
		_canvas->setScriptZ(getZ(), getContext());
		_canvas->registerGraphics();

		// Reading the image is quicker in the format of the picture
		_image.convertToInPlace(_canvas->_drawSurface.format);
	}

	_canvas->_drawSurface.clear(0);

	const ScriptMatrix transform = getTransform();
	for (uint i = 0; i < _count; ++i) {
		drawParticle(_particles[(_first + i) % _particles.size()], transform);
	}

	_canvas->setVisible(true);
	_canvas->setNeedsRedraw(true);
}

// Where a corner of a particle ends up in the viewport. Returns false when it is
// behind the camera.
static bool projectCorner(double x, double y, const ScriptMatrix &matrix, int width, int height, double &outX, double &outY) {
	double point[4];
	for (int i = 0; i < 4; ++i) {
		point[i] = x * matrix.m[0][i] + y * matrix.m[1][i] + matrix.m[3][i];
	}

	if (point[3] <= 0.0) {
		return false;
	}

	outX = (point[0] / point[3] + 1.0) * 0.5 * width;
	outY = (1.0 - point[1] / point[3]) * 0.5 * height;
	return point[2] >= 0.0 && point[2] <= point[3];
}

// A particle is a square of a part of the image, sized, turned and moved into
// place, then seen through the projection
void ParticleRecord::drawParticle(const Particle &particle, const ScriptMatrix &transform) {
	if (particle.width <= 0.0 || particle.height <= 0.0 || (particle.color >> 24) == 0) {
		return;
	}

	ScriptMatrix place;
	const double c = cos(particle.angle);
	const double s = sin(particle.angle);
	const double width = particle.width * particle.scale;
	const double height = particle.height * particle.scale;
	place.m[0][0] = width * c;
	place.m[0][1] = width * s;
	place.m[1][0] = -height * s;
	place.m[1][1] = height * c;
	place.m[3][0] = particle.position[0];
	place.m[3][1] = particle.position[1];
	place.m[3][2] = particle.position[2];

	const ScriptMatrix matrix = place * transform * _perspective;

	Graphics::ManagedSurface &target = _canvas->_drawSurface;

	// The top left, top right and bottom left corners
	double x0, y0, x1, y1, x2, y2;
	if (!projectCorner(-0.5, 0.5, matrix, target.w, target.h, x0, y0) ||
			!projectCorner(0.5, 0.5, matrix, target.w, target.h, x1, y1) ||
			!projectCorner(-0.5, -0.5, matrix, target.w, target.h, x2, y2)) {
		return;
	}

	// The quad stays a parallelogram, as all of it is at the same depth
	const double ax = x1 - x0;
	const double ay = y1 - y0;
	const double bx = x2 - x0;
	const double by = y2 - y0;
	const double determinant = ax * by - ay * bx;
	if (fabs(determinant) < 1e-9) {
		return;
	}

	const double x3 = x1 + bx;
	const double y3 = y1 + by;
	const int minX = MAX<int>(0, (int)floor(MIN(MIN(x0, x1), MIN(x2, x3))));
	const int maxX = MIN<int>(target.w - 1, (int)ceil(MAX(MAX(x0, x1), MAX(x2, x3))));
	const int minY = MAX<int>(0, (int)floor(MIN(MIN(y0, y1), MIN(y2, y3))));
	const int maxY = MIN<int>(target.h - 1, (int)ceil(MAX(MAX(y0, y1), MAX(y2, y3))));

	const Graphics::PixelFormat &format = target.format;
	const uint32 colorA = particle.color >> 24;
	const uint32 colorR = (particle.color >> 16) & 0xFF;
	const uint32 colorG = (particle.color >> 8) & 0xFF;
	const uint32 colorB = particle.color & 0xFF;

	for (int y = minY; y <= maxY; ++y) {
		for (int x = minX; x <= maxX; ++x) {
			// Where the middle of the pixel is on the square, from 0 to 1 across
			const double px = x + 0.5 - x0;
			const double py = y + 0.5 - y0;
			const double across = (px * by - py * bx) / determinant;
			const double down = (ax * py - ay * px) / determinant;
			if (across < 0.0 || across >= 1.0 || down < 0.0 || down >= 1.0) {
				continue;
			}

			const int u = CLIP<int>((int)(lerp(particle.left, particle.right, across) * _image.w), 0, _image.w - 1);
			const int v = CLIP<int>((int)(lerp(particle.top, particle.bottom, down) * _image.h), 0, _image.h - 1);

			uint8 a, r, g, b;
			format.colorToARGB(_image.getPixel(u, v), a, r, g, b);

			const uint32 alpha = a * colorA / 255;
			if (alpha == 0) {
				continue;
			}

			uint8 da, dr, dg, db;
			format.colorToARGB(target.getPixel(x, y), da, dr, dg, db);

			const uint32 sr = r * colorR / 255;
			const uint32 sg = g * colorG / 255;
			const uint32 sb = b * colorB / 255;
			const uint32 outA = alpha + da * (255 - alpha) / 255;
			const uint32 outR = (sr * alpha + dr * (255 - alpha)) / 255;
			const uint32 outG = (sg * alpha + dg * (255 - alpha)) / 255;
			const uint32 outB = (sb * alpha + db * (255 - alpha)) / 255;
			target.setPixel(x, y, format.ARGBToColor(outA, outR, outG, outB));
		}
	}
}

} // End of namespace Script
} // End of namespace Nancy
