#include "objectstate.h"

ObjectState::ObjectState(int setupID, ObjectState::Position pos,
			 const char *bitmap, const char *zbitmap,
			 bool unk1, bool unk2) :
	setupID_(setupID), pos_(pos), unk1_(unk1), unk2_(unk2) {
	bitmap_ = ResourceLoader::instance()->loadBitmap(bitmap);
	if (zbitmap)
		zbitmap_ = ResourceLoader::instance()->loadBitmap(zbitmap);
}
