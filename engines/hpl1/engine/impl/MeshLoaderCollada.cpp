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

#include "hpl1/engine/impl/MeshLoaderCollada.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/System.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/scene/ColliderEntity.h"
#include "hpl1/engine/scene/Light3DPoint.h"
#include "hpl1/engine/scene/Light3DSpot.h"
#include "hpl1/engine/scene/MeshEntity.h"
#include "hpl1/engine/scene/Node3D.h"
#include "hpl1/engine/scene/PortalContainer.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/SoundEntity.h"
#include "hpl1/engine/scene/World3D.h"

#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/Mesh.h"
#include "hpl1/engine/graphics/SubMesh.h"

#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/MaterialManager.h"
#include "hpl1/engine/resources/MeshManager.h"
#include "hpl1/engine/resources/Resources.h"

#include "hpl1/engine/graphics/Animation.h"
#include "hpl1/engine/graphics/AnimationTrack.h"
#include "hpl1/engine/graphics/Bone.h"
#include "hpl1/engine/graphics/Skeleton.h"

#include "hpl1/engine/graphics/Beam.h"
#include "hpl1/engine/graphics/BillBoard.h"
#include "hpl1/engine/graphics/ParticleSystem3D.h"

#include "hpl1/engine/physics/Physics.h"
#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsMaterial.h"
#include "hpl1/engine/physics/PhysicsWorld.h"
#include "hpl1/engine/physics/SurfaceData.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"

#include "hpl1/engine/math/Math.h"

namespace hpl {

#define GetAdress(sStr) \
	if (sStr[0] == '#') \
		sStr = cString::Sub(sStr, 1);

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMeshLoaderCollada::cMeshLoaderCollada(iLowLevelGraphics *apLowLevelGraphics)
	: iMeshLoader(apLowLevelGraphics) {
	mFlags = 0;
}

//-----------------------------------------------------------------------

cMeshLoaderCollada::~cMeshLoaderCollada() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cWorld3D *cMeshLoaderCollada::LoadWorld(const tString &asFile, cScene *apScene, tWorldLoadFlag aFlags) {
	// Images
	tColladaImageVec vColladaImages;
	// Textures
	tColladaTextureVec vColladaTextures;
	// Materials
	tColladaMaterialVec vColladaMaterials;
	// Geometries
	tColladaGeometryVec vColladaGeometries;
	// Lights
	tColladaLightVec vColladaLights;
	// Scene
	cColladaScene ColladaScene;

	mFlags = aFlags;

	unsigned long lStartTime = GetApplicationTime();

	// Fill the structures with collada file data
	bool bRet = FillStructures(asFile, &vColladaImages, &vColladaTextures,
							   &vColladaMaterials, &vColladaLights,
							   &vColladaGeometries,
							   NULL, NULL,
							   &ColladaScene, true);

	unsigned long lTime = GetApplicationTime() - lStartTime;
	Log("Loading collada for '%s' took: %d ms\n", asFile.c_str(), lTime);

	if (bRet == false)
		return NULL;

	cWorld3D *pWorld = apScene->CreateWorld3D(cString::SetFileExt(cString::GetFileName(asFile), ""));
	pWorld->SetFileName(cString::GetFileName(asFile));

	cPortalContainer *pPortalContainer = pWorld->GetPortalContainer();

	/////////////////////////////////////
	// Create the physics world
	iPhysicsWorld *pPhysiscsWorld = pWorld->GetPhysics()->CreateWorld(true);
	pWorld->SetPhysicsWorld(pPhysiscsWorld, true);

	//////////////////////////////////////////////////////////////
	// Find the rooms and and add them as sectors in the world.
	// Log("FIND ROOMS:\n");
	tColladaNodeListIt it = ColladaScene.mlstNodes.begin();
	for (; it != ColladaScene.mlstNodes.end(); it++) {
		cColladaNode *pNode = *it;

		// Only need to check if it is a special type
		if (pNode->msName[0] == '_') {
			// Get 5 first letters and check if these match "_room"
			tString sType = cString::Sub(pNode->msName, 0, 5);
			// Log("Type: %s\n",sType.c_str());
			if (cString::ToLowerCase(sType) == "_room") {
				// Get Room number
				tString sNum = cString::Sub(pNode->msName, 5);
				// int lNum = cString::ToInt(sNum.c_str(),-1);
				tString sRoomId = sNum;

				// Log("Adding room num %s\n",sRoomId.c_str());

				// Add sector
				pPortalContainer->AddSector(sRoomId);

				// Add all childs of this node to the room.
				tColladaNodeListIt ChildIt = pNode->mlstChildren.begin();
				for (; ChildIt != pNode->mlstChildren.end(); ChildIt++) {
					AddSectorChildren(*ChildIt, sRoomId, pWorld, vColladaGeometries, vColladaLights,
									  vColladaMaterials, vColladaTextures, vColladaImages);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////
	// Iterate the nodes and add remaining objects to the scene.
	// Log("ADD OBJECTS:\n");
	it = ColladaScene.mRoot.mlstChildren.begin();
	for (; it != ColladaScene.mRoot.mlstChildren.end(); it++) {
		AddSceneObjects(*it, pWorld, vColladaGeometries, vColladaLights,
						vColladaMaterials, vColladaTextures, vColladaImages, &ColladaScene);
	}

	pWorld->SetUpData();

	return pWorld;
}

//-----------------------------------------------------------------------

static cColladaNode *GetNodeFromController(const tString &asGeomId,
										   tColladaControllerVec &avColladaControllers,
										   cColladaScene &aColladaScene) {
	tString sControlId = "";
	bool bGuess = false;
	for (int ctrl = 0; ctrl < (int)avColladaControllers.size(); ctrl++) {
		cColladaController &Control = avColladaControllers[ctrl];
		if (Control.msTarget == asGeomId) {
			sControlId = Control.msId;
			bGuess = false;
		}
		// Guessing, if no controller found try the one with source "".
		else if (sControlId == "" && Control.msTarget == "") {
			sControlId = Control.msId;
			bGuess = true;
		}
	}

	if (bGuess)
		Warning("No controller for for geometry %s, guessing on %s target = ''\n",
				asGeomId.c_str(), sControlId.c_str());

	if (sControlId == "") {
		Warning("No controller refered to the geometry!\n");
		return NULL;
	}

	cColladaNode *pNode = aColladaScene.GetNodeFromSource(sControlId);
	if (pNode == NULL) {
		Warning("No node for controller '%s'\n", sControlId.c_str());
	}

	return pNode;
}

//-------------------------------------------------

static void FixLocalTransform(cMatrixf *apMatrix, cColladaNode *apNode,
							  tColladaAnimationVec &avColladaAnimations, cSkeleton *apSkeleton, bool abHasSeveralBodies) {
	cColladaNode *pParentNode = apNode->pParent;
	if (pParentNode && apSkeleton == NULL) {
		if (avColladaAnimations.empty() == false || abHasSeveralBodies) {
			*apMatrix = cMath::MatrixMul(cMath::MatrixScale(pParentNode->mvScale), *apMatrix);
		} else {
			*apMatrix = cMath::MatrixMul(pParentNode->m_mtxTransform, *apMatrix);
		}
	}
}

//-------------------------------------------------

static void FixLocalPosition(cVector3f *apPos, cColladaNode *apNode,
							 tColladaAnimationVec &avColladaAnimations, cSkeleton *apSkeleton, bool abHasSeveralBodies) {
	cColladaNode *pParentNode = apNode->pParent;
	if (pParentNode && apSkeleton == NULL) {
		if (avColladaAnimations.empty() == false || abHasSeveralBodies) {
			*apPos = cMath::MatrixMul(cMath::MatrixScale(pParentNode->mvScale), *apPos);
		} else {
			*apPos = cMath::MatrixMul(pParentNode->m_mtxTransform, *apPos);
		}
	}
}

//-------------------------------------------------

cMesh *cMeshLoaderCollada::LoadMesh(const tString &asFile, tMeshLoadFlag aFlags) {
	/////////////////////////////////////////////////
	// SETUP TEMP DATA STRUCTURES
	// Images
	tColladaImageVec vColladaImages;
	// Textures
	tColladaTextureVec vColladaTextures;
	// Materials
	tColladaMaterialVec vColladaMaterials;
	// Lights
	tColladaLightVec vColladaLights;
	// Geometries
	tColladaGeometryVec vColladaGeometries;
	// Controllers
	tColladaControllerVec vColladaControllers;
	// Animations
	tColladaAnimationVec vColladaAnimations;
	// Scene
	cColladaScene ColladaScene;

	mFlags = aFlags;

	// Fill the structures with collada file data
	tColladaGeometryVec *pGeomVec = (aFlags & eMeshLoadFlag_NoGeometry) ? NULL : &vColladaGeometries;

	bool bRet = FillStructures(asFile, &vColladaImages, &vColladaTextures,
							   &vColladaMaterials, &vColladaLights,
							   pGeomVec, &vColladaControllers,
							   &vColladaAnimations,
							   &ColladaScene, true);

	if (bRet == false)
		return NULL;

	////////////////////////
	// Create Skeleton
	cSkeleton *pSkeleton = NULL;
	if (vColladaControllers.empty() == false) {
		pSkeleton = hplNew(cSkeleton, ());

		tColladaNodeListIt it = ColladaScene.mRoot.mlstChildren.begin();
		for (; it != ColladaScene.mRoot.mlstChildren.end(); it++) {
			cColladaNode *pNode = *it;

			CreateSkeletonBone(pNode, pSkeleton->GetRootBone());
		}

		////////////////////////////////////
		// Set the bind position of the bones
		//(This will set the local matrix as the global bind)
		for (size_t i = 0; i < vColladaControllers.size(); i++) {
			cColladaController &Ctrl = vColladaControllers[i];

			for (size_t j = 0; j < Ctrl.mvJoints.size(); j++) {
				cBone *pBone = pSkeleton->GetBoneByName(Ctrl.mvJoints[j]);

				if (pBone) {
					pBone->SetTransform(cMath::MatrixInverse(Ctrl.mvMatrices[j]));
					pBone->SetValue(1);
				} else {
					Log("Bone '%s' does not exist\n", Ctrl.mvJoints[j].c_str());
				}
			}
		}

		////////////////////////////////////////////////
		// Do another pass and calculate the local matrix
		cBoneIterator BoneIt = pSkeleton->GetRootBone()->GetChildIterator();
		while (BoneIt.HasNext()) {
			cMatrixf mtxRoot = cMatrixf::Identity;
			CalcLocalMatrixRec(BoneIt.Next(), mtxRoot, 0);
		}
	}

	////////////////////////////////////
	// Check for joints or several bodies.
	bool bHasSeveralBodies = false;

	tString sColliderGroup = "";
	bool bFoundCollider = false;
	tColladaNodeListIt nodeIt = ColladaScene.mlstNodes.begin();
	for (; nodeIt != ColladaScene.mlstNodes.end(); ++nodeIt) {
		cColladaNode *pNode = *nodeIt;

		// Check if it is a joint
		if (cString::ToLowerCase(cString::Sub(pNode->msName, 0, 6)) == "_joint") {
			bHasSeveralBodies = true;
			break;
		}
		// Check if it is a collider. In that case check if this is a new group name.
		if (cString::ToLowerCase(cString::Sub(pNode->msName, 0, 9)) == "_collider") {
			tString sGroup = "";
			if (pNode->pParent)
				sGroup = pNode->pParent->msName;

			if (bFoundCollider == false) {
				bFoundCollider = true;
				sColliderGroup = sGroup;
			} else if (sGroup != sColliderGroup) {
				bHasSeveralBodies = true;

				break;
			}
		}
	}

	////////////////////////////////////
	// Create Mesh
	tString sMeshName = cString::GetFileName(asFile);
	cMesh *pMesh = hplNew(cMesh, (sMeshName, mpMaterialManager, mpAnimationManager));

	// Set the skeleton to the mesh
	if (pSkeleton)
		pMesh->SetSkeleton(pSkeleton);

	// Create Sub meshes
	for (int i = 0; i < (int)vColladaGeometries.size(); i++) {
		cColladaGeometry &Geom = vColladaGeometries[i];

		cColladaNode *pGeomNode = ColladaScene.GetNodeFromSource(Geom.msId);
		if (pGeomNode == NULL) {
			pGeomNode = GetNodeFromController(Geom.msId, vColladaControllers, ColladaScene);
			if (pGeomNode == NULL) {
				Error("No node with geometry id '%s'\n", Geom.msId.c_str());
				continue;
			}
		}
		tString sNodeName = pGeomNode->msName;

		/////////////////////////////////////////////////////
		// If the name starts with '_' it is a special object.
		if (sNodeName[0] == '_') {
			tStringVec vStrings;
			tString sSepp = "_";
			cString::GetStringVec(sNodeName, vStrings, &sSepp);

			/////////////////////////////////////
			// JOINT
			if (cString::ToLowerCase(vStrings[0]) == "joint" && vStrings.size() > 1) {
				tFloatVec vVertexVec;
				tVertexVec &vArray = Geom.mvVertexVec;
				vVertexVec.resize(vArray.size() * 3);

				for (size_t vtx = 0; vtx < vArray.size(); ++vtx) {
					vVertexVec[vtx * 3 + 0] = vArray[vtx].pos.x;
					vVertexVec[vtx * 3 + 1] = vArray[vtx].pos.y;
					vVertexVec[vtx * 3 + 2] = vArray[vtx].pos.z;
				}

				cBoundingVolume TempBV;
				TempBV.AddArrayPoints(&vVertexVec[0], (int)vArray.size());
				TempBV.CreateFromPoints(3);

				cColladaNode *pNode = ColladaScene.GetNodeFromSource(Geom.msId);
				if (pNode == NULL) {
					Warning("No node for geometry '%s' found when creating joint!\n", Geom.msId.c_str());
					continue;
				}

				tString sJointType = cString::ToLowerCase(vStrings[1]);
				ePhysicsJointType JointType;
				if (sJointType == "hinge")
					JointType = ePhysicsJointType_Hinge;
				else if (sJointType == "ball")
					JointType = ePhysicsJointType_Ball;
				else if (sJointType == "slider")
					JointType = ePhysicsJointType_Slider;
				else if (sJointType == "screw")
					JointType = ePhysicsJointType_Screw;
				else
					error("Unknown JointType: %s", sJointType.c_str());

				cMeshJoint *pJoint = pMesh->CreatePhysicsJoint(JointType);

				CreateMeshJoint(pJoint, JointType, TempBV, vStrings, pNode, ColladaScene, vColladaGeometries);
			}

			/////////////////////////////////////
			// COLLIDER
			else if (cString::ToLowerCase(vStrings[0]) == "collider" && vStrings.size() > 1) {
				tFloatVec vVertexVec;
				tVertexVec &vArray = Geom.mvVertexVec;
				vVertexVec.resize(vArray.size() * 3);

				for (size_t vtx = 0; vtx < vArray.size(); ++vtx) {
					vVertexVec[vtx * 3 + 0] = vArray[vtx].pos.x;
					vVertexVec[vtx * 3 + 1] = vArray[vtx].pos.y;
					vVertexVec[vtx * 3 + 2] = vArray[vtx].pos.z;
				}

				cBoundingVolume TempBV;
				TempBV.AddArrayPoints(&vVertexVec[0], (int)vArray.size());
				TempBV.CreateFromPoints(3);

				tString sShapeType = cString::ToLowerCase(vStrings[1]);
				eCollideShapeType ShapeType = eCollideShapeType_Box;
				cVector3f vShapeSize = TempBV.GetSize();

				if (sShapeType == "box") {
					ShapeType = eCollideShapeType_Box;
				} else if (sShapeType == "sphere") {
					ShapeType = eCollideShapeType_Sphere;
					vShapeSize *= cVector3f(0.5f);
				} else if (sShapeType == "capsule") {
					ShapeType = eCollideShapeType_Capsule;
					vShapeSize.x *= 0.5;
				} else if (sShapeType == "cylinder") {
					ShapeType = eCollideShapeType_Cylinder;
					vShapeSize.x *= 0.5;
				}

				cMeshCollider *pCollider = pMesh->CreateCollider(ShapeType);
				pCollider->mvSize = vShapeSize;

				cColladaNode *pNode = ColladaScene.GetNodeFromSource(Geom.msId);
				if (pNode == NULL) {
					Warning("No node for geometry '%s' when creating collider!\n", Geom.msId.c_str());
					continue;
				}

				// Set the name of the group it is in.
				if (pNode->pParent) {
					pCollider->msGroup = pNode->pParent->msName;
				} else {
					pCollider->msGroup = "";
				}

				// Set offset, some primitives are created with the centre at the bottom,
				// fix this.
				// Check if the centre is not in the middle
				TempBV.SetPosition(pNode->m_mtxWorldTransform.GetTranslation());
				if (TempBV.GetWorldCenter() != pNode->m_mtxWorldTransform.GetTranslation()) {
					cVector3f vOffset = TempBV.GetWorldCenter() -
										pNode->m_mtxWorldTransform.GetTranslation();
					vOffset = vOffset * pNode->mvScale;
					// Log("Centre is not a correct location! Offset: %s\n",vOffset.ToString().c_str());

					// Local postion add
					/*cMatrixf mtxTrans = cMath::MatrixTranslate(vOffset);
					pCollider->m_mtxOffset = cMath::MatrixMul( pNode->m_mtxWorldTransform,
																mtxTrans);*/

					// World postion add
					pCollider->m_mtxOffset = pNode->m_mtxWorldTransform;
					cVector3f vRotOffset = cMath::MatrixMul(pCollider->m_mtxOffset.GetRotation(),
															vOffset);
					pCollider->m_mtxOffset.SetTranslation(pCollider->m_mtxOffset.GetTranslation() +
														  vRotOffset);
				} else {
					pCollider->m_mtxOffset = pNode->m_mtxWorldTransform;
				}

				// Add scale
				pCollider->mvSize = pCollider->mvSize * pNode->mvScale;

				// Log("Collider scale: %s\n",pNode->mvScale.ToString().c_str());
				// Log("Collider size: %s\n",pCollider->mvSize.ToString().c_str());

				// This is to orient the cylinder along y axis instead of x.
				if (ShapeType == eCollideShapeType_Cylinder || ShapeType == eCollideShapeType_Capsule) {
					pCollider->m_mtxOffset = cMath::MatrixMul(pCollider->m_mtxOffset,
															  cMath::MatrixRotateZ(cMath::ToRad(90)));
				}

				// Log("Creating Collider %s, type: %d size: %s\n with matrix: %s\n", Geom.msName.c_str(),(int)ShapeType,
				//									pCollider->mvSize.ToString().c_str(),
				//									pCollider->m_mtxOffset.ToString().c_str());
			}

			///// BILLBOARD /////////////////////////////
			else if (cString::ToLowerCase(vStrings[0]) == "bb") {
				if (vStrings.size() < 3) {
					Error("Too few params in billboard entity '%s'\n", sNodeName.c_str());
				} else {
					tString sName = vStrings[vStrings.size() - 1];
					tString sFile = "";
					for (size_t i2 = 1; i2 < vStrings.size() - 1; ++i2) {
						sFile += vStrings[i2];
						if (i2 != vStrings.size() - 2)
							sFile += "_";
					}

					cMeshBillboard *pBillboard = pMesh->CreateBillboard();

					pBillboard->msName = sName;

					pBillboard->msParent = GetParentName(pGeomNode, &vColladaGeometries);
					pBillboard->msFile = cString::SetFileExt(sFile, "bnt");
					pBillboard->mvSize = cVector2f(pGeomNode->mvScale.x, pGeomNode->mvScale.y);
					pBillboard->mfOffset = pGeomNode->mvScale.z;
					pBillboard->mvPosition = pGeomNode->m_mtxTransform.GetTranslation();
					pBillboard->mvAxis = cMath::Vector3Normalize(cMath::MatrixInverse(pGeomNode->m_mtxWorldTransform).GetUp());
					FixLocalPosition(&pBillboard->mvPosition, pGeomNode, vColladaAnimations, pSkeleton, bHasSeveralBodies);
				}
			}
			///// BEAM /////////////////////////////
			else if (cString::ToLowerCase(vStrings[0]) == "beam") {
				if (vStrings.size() < 3) {
					Error("Too few params in billboard entity '%s'\n", sNodeName.c_str());
				} else {
					tString sName = vStrings[vStrings.size() - 1];
					tString sFile = "";
					for (size_t i2 = 1; i2 < vStrings.size() - 1; ++i2) {
						sFile += vStrings[i2];
						if (i2 != vStrings.size() - 2)
							sFile += "_";
					}

					tString sEndName = "_beamend_" + sName;
					cColladaNode *pEndNode = ColladaScene.GetNode(sEndName);

					if (pEndNode) {
						cMeshBeam *pBeam = pMesh->CreateBeam();

						pBeam->msName = sName;
						pBeam->msFile = sFile;

						pBeam->mvStartPosition = pGeomNode->m_mtxTransform.GetTranslation();
						pBeam->mvEndPosition = pEndNode->m_mtxTransform.GetTranslation();

						pBeam->msStartParent = GetParentName(pGeomNode, &vColladaGeometries);
						pBeam->msEndParent = GetParentName(pEndNode, &vColladaGeometries);
					} else {
						Error("Couldn't find beam end '%s'!\n", sEndName.c_str());
					}
				}
			}
			///// PARTICLE SYSTEM /////////////////////////////
			else if (cString::ToLowerCase(vStrings[0]) == "ps") {
				if (vStrings.size() < 3) {
					Error("Too few params in particle system entity '%s'\n", sNodeName.c_str());
				} else {
					tString sName = vStrings[vStrings.size() - 1];
					tString sType = "";
					for (size_t i2 = 1; i2 < vStrings.size() - 1; ++i2) {
						sType += vStrings[i2];
						if (i2 != vStrings.size() - 2)
							sType += "_";
					}

					cMeshParticleSystem *pPS = pMesh->CreateParticleSystem();

					pPS->msName = sName;
					pPS->msParent = GetParentName(pGeomNode, &vColladaGeometries);
					pPS->msType = sType;
					pPS->mvSize = pGeomNode->mvScale;
					pPS->m_mtxTransform = pGeomNode->m_mtxTransform;
					FixLocalTransform(&pPS->m_mtxTransform, pGeomNode, vColladaAnimations, pSkeleton, bHasSeveralBodies);
				}
			}
			///// REFERENCE /////////////////////////////
			else if (cString::ToLowerCase(vStrings[0]) == "ref") {
				if (vStrings.size() < 3) {
					Error("Too few params in ref entity '%s'\n", sNodeName.c_str());
				} else {
					tString sFile = "";
					for (size_t i2 = 1; i2 < vStrings.size() - 1; ++i2) {
						sFile += vStrings[i2];
						if (i2 != vStrings.size() - 2)
							sFile += "_";
					}
					tString sName = vStrings[vStrings.size() - 1];

					cMeshReference *pRef = pMesh->CreateReference();

					pRef->msName = sName;
					pRef->msFile = sFile;
					pRef->msParent = GetParentName(pGeomNode, &vColladaGeometries);
					pRef->m_mtxTransform = pGeomNode->m_mtxTransform;
					FixLocalTransform(&pRef->m_mtxTransform, pGeomNode, vColladaAnimations, pSkeleton, bHasSeveralBodies);
				}
			}
			///// SOUND ENTITY /////////////////////////////
			else if (cString::ToLowerCase(vStrings[0]) == "sound") {
				if (vStrings.size() < 3) {
					Error("Too few params in sound entity '%s'\n", sNodeName.c_str());
				} else {
					tString sType = "";
					for (size_t i2 = 1; i2 < vStrings.size() - 1; ++i2) {
						sType += vStrings[i2];
						if (i2 != vStrings.size() - 2)
							sType += "_";
					}
					tString sName = vStrings[vStrings.size() - 1];

					cMeshSoundEntity *pSound = pMesh->CreateSoundEntity();

					pSound->msName = sName;
					pSound->msParent = GetParentName(pGeomNode, &vColladaGeometries);
					pSound->msType = sType;
					pSound->mvPosition = pGeomNode->m_mtxTransform.GetTranslation();
					FixLocalPosition(&pSound->mvPosition, pGeomNode, vColladaAnimations, pSkeleton, bHasSeveralBodies);
				}
			}

			continue;
		}

		tString sSubMeshName = Geom.msName == "" ? Geom.msId : Geom.msName;
		cSubMesh *pSubMesh = pMesh->CreateSubMesh(sSubMeshName);

		// Getting group the sub mesh is in
		cColladaNode *pNode = ColladaScene.GetNodeFromSource(Geom.msId);
		if (pNode == NULL) {
			pNode = GetNodeFromController(Geom.msId, vColladaControllers, ColladaScene);
			if (pNode == NULL) {
				Warning("No node for geometry '%s' found when trying to create sub mesh, check in controllers.\n", Geom.msId.c_str());
				continue;
			}
		}

		pSubMesh->SetNodeName(pNode->msName);
		pSubMesh->SetLocalTransform(pNode->m_mtxTransform);
		if (pNode->pParent) {
			pSubMesh->SetGroup(pNode->pParent->msName);
		}

		// Set the scale
		pSubMesh->SetModelScale(pNode->mvScale);

		// Log("Creating submesh: '%s'\n",Geom.msName.c_str());

		// To be filled by extra vertex positions (not used, use the one in Geometry)
		tColladaExtraVtxListVec &vExtraVtxVec = Geom.mvExtraVtxVec;

		//////////////////////////////
		// Create Vertex buffer
		eVertexBufferUsageType UsageType = eVertexBufferUsageType_Static;
		// The streams will be the entity copies.
		// if(vColladaControllers.empty()==false) UsageType = eVertexBufferUsageType_Stream;

		iVertexBuffer *pVtxBuffer = CreateVertexBuffer(Geom, UsageType); //, vExtraVtxVec);
		pSubMesh->SetVertexBuffer(pVtxBuffer);

		// Create an extra set of vertices for shadow rendering
		pVtxBuffer->CreateShadowDouble(true);

		/////////////////////////////
		// Add material
		tString sMatName = GetMaterialTextureFile(Geom.msMaterial, vColladaMaterials, vColladaTextures,
												  vColladaImages);
		// Log("Material name: '%s'\n",sMatName.c_str());
		iMaterial *pMaterial = mpMaterialManager->CreateMaterial(sMatName);
		if (pMaterial == NULL) {
			Error("Couldn't create material '%s' for object '%s'\n", sMatName.c_str(),
				  Geom.msName.c_str());
			continue;
		}
		pSubMesh->SetMaterial(pMaterial);

		/////////////////////////////
		// If there is a controller for the mesh, get vertex-bone pairs.
		// First find the controller.
		cColladaController *pCtrl = NULL;
		for (size_t j = 0; j < vColladaControllers.size(); j++) {
			if (vColladaControllers[j].msTarget == Geom.msId) {
				pCtrl = &vColladaControllers[j];
				break;
			}
		}
		// If no controller can be found, guess the one with target=""
		if (pCtrl == NULL) {
			for (size_t j = 0; j < vColladaControllers.size(); j++) {
				if (vColladaControllers[j].msTarget == "") {
					pCtrl = &vColladaControllers[j];
					Warning("Guessing controller as target=''!\n");
					break;
				}
			}
		}

		// Add the pairs
		if (pCtrl && pSkeleton) {
			// Log("Adding vertex-bone pairs!\n");
			// Iterate the pairs
			for (size_t j = 0; j < pCtrl->mvPairs.size(); j++) {
				// Get all vertices for this vertex pos
				tColladaExtraVtxListIt ExtraIt = vExtraVtxVec[j].begin();
				for (; ExtraIt != vExtraVtxVec[j].end(); ++ExtraIt) {
					cColladaExtraVtx &Extra = *ExtraIt;

					// Iterate all the influences for this vertex.
					tColladaJointPairListIt PairIt = pCtrl->mvPairs[j].begin();
					for (; PairIt != pCtrl->mvPairs[j].end(); ++PairIt) {
						cColladaJointPair &SrcPair = *PairIt;
						cVertexBonePair DestPair;

						int lBoneIdx = pSkeleton->GetBoneIndexByName(pCtrl->mvJoints[SrcPair.mlJoint]);
						DestPair.boneIdx = lBoneIdx;
						DestPair.weight = pCtrl->mvWeights[SrcPair.mlWeight];
						DestPair.vtxIdx = Extra.mlNewVtx;

						// Add pair in sub mesh
						pSubMesh->AddVertexBonePair(DestPair);

						// Log("Added pair: bone %d vtx %d weight: %f\n", DestPair.boneIdx,DestPair.vtxIdx,
						//												DestPair.weight);
					}
				}
			}

			// Compile the pairs into a more friendly format.
			pSubMesh->CompileBonePairs();

			// Transform the mesh according to the bind transform.
			pVtxBuffer->Transform(pCtrl->m_mtxBindShapeMatrix);
		}
		//////////////////////////////////////////////////
		// No controller, we have a non skinned mesh!
		else {
			// We have an animated non-skinned mesh!
			// Need to add the node hierarchy
			if (vColladaAnimations.empty() == false || bHasSeveralBodies) {
				// If the buffer has animations or joints, we only add the scale part of the
				// transform
				cMatrixf mtxScale = cMath::MatrixScale(pNode->mvScale);
				pVtxBuffer->Transform(mtxScale);

				// Log("Scaling %s: (%s) %s!\n",pNode->msName.c_str(),pNode->mvScale.ToString().c_str(),mtxScale.ToString().c_str());
			}
			// We have a normal mesh without any geometry and NO joints
			else if (bHasSeveralBodies == false) {
				// There are no bones so transform the vertex buffer by the nodes world transform.
				cColladaNode *ptNode = ColladaScene.GetNodeFromSource(Geom.msId);

				if (ptNode) {
					pVtxBuffer->Transform(ptNode->m_mtxWorldTransform);
					// Log("Transforming buffer %s\n",pNode->msName.c_str());
				} else {
					Warning("No node for geometry '%s' found when trying to transform geometry!\n", Geom.msId.c_str());
				}
			}
		}

		// Create triangle data for the geometry
		cMath::CreateTriangleData(*pSubMesh->GetTriangleVecPtr(),
								  pVtxBuffer->GetIndices(), pVtxBuffer->GetIndexNum(),
								  pVtxBuffer->GetArray(eVertexFlag_Position),
								  kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)],
								  pVtxBuffer->GetVertexNum());

		// Create edges for the geometry.
		bool bDoubleSided = false;
		cMath::CreateEdges(*pSubMesh->GetEdgeVecPtr(),
						   pVtxBuffer->GetIndices(), pVtxBuffer->GetIndexNum(),
						   pVtxBuffer->GetArray(eVertexFlag_Position),
						   kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)],
						   pVtxBuffer->GetVertexNum(),
						   &bDoubleSided);
		pSubMesh->SetDoubleSided(bDoubleSided);

		pSubMesh->Compile();
	}

	///////////////////////////////////////////////
	// Add Node Hierarchy
	// only add if there are animations and no skeleton.
	if ((vColladaAnimations.empty() == false || bHasSeveralBodies) && !pSkeleton) {
		cNode3D *pRootNode = pMesh->GetRootNode();

		tColladaNodeListIt it = ColladaScene.mRoot.mlstChildren.begin();
		for (; it != ColladaScene.mRoot.mlstChildren.end(); ++it) {
			CreateHierarchyNodes(pMesh, pRootNode, *it, vColladaGeometries);
		}

		// Clear scale on all joint nodes.
		if (bHasSeveralBodies) {
			for (int i = 0; i < pMesh->GetNodeNum(); i++) {
				cNode3D *pMeshNode = pMesh->GetNode(i);
				cMatrixf mtxNode = pMeshNode->GetLocalMatrix();

				cSubMesh *pSubMesh = pMesh->GetSubMeshName(pMeshNode->GetSource());

				if (pSubMesh) {
					// Clear the scale
					cMatrixf mtxScale = cMath::MatrixScale(pSubMesh->GetModelScale());
					mtxNode = cMath::MatrixMul(mtxNode, cMath::MatrixInverse(mtxScale));
				} else if (tString(pMeshNode->GetSource()) != "" && pMeshNode->GetSource()[0] != '_') {
					Error("Cannot find submesh '%s'\n", pMeshNode->GetSource());
				}

				pMeshNode->SetMatrix(mtxNode);
			}
		}
	}

	// Shouldn't be needed any more.
	// vColladaGeometries[i].Clear();

	///////////////////////////////////////////////
	// Create Lights
	for (int i = 0; i < (int)vColladaLights.size(); i++) {
		////////////////////////////////////////
		// Get the Light and Node
		cColladaLight &ColladaLight = vColladaLights[i];

		cColladaNode *pLightNode = ColladaScene.GetNodeFromSource(ColladaLight.msId);
		if (pLightNode == NULL) {
			Error("Couldn't find node for light '%s'\n", ColladaLight.msId.c_str());
			continue;
		}

		tStringVec vParams;
		tString sSepp = "_";
		cString::GetStringVec(pLightNode->msName, vParams, &sSepp);
		tString sLightName = "";
		tString sLightFile = "";

		/////////////////////////////////////
		// Check if there is any light entity file and get name.

		// file parameter
		if ((int)vParams.size() >= 2) {
			for (size_t i2 = 0; i2 < vParams.size() - 1; ++i2) {
				sLightFile += vParams[i2];
				if (i2 != vParams.size() - 2)
					sLightFile += "_";
			}
			sLightFile = cString::SetFileExt(sLightFile, "lnt");

			sLightName = vParams[vParams.size() - 1];
		}
		// No file parameter
		else {
			sLightName = vParams[0];
		}

		cMeshLight *pMeshLight = pMesh->CreateLight(eLight3DType_Point);

		pMeshLight->m_mtxTransform = pLightNode->m_mtxTransform;

		pMeshLight->msParent = GetParentName(pLightNode, &vColladaGeometries);

		pMeshLight->mColor = ColladaLight.mDiffuseColor;
		pMeshLight->mfFOV = cMath::ToRad(ColladaLight.mfAngle);
		pMeshLight->mfRadius = pLightNode->mvScale.x;

		if (ColladaLight.msType == "point") {
			pMeshLight->mType = eLight3DType_Point;
			pMeshLight->mColor.a = pLightNode->mvScale.y;
			pMeshLight->mbCastShadows = pLightNode->mvScale.z < 0.1f ? false : true;
		} else {
			pMeshLight->mType = eLight3DType_Spot;
		}

		pMeshLight->msName = sLightName;
		pMeshLight->msFile = sLightFile;

		//////////////////////////////////
		// Fix the transform
		FixLocalTransform(&pMeshLight->m_mtxTransform, pLightNode, vColladaAnimations, pSkeleton, bHasSeveralBodies);
	}

	///////////////////////////////////////////////
	// Create Animations
	cAnimation *pAnimation = NULL;
	if (vColladaAnimations.empty() == false) {
		pAnimation = hplNew(cAnimation, (pMesh->GetName(), cString::GetFileName(asFile)));

		pAnimation->SetAnimationName("Default");
		pAnimation->SetLength(ColladaScene.mfDeltaTime);
		pAnimation->ResizeTracks((int)vColladaAnimations.size());

		pMesh->AddAnimation(pAnimation);

		///////////////////////////////////////////////////
		// Go through all tracks and add them to the animation
		for (size_t i = 0; i < vColladaAnimations.size(); i++) {
			cAnimationTrack *pTrack = CreateAnimTrack(pAnimation, pSkeleton,
													  vColladaAnimations[i], &ColladaScene);
			if (pTrack == NULL)
				continue;
			if (pSkeleton) {
				// cBone *pBone = pSkeleton->GetBoneByName(pTrack->GetName());
				int lBoneIdx = pSkeleton->GetBoneIndexByName(pTrack->GetName());
				pTrack->SetNodeIndex(lBoneIdx);
			} else {
				// Find sub mesh.
				pTrack->SetNodeIndex(-1);
			}
		}

		/////////////////////////////////////////////////////
		// Go through all bones and check if they have an animation
		// If not create single frame with the node pose.
		if (pSkeleton) {
			for (int i = 0; i < pSkeleton->GetBoneNum(); ++i) {
				cBone *pBone = pSkeleton->GetBoneByIndex(i);

				if (pAnimation->GetTrackByName(pBone->GetName()) == NULL) {
					cColladaNode *pNode = ColladaScene.GetNode(pBone->GetName());
					if (pNode) {
						cMatrixf mtxLocal = pNode->m_mtxTransform;
						cMatrixf mtxInvBone = cMath::MatrixInverse(pBone->GetLocalTransform());

						cMatrixf mtxChange = cMath::MatrixMul(mtxInvBone, mtxLocal);

						// Log("Bone %s change mtx: %s\n",pBone->GetName().c_str(),
						//								mtxChange.ToString().c_str());

						cAnimationTrack *pTrack = pAnimation->CreateTrack(pBone->GetName(), eAnimTransformFlag_Rotate | eAnimTransformFlag_Translate | eAnimTransformFlag_Scale);
						pTrack->SetNodeIndex(i);
						cKeyFrame *pFrame = pTrack->CreateKeyFrame(0);

						pFrame->trans = mtxChange.GetTranslation();
						pFrame->scale = 1;
						pFrame->rotation = cQuaternion::Identity;
						// Get the quaternion from the rotation matrix
						mtxChange.SetTranslation(0);
						pFrame->rotation.FromRotationMatrix(mtxChange);
					} else {
						Warning("Couldn't find node for bone '%s'\n", pBone->GetName().c_str());
					}
				}
			}
		}
	}

	///////////////////////////////////
	///////// DEBUG ///////////////////
	///////////////////////////////////
	// Test anim data
	/*Log("ANIMATIONS:\n");
	for(size_t i=0; i < vColladaAnimations.size(); i++)
	{
		cColladaAnimation &Anim = vColladaAnimations[i];

		Log("Anim: id: '%s' name: '%s'\n",Anim.msId.c_str(), Anim.msName.c_str());

		Log("Channels:\n");
		for(size_t j=0; j< Anim.mvChannels.size(); j++)
			Log("Id: '%s' Source: '%s' Target: '%s'\n",
															Anim.mvChannels[j].msId.c_str(),
															Anim.mvChannels[j].msSource.c_str(),
															Anim.mvChannels[j].msTarget.c_str());

		Log("Samplers:\n");
		for(size_t j=0; j< Anim.mvSamplers.size(); j++)
			Log("Id: '%s' Target: '%s' Time: '%s' Values: '%s'\n",		Anim.mvSamplers[j].msId.c_str(),
															Anim.mvSamplers[j].msTarget.c_str(),
															Anim.mvSamplers[j].msTimeArray.c_str(),
															Anim.mvSamplers[j].msValueArray.c_str());
		Log("Sources:\n");
		for(size_t j=0; j< Anim.mvSources.size(); j++)
		{
			Log("Id: %s\n",Anim.mvSources[j].msId.c_str());
			for(size_t k=0;k < Anim.mvSources[j].mvValues.size(); k++)
				Log("%f, ",Anim.mvSources[j].mvValues[k]);
			Log("\n");
		}

		//CreateAnimTrack(NULL, Anim,&ColladaScene);
	}*/

	return pMesh;
}

//-----------------------------------------------------------------------

cAnimation *cMeshLoaderCollada::LoadAnimation(const tString &asFile) {

	// Controllers
	tColladaControllerVec vColladaControllers;
	// Animations
	tColladaAnimationVec vColladaAnimations;
	// Scene
	cColladaScene ColladaScene;

	// Fill the structures with collada file data
	bool bRet = FillStructures(asFile, NULL, NULL,
							   NULL, NULL,
							   NULL, &vColladaControllers,
							   &vColladaAnimations,
							   &ColladaScene, true);

	if (bRet == false)
		return NULL;

	////////////////////////
	// Create Skeleton
	cSkeleton *pSkeleton = NULL;
	if (vColladaControllers.empty() == false) {
		pSkeleton = hplNew(cSkeleton, ());

		tColladaNodeListIt it = ColladaScene.mRoot.mlstChildren.begin();
		for (; it != ColladaScene.mRoot.mlstChildren.end(); it++) {
			cColladaNode *pNode = *it;

			CreateSkeletonBone(pNode, pSkeleton->GetRootBone());
		}

		////////////////////////////////////
		// Set the bind position of the bones
		//(This will set the local matrix as the global bind)
		for (size_t i = 0; i < vColladaControllers.size(); i++) {
			cColladaController &Ctrl = vColladaControllers[i];

			for (size_t j = 0; j < Ctrl.mvJoints.size(); j++) {
				cBone *pBone = pSkeleton->GetBoneByName(Ctrl.mvJoints[j]);

				if (pBone) {
					pBone->SetTransform(cMath::MatrixInverse(Ctrl.mvMatrices[j]));
					pBone->SetValue(1);
				} else {
					Log("Bone '%s' does not exist\n", Ctrl.mvJoints[j].c_str());
				}
			}
		}

		////////////////////////////////////////////////
		// Do another pass and calculate the local matrix
		cBoneIterator BoneIt = pSkeleton->GetRootBone()->GetChildIterator();
		while (BoneIt.HasNext()) {
			cMatrixf mtxRoot = cMatrixf::Identity;
			CalcLocalMatrixRec(BoneIt.Next(), mtxRoot, 0);
		}
	}

	///////////////////////////////////////////////
	// Create Animations
	cAnimation *pAnimation = NULL;
	if (vColladaAnimations.empty() == false) {
		pAnimation = hplNew(cAnimation, ("Default", cString::GetFileName(asFile)));

		pAnimation->SetLength(ColladaScene.mfDeltaTime);
		pAnimation->ResizeTracks((int)vColladaAnimations.size());

		for (size_t i = 0; i < vColladaAnimations.size(); i++) {
			cAnimationTrack *pTrack = CreateAnimTrack(pAnimation, pSkeleton,
													  vColladaAnimations[i], &ColladaScene);
			if (pTrack == NULL)
				continue;
			if (pSkeleton) {
				// cBone *pBone = pSkeleton->GetBoneByName(pTrack->GetName());
				int lBoneIdx = pSkeleton->GetBoneIndexByName(pTrack->GetName());
				pTrack->SetNodeIndex(lBoneIdx);
			} else {
				// Find sub mesh.
				pTrack->SetNodeIndex(-1);
			}
		}
	}

	if (pSkeleton)
		hplDelete(pSkeleton);

	return pAnimation;
}

//-----------------------------------------------------------------------

bool cMeshLoaderCollada::IsSupported(const tString asFileType) {
	if (asFileType == "dae")
		return true;

	return false;
}

//-----------------------------------------------------------------------

void cMeshLoaderCollada::AddSupportedTypes(tStringVec *avFileTypes) {
	avFileTypes->push_back("dae");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
/////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

tString cMeshLoaderCollada::GetParentName(cColladaNode *apNode, tColladaGeometryVec *apColladaGeometries) {
	tString sParent = "";
	if (apNode->pParent) {
		sParent = apNode->pParent->msName;
		// If the parent is a geometry the geometry name must be used.
		if (apNode->pParent->msSource != "") {
			cColladaGeometry *pGeom = GetGeometry(apNode->pParent->msSource, *apColladaGeometries);
			if (pGeom) {
				sParent = pGeom->msName;
			}
		}
	}

	return sParent;
}

//-----------------------------------------------------------------------

static bool HasParam(const tStringVec &avVec, const tString &asParam) {
	for (int i = 0; i < (int)avVec.size(); i++) {
		if (cString::ToLowerCase(avVec[i]) == asParam) {
			return true;
		}
	}

	return false;
}
//-----------------------------------------------------------------------

void cMeshLoaderCollada::CreateMeshJoint(cMeshJoint *apJoint, ePhysicsJointType aJointType,
										 cBoundingVolume &aBV,
										 tStringVec &avStrings, cColladaNode *apNode,
										 cColladaScene &aColladaScene,
										 tColladaGeometryVec &avColladaVec) {

	//////////////////////////////////////////////
	// GENERAL

	if (avStrings.size() < 5) {
		Error("Joint node '%s' has to few args!\n", apNode->msName.c_str());
	}

	apJoint->msName = avStrings[avStrings.size() - 1];

	//////////////////////////////////////
	// Get if the joint bodies should collide
	if (HasParam(avStrings, "nocollide")) {
		apJoint->mbCollide = false;
	} else {
		apJoint->mbCollide = true;
	}

	//////////////////////////////////////
	// Get parent and child body
	apJoint->mvPivot = apNode->m_mtxWorldTransform.GetTranslation();
	apJoint->mvPinDir = cMath::Vector3Normalize(apNode->m_mtxWorldTransform.GetUp());

	cColladaNode *pCNode = aColladaScene.GetNode(avStrings[2]);
	cColladaNode *pPNode = aColladaScene.GetNode(avStrings[3]);

	if (pCNode) {
		cColladaGeometry *pGeom = GetGeometry(pCNode->msSource, avColladaVec);
		if (pGeom) {
			apJoint->msChildBody = pGeom->msName;
		} else {
			Warning("Geometry for joint child '%s' is missing! Might be connected to bone.\n", pCNode->msSource.c_str());
			apJoint->msChildBody = pCNode->msName;
		}
	} else {
		Error("Child node '%s' is missing for joint '%s'!\n", avStrings[2].c_str(),
			  apJoint->msName.c_str());
		return;
	}

	if (pPNode) {
		cColladaGeometry *pGeom = GetGeometry(pPNode->msSource, avColladaVec);
		if (pGeom) {
			apJoint->msParentBody = pGeom->msName;
		} else {
			Warning("Geometry  joint parten '%s' is missing! Might be connected to bone.\n", pPNode->msSource.c_str());
			apJoint->msParentBody = pPNode->msName;
		}
	}

	//////////////////////////////////////
	// Get min and max values.

	if ((aJointType == ePhysicsJointType_Slider || aJointType == ePhysicsJointType_Screw) && avStrings.size() <= 6) {
		apJoint->mfMax = aBV.GetMax().y;
		apJoint->mfMin = aBV.GetMin().y;

		if (apJoint->mfMin > 0)
			apJoint->mfMin = 0;
		if (apJoint->mfMax < 0)
			apJoint->mfMax = 0;

		// Log("Min: %f Max: %f\n",apJoint->mfMin,apJoint->mfMax);
	} else {
		if (avStrings.size() >= 7) {
			apJoint->mfMin = cString::ToFloat(avStrings[4].c_str(), 0);
			apJoint->mfMax = cString::ToFloat(avStrings[5].c_str(), 0);

			if (aJointType == ePhysicsJointType_Slider || aJointType == ePhysicsJointType_Screw) {
				apJoint->mfMin = -apJoint->mfMin / 100.0f;
				apJoint->mfMax = apJoint->mfMax / 100.0f;
			}
		} else {
			apJoint->mfMin = 0;
			apJoint->mfMax = 0;
		}
	}
}

//-----------------------------------------------------------------------

void cMeshLoaderCollada::CreateHierarchyNodes(cMesh *apMesh, cNode3D *mpParentNode,
											  cColladaNode *apColladaNode,
											  tColladaGeometryVec &avColladaGeom) {
	cNode3D *pNode = mpParentNode->CreateChild3D(apColladaNode->msName);
	apMesh->AddNode(pNode);

	pNode->SetMatrix(apColladaNode->m_mtxTransform);

	// Log("Node: %s\n",apColladaNode->msName.c_str());
	// Log(" local node transform: %s\n",pNode->GetLocalPosition().ToString().c_str());
	// Log(" local collada transform: %s\n",apColladaNode->m_mtxTransform.GetTranslation().ToString().c_str());
	// Log(" world transform: %s\n",apColladaNode->m_mtxWorldTransform.GetTranslation().ToString().c_str());

	pNode->SetPosition(pNode->GetLocalPosition());

	// Get source name
	if (apColladaNode->msSource != "") {
		for (int i = 0; i < (int)avColladaGeom.size(); i++) {
			if (avColladaGeom[i].msId == apColladaNode->msSource) {
				// Log("Setting source for %s to %s\n",pNode->GetName(), avColladaGeom[i].msName.c_str());
				pNode->SetSource(avColladaGeom[i].msName);
				break;
			}
		}
	}

	// Iterate children
	tColladaNodeListIt it = apColladaNode->mlstChildren.begin();
	for (; it != apColladaNode->mlstChildren.end(); ++it) {
		CreateHierarchyNodes(apMesh, pNode, *it, avColladaGeom);
	}
}

//-----------------------------------------------------------------------

cColladaGeometry *cMeshLoaderCollada::GetGeometry(const tString &asId, tColladaGeometryVec &avGeomVec) {
	for (size_t i = 0; i < avGeomVec.size(); i++) {
		if (avGeomVec[i].msId == asId) {
			return &avGeomVec[i];
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------

cColladaLight *cMeshLoaderCollada::GetLight(const tString &asId, tColladaLightVec &avLightVec) {
	for (size_t i = 0; i < avLightVec.size(); i++) {
		if (avLightVec[i].msId == asId) {
			return &avLightVec[i];
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------

cMeshEntity *cMeshLoaderCollada::CreateStaticMeshEntity(cColladaNode *apNode, cWorld3D *apWorld,
														cColladaGeometry *apGeom, bool abInRoomGroup,
														tColladaMaterialVec &avColladaMaterials,
														tColladaTextureVec &avColladaTextures,
														tColladaImageVec &avColladaImages) {
	// Get the settings "in the name"
	tStringVec vParams;
	tString sSepp = "_";
	cString::GetStringVec(apNode->msName, vParams, &sSepp);

	cMeshEntity *pEntity = NULL;
	iVertexBuffer *pVtxBuffer = NULL;

	tString sMatName = GetMaterialTextureFile(apGeom->msMaterial,
											  avColladaMaterials, avColladaTextures,
											  avColladaImages);

	cMesh *pMesh = NULL;
	cSubMesh *pSubMesh = NULL;

	bool bDrawn = false;

	////////////////////////////////////////////
	// Check if the mesh is drawn
	if (HasParam(vParams, "nodraw") == false) {
		bDrawn = true;

		// Create mesh and sub mesh
		pMesh = hplNew(cMesh, (apNode->msName, mpMaterialManager, mpAnimationManager));
		pSubMesh = pMesh->CreateSubMesh(apNode->msName + "_Sub");

		// Create vertex buffer
		// tColladaExtraVtxListVec vExtraVtxVec;
		pVtxBuffer = CreateVertexBuffer(*apGeom, eVertexBufferUsageType_Static); ///,&apNode->m_mtxWorldTransform);

		// Check if If the mesh casts shadows:
		pVtxBuffer->CreateShadowDouble(true);

		// Transform vertex buffer with world transform
		pVtxBuffer->Transform(apNode->m_mtxWorldTransform);
		// Log("%s transform: (%s)\n", apNode->msName.c_str(), apNode->m_mtxWorldTransform.ToString().c_str());

		pSubMesh->SetVertexBuffer(pVtxBuffer);

		// Create triangle data for the geometry
		cMath::CreateTriangleData(*pSubMesh->GetTriangleVecPtr(),
								  pVtxBuffer->GetIndices(), pVtxBuffer->GetIndexNum(),
								  pVtxBuffer->GetArray(eVertexFlag_Position),
								  kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)],
								  pVtxBuffer->GetVertexNum());

		// Create edges for the geometry.
		bool bDoubleSided = false;
		cMath::CreateEdges(*pSubMesh->GetEdgeVecPtr(),
						   pVtxBuffer->GetIndices(), pVtxBuffer->GetIndexNum(),
						   pVtxBuffer->GetArray(eVertexFlag_Position),
						   kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)],
						   pVtxBuffer->GetVertexNum(),
						   &bDoubleSided);

		pSubMesh->SetDoubleSided(bDoubleSided);

		pSubMesh->Compile();

		// Add material
		iMaterial *pMaterial = mpMaterialManager->CreateMaterial(sMatName);
		if (pMaterial == NULL) {
			Error("Couldn't create material '%s' for object '%s'\n", sMatName.c_str(),
				  apNode->msName.c_str());
			hplDelete(pMesh);
			return NULL;
		}

		pSubMesh->SetMaterial(pMaterial);

		// Create mesh entity
		pEntity = apWorld->CreateMeshEntity(apNode->msName, pMesh, false);
		pEntity->SetMatrix(cMatrixf::Identity);
		// pEntity->SetMatrix(apNode->m_mtxWorldTransform); //<- Debug!

		// Set the mesh as static.
		pEntity->SetStatic(true);
		pEntity->SetIsSaved(false);

		if (HasParam(vParams, "noshadow"))
			pEntity->SetCastsShadows(false);
		else
			pEntity->SetCastsShadows(true);
	}

	if (HasParam(vParams, "nocollide") == false) {
		if (bDrawn == false) {
			pVtxBuffer = CreateVertexBuffer(*apGeom, eVertexBufferUsageType_Static);
			pVtxBuffer->Transform(apNode->m_mtxWorldTransform);
		}

		iCollideShape *pShape = apWorld->GetPhysicsWorld()->CreateMeshShape(pVtxBuffer);
		iPhysicsBody *pBody = apWorld->GetPhysicsWorld()->CreateBody(apNode->msName, pShape);

		if (pBody) {
			pBody->SetMass(0);
			pBody->SetIsSaved(false);
			// Log("Created body %s!\n",pBody->GetName().c_str());
		} else {
			Log("Body creation failed!\n");
		}

		// Check if it blocks light
		pBody->SetBlocksLight(true);
		if (bDrawn) {
			if (pEntity->IsShadowCaster() == false ||
				(pSubMesh->GetMaterial() && pSubMesh->GetMaterial()->IsTransperant())) {
				pBody->SetBlocksLight(false);
			}
		} else {
			pBody->SetBlocksLight(false);
		}

		bool bBlocksSound = false;
		if (abInRoomGroup)
			bBlocksSound = true;

		if (HasParam(vParams, "nosoundblock"))
			bBlocksSound = false;
		if (HasParam(vParams, "soundblock"))
			bBlocksSound = true;

		pBody->SetBlocksSound(bBlocksSound);

		if (HasParam(vParams, "nocharcollide"))
			pBody->SetCollideCharacter(false);

		tString sPhysicsMatName = apWorld->GetResources()->GetMaterialManager()->GetPhysicsMaterialName(sMatName);
		iPhysicsMaterial *pPhysicsMat = apWorld->GetPhysicsWorld()->GetMaterialFromName(
			sPhysicsMatName);
		if (pPhysicsMat) {
			pBody->SetMaterial(pPhysicsMat);
		}

		if (bDrawn == false) {
			hplDelete(pVtxBuffer);
		}
	}

	return pEntity;
}

//-----------------------------------------------------------------------

static iCollideShape *CreatePhysicsCollider(iPhysicsWorld *pPhysicsWorld, const cVector3f &avSize,
											eCollideShapeType aShapeType) {
	if (aShapeType == eCollideShapeType_Cylinder) {
		// This is to orient the cylinder along y axis instead of x.
		cMatrixf mtxOffset = cMath::MatrixRotateZ(cMath::ToRad(90));
		return pPhysicsWorld->CreateCylinderShape(avSize.x, avSize.y, &mtxOffset);
	} else if (aShapeType == eCollideShapeType_Capsule) {
		// This is to orient the cylinder along y axis instead of x.
		cMatrixf mtxOffset = cMath::MatrixRotateZ(cMath::ToRad(90));
		return pPhysicsWorld->CreateCapsuleShape(avSize.x, avSize.y, &mtxOffset);
	} else if (aShapeType == eCollideShapeType_Box) {
		return pPhysicsWorld->CreateBoxShape(avSize, NULL);
	} else if (aShapeType == eCollideShapeType_Sphere) {
		return pPhysicsWorld->CreateSphereShape(avSize.x, NULL);
	}

	return NULL;
}

cColliderEntity *cMeshLoaderCollada::CreateStaticCollider(cColladaNode *apNode, cWorld3D *apWorld,
														  cColladaGeometry *apGeom,
														  tColladaMaterialVec &avColladaMaterials,
														  tColladaTextureVec &avColladaTextures,
														  tColladaImageVec &avColladaImages,
														  bool abCharacterCollider) {
	tStringVec vStrings;
	tString sSepp = "_";
	cString::GetStringVec(apNode->msName, vStrings, &sSepp);

	tFloatVec vVertexVec;
	tVertexVec &vArray = apGeom->mvVertexVec;
	vVertexVec.resize(vArray.size() * 3);

	for (size_t vtx = 0; vtx < vArray.size(); ++vtx) {
		vVertexVec[vtx * 3 + 0] = vArray[vtx].pos.x;
		vVertexVec[vtx * 3 + 1] = vArray[vtx].pos.y;
		vVertexVec[vtx * 3 + 2] = vArray[vtx].pos.z;
	}

	cBoundingVolume TempBV;
	TempBV.AddArrayPoints(&vVertexVec[0], (int)vArray.size());
	TempBV.CreateFromPoints(3);

	tString sShapeType = cString::ToLowerCase(vStrings[1]);
	eCollideShapeType ShapeType = eCollideShapeType_Box;
	cVector3f vShapeSize = TempBV.GetSize() * apNode->mvScale;

	if (sShapeType == "box") {
		ShapeType = eCollideShapeType_Box;
	} else if (sShapeType == "sphere") {
		ShapeType = eCollideShapeType_Sphere;
		vShapeSize *= cVector3f(0.5f);
	} else if (sShapeType == "capsule") {
		ShapeType = eCollideShapeType_Capsule;
		vShapeSize.x *= 0.5;
	} else if (sShapeType == "cylinder") {
		ShapeType = eCollideShapeType_Cylinder;
		vShapeSize.x *= 0.5;
	}

	iCollideShape *pCollideShape = NULL;

	pCollideShape = CreatePhysicsCollider(apWorld->GetPhysicsWorld(), vShapeSize, ShapeType);

	if (pCollideShape == NULL) {
		Error("Collider was not created!");
		return NULL;
	}

	// Log("Creating Collider %s, type: %d size: %s\n", apGeom->msName.c_str(),(int)ShapeType,
	//												pCollideShape->GetSize().ToString().c_str());

	// Create body
	iPhysicsBody *pBody = apWorld->GetPhysicsWorld()->CreateBody(apNode->msName, pCollideShape);
	pBody->SetMatrix(apNode->m_mtxWorldTransform);

	// Set light block
	pBody->SetBlocksLight(false);

	// Set material
	tString sMatName = GetMaterialTextureFile(apGeom->msMaterial,
											  avColladaMaterials, avColladaTextures,
											  avColladaImages);
	if (sMatName != "") {
		tString sPhysicsMatName = apWorld->GetResources()->GetMaterialManager()->GetPhysicsMaterialName(sMatName);

		if (sPhysicsMatName != "") {
			iPhysicsMaterial *pPhysicsMat = apWorld->GetPhysicsWorld()->GetMaterialFromName(
				sPhysicsMatName);
			if (pPhysicsMat) {
				pBody->SetMaterial(pPhysicsMat);
			}
		}
	}

	// Check if it blocks sound
	bool bBlocksSound = false;
	if (HasParam(vStrings, "soundblock"))
		bBlocksSound = true;

	pBody->SetBlocksSound(bBlocksSound);
	pBody->SetIsSaved(false);
	pBody->SetCollideCharacter(true);
	if (abCharacterCollider)
		pBody->SetCollide(false);
	else
		pBody->SetCollide(true);

	return apWorld->CreateColliderEntity(apNode->msName, pBody);
}

//-----------------------------------------------------------------------

void cMeshLoaderCollada::AddSceneObjects(cColladaNode *apNode, cWorld3D *apWorld,
										 tColladaGeometryVec &avColladaGeometries,
										 tColladaLightVec &avColladaLights,
										 tColladaMaterialVec &avColladaMaterials,
										 tColladaTextureVec &avColladaTextures,
										 tColladaImageVec &avColladaImages,
										 cColladaScene *apColladaScene) {
	//////////////////////////////////////////////////
	// Check if we are dealing with a special type.
	if (apNode->msName.size() > 1 && apNode->msName[0] == '_') {
		tString sType = cString::Sub(apNode->msName, 0, 5);
		if (cString::ToLowerCase(sType) == "_room") {
			// Log("Found room, leaving branch...\n");
			return;
		}

		tStringVec vParams;
		tString sSepp = "_";

		cString::GetStringVec(apNode->msName, vParams, &sSepp);

		////////////////////////////////////////
		///// COLLIDER /////////////////////////////
		if (cString::ToLowerCase(vParams[0]) == "collider") {
			cColladaGeometry *pGeom = GetGeometry(apNode->msSource, avColladaGeometries);
			if (pGeom) {
				cColliderEntity *pCollider = CreateStaticCollider(apNode, apWorld, pGeom, avColladaMaterials,
																  avColladaTextures, avColladaImages,
																  false);

				apWorld->GetPortalContainer()->Add(pCollider, true);
			} else {
				Error("Node '%s' does not have any geometry! Could not create collider!\n", apNode->msName.c_str());
			}
		}
		////////////////////////////////////////
		///// CHARACTER COLLIDER ///////////////
		else if (cString::ToLowerCase(vParams[0]) == "charcollider") {
			cColladaGeometry *pGeom = GetGeometry(apNode->msSource, avColladaGeometries);
			if (pGeom) {
				cColliderEntity *pCollider = CreateStaticCollider(apNode, apWorld, pGeom, avColladaMaterials,
																  avColladaTextures, avColladaImages,
																  true);

				apWorld->GetPortalContainer()->Add(pCollider, true);
			} else {
				Error("Node '%s' does not have any geometry!C ould not create charcollider!\n", apNode->msName.c_str());
			}
		}
		////////////////////////////////////////
		///// SOUND /////////////////////////////
		else if (cString::ToLowerCase(vParams[0]) == "sound" && !(mFlags & eWorldLoadFlag_NoEntities)) {
			if (vParams.size() < 3) {
				Error("Too few params in sound entity '%s'\n", apNode->msName.c_str());
			} else {
				tString sFile = "";
				for (size_t i = 1; i < vParams.size() - 1; ++i) {
					sFile += vParams[i];
					if (i != vParams.size() - 2)
						sFile += "_";
				}
				tString sName = vParams[vParams.size() - 1];

				cSoundEntity *pEntity = apWorld->CreateSoundEntity(sName, sFile, false);

				if (pEntity == NULL) {
					sName = vParams[1];
					sFile = cString::Sub(apNode->msName, 7 + (int)+sName.size() + 1);

					pEntity = apWorld->CreateSoundEntity(sName, sFile, false);
				}
				if (pEntity)
					pEntity->SetMatrix(apNode->m_mtxWorldTransform);
			}
		}
		////////////////////////////////////////
		///// START /////////////////////////////
		else if (cString::ToLowerCase(vParams[0]) == "start" && !(mFlags & eWorldLoadFlag_NoEntities)) {
			if (vParams.size() < 2) {
				Error("Too few params in start entity '%s'\n", apNode->msName.c_str());
			} else {
				tString sName = cString::Sub(apNode->msName, 7);

				cStartPosEntity *pStart = apWorld->CreateStartPos(sName);
				if (pStart) {
					pStart->SetMatrix(apNode->m_mtxWorldTransform);
				}
			}
		}
		////////////////////////////////////////
		///// REF /////////////////////////////
		else if (cString::ToLowerCase(vParams[0]) == "ref") {
			if ((mFlags & eWorldLoadFlag_NoGameEntities))
				return;

			if (vParams.size() < 3) {
				Error("Too few params in ref entity '%s'\n", apNode->msName.c_str());
			} else {
				tString sFile = "";
				for (size_t i = 1; i < vParams.size() - 1; ++i) {
					sFile += vParams[i];
					if (i != vParams.size() - 2)
						sFile += "_";
				}
				tString sName = vParams[vParams.size() - 1];

				iEntity3D *pEntity = apWorld->CreateEntity(sName, apNode->m_mtxWorldTransform, sFile, true);

				// If no entity was loaded, try old style.
				if (pEntity == NULL) {
					sName = vParams[1];
					sFile = cString::Sub(apNode->msName, 5 + (int)+sName.size() + 1);
					sFile = cString::SetFileExt(sFile, "ent");

					apWorld->CreateEntity(sName, apNode->m_mtxWorldTransform, sFile, true);
				}
			}

			// Skip the children of the ref!
			return;
		}
		////////////////////////////////////////
		///// AI NODE //////////////////////////
		else if (cString::ToLowerCase(vParams[0]) == "node") {
			if (vParams.size() < 3) {
				Error("Too few params in ref entity '%s'\n", apNode->msName.c_str());
			} else {
				sType = vParams[1];
				tString sName = cString::Sub(apNode->msName, 6 + (int)sType.size() + 1);

				apWorld->AddAINode(sName, sType, apNode->m_mtxWorldTransform.GetTranslation());
			}

			// Skip the children of the ref!
			return;
		}
		////////////////////////////////////////
		///// BILLBOARD /////////////////////////////
		else if (cString::ToLowerCase(vParams[0]) == "bb" && !(mFlags & eWorldLoadFlag_NoEntities)) {
			if (vParams.size() < 3) {
				Error("Too few params in billboard entity entity '%s'\n", apNode->msName.c_str());
			} else {
				cVector2f vSize(apNode->mvScale.x, apNode->mvScale.y);
				float fOffset = apNode->mvScale.z;

				tString sName = vParams[vParams.size() - 1];
				tString sFile = "";
				for (size_t i = 1; i < vParams.size() - 1; ++i) {
					sFile += vParams[i];
					if (i != vParams.size() - 2)
						sFile += "_";
				}

				cBillboard *pBill = apWorld->CreateBillboard(sName, vSize);

				// To support old versions
				if (pBill == NULL) {
					sName = vParams[1];
					sFile = cString::Sub(apNode->msName, 4 + (int)sName.size() + 1);

					pBill = apWorld->CreateBillboard(sName, vSize);
				}

				if (pBill) {
					pBill->SetForwardOffset(fOffset);

					// Old version , trying to skip because bounding box calc for this is far from
					// optimal
					// pBill->SetPosition(apNode->m_mtxWorldTransform.GetTranslation());
					// pBill->SetAxis(cMath::Vector3Normalize(cMath::MatrixInverse(apNode->m_mtxWorldTransform).GetUp()));

					// remove scale
					cMatrixf mtxScale = cMath::MatrixScale(apNode->mvScale);
					cMatrixf mtxWorld = cMath::MatrixMul(apNode->m_mtxWorldTransform, cMath::MatrixInverse(mtxScale));
					pBill->SetMatrix(mtxWorld);
					pBill->SetAxis(cVector3f(0, 1, 0));

					sFile = cString::SetFileExt(sFile, "bnt");
					pBill->LoadXMLProperties(sFile);
				}
			}
		}
		////////////////////////////////////////
		///// BEAM /////////////////////////////
		else if (cString::ToLowerCase(vParams[0]) == "beam" && !(mFlags & eWorldLoadFlag_NoEntities)) {
			if (vParams.size() < 3) {
				Error("Too few params in billboard entity entity '%s'\n", apNode->msName.c_str());
			} else {
				cVector2f vSize(apNode->mvScale.x, apNode->mvScale.y);
				// float fOffset = apNode->mvScale.z;

				tString sName = vParams[vParams.size() - 1];
				tString sFile = "";
				for (size_t i = 1; i < vParams.size() - 1; ++i) {
					sFile += vParams[i];
					if (i != vParams.size() - 2)
						sFile += "_";
				}

				// Find end point node
				tString sEndName = "_beamend_" + sName;
				cColladaNode *pEndNode = apColladaScene->GetNode(sEndName);

				if (pEndNode) {
					cBeam *pBeam = apWorld->CreateBeam(sName);

					if (pBeam) {
						pBeam->SetPosition(apNode->m_mtxWorldTransform.GetTranslation());
						pBeam->GetEnd()->SetPosition(pEndNode->m_mtxWorldTransform.GetTranslation());

						sFile = cString::SetFileExt(sFile, "beam");
						pBeam->LoadXMLProperties(sFile);
					}
				} else {
					Error("Couldn't find beam end '%s'!\n", sEndName.c_str());
				}
			}

		}
		////////////////////////////////////////
		///// PARTICLE SYSTEM //////////////////
		else if (cString::ToLowerCase(vParams[0]) == "ps" && !(mFlags & eWorldLoadFlag_NoEntities)) {
			if (vParams.size() < 3) {
				Error("Too few params in billboard entity entity '%s'\n", apNode->msName.c_str());
			} else {
				tString sName = vParams[vParams.size() - 1];
				sType = "";
				for (size_t i = 1; i < vParams.size() - 1; ++i) {
					sType += vParams[i];
					if (i != vParams.size() - 2)
						sType += "_";
				}

				cParticleSystem3D *pPS = apWorld->CreateParticleSystem(sName, sType, apNode->mvScale,
																	   apNode->m_mtxWorldTransform);

				if (pPS == NULL) {
					sName = vParams[1];
					sType = cString::Sub(apNode->msName, 4 + (int)sName.size() + 1);
					pPS = apWorld->CreateParticleSystem(sName, sType, apNode->mvScale,
														apNode->m_mtxWorldTransform);

					if (pPS == NULL) {
						Error("Couldn't load particle system '%s' with type '%s'\n",
							  sName.c_str(), sType.c_str());
					}
				}
			}
		}
		////////////////////////////////////////
		///// AREA /////////////////////////////
		if (cString::ToLowerCase(vParams[0]) == "area" && !(mFlags & eWorldLoadFlag_NoEntities)) {
			if (vParams.size() < 2) {
				Error("Too few params in area entity '%s'\n", apNode->msName.c_str());
			} else {
				cColladaGeometry *pGeom = GetGeometry(apNode->msSource, avColladaGeometries);

				tFloatVec vVertexVec;
				tVertexVec &vArray = pGeom->mvVertexVec;
				vVertexVec.resize(vArray.size() * 3);

				for (size_t vtx = 0; vtx < vArray.size(); ++vtx) {
					vVertexVec[vtx * 3 + 0] = vArray[vtx].pos.x;
					vVertexVec[vtx * 3 + 1] = vArray[vtx].pos.y;
					vVertexVec[vtx * 3 + 2] = vArray[vtx].pos.z;
				}

				cBoundingVolume TempBV;
				TempBV.AddArrayPoints(&vVertexVec[0], (int)vArray.size());
				TempBV.CreateFromPoints(3);

				tString sName;
				if (vParams.size() < 3 || (mFlags & eWorldLoadFlag_NoGameEntities)) {
					sType = "";
					if (vParams.size() < 3)
						sName = cString::ToLowerCase(vParams[1]);
					else
						sName = cString::Sub(apNode->msName, (int)vParams[0].size() + (int)vParams[1].size() + 3);
				} else {
					sType = cString::ToLowerCase(vParams[1]);
					sName = cString::Sub(apNode->msName, (int)vParams[0].size() + (int)vParams[1].size() + 3);
				}

				cVector3f vSize = TempBV.GetSize() * apNode->mvScale;

				if (sType != "") {
					iArea3DLoader *pLoader = apWorld->GetResources()->GetArea3DLoader(sType);
					if (pLoader) {
						pLoader->Load(sName, vSize, apNode->m_mtxWorldTransform, apWorld);
					}
				}

				// Create engine area.
				cAreaEntity *pArea = apWorld->CreateAreaEntity(sName);
				pArea->m_mtxTransform = apNode->m_mtxWorldTransform;
				pArea->msType = sType;
				pArea->mvSize = vSize;
			}
		}

	}
	//////////////////////////////////////////////////
	// Load light or geometry
	else {
		// The node has a source
		if (apNode->msSource != "") {
			// Get number of chars in prefix.
			int lPrefixChars = 1;
			while (lPrefixChars < (int)apNode->msName.size() &&
				   apNode->msName[lPrefixChars] != '_' &&
				   apNode->msName[lPrefixChars] != '\0') {
				lPrefixChars++;
			}

			// Get prefix
			tString sPrefix = cString::ToLowerCase(cString::Sub(apNode->msName, 0, lPrefixChars));

			/////////////////////////////////////////////////
			// Load source from external file
			if (apNode->mbSourceIsFile) {
				tString sFile = cString::SetFileExt(cString::GetFileName(apNode->msSource), "");

				// If static, load mesh from mesh file.
				if (mFlags & eWorldLoadFlag_NoGameEntities) {
					// Do nothing...
				} else if (sPrefix == "static") {
					cMesh *pMesh = mpMeshManager->CreateMesh(sFile);
					if (pMesh) {
						// Create the entity
						cMeshEntity *pEntity = apWorld->CreateMeshEntity(apNode->msName, pMesh, false);

						// Log("Static mesh entity mtx: %s\n",cMath::MatrixToChar(apNode->m_mtxWorldTransform));
						pEntity->SetMatrix(apNode->m_mtxWorldTransform);

						apWorld->GetPortalContainer()->Add(pEntity, true);

						// Set parameters
						tStringVec vParams;
						tString sSepp = "_";
						cString::GetStringVec(apNode->msName, vParams, &sSepp);

						if (HasParam(vParams, "noshadow"))
							pEntity->SetCastsShadows(false);
						else
							pEntity->SetCastsShadows(true);
					}
				}
				// If it is not static it is an entity. Load form entity file.
				else {
					tString sEntityFile = cString::SetFileExt(sFile, "ent");

					apWorld->CreateEntity(apNode->msName, apNode->m_mtxWorldTransform, sEntityFile, true);
				}

			}
			///////////////////////////////////////
			// Load source from collada data
			else {
				// Create an entity from the geometry.
				cColladaGeometry *pGeom = GetGeometry(apNode->msSource, avColladaGeometries);
				if (pGeom) {
					// Log("Creating mesh '%s'!\n",pGeom->msName.c_str());

					cMeshEntity *pEntity = CreateStaticMeshEntity(apNode, apWorld,
																  pGeom, false,
																  avColladaMaterials,
																  avColladaTextures, avColladaImages);

					if (pEntity) {
						apWorld->GetPortalContainer()->Add(pEntity, true);
					}
				} else if (!(mFlags & eWorldLoadFlag_NoLights)) {
					// If there wasn't any geometry, try and find a light.
					cColladaLight *pColladaLight = GetLight(apNode->msSource, avColladaLights);
					if (pColladaLight) {
						tStringVec vParams;
						tString sSepp = "_";
						cString::GetStringVec(apNode->msName, vParams, &sSepp);

						// check if this is an dynamic light
						bool bStatic = true;
						int lParamAdd = 0; // Too make it easier to support dynamic param
						if (cString::ToLowerCase(vParams[0]) == "dynamic") {
							bStatic = false;
							lParamAdd = 1;
						}

						tString sLightName = "";
						tString sLightFile = "";

						// Check if light might have file parameter
						if ((int)vParams.size() >= 2 + lParamAdd) {
							for (size_t i = lParamAdd; i < vParams.size() - 1; ++i) {
								sLightFile += vParams[i];
								if (i != vParams.size() - 2)
									sLightFile += "_";
							}
							sLightFile = cString::SetFileExt(sLightFile, "lnt");

							sLightName = vParams[vParams.size() - 1];
						}
						// No file parameter
						else {
							sLightName = vParams[lParamAdd];
						}

						/////////////////////////////////////
						// Load the specific light type
						if (pColladaLight->msType == "point") {
							cLight3DPoint *pLight = apWorld->CreateLightPoint(sLightName, false);

							pLight->SetMatrix(apNode->m_mtxWorldTransform);
							pColladaLight->mDiffuseColor.a = apNode->mvScale.y;
							pLight->SetDiffuseColor(pColladaLight->mDiffuseColor);
							pLight->SetFarAttenuation(apNode->mvScale.x);

							if (apNode->mvScale.z < 0.1f)
								pLight->SetCastShadows(false);
							else
								pLight->SetCastShadows(true);

							if (bStatic)
								pLight->SetStatic(bStatic);

							if (mbRestricStaticLightToSector)
								pLight->SetOnlyAffectInSector(true);

							apWorld->GetPortalContainer()->Add(pLight, bStatic);

							// Log("Added light '%s' attenuation:  %f a: %f\n",pLight->GetName().c_str(),
							//		pLight->GetFarAttenuation(), pLight->GetDiffuseColor().a);

							if (sLightFile != "")
								pLight->LoadXMLProperties(sLightFile);
						} else if (pColladaLight->msType == "spot") {
							cLight3DSpot *pLight = apWorld->CreateLightSpot(sLightName, "", false);

							pLight->SetMatrix(apNode->m_mtxWorldTransform);
							pLight->SetDiffuseColor(pColladaLight->mDiffuseColor);
							pLight->SetFarAttenuation(apNode->mvScale.x);
							pLight->SetFOV(cMath::ToRad(pColladaLight->mfAngle));

							if (bStatic)
								pLight->SetStatic(bStatic);

							if (mbRestricStaticLightToSector)
								pLight->SetOnlyAffectInSector(true);

							apWorld->GetPortalContainer()->Add(pLight, bStatic);

							if (sLightFile != "")
								pLight->LoadXMLProperties(sLightFile);
						} else {
							Warning("Invalid light type '%s' for light '%s'\n", pColladaLight->msType.c_str(),
									apNode->msName.c_str());
						}
					} else {
						Warning("Source '%s' is not found!\n", apNode->msSource.c_str());
					}
				}
			}
		}
	}

	// Iterate children.
	tColladaNodeListIt ChildIt = apNode->mlstChildren.begin();
	for (; ChildIt != apNode->mlstChildren.end(); ChildIt++) {
		AddSceneObjects(*ChildIt, apWorld, avColladaGeometries, avColladaLights,
						avColladaMaterials, avColladaTextures, avColladaImages, apColladaScene);
	}
}

//-----------------------------------------------------------------------

void cMeshLoaderCollada::AddSectorChildren(cColladaNode *apNode, tString asSector, cWorld3D *apWorld,
										   tColladaGeometryVec &avColladaGeometries,
										   tColladaLightVec &avColladaLights,
										   tColladaMaterialVec &avColladaMaterials,
										   tColladaTextureVec &avColladaTextures,
										   tColladaImageVec &avColladaImages) {
	// Log("--- Node: %s\n",apNode->msName.c_str());

	//////////////////////////////////////////////////
	// Check if we are dealing with a special type.
	if (apNode->msName[0] == '_') {
		// Check if it is a portal
		tString sType = cString::Sub(apNode->msName, 0, 7);
		if (cString::ToLowerCase(sType) == "_portal") {
			////////////////////////////////
			// Get portal number

			// Get digits in num:
			int lDigits = 1;
			while (apNode->msName[7 + lDigits] != '_' && apNode->msName[7 + lDigits] != 0)
				lDigits++;

			// get string and convert to int
			tString sNum = cString::Sub(apNode->msName, 7, lDigits);
			int lNum = cString::ToInt(sNum.c_str(), -1);
			if (lNum == -1) {
				Warning("Bad portal name: '%s'!\n", apNode->msName.c_str());
			}

			///////////////////////////////////////////
			// Get target room

			// Get the char pos for the target room
			int lStartChar = 7 + lDigits + 1;
			tString sTest = cString::ToLowerCase(cString::Sub(apNode->msName, lStartChar, 4));
			if (sTest != "room") {
				Error("Bad portal id 's'!\n", apNode->msName.c_str());
				return;
			}

			// Get number of digits
			lDigits = 1;
			while ((int)apNode->msName.size() > lStartChar + 4 + lDigits &&
				   apNode->msName[lStartChar + 4 + lDigits] != '_' &&
				   (int)apNode->msName.size() >= 7 + lDigits &&
				   apNode->msName[7 + lDigits] != 0) {
				lDigits++;
			}

			sNum = cString::Sub(apNode->msName, lStartChar + 4, lDigits);
			// int lTargetSector = cString::ToInt(sNum.c_str(),-1);
			tString sTargetSector = sNum;

			////////////////////////////////////////////
			// Get the portals in the target room it can see.

			lStartChar = lStartChar + 4 + lDigits;
			tIntVec vPortalIds;

			tString sValueString = cString::Sub(apNode->msName, lStartChar);
			tString sSepp = "_";
			cString::GetIntVec(sValueString, vPortalIds, &sSepp);

			cColladaGeometry *pGeom = GetGeometry(apNode->msSource, avColladaGeometries);
			if (pGeom) {
				cPortal *pPortal = hplNew(cPortal, (lNum, apWorld->GetPortalContainer()));

				// Set target
				pPortal->SetTargetSector(sTargetSector);

				// Add portals that can be seen.
				for (size_t i = 0; i < vPortalIds.size(); i++) {
					pPortal->AddPortalId(vPortalIds[i]);
					// Log("Seeing: %d\n", vPortalIds[i]);
				}

				/////////////////////////////////////
				// Calculate the normal of the portal
				cVector3f vNormal = cVector3f(0, 0, 0);
				for (size_t i = 0; i < pGeom->mvVertexVec.size(); i++) {
					vNormal += pGeom->mvVertexVec[i].norm;
				}
				vNormal = cMath::MatrixMul(apNode->m_mtxWorldTransform.GetRotation(), vNormal);
				vNormal.Normalise();
				pPortal->SetNormal(vNormal);

				/////////////////////////////////
				// Add the points
				for (size_t i = 0; i < pGeom->mvVertexVec.size(); i++) {
					cVector3f vPos = cMath::MatrixMul(apNode->m_mtxWorldTransform,
													  pGeom->mvVertexVec[i].pos);
					pPortal->AddPoint(vPos);
				}

				//////////////////////////
				// Set the world transform.
				pPortal->SetTransform(cMatrixf::Identity); // apNode->m_mtxWorldTransform);

				/////////////////////
				// Compile the portal
				pPortal->Compile();

				////////////////////////
				// Add portal to sector.
				apWorld->GetPortalContainer()->AddPortal(pPortal, asSector);
			}
		}
	}
	////////////////////////////////////////////
	// Add normal geometry or light
	else {
		if (apNode->msSource != "") {
			if (apNode->mbSourceIsFile) {
				Error("Entities are NOT allowed in the room tree!\n");
			} else {
				// Create an entity from the geometry.
				cColladaGeometry *pGeom = GetGeometry(apNode->msSource, avColladaGeometries);
				if (pGeom) {
					// Log("Creating mesh '%s' from source '%s'!\n",pGeom->msName.c_str(),apNode->msSource.c_str());

					cMeshEntity *pEntity = CreateStaticMeshEntity(apNode, apWorld,
																  pGeom, true,
																  avColladaMaterials,
																  avColladaTextures, avColladaImages);

					if (pEntity) {
						apWorld->GetPortalContainer()->AddToSector(pEntity, asSector);
					}
				} else {
					cColladaLight *pLight = GetLight(apNode->msSource, avColladaLights);
					if (pLight) {
						Error("Lights are NOT allowed in the room tree!\n");
					} else {
						Warning("Source '%s' is not found!\n", apNode->msSource.c_str());
					}
				}
			}
		}
	}

	// Log("---\n");

	//////////////////////////////////////////////////
	// Iterate children.
	tColladaNodeListIt ChildIt = apNode->mlstChildren.begin();
	for (; ChildIt != apNode->mlstChildren.end(); ChildIt++) {
		AddSectorChildren(*ChildIt, asSector, apWorld, avColladaGeometries, avColladaLights,
						  avColladaMaterials, avColladaTextures, avColladaImages);
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
