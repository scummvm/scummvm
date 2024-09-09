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

#define TYPE_3D 0
#define TYPE_ISO 1

namespace TwinE {

class BodyData;
class TwinEEngine;

struct ComputedVertex {
	int16 intensity = 0;
	int16 x = 0;
	int16 y = 0;
};

bool isPolygonVisible(const ComputedVertex *vertices);

struct CmdRenderPolygon {
	uint8 renderType = 0;
	uint8 numVertices = 0;
	int16 colorIndex = 0; // intensity
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
		I16Vec3 computedPoints[800]; // List_Anim_Point
		I16Vec3 flattenPoints[800];
		int16 normalTable[500]{0};
	};

	// this is a member var, because 10k on the stack is not supported by every platform
	ModelData _modelData;

	// AnimNuage
	void animModel(ModelData *modelData, const BodyData &bodyData, RenderCommand *renderCmds, const IVec3 &angleVec, const IVec3 &renderPos, Common::Rect &modelRect);
	bool computeSphere(int32 x, int32 y, int32 radius, int &vtop, int &vbottom);
	bool renderObjectIso(const BodyData &bodyData, RenderCommand **renderCmds, ModelData *modelData, Common::Rect &modelRect); // RenderObjetIso
	IVec3 longInverseRot(int32 x, int32 y, int32 z);
	inline IVec3 getCameraAnglePositions(const IVec3 &vec) {
		return longInverseRot(vec.x, vec.y, vec.z);
	}
	void rotMatIndex2(IMatrix3x3 *targetMatrix, const IMatrix3x3 *currentMatrix, const IVec3 &angleVec);
	void rotList(const Common::Array<BodyVertex>& vertices, int32 firstPoint, int32 numPoints, I16Vec3 *destPoints, const IMatrix3x3 *rotationMatrix, const IVec3 &destPos);
	void processRotatedElement(IMatrix3x3 *targetMatrix, const Common::Array<BodyVertex>& vertices, int32 rotX, int32 rotY, int32 rotZ, const BodyBone &bone, ModelData *modelData);
	void transRotList(const Common::Array<BodyVertex>& vertices, int32 firstPoint, int32 numPoints, I16Vec3 *destPoints, const IMatrix3x3 *translationMatrix, const IVec3 &angleVec, const IVec3 &destPos);
	void translateGroup(IMatrix3x3 *targetMatrix, const Common::Array<BodyVertex>& vertices, int32 rotX, int32 rotY, int32 rotZ, const BodyBone &bone, ModelData *modelData);
	/**
	 * @brief Rotate the given coordinates by the given rotation matrix
	 */
	IVec3 rot(const IMatrix3x3 &matrix, int32 x, int32 y, int32 z);

	IVec3 _cameraPos; // CameraX, CameraY, CameraZ
	IVec3 _projectionCenter{320, 200, 0}; // XCentre, YCentre, IsoScale

	int32 _kFactor = 128;
	int32 _lFactorX = 1024;
	int32 _lFactorY = 840;

	IMatrix3x3 _matrixWorld; // LMatriceWorld
	IMatrix3x3 _matricesTable[30 + 1];
	IVec3 _normalLight; // NormalXLight, NormalYLight, NormalZLight
	IVec3 _cameraRot; // CameraXr, CameraYr, CameraZr

	RenderCommand _renderCmds[1000];
	/**
	 * @brief Raw buffer for holding the render commands. This is a type followed by the command data
	 * that is needed to render the primitive.
	 */
	uint8 _renderCoordinatesBuffer[10000]{0};
	ComputedVertex _clippedPolygonVertices1[128];
	ComputedVertex _clippedPolygonVertices2[128];

	int16* _tabVerticG = nullptr;
	int16* _tabVerticD = nullptr;
	int16* _tabCoulG = nullptr;
	int16* _tabCoulD = nullptr;
	int16* _taby0 = nullptr;
	int16* _taby1 = nullptr;
	int16* _tabx0 = nullptr; // also _tabCoulG
	int16* _tabx1 = nullptr; // also _tabCoulD

	bool _typeProj = TYPE_3D;

	void svgaPolyCopper(int16 vtop, int16 vbottom, uint16 color) const;
	void svgaPolyBopper(int16 vtop, int16 vbottom, uint16 color) const;
	void svgaPolyTriste(int16 vtop, int16 vbottom, uint16 color) const;
	void svgaPolyTele(int16 vtop, int16 vbottom, uint16 color) const;
	void svgaPolyTrans(int16 vtop, int16 vbottom, uint16 color) const;
	void svgaPolyTrame(int16 vtop, int16 vbottom, uint16 color) const;
	void svgaPolyGouraud(int16 vtop, int16 vbottom) const;
	void svgaPolyDith(int16 vtop, int16 vbottom) const;
	void svgaPolyMarbre(int16 vtop, int16 vbottom, uint16 color) const;
	void svgaPolyTriche(int16 vtop, int16 vbottom, uint16 color) const;
	bool computePoly(int16 polyRenderType, const ComputedVertex *vertices, int32 numVertices, int16 &vtop, int16 &vbottom);

	const RenderCommand *depthSortRenderCommands(int32 numOfPrimitives);
	uint8 *preparePolygons(const Common::Array<BodyPolygon>& polygons, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);
	uint8 *prepareSpheres(const Common::Array<BodySphere>& spheres, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);
	uint8 *prepareLines(const Common::Array<BodyLine>& lines, int32 &numOfPrimitives, RenderCommand **renderCmds, uint8 *renderBufferPtr, ModelData *modelData);

	void flipMatrix();

	void fillTextPolyNoClip(int32 top, int32 bottom, const uint8 *holomapImage, uint32 holomapImageSize);
	void fillHolomapTriangle(int16 *pDest, int32 x1, int32 y1, int32 x2, int32 y2);
	void fillHolomapTriangles(const ComputedVertex &vertex1, const ComputedVertex &vertex2, const ComputedVertex &texCoord1, const ComputedVertex &texCoord2, int32 &top, int32 &bottom);

	// ClipGauche, ClipDroite, ClipHaut, ClipBas
	int16 leftClip(int16 polyRenderType, ComputedVertex** offTabPoly, int32 numVertices);
	int16 rightClip(int16 polyRenderType, ComputedVertex** offTabPoly, int32 numVertices);
	int16 topClip(int16 polyRenderType, ComputedVertex** offTabPoly, int32 numVertices);
	int16 bottomClip(int16 polyRenderType, ComputedVertex** offTabPoly, int32 numVertices);
	int32 computePolyMinMax(int16 polyRenderType, ComputedVertex **offTabPoly, int32 numVertices, int16 &vtop, int16 &vbottom);
public:
	Renderer(TwinEEngine *engine);
	~Renderer();

	void init(int32 w, int32 h);

	void setCameraRotation(int32 x, int32 y, int32 z);

	/**
	 * Calculate offset for the side and forward distances by the given angle of an actor
	 * @param side Actor current X coordinate
	 * @param forward Actor current Z coordinate
	 * @param angle Actor angle to rotate
	 */
	IVec2 rotate(int32 side, int32 forward, int32 angle) const;

	void setLightVector(int32 angleX, int32 angleY, int32 angleZ);
	IVec3 longWorldRot(int32 x, int32 y, int32 z);

	IVec3 worldRotatePoint(const IVec3& vec);

	void fillVertices(int16 vtop, int16 vbottom, uint8 renderType, uint16 color);
	void renderPolygons(const CmdRenderPolygon &polygon, ComputedVertex *vertices);

	inline IVec3 projectPoint(const IVec3& pos) { // ProjettePoint
		return projectPoint(pos.x, pos.y, pos.z);
	}

	void projIso(IVec3 &pos, int32 x, int32 y, int32 z);

	IVec3 projectPoint(int32 cX, int32 cY, int32 cZ);

	void setFollowCamera(int32 transPosX, int32 transPosY, int32 transPosZ, int32 cameraAlpha, int32 cameraBeta, int32 cameraGamma, int32 cameraZoom);
	void setPosCamera(int32 x, int32 y, int32 z);
	IVec3 setAngleCamera(int32 alpha, int32 beta, int32 gamma);
	IVec3 setInverseAngleCamera(int32 alpha, int32 beta, int32 gamma);

	inline IVec3 setBaseRotation(const IVec3 &rot) {
		return setAngleCamera(rot.x, rot.y, rot.z);
	}

	void setProjection(int32 x, int32 y, int32 depthOffset, int32 scaleX, int32 scaleY);
	void setIsoProjection(int32 x, int32 y, int32 scale);

	bool affObjetIso(int32 x, int32 y, int32 z, int32 angleX, int32 angleY, int32 angleZ, const BodyData &bodyData, Common::Rect &modelRect);

	inline bool renderIsoModel(const IVec3 &pos, int32 angleX, int32 angleY, int32 angleZ, const BodyData &bodyData, Common::Rect &modelRect) {
		return affObjetIso(pos.x, pos.y, pos.z, angleX, angleY, angleZ, bodyData, modelRect);
	}

	/**
	 * @param angle A value of @c -1 means that the model is automatically rotated
	 */
	void renderBehaviourModel(int32 boxLeft, int32 boxTop, int32 boxRight, int32 boxBottom, int32 y, int32 angle, const BodyData &bodyData, ActorMoveStruct &move);
	/**
	 * @param angle A value of @c -1 means that the model is automatically rotated
	 */
	void drawObj3D(const Common::Rect &rect, int32 y, int32 angle, const BodyData &bodyData, ActorMoveStruct &move);

	/**
	 * @brief Render an inventory item
	 */
	void draw3dObject(int32 x, int32 y, const BodyData &bodyData, int32 angle, int32 cameraZoom);

	void asmTexturedTriangleNoClip(const ComputedVertex vertexCoordinates[3], const ComputedVertex textureCoordinates[3], const uint8 *holomapImage, uint32 holomapImageSize);
};

inline void Renderer::setCameraRotation(int32 x, int32 y, int32 z) {
	_cameraRot.x = x;
	_cameraRot.y = y;
	_cameraRot.z = z;
}

} // namespace TwinE

#endif
