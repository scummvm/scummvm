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

#ifndef NANCY_SCRIPT_PARTICLES_H
#define NANCY_SCRIPT_PARTICLES_H

#include "common/array.h"
#include "common/list.h"

#include "graphics/managed_surface.h"

#include "engines/nancy/script/record.h"
#include "engines/nancy/script/records.h"

namespace Nancy {
namespace Script {

class ParticleStage;

// One of the particles of a ParticleRecord. Speeds and forces are per step of
// the simulation, which takes 30 steps a second.
struct Particle {
	double position[3] = { 0.0, 0.0, 0.0 };
	double velocity[3] = { 0.0, 0.0, 0.0 };
	double acceleration[3] = { 0.0, 0.0, 0.0 };
	double life = 0.0;			// from 0 when made to 1 when gone
	double angle = 0.0;
	double scale = 1.0;
	double width = 0.0;			// of the part of the image it shows
	double height = 0.0;
	double left = 0.0;			// the part of the image, from 0 to 1
	double right = 0.0;
	double top = 0.0;
	double bottom = 0.0;
	uint32 color = 0;			// 0xAARRGGBB
};

// Nancy21's falling snow. A Particle record keeps making particles out of parts
// of an image, which it moves along and draws until their life is over:
//
//   AR:Particle({ ovl = "Flakes_OVL", frequency = 50, life = 6, perspective = ...,
//                 stages = { { name = "velocity", minSpeed = 300, ... }, ... } })
//
// The stages decide what happens to the particles: where they are made, how
// fast they go, what pulls at them, how they fade. A stage with "init" set picks
// what it gives every particle once, when the record is made, instead of every
// time a particle is made. The particles go through the "perspective"
// projection, so they get smaller further off.
class ParticleRecord : public Record {
public:
	ParticleRecord(SceneContext &context) : Record(context, "ND.ParticleAR") {}
	~ParticleRecord() override;

	void updateGraphics() override;

	// For the stages
	Particle &getParticle(uint index) { return _particles[index]; }
	uint getMaxParticles() const { return _particles.size(); }
	double getLife() const { return _life; }
	const Graphics::ManagedSurface &getImage() const { return _image; }

protected:
	void readTable(int tableIndex) override;
	void run(double sceneTime, double elapsedTime) override;
	void onZChanged() override;
	bool hasTransform() const override { return true; }
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	// The particles that were made at one time, which go at the same time
	struct Batch {
		double time;
		uint count;
	};

	void addStage(ParticleStage *stage);
	void simulate(double seconds);
	void step();
	void spawn(uint count);
	void grow();
	void dropOldestBatch();
	void runStages(uint first, uint count, bool isSpawn);
	void drawParticle(const Particle &particle, const ScriptMatrix &transform);

	Graphics::ManagedSurface _image;
	Common::Array<Particle> _particles;	// used as a ring, which grows when full
	uint _first = 0;
	uint _count = 0;
	Common::List<Batch> _batches;
	Common::Array<ParticleStage *> _stages;

	double _frequency = 0.0;	// particles a second
	double _life = 0.0;			// seconds
	ScriptMatrix _perspective;

	double _stepTime = 0.0;
	int _numSteps = 0;
	double _spawnTime = 0.0;
	int _numSpawned = 0;

	RecordGraphic *_canvas = nullptr;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_PARTICLES_H
