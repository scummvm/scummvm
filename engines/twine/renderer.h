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
#include "common/rect.h"

#define POLYGONTYPE_FLAT 0
#define POLYGONTYPE_COPPER 1
#define POLYGONTYPE_BOPPER 2
#define POLYGONTYPE_MARBLE 3
#define POLYGONTYPE_TELE 4
#define POLYGONTYPE_TRAS 5
#define POLYGONTYPE_TRAME 6
#define POLYGONTYPE_GOURAUD 7
#define POLYGONTYPE_DITHER 8

namespace Common {
class MemoryReadStream;
}

namespace TwinE {

class TwinEEngine;

struct Vertex {
	int16 colorIndex = 0;
	int16 x = 0;
	int16 y = 0;
};

struct CmdRenderPolygon {
	uint8 renderType = 0;
	uint8 numVertices = 0;
	int16 colorIndex = 0;
	// followed by Vertex array
};

struct Matrix {
	int32 row1[3] {0, 0, 0};
	int32 row2[3] {0, 0, 0};
	int32 row3[3] {0, 0, 0};
};

class Renderer {
private:
	TwinEEngine *_engine;

	struct RenderCommand {
		int16 depth = 0;
		int16 renderType = 0;
		uint8 *dataPtr = nullptr;
	};

	struct CmdRenderLine {
		uint8 colorIndex = 0;
		uint8 unk1 = 0;
		uint8 unk2 = 0;
		uint8 unk3 = 0;
		int16 x1 = 0;
		int16 y1 = 0;
		int16 x2 = 0;
		int16 y2 = 0;
	};

	struct CmdRenderSphere {
		int8 colorIndex = 0;
		int16 x = 0;
		int16 y = 0;
		int16 radius = 0;
	};

	#include "common/pack-start.h"
	struct pointTab {
		int16 x = 0;
		int16 y = 0;
		int16 z = 0;
	};
	#include "common/pack-end.h"
	static_assert(sizeof(pointTab) == 6, "Unexpected pointTab size");

	#include "common/pack-start.h"
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
	#include "common/pack-end.h"
	static_assert(sizeof(elementEntry) == 38, "Unexpected elementEntry size");

	struct lineData {
		uint8 colorIndex = 0;
		uint8 unk1 = 0;
		uint8 unk2 = 0;
		uint8 unk3 = 0;
		int16 firstPointOffset = 0;  /**< byte offsets */
		int16 secondPointOffset = 0; /**< byte offsets */
	};

	struct polyVertexHeader {
		int16 shadeEntry = 0;
		int16 dataOffset = 0;
	};

	struct bodyHeaderStruct {
		int16 bodyFlag = 0;
		int16 minsx = 0;
		int16 maxsx = 0;
		int16 minsy = 0;
		int16 maxsy = 0;
		int16 minsz = 0;
		int16 maxsz = 0;
		int16 offsetToData = 0;
		int8 *ptrToKeyFrame = nullptr;
		int32 keyFrameTime = 0;
	};

	struct ModelData {
		pointTab computedPoints[800];
		pointTab flattenPoints[800];
		int16 shadeTable[500] {0};
	};

	ModelData _modelData;

	int32 renderAnimatedModel(ModelData *modelData, uint8 *bodyPtr, RenderCommand *renderCmds);
	void circleFill(int32 x, int32 y, int32 radius, uint8 color);
	int32 renderModelElements(int32 numOfPrimitives, uint8 *pointer, RenderCommand** renderCmds, ModelData *modelData);
	void getBaseRotationPosition(int32 x, int32 y, int32 z);
	void getCameraAnglePositions(int32 x, int32 y, int32 z);
	void applyRotation(Matrix *targetMatrix, const Matrix *currentMatrix);
	void applyPointsRotation(const pointTab *pointsPtr, int32 numPoints, pointTab *destPoints, const Matrix *rotationMatrix);
	void processRotatedElement(Matrix *targetMatrix, const uint8 *pointsPtr, int32 rotZ, int32 rotY, int32 rotX, const elementEntry *elemPtr, ModelData *modelData);
	void applyPointsTranslation(const pointTab *pointsPtr, int32 numPoints, pointTab *destPoints, const Matrix *translationMatrix);
	void processTranslatedElement(Matrix *targetMatrix, const uint8 *pointsPtr, int32 rotX, int32 rotY, int32 rotZ, const elementEntry *elemPtr, ModelData *modelData);
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

	Matrix baseMatrix;
	Matrix matricesTable[30 + 1];
	Matrix shadeMatrix;
	int32 lightX = 0;
	int32 lightY = 0;
	int32 lightZ = 0;

	RenderCommand _renderCmds[1000];
	RenderCommand _renderCmdsSortedByDepth[1000];
	uint8 renderCoordinatesBuffer[10000] {0};

	int16 polyTab[960] {0};
	int16 polyTab2[960] {0};
	// end render polygon vars

	bool isUsingOrhoProjection = false;

	void renderPolygonsCopper(uint8 *out, int vtop, int32 vsize, int32 color) const;
	void renderPolygonsBopper(uint8 *out, int vtop, int32 vsize, int32 color) const;
	void renderPolygonsFlat(uint8 *out, int vtop, int32 vsize, int32 color) const;
	void renderPolygonsTele(uint8 *out, int vtop, int32 vsize, int32 color) const;
	void renderPolygonsTras(uint8 *out, int vtop, int32 vsize, int32 color) const;
	void renderPolygonTrame(uint8 *out, int vtop, int32 vsize, int32 color) const;
	void renderPolygonsGouraud(uint8 *out, int vtop, int32 vsize, int32 color) const;
	void renderPolygonsDither(uint8 *out, int vtop, int32 vsize, int32 color) const;
	void renderPolygonsMarble(uint8 *out, int vtop, int32 vsize, int32 color) const;

	void computePolygons(int16 polyRenderType, Vertex *vertices, int32 numVertices, int &vleft, int &vright, int &vtop, int &vbottom);

	const RenderCommand *depthSortRenderCommands(int32 numOfPrimitives);
	uint8* preparePolygons(Common::MemoryReadStream &stream, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);
	uint8* prepareSpheres(Common::MemoryReadStream &stream, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);
	uint8* prepareLines(Common::MemoryReadStream &stream, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);

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

	int16 vertexCoordinates[193] {0};

	void setLightVector(int32 angleX, int32 angleY, int32 angleZ);

	void prepareIsoModel(uint8 *bodyPtr); // loadGfxSub
	void renderPolygons(const CmdRenderPolygon &polygon, Vertex *vertices);

	int32 projectPositionOnScreen(int32 cX, int32 cY, int32 cZ);
	void setCameraPosition(int32 x, int32 y, int32 cX, int32 cY, int32 cZ);
	void setCameraAngle(int32 transPosX, int32 transPosY, int32 transPosZ, int32 rotPosX, int32 rotPosY, int32 rotPosZ, int32 param6);
	void setBaseTranslation(int32 x, int32 y, int32 z);
	void setBaseRotation(int32 x, int32 y, int32 z);
	void setOrthoProjection(int32 x, int32 y, int32 z);

	int32 renderIsoModel(int32 x, int32 y, int32 z, int32 angleX, int32 angleY, int32 angleZ, uint8 *bodyPtr);

	void copyActorInternAnim(const uint8 *bodyPtrSrc, uint8 *bodyPtrDest);

	void renderBehaviourModel(int32 boxLeft, int32 boxTop, int32 boxRight, int32 boxBottom, int32 y, int32 angle, uint8 *entityPtr);
	void renderBehaviourModel(const Common::Rect &rect, int32 y, int32 angle, uint8 *entityPtr);

	void renderInventoryItem(int32 x, int32 y, uint8 *itemBodyPtr, int32 angle, int32 param);
};

} // namespace TwinE

#endif
