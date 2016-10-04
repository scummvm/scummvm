#include "bladerunner/obstacles.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

Obstacles::Obstacles(BladeRunnerEngine* vm) {
	_vm = vm;
	_polygons  = new ObstaclesPolygon[50];
	_polygons2 = new ObstaclesPolygon[50];
	_unknown = new int[50];
	clear();
}

Obstacles::~Obstacles() {
	delete[] _unknown;
	delete[] _polygons2;
	delete[] _polygons;
}

void Obstacles::clear() {
	for(int i = 0; i < 50; i++) {
		_polygons[i]._isPresent = false;
		_polygons[i]._verticesCount = 0;
		for(int j = 0; j < 160; j++) {
			_polygons[i]._vertices[j].x = 0.0f;
			_polygons[i]._vertices[j].y = 0.0f;
		}
	}
	_count = 0;
	_processed = false;
}

void Obstacles::add(float x0, float z0, float x1, float z1) {

}

bool Obstacles::find(const Vector3 &from, const Vector3 &to, Vector3 *next) {
	//TODO
	*next = to;
	return true;
}

void Obstacles::process() {
}
} // End of namespace BladeRunner
