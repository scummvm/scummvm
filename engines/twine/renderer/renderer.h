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

#ifndef TWINE_RENDERER_RENDERER_H
#define TWINE_RENDERER_RENDERER_H

#include "common/endian.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "twine/parser/bodytypes.h"
#include "twine/twine.h"

#define POLYGONTYPE_FLAT 0
#define POLYGONTYPE_TELE 1
// horizontal color adjustment with changing pattern over the polygon
#define POLYGONTYPE_COPPER 2
#define POLYGONTYPE_BOPPER 3
#define POLYGONTYPE_MARBLE 4
#define POLYGONTYPE_TRANS 5
#define POLYGONTYPE_TRAME 6
#define POLYGONTYPE_GOURAUD 7
#define POLYGONTYPE_DITHER 8
#define POLYGONTYPE_OUTLINE 9

#define MAT_TRISTE 0
#define MAT_PIERRE 1
#define MAT_COPPER 2
#define MAT_BOPPER 3
#define MAT_MARBRE 4
#define MAT_TRANS 5
#define MAT_TRAME 6
#define MAT_FLAT 7
#define MAT_GRANIT 8
#define MAT_GOURAUD 9
#define MAT_DITHER 10

namespace TwinE {

class BodyData;
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
	int16 top = 0;
	int16 bottom = 0;
	// followed by Vertex array
};

struct IMatrix3x3 {
	IVec3 row1;
	IVec3 row2;
	IVec3 row3;
};

inline IMatrix3x3 operator*(const IMatrix3x3 &matrix, const IVec3 &vec) {
	IMatrix3x3 out;
	out.row1.x = matrix.row1.x * vec.x;
	out.row1.y = matrix.row1.y * vec.x;
	out.row1.z = matrix.row1.z * vec.x;

	out.row2.x = matrix.row2.x * vec.y;
	out.row2.y = matrix.row2.y * vec.y;
	out.row2.z = matrix.row2.z * vec.y;

	out.row3.x = matrix.row3.x * vec.z;
	out.row3.y = matrix.row3.y * vec.z;
	out.row3.z = matrix.row3.z * vec.z;
	return out;
}

class Renderer {
private:
	TwinEEngine *_engine;

	struct RenderCommand {
		int16 depth = 0;
		int16 renderType = 0;
		/**
		 * Pointer to the command data
		 * @sa renderCoordinatesBuffer
		 * @sa CmdRenderLine
		 * @sa CmdRenderSphere
		 * @sa CmdRenderPolygon
		 */
		uint8 *dataPtr = nullptr;
	};

	/**
	 * @brief A render command payload for drawing a line
	 *
	 * @sa RenderCommand
	 */
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

	/**
	 * @brief A render command payload for drawing a sphere
	 *
	 * @sa RenderCommand
	 */
	struct CmdRenderSphere {
		uint16 color = 0; // color start and end values
		uint8 polyRenderType = 0;
		int16 radius = 0;
		int16 x = 0;
		int16 y = 0;
		int16 z = 0;
	};

	struct ModelData {
		I16Vec3 computedPoints[800];
		I16Vec3 flattenPoints[800];
		int16 shadeTable[500]{0};
	};

	ModelData _modelData;

	bool renderAnimatedModel(ModelData *modelData, const BodyData &bodyData, RenderCommand *renderCmds, const IVec3 &angleVec, const IVec3 &renderPos, Common::Rect &modelRect);
	bool prepareCircle(int32 x, int32 y, int32 radius);
	bool renderModelElements(int32 numOfPrimitives, const BodyData &bodyData, RenderCommand **renderCmds, ModelData *modelData, Common::Rect &modelRect);
	IVec3 getCameraAnglePositions(int32 x, int32 y, int32 z);
	inline IVec3 getCameraAnglePositions(const IVec3 &vec) {
		return getCameraAnglePositions(vec.x, vec.y, vec.z);
	}
	void applyRotation(IMatrix3x3 *targetMatrix, const IMatrix3x3 *currentMatrix, const IVec3 &angleVec);
	void applyPointsRotation(const Common::Array<BodyVertex>& vertices, int32 firstPoint, int32 numPoints, I16Vec3 *destPoints, const IMatrix3x3 *rotationMatrix, const IVec3 &destPos);
	void processRotatedElement(IMatrix3x3 *targetMatrix, const Common::Array<BodyVertex>& vertices, int32 rotX, int32 rotY, int32 rotZ, const BodyBone &bone, ModelData *modelData);
	void applyPointsTranslation(const Common::Array<BodyVertex>& vertices, int32 firstPoint, int32 numPoints, I16Vec3 *destPoints, const IMatrix3x3 *translationMatrix, const IVec3 &angleVec, const IVec3 &destPos);
	void processTranslatedElement(IMatrix3x3 *targetMatrix, const Common::Array<BodyVertex>& vertices, int32 rotX, int32 rotY, int32 rotZ, const BodyBone &bone, ModelData *modelData);
	IVec3 translateGroup(int32 x, int32 y, int32 z);

	IVec3 _baseTransPos;
	IVec3 _orthoProjPos;

	int32 _cameraDepthOffset = 0;
	int32 _cameraScaleX = 0;
	int32 _cameraScaleY = 0;

	IMatrix3x3 _baseMatrix;
	IMatrix3x3 _matricesTable[30 + 1];
	IMatrix3x3 _shadeMatrix;
	IVec3 _lightNorm;
	IVec3 _baseRotPos;

	RenderCommand _renderCmds[1000];
	/**
	 * @brief Raw buffer for holding the render commands. This is a type followed by the command data
	 * that is needed to render the primitive.
	 */
	uint8 _renderCoordinatesBuffer[10000]{0};
	Vertex _clippedPolygonVertices1[128];
	Vertex _clippedPolygonVertices2[128];

	int32 _polyTabSize = 0;
	int16 *_polyTab = nullptr;
	int16 *_colorProgressionBuffer = nullptr;
	int16* _holomap_polytab_1_1 = nullptr;
	int16* _holomap_polytab_1_2 = nullptr;
	int16* _holomap_polytab_1_3 = nullptr;
	int16* _holomap_polytab_2_3 = nullptr;
	int16* _holomap_polytab_2_2 = nullptr;
	int16* _holomap_polytab_2_1 = nullptr;

	bool _isUsingOrthoProjection = false;

	void renderPolygonsCopper(int vtop, int32 vsize, uint16 color) const;
	void renderPolygonsBopper(int vtop, int32 vsize, uint16 color) const;
	void renderPolygonsFlat(int vtop, int32 vsize, uint16 color) const;
	void renderPolygonsTele(int vtop, int32 vsize, uint16 color) const;
	void renderPolygonsTrans(int vtop, int32 vsize, uint16 color) const;
	void renderPolygonsTrame(int vtop, int32 vsize, uint16 color) const;
	void renderPolygonsGouraud(int vtop, int32 vsize) const;
	void renderPolygonsDither(int vtop, int32 vsize) const;
	void renderPolygonsMarble(int vtop, int32 vsize, uint16 color) const;
	void renderPolygonsSimplified(int vtop, int32 vsize, uint16 color) const;
	bool computePoly(int16 polyRenderType, const Vertex *vertices, int32 numVertices);

	const RenderCommand *depthSortRenderCommands(int32 numOfPrimitives);
	uint8 *preparePolygons(const Common::Array<BodyPolygon>& polygons, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);
	uint8 *prepareSpheres(const Common::Array<BodySphere>& spheres, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);
	uint8 *prepareLines(const Common::Array<BodyLine>& lines, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);

	void baseMatrixTranspose();

	void renderHolomapPolygons(int32 top, int32 bottom, uint8 *holomapImage, uint32 holomapImageSize);
	void computeHolomapPolygon(int32 y1, int32 x1, int32 y2, int32 x2, int16 *polygonTabPtr);
	void fillHolomapPolygons(const Vertex &vertex1, const Vertex &vertex2, const Vertex &texCoord1, const Vertex &texCoord2, int32 &top, int32 &bottom);

	int16 leftClip(int16 polyRenderType, Vertex** offTabPoly, int32 numVertices);
	int16 rightClip(int16 polyRenderType, Vertex** offTabPoly, int32 numVertices);
	int16 topClip(int16 polyRenderType, Vertex** offTabPoly, int32 numVertices);
	int16 bottomClip(int16 polyRenderType, Vertex** offTabPoly, int32 numVertices);
	int32 computePolyMinMax(int16 polyRenderType, Vertex **offTabPoly, int32 numVertices);
public:
	Renderer(TwinEEngine *engine);
	~Renderer();

	void init(int32 w, int32 h);

	IVec3 _projPos;

	void setBaseRotationPos(int32 x, int32 y, int32 z);
	IVec3 getHolomapRotation(const int32 angleX, const int32 angleY, const int32 angleZ) const;

	void setLightVector(int32 angleX, int32 angleY, int32 angleZ);
	IVec3 getBaseRotationPosition(int32 x, int32 y, int32 z);

	inline IVec3 getBaseRotationPosition(const IVec3& vec) {
		return getBaseRotationPosition(vec.x, vec.y, vec.z);
	}

	void fillVertices(int vtop, int32 vsize, uint8 renderType, uint16 color);
	void renderPolygons(const CmdRenderPolygon &polygon, Vertex *vertices, int vtop, int vbottom);

	inline IVec3 &projectPositionOnScreen(const IVec3& pos) { // ProjettePoint
		return projectPositionOnScreen(pos.x, pos.y, pos.z);
	}

	IVec3 &projectPositionOnScreen(int32 cX, int32 cY, int32 cZ);

	void setCameraPosition(int32 x, int32 y, int32 depthOffset, int32 scaleX, int32 scaleY);
	void setCameraAngle(int32 transPosX, int32 transPosY, int32 transPosZ, int32 rotPosX, int32 rotPosY, int32 rotPosZ, int32 param6);
	IVec3 updateCameraAnglePositions(int zShift = 0);
	void setBaseTranslation(int32 x, int32 y, int32 z);
	IVec3 setBaseRotation(int32 x, int32 y, int32 z, bool transpose = false);

	inline IVec3 setBaseRotation(const IVec3 &rot, bool transpose = false) {
		return setBaseRotation(rot.x, rot.y, rot.z, transpose);
	}

	void setOrthoProjection(int32 x, int32 y, int32 z);

	bool renderIsoModel(int32 x, int32 y, int32 z, int32 angleX, int32 angleY, int32 angleZ, const BodyData &bodyData, Common::Rect &modelRect);

	inline bool renderIsoModel(const IVec3 &pos, int32 angleX, int32 angleY, int32 angleZ, const BodyData &bodyData, Common::Rect &modelRect) {
		return renderIsoModel(pos.x, pos.y, pos.z, angleX, angleY, angleZ, bodyData, modelRect);
	}

	/**
	 * @param angle A value of @c -1 means that the model is automatically rotated
	 */
	void renderBehaviourModel(int32 boxLeft, int32 boxTop, int32 boxRight, int32 boxBottom, int32 y, int32 angle, const BodyData &bodyData, ActorMoveStruct &move);
	/**
	 * @param angle A value of @c -1 means that the model is automatically rotated
	 */
	void renderBehaviourModel(const Common::Rect &rect, int32 y, int32 angle, const BodyData &bodyData, ActorMoveStruct &move);

	void renderInventoryItem(int32 x, int32 y, const BodyData &bodyData, int32 angle, int32 param);

	void renderHolomapVertices(const Vertex vertexCoordinates[3], const Vertex textureCoordinates[3], uint8 *holomapImage, uint32 holomapImageSize);
};

inline void Renderer::setBaseRotationPos(int32 x, int32 y, int32 z) {
	_baseRotPos.x = x;
	_baseRotPos.y = y;
	_baseRotPos.z = z;
}

} // namespace TwinE

#endif
