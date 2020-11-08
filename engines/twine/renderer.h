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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TWINE_RENDERER_H
#define TWINE_RENDERER_H

#include "common/scummsys.h"

#define POLYGONTYPE_FLAT 0
#define POLYGONTYPE_COPPER 1
#define POLYGONTYPE_BOPPER 2
#define POLYGONTYPE_MARBLE 3
#define POLYGONTYPE_TELE 4
#define POLYGONTYPE_TRAS 5
#define POLYGONTYPE_TRAME 6
#define POLYGONTYPE_GOURAUD 7
#define POLYGONTYPE_DITHER 8

namespace TwinE {
class TwinEEngine;
class Renderer {
private:
	TwinEEngine *_engine;

	struct renderTabEntry {
		int16 depth = 0;
		int16 renderType = 0;
		uint8 *dataPtr = nullptr;
	};

	struct pointTab {
		int16 x = 0;
		int16 y = 0;
		int16 z = 0;
	};

	struct elementEntry {
		int16 firstPoint = 0;  // data1
		int16 numOfPoints = 0; // data2
		int16 basePoint = 0;   // data3
		int16 baseElement = 0; // param
		int16 flag = 0;
		int16 rotateZ = 0;
		int16 rotateY = 0;
		int16 rotateX = 0;
		int32 numOfShades = 0; // field_10
		int32 field_14 = 0;
		int32 field_18 = 0;
		int32 y = 0;
		int32 field_20 = 0;
		int16 field_24 = 0;
	};

	struct lineCoordinates {
		int32 data = 0;
		int16 x1 = 0;
		int16 y1 = 0;
		int16 x2 = 0;
		int16 y2 = 0;
	};

	struct lineData {
		int32 data = 0;
		int16 p1 = 0;
		int16 p2 = 0;
	};

	struct polyHeader {
		uint8 renderType = 0; //FillVertic_AType
		uint8 numOfVertex = 0;
		int16 colorIndex = 0;
	};

	struct polyVertexHeader {
		int16 shadeEntry = 0;
		int16 dataOffset = 0;
	};

	struct computedVertex {
		int16 shadeValue = 0;
		int16 x = 0;
		int16 y = 0;
	};

	struct bodyHeaderStruct {
		int16 bodyFlag = 0;
		int16 unk0 = 0;
		int16 unk1 = 0;
		int16 unk2 = 0;
		int16 unk3 = 0;
		int16 unk4 = 0;
		int16 unk5 = 0;
		int16 offsetToData = 0;
		int8 *ptrToKeyFrame = nullptr;
		int32 keyFrameTime = 0;
	};

	struct vertexData {
		uint8 param = 0;
		int16 x = 0;
		int16 y = 0;
	};

	union packed16 {
		struct {
			uint8 al = 0;
			uint8 ah = 0;
		} bit;
		uint16 temp = 0;
	};

	int32 renderAnimatedModel(uint8 *bodyPtr, renderTabEntry *renderTabEntryPtr);
	void circleFill(int32 x, int32 y, int32 radius, int8 color);
	int32 renderModelElements(int32 numOfPrimitives, uint8 *pointer, renderTabEntry** renderTabEntryPtr);
	void getBaseRotationPosition(int32 x, int32 y, int32 z);
	void getCameraAnglePositions(int32 x, int32 y, int32 z);
	void applyRotation(int32 *targetMatrix, const int32 *currentMatrix);
	void applyPointsRotation(const uint8 *firstPointsPtr, int32 numPoints, pointTab *destPoints, const int32 *rotationMatrix);
	void processRotatedElement(int32 *targetMatrix, const uint8 *pointsPtr, int32 rotZ, int32 rotY, int32 rotX, const elementEntry *elemPtr);
	void applyPointsTranslation(const uint8 *firstPointsPtr, int32 numPoints, pointTab *destPoints, const int32 *translationMatrix);
	void processTranslatedElement(int32 *targetMatrix, const uint8 *pointsPtr, int32 rotX, int32 rotY, int32 rotZ, const elementEntry *elemPtr);
	void translateGroup(int16 ax, int16 bx, int16 cx);

	// ---- variables ----

	int32 baseTransPosX = 0; // setSomething2Var1
	int32 baseTransPosY = 0; // setSomething2Var2
	int32 baseTransPosZ = 0; // setSomething2Var3

	int32 baseRotPosX = 0; // setSomething3Var12
	int32 baseRotPosY = 0; // setSomething3Var14
	int32 baseRotPosZ = 0; // setSomething3Var16

	int32 cameraPosX = 0; // cameraVar1
	int32 cameraPosY = 0; // cameraVar2
	int32 cameraPosZ = 0; // cameraVar3

	// ---

	int32 renderAngleX = 0; // _angleX
	int32 renderAngleY = 0; // _angleY
	int32 renderAngleZ = 0; // _angleZ

	int32 renderX = 0; // _X
	int32 renderY = 0; // _Y
	int32 renderZ = 0; // _Z

	// ---

	int32 baseMatrix[3 * 3] {0};

	int32 matricesTable[30 * 3 * 3 + 1] {0};

	int32 shadeMatrix[3 * 3] {0};
	int32 lightX = 0;
	int32 lightY = 0;
	int32 lightZ = 0;

	pointTab computedPoints[800]; // _projectedPointTable
	pointTab flattenPoints[800];  // _flattenPointTable
	int16 shadeTable[500] {0};

	renderTabEntry renderTab[1000];
	renderTabEntry renderTabSorted[1000];
	uint8 renderTab7[10000] {0};

	int16 polyTab[960] {0};
	int16 polyTab2[960] {0};
	// end render polygon vars

	bool isUsingOrhoProjection = false;

	void computePolygons(int16 polyRenderType, vertexData *vertices, int32 numVertices, int &vleft, int &vright, int &vtop, int &vbottom);
	void renderPolygons(int32 renderType, vertexData *vertices, int32 numVertices, int32 color, int vleft, int vright, int vtop, int vbottom);

public:
	Renderer(TwinEEngine *engine) : _engine(engine) {}

	int16 projPosXScreen = 0; // fullRedrawVar1
	int16 projPosYScreen = 0; // fullRedrawVar2
	int16 projPosZScreen = 0; // fullRedrawVar3
	int16 projPosX = 0;
	int16 projPosY = 0;
	int16 projPosZ = 0;

	int32 orthoProjX = 0; // setSomethingVar1
	int32 orthoProjY = 0; // setSomethingVar2
	int32 orthoProjZ = 0; // setSomethingVar2

	int32 destX = 0;
	int32 destY = 0;
	int32 destZ = 0;

	const int16 *shadeAngleTab3 = nullptr; // tab3

	int32 numOfVertex = 0;
	int16 vertexCoordinates[193] {0};

	void setLightVector(int32 angleX, int32 angleY, int32 angleZ);

	void prepareIsoModel(uint8 *bodyPtr); // loadGfxSub
	void renderPolygons(int32 polyRenderType, int32 color);

	int32 projectPositionOnScreen(int32 cX, int32 cY, int32 cZ);
	void setCameraPosition(int32 x, int32 y, int32 cX, int32 cY, int32 cZ);
	void setCameraAngle(int32 transPosX, int32 transPosY, int32 transPosZ, int32 rotPosX, int32 rotPosY, int32 rotPosZ, int32 param6);
	void setBaseTranslation(int32 x, int32 y, int32 z);
	void setBaseRotation(int32 x, int32 y, int32 z);
	void setOrthoProjection(int32 x, int32 y, int32 z);

	int32 renderIsoModel(int32 x, int32 y, int32 z, int32 angleX, int32 angleY, int32 angleZ, uint8 *bodyPtr);

	void copyActorInternAnim(const uint8 *bodyPtrSrc, uint8 *bodyPtrDest);

	void renderBehaviourModel(int32 boxLeft, int32 boxTop, int32 boxRight, int32 boxBottom, int32 Y, int32 angle, uint8 *entityPtr);

	void renderInventoryItem(int32 X, int32 Y, uint8 *itemBodyPtr, int32 angle, int32 param);
};

} // namespace TwinE

#endif
