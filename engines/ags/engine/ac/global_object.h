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

#ifndef AGS_ENGINE_AC_GLOBALOBJECT_H
#define AGS_ENGINE_AC_GLOBALOBJECT_H

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
}
}
using namespace AGS; // FIXME later

// TODO: merge with other Rect declared in bitmap unit
struct _Rect {
	int x1, y1, x2, y2;
};

// Get object at the given screen coordinates
int  GetObjectIDAtScreen(int xx, int yy);
// Get object at the given room coordinates
int  GetObjectIDAtRoom(int roomx, int roomy);
void SetObjectTint(int obj, int red, int green, int blue, int opacity, int luminance);
void RemoveObjectTint(int obj);
void SetObjectView(int obn, int vii);
// Assigns given object to the view's frame, and activates frame (plays linked sound, etc)
void SetObjectFrame(int obn, int viw, int lop, int fra);
// Assigns given object to the view's frame
bool SetObjectFrameSimple(int obn, int viw, int lop, int fra);
// pass trans=0 for fully solid, trans=100 for fully transparent
void SetObjectTransparency(int obn, int trans);
void SetObjectBaseline(int obn, int basel);
int  GetObjectBaseline(int obn);
void AnimateObject6(int obn, int loopn, int spdd, int rept, int direction, int blocking);
void AnimateObject4(int obn, int loopn, int spdd, int rept);
void AnimateObjectImpl(int obn, int loopn, int spdd, int rept, int direction, int blocking, int sframe, int volume = 100);
void MergeObject(int obn);
void StopObjectMoving(int objj);
void ObjectOff(int obn);
void ObjectOn(int obn);
int  IsObjectOn(int objj);
void SetObjectGraphic(int obn, int slott);
int  GetObjectGraphic(int obn);
int  GetObjectX(int objj);
int  GetObjectY(int objj);
int  IsObjectAnimating(int objj);
int  IsObjectMoving(int objj);
void SetObjectPosition(int objj, int tox, int toy);
void GetObjectName(int obj, char *buffer);
void MoveObject(int objj, int xx, int yy, int spp);
void MoveObjectDirect(int objj, int xx, int yy, int spp);
void SetObjectClickable(int cha, int clik);
void SetObjectIgnoreWalkbehinds(int cha, int clik);
void RunObjectInteraction(int aa, int mood);
int  AreObjectsColliding(int obj1, int obj2);
int  GetThingRect(int thing, _Rect *rect);
int  AreThingsOverlapping(int thing1, int thing2);

int  GetObjectProperty(int hss, const char *property);
void GetObjectPropertyText(int item, const char *property, char *bufer);

Shared::Bitmap *GetObjectImage(int obj, bool *is_original = nullptr);

} // namespace AGS3

#endif
