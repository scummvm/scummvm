#include "objectstate.h"

ObjectState::ObjectState(int setupID, ObjectState::Position pos,
			 const char *bitmap, const char *zbitmap,
			 bool visible) :
	setupID_(setupID), pos_(pos) {
	bitmap_ = ResourceLoader::instance()->loadBitmap(bitmap);
	if (zbitmap)
		zbitmap_ = ResourceLoader::instance()->loadBitmap(zbitmap);

	int initialImage = 0;
	if (visible)
		initialImage = 1;
	bitmap_->setNumber(initialImage);
	if (zbitmap_)
		zbitmap_->setNumber(initialImage);
}
