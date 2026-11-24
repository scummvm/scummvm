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
	// A place a scene sets aside for whoever is to stand there.  The scene
	// gives the spot and how the figure standing on it is to be drawn; who
	// that is comes from the game itself.
	struct Marker {
		uint16 x;
		uint16 y;
		byte   palOffset;
		uint16 flags;
	};

	Room(DuneEngine *e, Resource r);

	int count() { return _count; }

	const Common::Array<Marker> &markers() const { return _markers; }

	void add_gradient_section(int x0, int y0, int x1, int y1, Common::Array<uint16> &xs, int &row);
	// void add_relative_gradient_section(int x0, int y0, int x1, int y1);

	// Gameplay paints figures at their SAL commands, so later geometry can
	// occlude them. Bank/scene diagnostics can still collect empty markers.
	void draw(byte *framebuffer, int index, bool drawPeople = false);

private:
	Common::Array<Marker> _markers;
};

} // End of namespace Dune

#endif
