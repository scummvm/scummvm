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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_MESH_LOADER_COLLADA_H
#define HPL_MESH_LOADER_COLLADA_H

#include "common/list.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/physics/PhysicsJoint.h"
#include "hpl1/engine/resources/MeshLoader.h"
#include "hpl1/engine/system/MemoryManager.h"

class TiXmlElement;

namespace hpl {

class cMesh;
class cNode3D;
class iVertexBuffer;
class cBone;
class cAnimation;
class cAnimationTrack;
class cSkeleton;
class cWorld3D;
class cSector;
class cMeshEntity;
class iPhysicsBody;
class cColliderEntity;
class cMeshJoint;
class cSystem;

//////////////////////////////////////////////
// HELP CLASSES

class cColladaImage {
public:
	tString msId;
	tString msName;

	tString msSource;
};

typedef Common::Array<cColladaImage> tColladaImageVec;

//------------------------------------------------

class cColladaTexture {
public:
	tString msId;
	tString msName;

	tString msImage;
};

typedef Common::Array<cColladaTexture> tColladaTextureVec;

//------------------------------------------------

class cColladaMaterial {
public:
	tString msId;
	tString msName;

	tString msTexture;
	cColor mDiffuseColor;
};

typedef Common::Array<cColladaMaterial> tColladaMaterialVec;

//------------------------------------------------

class cColladaLight {
public:
	tString msId;
	tString msName;
	tString msType;

	cColor mDiffuseColor;
	float mfAngle;
};

typedef Common::Array<cColladaLight> tColladaLightVec;

//------------------------------------------------

class cColladaVtxArray {
public:
	cColladaVtxArray() : mbIsInVertex(false) {}

	tString msId;
	tString msType;
	bool mbIsInVertex;

	tVector3fVec mvArray;
};

typedef Common::Array<cColladaVtxArray> tColladaVtxArrayVec;

class cColladaVtxIndex {
public:
	int mlVtx;
	int mlNorm;
	int mlTex;
};

typedef Common::Array<cColladaVtxIndex> tColladaVtxIndexVec;

//------------------------------------------------

class cColladaExtraVtx {
public:
	cColladaExtraVtx(int alVtx, int alNorm, int alTex, int alNewVtx) {
		mlVtx = alVtx;
		mlNorm = alNorm;
		mlTex = alTex;
		mlNewVtx = alNewVtx;
	}

	cColladaExtraVtx() {}

	int mlVtx;
	int mlNorm;
	int mlTex;

	int mlNewVtx;

	bool Equals(const cColladaVtxIndex &aData) {
		if (mlVtx == aData.mlVtx && mlNorm == aData.mlNorm && mlTex == aData.mlTex) {
			return true;
		}

		return false;
	}
};

typedef Common::List<cColladaExtraVtx> tColladaExtraVtxList;
typedef tColladaExtraVtxList::iterator tColladaExtraVtxListIt;

typedef Common::Array<tColladaExtraVtxList> tColladaExtraVtxListVec;

//------------------------------------------------

class cColladaGeometry {
public:
	cColladaGeometry() : mlPosArrayIdx(-1), mlNormArrayIdx(-1), mlTexArrayIdx(-1),
						 mlPosIdxNum(-1), mlNormIdxNum(-1), mlTexIdxNum(-1) {}

	void Clear() {
		mvIndices.clear();

		for (int i = 0; i < (int)mvArrayVec.size(); i++) {
			mvArrayVec[i].mvArray.clear();
		}
		/*mvVertexVec.clear();
		mvIndexVec.clear();
		mvExtraVtxVec.clear();*/
	}

	tString msId;
	tString msName;

	tVertexVec mvVertexVec;
	tUIntVec mvIndexVec;
	tColladaExtraVtxListVec mvExtraVtxVec;
	tFloatVec mvTangents;

	tString msMaterial;

	// THe following are ONLY used when loading from a geometry element.!!
	tColladaVtxArrayVec mvArrayVec;
	tColladaVtxIndexVec mvIndices;

	int mlPosIdxNum;  // The position in the triangle element
	int mlNormIdxNum; // for eternal use only
	int mlTexIdxNum;

	int mlPosArrayIdx;  // The index for array containing positions
	int mlNormArrayIdx; // The index for array containing normals
	int mlTexArrayIdx;  // The index for array containing texcoords
};

typedef Common::Array<cColladaGeometry> tColladaGeometryVec;

//------------------------------------------------

class cColladaJointPair {
public:
	cColladaJointPair() {}
	cColladaJointPair(int alJoint, int alWeight) {
		mlJoint = alJoint;
		mlWeight = alWeight;
	}

	int mlJoint;
	int mlWeight;
};

typedef Common::List<cColladaJointPair> tColladaJointPairList;
typedef tColladaJointPairList::iterator tColladaJointPairListIt;

typedef Common::Array<tColladaJointPairList> tColladaJointPairListVec;

class cColladaController {
public:
	cColladaController() : mlJointPairIdx(-1), mlWeightPairIdx(-1) {}

	tString msTarget;
	tString msId;

	cMatrixf m_mtxBindShapeMatrix;

	int mlJointPairIdx;
	int mlWeightPairIdx;

	tStringVec mvJoints;
	tFloatVec mvWeights;
	tMatrixfVec mvMatrices;

	tColladaJointPairListVec mvPairs;
};

typedef Common::Array<cColladaController> tColladaControllerVec;

//------------------------------------------------

class cColladaChannel {
public:
	tString msId;
	tString msTarget;
	tString msSource;
};

typedef Common::Array<cColladaChannel> tColladaChannelVec;

class cColladaSampler {
public:
	tString msId;
	tString msTimeArray;
	tString msValueArray;

	tString msTarget;
};

typedef Common::Array<cColladaSampler> tColladaSamplerVec;

class cColladaAnimSource {
public:
	tString msId;
	tFloatVec mvValues;
};

typedef Common::Array<cColladaAnimSource> tColladaAnimSourceVec;

class cColladaAnimation {
public:
	tString msId;
	// tString msName;
	tString msTargetNode;

	tColladaChannelVec mvChannels;
	tColladaSamplerVec mvSamplers;

	tColladaAnimSourceVec mvSources;

	tFloatVec *GetSourceVec(const tString &asId) {
		for (size_t i = 0; i < mvSources.size(); i++) {
			if (mvSources[i].msId == asId) {
				return &mvSources[i].mvValues;
			}
		}

		return NULL;
	}
};

typedef Common::Array<cColladaAnimation> tColladaAnimationVec;

//------------------------------------------------

class cColladaTransform {
public:
	tString msSid;
	tString msType;
	tFloatVec mvValues;
};

typedef Common::List<cColladaTransform> tColladaTransformList;
typedef tColladaTransformList::iterator tColladaTransformListIt;

class cColladaNode;
typedef Common::List<cColladaNode *> tColladaNodeList;
typedef tColladaNodeList::iterator tColladaNodeListIt;

class cColladaNode {
public:
	cColladaNode() : mlCount(0), pParent(NULL), mvScale(1, 1, 1), mbSourceIsFile(false) {}

	tString msId;
	tString msName;
	tString msType;

	tString msSource;
	bool mbSourceIsFile;

	cMatrixf m_mtxTransform;
	cMatrixf m_mtxWorldTransform;

	cColladaNode *pParent;

	cVector3f mvScale;

	int mlCount;

	tColladaNodeList mlstChildren;

	tColladaTransformList mlstTransforms;

	/*void DeleteChildren()
	{
		STLDeleteAll(mlstChildren);
	}*/

	cColladaNode *CreateChild() {
		cColladaNode *pNode = hplNew(cColladaNode, ());
		mlstChildren.push_back(pNode);
		pNode->pParent = this;
		return pNode;
	}

	cColladaTransform *GetTransform(const tString &asSid) {
		tColladaTransformListIt it = mlstTransforms.begin();
		for (; it != mlstTransforms.end(); it++) {
			cColladaTransform &Trans = *it;
			if (Trans.msSid == asSid) {
				return &Trans;
			}
		}
		return NULL;
	}
};

class cColladaScene {
public:
	cColladaScene() {
		mRoot.m_mtxTransform = cMatrixf::Identity;
		mRoot.m_mtxWorldTransform = cMatrixf::Identity;

		mfDeltaTime = 0;
	}

	~cColladaScene() {
		// mRoot.DeleteChildren();
		STLDeleteAll(mlstNodes);
	}

	void ResetNodes() {
		STLDeleteAll(mlstNodes);
		mRoot.mlstChildren.clear();
	}

	cColladaNode *GetNode(const tString asId) {
		tColladaNodeListIt it = mlstNodes.begin();
		for (; it != mlstNodes.end(); ++it) {
			cColladaNode *pNode = *it;
			if (pNode->msId == asId || pNode->msName == asId) {
				return pNode;
			}
		}

		return NULL;
	}

	cColladaNode *GetNodeFromSource(const tString asSource) {
		tColladaNodeListIt it = mlstNodes.begin();
		for (; it != mlstNodes.end(); ++it) {
			cColladaNode *pNode = *it;
			if (pNode->msSource == asSource) {
				return pNode;
			}
		}

		return NULL;
	}

	float mfStartTime;
	float mfEndTime;
	float mfDeltaTime;

	cColladaNode mRoot;

	tColladaNodeList mlstNodes;
};

//------------------------------------------------

class cMeshLoaderCollada : public iMeshLoader {
public:
	cMeshLoaderCollada(iLowLevelGraphics *apLowLevelGraphics);
	virtual ~cMeshLoaderCollada();

	cMesh *LoadMesh(const tString &asFile, tMeshLoadFlag aFlags);
	bool SaveMesh(cMesh *apMesh, const tString &asFile) { return false; }

	cWorld3D *LoadWorld(const tString &asFile, cScene *apScene, tWorldLoadFlag aFlags);

	cAnimation *LoadAnimation(const tString &asFile);

	bool IsSupported(const tString asFileType);

	void AddSupportedTypes(tStringVec *avFileTypes);

private:
	bool mbZToY;

	tWorldLoadFlag mFlags;

	tString GetParentName(cColladaNode *apNode, tColladaGeometryVec *apColladaGeometries);

	void CreateMeshJoint(cMeshJoint *apJoint, ePhysicsJointType aJointType, cBoundingVolume &aBV,
						 tStringVec &avStrings, cColladaNode *apNode, cColladaScene &aColladaScene,
						 tColladaGeometryVec &avColladaGeom);

	void CreateHierarchyNodes(cMesh *apMesh, cNode3D *mpParentNode,
							  cColladaNode *apColladaNode,
							  tColladaGeometryVec &avColladaGeom);

	cColladaGeometry *GetGeometry(const tString &asId, tColladaGeometryVec &avGeomVec);
	cColladaLight *GetLight(const tString &asId, tColladaLightVec &avLightVec);

	cMeshEntity *CreateStaticMeshEntity(cColladaNode *apNode, cWorld3D *apWorld,
										cColladaGeometry *apGeom, bool abInRoomGroup,
										tColladaMaterialVec &avColladaMaterials,
										tColladaTextureVec &avColladaTextures,
										tColladaImageVec &avColladaImages);

	cColliderEntity *CreateStaticCollider(cColladaNode *apNode, cWorld3D *apWorld,
										  cColladaGeometry *apGeom,
										  tColladaMaterialVec &avColladaMaterials,
										  tColladaTextureVec &avColladaTextures,
										  tColladaImageVec &avColladaImages,
										  bool abCharacterCollider);

	void AddSceneObjects(cColladaNode *apNode, cWorld3D *apWorld,
						 tColladaGeometryVec &avColladaGeometries, tColladaLightVec &avColladaLights,
						 tColladaMaterialVec &avColladaMaterials, tColladaTextureVec &avColladaTextures,
						 tColladaImageVec &avColladaImages, cColladaScene *apColladaScene);

	void AddSectorChildren(cColladaNode *apNode, tString asSector, cWorld3D *apWorld,
						   tColladaGeometryVec &avColladaGeometries, tColladaLightVec &avColladaLights,
						   tColladaMaterialVec &avColladaMaterials, tColladaTextureVec &avColladaTextures,
						   tColladaImageVec &avColladaImages);

	cAnimationTrack *CreateAnimTrack(cAnimation *apAnimation, cSkeleton *apSkeleton,
									 cColladaAnimation &aAnim, cColladaScene *apScene);

	void CalcLocalMatrixRec(cBone *apBone, cMatrixf a_mtxParentGlobal, int alDepth);

	void CreateSkeletonBone(cColladaNode *apColladaNode, cBone *apParentBone);

	iVertexBuffer *CreateVertexBuffer(cColladaGeometry &aGeometry,
									  eVertexBufferUsageType aUsageType);
	// tColladaExtraVtxListVec &vExtraVtxVec);

	bool FillStructures(const tString &asFile,
						tColladaImageVec *apColladaImageVec,
						tColladaTextureVec *apColladaTextureVec,
						tColladaMaterialVec *apColladaMaterialVec,
						tColladaLightVec *apColladaLightVec,
						tColladaGeometryVec *apColladaGeometryVec,
						tColladaControllerVec *apColladaControllerVec,
						tColladaAnimationVec *apColladaAnimVec,
						cColladaScene *apColladaScene,
						bool abCache);

	bool SaveStructures(const tString &asFile,
						tColladaImageVec *apColladaImageVec,
						tColladaTextureVec *apColladaTextureVec,
						tColladaMaterialVec *apColladaMaterialVec,
						tColladaLightVec *apColladaLightVec,
						tColladaGeometryVec *apColladaGeometryVec,
						tColladaControllerVec *apColladaControllerVec,
						tColladaAnimationVec *apColladaAnimVec,
						cColladaScene *apColladaScene);

	bool LoadStructures(const tString &asFile,
						tColladaImageVec *apColladaImageVec,
						tColladaTextureVec *apColladaTextureVec,
						tColladaMaterialVec *apColladaMaterialVec,
						tColladaLightVec *apColladaLightVec,
						tColladaGeometryVec *apColladaGeometryVec,
						tColladaControllerVec *apColladaControllerVec,
						tColladaAnimationVec *apColladaAnimVec,
						cColladaScene *apColladaScene);

	void LoadColladaScene(TiXmlElement *apRootElem, cColladaNode *apParentNode, cColladaScene *apScene,
						  tColladaLightVec *apColladaLightVec);

	void LoadAnimations(TiXmlElement *apRootElem, tColladaAnimationVec &avAnimations,
						cColladaScene *apColladaScene);

	void LoadImages(TiXmlElement *apRootElem, tColladaImageVec &avColladaImageVec);

	void LoadTextures(TiXmlElement *apRootElem, tColladaTextureVec &avColladaTextureVec);

	void LoadMaterials(TiXmlElement *apRootElem, tColladaMaterialVec &avColladaMaterialVec);

	void LoadLights(TiXmlElement *apRootElem, tColladaLightVec &avColladaLightVec);

	void LoadGeometry(TiXmlElement *apRootElem, tColladaGeometryVec &avColladaGeometryVec, cColladaScene *apColladaScene);
	void LoadVertexData(TiXmlElement *apSourceElem, tVector3fVec &avVtxVec);

	void LoadControllers(TiXmlElement *apRootElem, tColladaControllerVec &avColladaControllerVec,
						 tColladaGeometryVec *apColladaGeometryVec);
	void LoadJointData(TiXmlElement *apSourceElem, cColladaController &aController);

	// Helpers
	void SplitVertices(cColladaGeometry &aGeometry, tColladaExtraVtxListVec &avExtraVtxVec,
					   tVertexVec &avVertexVec, tUIntVec &avIndexVec);

	void FillVertexVec(const char *apChars, tVector3fVec &avVtxVec, int alElements, int alVtxCount);
	tString GetTopString(const tString asPath);
	tString GetMaterialTextureFile(const tString &asMaterial, tColladaMaterialVec &avColladaMaterialVec,
								   tColladaTextureVec &avColladaTextureVec,
								   tColladaImageVec &avColladaImageVec);

	cVector3f GetVectorPos(const cVector3f &avVec);
	cVector3f GetVectorPosFromPtr(float *apVec);
	cVector3f GetVectorScaleFromPtr(float *apVec);
};

} // namespace hpl

#endif // HPL_MESH_LOADER_COLLADA_H
