#include "objectstate.h"

ObjectState::ObjectState(int setupID, ObjectState::Position pos, const char *bitmap, const char *zbitmap, bool visible) :
		_setupID(setupID), _pos(pos) {
	_bitmap = g_resourceloader->loadBitmap(bitmap);
	if (zbitmap)
		_zbitmap = g_resourceloader->loadBitmap(zbitmap);

	int initialImage = 0;
	if (visible)
		initialImage = 1;
	_bitmap->setNumber(initialImage);
	if (_zbitmap)
		_zbitmap->setNumber(initialImage);
}
