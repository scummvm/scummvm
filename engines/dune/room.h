#ifndef DUNE_ROOM_H
#define DUNE_ROOM_H

#include "dune/archive.h"

#include "common/array.h"
#include "common/memstream.h"

namespace Dune {

class DuneEngine;

class Room {
	DuneEngine           *_engine;
	Resource              _r;
	int                   _count;
	Common::Array<uint16> _offsets;

	Common::Array<uint16> _polygon_side_down;
	Common::Array<uint16> _polygon_side_up;

public:
	Room(DuneEngine *e, Resource r);

	int count() { return _count; }

	void add_gradient_section(int x0, int y0, int x1, int y1, Common::Array<uint16> &xs);
	// void add_relative_gradient_section(int x0, int y0, int x1, int y1);

	void draw(byte *framebuffer, int index);
};

} // End of namespace Dune

#endif
