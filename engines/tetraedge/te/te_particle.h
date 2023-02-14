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

#ifndef TETRAEDGE_TE_TE_PARTICLE_H
#define TETRAEDGE_TE_TE_PARTICLE_H

#include "common/str.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_real_timer.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_3d_texture.h"
#include "tetraedge/te/te_scene.h"

namespace Tetraedge {

class TeParticle : public TeReferencesCounter {
public:
	class TeElement : public TeReferencesCounter {};

	TeParticle(TeScene *scene);
	~TeParticle();

	void setName(const Common::String &name) { _name = name; }
	bool loadTexture(const Common::String &name);
	void setPosition(const TeVector3f32 &pos) { _position = pos; }
	void setDirection(const TeVector3f32 &dir) { _direction = dir; }
	void setSize(float size) { _size = size; }
	void setVolumeSize(const TeVector3f32 &size) { _volumeSize = size; }
	void setStartColor(const TeColor &col) { _startColor = col; }
	void setEndColor(const TeColor &col) { _endColor = col; }
	void setColorTime(int time) { _colorTime = time; }
	void setTime(int time) { _time = time; }
	void setPeriod(int period) { _period = period; }
	void setParticlePerPeriod(int val) { _particlePerPeriod = val; }
	void setEnabled(bool enabled) { _enabled = enabled; }
	void setStartLoop(int startloop) { _startLoop = startloop; }
	void setGravity(float gravity) { _gravity = gravity; }
	void setRandomDir(bool val) { _randomDir = val; }
	void setOrientation(const TeVector3f32 &orientation) { _orientation = orientation; }

	static int getIndex(const Common::String &name);
	static TeParticle *getIndexedParticle(int idx);
	static void cleanup();

private:
	Common::Array<TeIntrusivePtr<TeElement>> _elements;
	TeScene *_scene;
	TeRealTimer _realTimer;
	Common::String _name;
	TeIntrusivePtr<Te3DTexture> _texture;
	TeVector3f32 _position;
	TeVector3f32 _direction;
	float _size;
	TeVector3f32 _volumeSize;
	TeColor _startColor;
	TeColor _endColor;
	int	_colorTime;
	int	_time;
	int	_period;
	int	_particlePerPeriod;
	bool _enabled;
	int _startLoop;
	float _gravity;
	bool _randomDir;
	TeVector3f32 _orientation;
	
	static Common::Array<TeParticle *> *indexedParticles();
	static Common::Array<TeParticle *> *_indexedParticles;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_PARTICLE_H
