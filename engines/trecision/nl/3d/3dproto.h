/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * aLONG with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

void warning(const char *format, ...);

namespace Trecision {

int actionInRoom(int curA);
void actorDoAction(int whatAction);
void actorStop();
int actorDoNextFrame();
void setPosition(int num);
void goToPosition(int num);
void lookAt(float x, float z);
float sinCosAngle(float sinus, float cosinus);
void createTextureMapping(int16 mat);
void textureTriangle(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2, int32 x3, int32 y3, int32 z3, int32 c3, int32 tx3, int32 ty3, STexture *t);
void textureScanEdge(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2);
void shadowTriangle(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 cv, int32 zv);
void shadowScanEdge(int32 x1, int32 y1, int32 x2, int32 y2);
void gouraudTriangle(int32 x1, int32 y1, int32 z1, int32 c1, int32 x2, int32 y2, int32 z2, int32 c2, int32 x3, int32 y3, int32 z3, int32 c3, uint32 *mat);
void gouraudScanEdge(int32 x1, int32 y1, int32 z1, int32 c1, int32 x2, int32 y2, int32 z2, int32 c2);
void init3DRoom(int16 dx, uint16 *destBuffer, int16 *zBuffer);
void setClipping(int16 x1, int16 y1, int16 x2, int16 y2);
void setZBufferRegion(int16 sx, int16 sy, int16 dx);
signed char clockWise(int16 x1, int16 y1, int16 x2, int16 y2, int16 x3, int16 y3);
void drawCharacter(uint8 flag);
int read3D(const char *c);
void findPath();
void findShortPath();
float evalPath(int a, float destX, float destZ, int nearP);
void buildFramelist();
int nextStep();
void displayPath();
int findAttachedPanel(int srcP, int destP);
int pathCompare(const void *arg1, const void *arg2);
void sortPath();
float distF(float x1, float y1, float x2, float y2);
float dist3D(float x1, float y1, float z1, float x2, float y2, float z2);
void putPix(int x, int y, uint16 c);
void cross3D(float x, float y, float z, uint16 c);
void whereIs(int px, int py);
void pointOut();
void putLine(int x0, int y0, int x1, int y1, uint16 c);
void viewPanel(SPan *p);
void pointProject(float x, float y, float z);
void invPointProject(int x, int y);
int intersectLinePanel(SPan *p, float x, float y, float z);
int intersectLineFloor(float x, float y, float z);
int intersectLineLine(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd);
void initSortPan();
int panCompare(const void *arg1, const void *arg2);
void sortPanel();
void actorOrder();
char waitKey();

} // End of namespace Trecision
