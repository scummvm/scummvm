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

	ObjectState(int setupID, ObjectState::Position pos,
		    const char *bitmap, const char *zbitmap,
		    bool unk1, bool unk2);

	int setupID() const { return setupID_; }
	Position pos() const { return pos_; }
	const char *bitmapFilename() const {
		return bitmap_->filename();
	}

	void draw() {
		bitmap_->draw();
		if (zbitmap_)
			zbitmap_->draw();
	}

	private:
	int setupID_;
	Position pos_;
	ResPtr<Bitmap> bitmap_, zbitmap_;
	bool unk1_, unk2_;
};


#endif
