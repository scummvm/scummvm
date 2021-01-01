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

#include "common/endian.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "twine/parser/body.h"
#include "twine/twine.h"

#define POLYGONTYPE_FLAT 0
#define POLYGONTYPE_COPPER 1
#define POLYGONTYPE_BOPPER 2
#define POLYGONTYPE_MARBLE 3
#define POLYGONTYPE_TELE 4
#define POLYGONTYPE_TRAS 5
#define POLYGONTYPE_TRAME 6
#define POLYGONTYPE_GOURAUD 7
#define POLYGONTYPE_DITHER 8

#define POLYTABSIZE (SCREEN_HEIGHT * 2)

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
	int32 row1[3]{0, 0, 0};
	int32 row2[3]{0, 0, 0};
	int32 row3[3]{0, 0, 0};
};

inline Matrix operator*(const Matrix &matrix, const Vec3 &vec) {
	Matrix out;
	out.row1[0] = matrix.row1[0] * vec.x;
	out.row1[1] = matrix.row1[1] * vec.x;
	out.row1[2] = matrix.row1[2] * vec.x;

	out.row2[0] = matrix.row2[0] * vec.y;
	out.row2[1] = matrix.row2[1] * vec.y;
	out.row2[2] = matrix.row2[2] * vec.y;

	out.row3[0] = matrix.row3[0] * vec.z;
	out.row3[1] = matrix.row3[1] * vec.z;
	out.row3[2] = matrix.row3[2] * vec.z;
	return out;
}

struct Model {
	struct BodyFlags {
		uint16 unk1 : 1;            // 1 << 0
		uint16 animated : 1;        // 1 << 1
		uint16 unk3 : 1;            // 1 << 2
		uint16 unk4 : 1;            // 1 << 3
		uint16 unk5 : 1;            // 1 << 4
		uint16 unk6 : 1;            // 1 << 5
		uint16 unk7 : 1;            // 1 << 6
		uint16 alreadyPrepared : 1; // 1 << 7
		uint16 unk9 : 1;            // 1 << 8
		uint16 unk10 : 1;           // 1 << 9
		uint16 unk11 : 1;           // 1 << 10
		uint16 unk12 : 1;           // 1 << 11
		uint16 unk13 : 1;           // 1 << 12
		uint16 unk14 : 1;           // 1 << 13
		uint16 unk15 : 1;           // 1 << 14
		uint16 unk16 : 1;           // 1 << 15
	} bodyFlag;
	int16 minsx = 0;
	int16 maxsx = 0;
	int16 minsy = 0;
	int16 maxsy = 0;
	int16 minsz = 0;
	int16 maxsz = 0;
	int16 offsetToData = 0;

	static inline bool isAnimated(const uint8 *bodyPtr) {
		const int16 bodyHeader = READ_LE_INT16(bodyPtr);
		return (bodyHeader & 2) != 0;
	}

	static inline bool isAnimated(const BodyData &bodyPtr) {
		return bodyPtr.isAnimated();
	}

	static uint8 *getData(uint8 *bodyPtr) {
		return bodyPtr + 0x1A;
	}

	static const uint8 *getData(const uint8 *bodyPtr) {
		return bodyPtr + 0x1A;
	}

	static const uint8 *getVerticesBaseData(const uint8 *bodyPtr) {
		return getData(bodyPtr) + 2;
	}

	static const Common::Array<BodyVertex> &getVerticesBaseData(const BodyData &bodyPtr) {
		return bodyPtr.getVertices();
	}

	static uint8 *getBonesData(uint8 *bodyPtr) {
		uint8 *verticesBase = getData(bodyPtr);
		const int16 numVertices = READ_LE_INT16(verticesBase);
		return verticesBase + 2 + numVertices * 6;
	}

	static const uint8 *getBonesData(const uint8 *bodyPtr) {
		const uint8 *verticesBase = getData(bodyPtr);
		const int16 numVertices = READ_LE_INT16(verticesBase);
		return verticesBase + 2 + numVertices * 6;
	}

	static const uint8 *getBonesStateData(const uint8 *bodyPtr, int boneIdx) {
		return getBonesBaseData(bodyPtr) + 8 + (boneIdx * 38);
	}

	static uint8 *getBonesStateData(uint8 *bodyPtr, int boneIdx) {
		return getBonesBaseData(bodyPtr) + 8 + (boneIdx * 38);
	}

	static BoneFrame *getBonesStateData(BodyData &bodyPtr, int boneIdx) {
		return bodyPtr.getBoneState(boneIdx);
	}

	static const BoneFrame *getBonesStateData(const BodyData &bodyPtr, int boneIdx) {
		return bodyPtr.getBoneState(boneIdx);
	}

	static uint8 *getBonesBaseData(uint8 *bodyPtr) {
		return getBonesData(bodyPtr) + 2;
	}

	static const uint8 *getBonesBaseData(const uint8 *bodyPtr, int boneIdx = 0) {
		return getBonesData(bodyPtr) + 2 + (boneIdx * 38);
	}

	static const BoneFrame *getBonesBaseData(const BodyData &bodyPtr, int boneIdx = 0) {
		return bodyPtr.getBoneState(boneIdx);
	}

	static int16 getNumBones(const uint8 *bodyPtr) {
		const uint8 *verticesBase = getData(bodyPtr);
		const int16 numVertices = READ_LE_INT16(verticesBase);
		const uint8 *bonesBase = verticesBase + 2 + numVertices * 6;
		return READ_LE_INT16(bonesBase);
	}

	static int16 getNumBones(const BodyData &bodyPtr) {
		return bodyPtr.getNumBones();
	}

	static int16 getNumVertices(const uint8 *bodyPtr) {
		const uint8 *verticesBase = getData(bodyPtr);
		return READ_LE_INT16(verticesBase);
	}

	static int16 getNumVertices(const BodyData &bodyPtr) {
		return bodyPtr.getNumVertices();
	}

	static uint8 *getShadesData(uint8 *bodyPtr) {
		uint8 *bonesBase = getBonesBaseData(bodyPtr);
		const int16 numBones = getNumBones(bodyPtr);
		return bonesBase + numBones * 38;
	}

	static const uint8 *getShadesBaseData(const uint8 *bodyPtr, int16 shadeIdx = 0) {
		return getShadesData(bodyPtr) + 2 + (shadeIdx * 8);
	}

	static const BodyShade *getShadesBaseData(const BodyData &bodyPtr, int16 shadeIdx = 0) {
		return bodyPtr.getShade(shadeIdx);
	}

	static const uint8 *getShadesData(const uint8 *bodyPtr) {
		const uint8 *bonesBase = getBonesBaseData(bodyPtr);
		const int16 numBones = getNumBones(bodyPtr);
		return bonesBase + numBones * 38;
	}

	static int16 getNumShades(const uint8 *bodyPtr) {
		const uint8 *shadesBase = getShadesData(bodyPtr);
		return READ_LE_INT16(shadesBase);
	}

	static int16 getNumShades(const BodyData &bodyPtr) {
		return bodyPtr.getShades().size();
	}

	static int16 getNumShadesBone(const uint8 *bodyPtr, int boneIdx) {
		const uint8 *bonesBase = getBonesBaseData(bodyPtr);
		return READ_LE_INT16(bonesBase + (boneIdx * 38) + 18);
	}

	static int16 getNumShadesBone(const BodyData &bodyPtr, int boneIdx) {
		return bodyPtr.getBone(boneIdx)->numOfShades;
	}

	static const uint8 *getPolygonData(const uint8 *bodyPtr) {
		const uint8 *shades = getShadesBaseData(bodyPtr);
		const int16 numShades = getNumShades(bodyPtr);
		if (numShades <= 0) {
			return shades;
		}
		const int16 bones = getNumBones(bodyPtr);
		for (int16 boneIdx = 0; boneIdx < bones; ++boneIdx) {
			int16 numOfShades = Model::getNumShadesBone(bodyPtr, boneIdx);
			shades += numOfShades * 8;
		}
		return shades;
	}
};

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

	struct polyVertexHeader {
		int16 shadeEntry = 0;
		int16 dataOffset = 0;
	};

	struct ModelData {
		pointTab computedPoints[800];
		pointTab flattenPoints[800];
		int16 shadeTable[500]{0};
	};

	ModelData _modelData;

	bool renderAnimatedModel(ModelData *modelData, const uint8 *bodyPtr, RenderCommand *renderCmds);
	void circleFill(int32 x, int32 y, int32 radius, uint8 color);
	bool renderModelElements(int32 numOfPrimitives, const uint8 *polygonPtr, RenderCommand **renderCmds, ModelData *modelData);
	void getBaseRotationPosition(int32 x, int32 y, int32 z);
	void getCameraAnglePositions(int32 x, int32 y, int32 z);
	void applyRotation(Matrix *targetMatrix, const Matrix *currentMatrix);
	void applyPointsRotation(const pointTab *pointsPtr, int32 numPoints, pointTab *destPoints, const Matrix *rotationMatrix);
	void processRotatedElement(Matrix *targetMatrix, const pointTab *pointsPtr, int32 rotZ, int32 rotY, int32 rotX, const elementEntry *elemPtr, ModelData *modelData);
	void applyPointsTranslation(const pointTab *pointsPtr, int32 numPoints, pointTab *destPoints, const Matrix *translationMatrix);
	void processTranslatedElement(Matrix *targetMatrix, const pointTab *pointsPtr, int32 rotX, int32 rotY, int32 rotZ, const elementEntry *elemPtr, ModelData *modelData);
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
	Vec3 lightPos;

	RenderCommand _renderCmds[1000];
	uint8 renderCoordinatesBuffer[10000]{0};

	int16 polyTab[POLYTABSIZE]{0};
	int16 polyTab2[POLYTABSIZE]{0};
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

	void computeBoundingBox(Vertex *vertices, int32 numVertices, int &vleft, int &vright, int &vtop, int &vbottom) const;
	void computePolygons(int16 polyRenderType, Vertex *vertices, int32 numVertices, int &vleft, int &vright, int &vtop, int &vbottom);

	const RenderCommand *depthSortRenderCommands(int32 numOfPrimitives);
	uint8 *preparePolygons(Common::MemoryReadStream &stream, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);
	uint8 *prepareSpheres(Common::MemoryReadStream &stream, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);
	uint8 *prepareLines(Common::MemoryReadStream &stream, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);

public:
	Renderer(TwinEEngine *engine);

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

	const int16 *const shadeAngleTab3;

	void setLightVector(int32 angleX, int32 angleY, int32 angleZ);

	static void prepareIsoModel(uint8 *bodyPtr);
	void renderPolygons(const CmdRenderPolygon &polygon, Vertex *vertices);

	int32 projectPositionOnScreen(int32 cX, int32 cY, int32 cZ);
	void setCameraPosition(int32 x, int32 y, int32 cX, int32 cY, int32 cZ);
	void setCameraAngle(int32 transPosX, int32 transPosY, int32 transPosZ, int32 rotPosX, int32 rotPosY, int32 rotPosZ, int32 param6);
	void setBaseTranslation(int32 x, int32 y, int32 z);
	void setBaseRotation(int32 x, int32 y, int32 z);
	void setOrthoProjection(int32 x, int32 y, int32 z);

	bool renderIsoModel(int32 x, int32 y, int32 z, int32 angleX, int32 angleY, int32 angleZ, const uint8 *bodyPtr);

	void renderBehaviourModel(int32 boxLeft, int32 boxTop, int32 boxRight, int32 boxBottom, int32 y, int32 angle, const uint8 *bodyPtr);
	void renderBehaviourModel(const Common::Rect &rect, int32 y, int32 angle, const uint8 *bodyPtr);

	void renderInventoryItem(int32 x, int32 y, const uint8 *bodyPtr, int32 angle, int32 param);
};

} // namespace TwinE

#endif
