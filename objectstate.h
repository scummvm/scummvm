#ifndef OSTATE_H
#define OSTATE_H

#include "vector3d.h"
#include "resource.h"
#include "bitmap.h"
#include <string>
#include <list>


class ObjectState {
	public:
	enum Position {
		OBJSTATE_UNDERLAY = 1,
		OBJSTATE_OVERLAY = 2,
		OBJSTATE_STATE = 3
	};

	ObjectState(int setupID, ObjectState::Position pos, const char *bitmap, const char *zbitmap, bool visible);
	int setupID() const { return _setupID; }
	Position pos() const { return _pos; }
	const char *bitmapFilename() const {
		return _bitmap->filename();
	}

	void setNumber(int val) {
		_bitmap->setNumber(val);
		if (_zbitmap)
			_zbitmap->setNumber(val);
	}
	void draw() {
		_bitmap->draw();
		if (_zbitmap)
			_zbitmap->draw();
	}

	private:
	int _setupID;
	Position _pos;
	ResPtr<Bitmap> _bitmap, _zbitmap;
};

#endif
