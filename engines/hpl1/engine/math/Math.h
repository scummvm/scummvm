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

#ifndef HPL_MATH_H
#define HPL_MATH_H

#include "hpl1/engine/math/BoundingVolume.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/math/MeshTypes.h"

#include "hpl1/engine/graphics/Color.h"

namespace hpl {

class cMath {
public:
	//////////////////////////////////////////////////////
	////////// RANDOM GENERATION ////////////////////////
	//////////////////////////////////////////////////////
	/**
	 * Generates a random integer from min to max
	 * \param alMin
	 * \param alMax
	 * \return
	 */
	static int RandRectl(int alMin, int alMax);

	/**
	 * Generates a random float from min to max
	 * \param alMin
	 * \param alMax
	 * \return
	 */
	static float RandRectf(float alMin, float alMax);

	/**
	 * Generates a random float from min to max
	 */
	static cVector2f RandRectVector2f(const cVector3f &avMin, const cVector3f &avMax);

	/**
	 * Generates a random float from min to max
	 */
	static cVector3f RandRectVector3f(const cVector3f &avMin, const cVector3f &avMax);

	/**
	 * Generates a random float from min to max
	 */
	static cColor RandRectColor(const cColor &aMin, const cColor &aMax);

	/**
	 * Randomize the rand funcs.
	 * \param alSeed the seed, -1 = random seed.
	 */
	static void Randomize(int alSeed = -1);

	//////////////////////////////////////////////////////
	////////// COLLSION //////////////////////////////////
	//////////////////////////////////////////////////////

	static bool BoxCollision(cRect2l aRect1, cRect2l aRect2);
	static bool BoxCollision(cRect2f aRect1, cRect2f aRect2);

	static bool PointBoxCollision(cVector2f avPoint, cRect2f aRect);

	static bool BoxFit(cRect2l aRectSrc, cRect2l aRectDest);
	static bool BoxFit(cRect2f aRectSrc, cRect2f aRectDest);

	static float Dist2D(const cVector2f &avPosA, const cVector2f &avPosB);
	static float Dist2D(const cVector3f &avPosA, const cVector3f &avPosB);

	static float SqrDist2D(const cVector2f &avPosA, const cVector2f &avPosB);
	static float SqrDist2D(const cVector3f &avPosA, const cVector3f &avPosB);

	static cRect2f &ClipRect(cRect2f &aRectSrc, const cRect2f &aRectDest);

	/**
	 * Checks collison between two bounding volumes.
	 * \return true if collision, else false.
	 */
	static bool CheckCollisionBV(cBoundingVolume &aBV1, cBoundingVolume &aBV2);

	static bool PointBVCollision(const cVector3f &avPoint, cBoundingVolume &aBV2);

	/**
	 * Creates a clip rect for a bounding volume in screen space.
	 * \return false if behind near clip.
	 */
	static bool GetClipRectFromBV(cRect2l &aDestRect, cBoundingVolume &aBV,
								  const cMatrixf &a_mtxView, const cMatrixf &a_mtxProj,
								  float afNearClipPlane, const cVector2l &avScreenSize);

	static bool CheckSphereInPlanes(const cVector3f &avCenter, float afRadius,
									const cPlanef *apPlanes, int alPlaneCount);

	//////////////////////////////////////////////////////
	////////// FLOAT OPERATIONS ////////////////////////
	//////////////////////////////////////////////////////

	/**
	 * Get fraction part of a float
	 * \param afVal
	 * \return
	 */
	static float GetFraction(float afVal);
	/**
	 * Moldus (%) of a float
	 * \param afDividend
	 * \param afDivisor
	 * \return
	 */
	static float Modulus(float afDividend, float afDivisor);

	static float ToRad(float afAngle);
	static float ToDeg(float afAngle);

	/**
	 * Get the Log 2 of an int.
	 */
	static int Log2ToInt(int alX);

	/**
	 * Checks if the number is a power of two.
	 */
	static bool IsPow2(int alX);

	/**
	 * Wraps a value (afX) between min and max. Example: Wrap(-1, 0,10) returns 9.
	 * \param afMin The minimum value. Must be lower than max.
	 * \param afMax The maximum value. Must be higher than min and NOT 0.
	 */
	static float Wrap(float afX, float afMin, float afMax);

	/**
	 * Clamps a value between min and max. Example Clamp(-1, 0,1) return 0.
	 */
	static float Clamp(float afX, float afMin, float afMax);

	inline static float Max(float afX, float afY) {
		if (afX > afY)
			return afX;
		return afY;
	}
	inline static float Min(float afX, float afY) {
		if (afX < afY)
			return afX;
		return afY;
	}

	inline static int Max(int alX, int alY) {
		if (alX > alY)
			return alX;
		return alY;
	}
	inline static int Min(int alX, int alY) {
		if (alX < alY)
			return alX;
		return alY;
	}

	inline static float Abs(float afX) { return afX < 0 ? -afX : afX; }
	inline static int Abs(int alX) { return alX < 0 ? -alX : alX; }

	static float GetAngleDistance(float afAngle1, float afAngle2, float afMaxAngle);
	static float GetAngleDistanceRad(float afAngle1, float afAngle2);
	static float GetAngleDistanceDeg(float afAngle1, float afAngle2);

	static float TurnAngle(float afAngle, float afFinalAngle, float afSpeed, float afMaxAngle);
	static float TurnAngleRad(float afAngle, float afFinalAngle, float afSpeed);
	static float TurnAngleDeg(float afAngle, float afFinalAngle, float afSpeed);

	static float InterpolateFloat(float afA, float afB, float afT);

	template<typename Vector>
		static decltype(Vector::x) & GetVectorX(Vector &v) {
		return v.x;
	}

	template<typename Vector>
		static decltype(Vector::y) & GetVectorY(Vector &v) {
		return v.y;
	}
	template<typename Vector>
		static decltype(Vector::z) & GetVectorZ(Vector &v) {
		return v.z;
	}

	//////////////////////////////////////////////////////
	////////// VECTOR 2D ///////////////////////////////
	//////////////////////////////////////////////////////
	/**
	 * Get the angle a vector at aStartPos has to have to look at aGoalPos
	 * \param &aStartPos
	 * \param &avGoalPos
	 * \return
	 */
	static float GetAngleFromPoints2D(const cVector2f &aStartPos, const cVector2f &avGoalPos);
	/**
	 * Get a vector from an angle and a length
	 * \param afAngle
	 * \param afLength
	 * \return
	 */
	static cVector2f GetVectorFromAngle2D(float afAngle, float afLength);
	/**
	 * Get angle and length of a vector
	 * \param &avVec
	 * \param *apAngle
	 * \param *apLength
	 */
	static void GetAngleFromVector(const cVector2f &avVec, float *apAngle, float *apLength);

	/**
	 * Project Src on Dest
	 * \param &avSrcVec
	 * \param &avDestVec
	 * \return
	 */
	static cVector2f ProjectVector2D(const cVector2f &avSrcVec, const cVector2f &avDestVec);

	//////////////////////////////////////////////////////
	////////// VECTOR 3D ///////////////////////////////
	//////////////////////////////////////////////////////

	static inline cVector3f Vector3ToRad(const cVector3f &avVec) {
		return cVector3f(ToRad(avVec.x), ToRad(avVec.y), ToRad(avVec.z));
	}

	static inline cVector3f Vector3ToDeg(const cVector3f &avVec) {
		return cVector3f(ToDeg(avVec.x), ToDeg(avVec.y), ToDeg(avVec.z));
	}

	static inline cVector3f Vector3Normalize(const cVector3f &avVec) {
		cVector3f vNorm = avVec;
		vNorm.Normalise();
		return vNorm;
	}

	static inline float Vector3DistSqr(const cVector3f &avStartPos, const cVector3f &avEndPos) {
		float fDX = avEndPos.x - avStartPos.x;
		float fDY = avEndPos.y - avStartPos.y;
		float fDZ = avEndPos.z - avStartPos.z;

		return fDX * fDX + fDY * fDY + fDZ * fDZ;
	}

	static inline float Vector3Dist(const cVector3f &avStartPos, const cVector3f &avEndPos) {
		return sqrt(Vector3DistSqr(avStartPos, avEndPos));
	}

	static cVector3f GetAngleFromPoints3D(const cVector3f &avStartPos, const cVector3f &avGoalPos);

	/**
	 * Vector cross product, A x B = R
	 * \param avVecA
	 * \param avVecB
	 * \return
	 */
	static cVector3f Vector3Cross(const cVector3f &avVecA, const cVector3f &avVecB);

	/**
	 * Vector dot product, A * B = R
	 * \param avVecA
	 * \param avVecB
	 * \return
	 */
	static float Vector3Dot(const cVector3f &avVecA, const cVector3f &avVecB);

	/**
	 * Project Src on Dest
	 * \param &avSrcVec, must be normalized
	 * \param &avDestVec, must be normalized
	 * \return
	 */
	static cVector3f ProjectVector3D(const cVector3f &avSrcVec, const cVector3f &avDestVec);

	/**
	 * Calculates the angle between two vectors.
	 * \param avVecA
	 * \param avVecB
	 * \return
	 */
	static float Vector3Angle(const cVector3f &avVecA, const cVector3f &avVecB);

	/**
	 * Unprojects a vector from screen size and coords.
	 */
	static cVector3f Vector3UnProject(const cVector3f &avVec, const cRect2f &aScreenRect,
									  cMatrixf a_mtxViewProj);

	/**
	 * Calculates distance from a point to a plane
	 * \param aPlane The plane must be normalised!
	 * \param avVec
	 * \return >0 if in front of plane, 0 if on plane and <0 if behind plane
	 */
	static float PlaneToPointDist(const cPlanef &aPlane, const cVector3f &avVec);

	/**
	 * Get the line defining the intersection of 2 planes.
	 * \param aPA
	 * \param aPB
	 * \param avDir The direction of the line will be placed here
	 * \param avPoint A point on the line will be placed here.
	 */
	static void PlaneIntersectionLine(const cPlanef &aPA, const cPlanef &aPB,
									  cVector3f &avDir, cVector3f &avPoint);

	/**
	 * Checks intersection with a frustum, an array of alPairNum plane pairs(alPairNum*2 planes), and a line,
	 */
	static bool CheckFrustumLineIntersection(const cPlanef *apPlanePairs, const cVector3f &avPoint1,
											 const cVector3f &avPoint2, int alPairNum);

	/**
	 * Checks intersection with a a frustum, an array of alPairNum plane pairs(alPairNum * 2 planes), and a quad mesh.
	 * \param apPoints the points of the quad mesh, every 4 points is a face.
	 */
	static bool CheckFrustumQuadMeshIntersection(const cPlanef *apPlanePairs, tVector3fVec *apPoints,
												 int alPairNum);

	//////////////////////////////////////////////////////
	////////// QUATERNIONS ///////////////////////////////
	//////////////////////////////////////////////////////

	/**
	 * Spherical Linear Interpolation between quaternions A and B
	 * \param afT The amount inbetween the quaternions. 0.0 is A and 1 is B.
	 * \param abShortestPath Move the the shortest path.
	 * \return
	 */
	static cQuaternion QuaternionSlerp(float afT, const cQuaternion &aqA, const cQuaternion &aqB,
									   bool abShortestPath);

	static float QuaternionDot(const cQuaternion &aqA, const cQuaternion &aqB);

	//////////////////////////////////////////////////////
	////////// MATRIX ////////////////////////////////////
	//////////////////////////////////////////////////////

	/**
	 * Spherical Linear Interpolation between matrix A and B
	 * \param afT The amount inbetween the quaternions. 0.0 is A and 1 is B.
	 * \param abShortestPath Move the the shortest path.
	 * \return
	 */
	static cMatrixf MatrixSlerp(float afT, const cMatrixf &a_mtxA, const cMatrixf &a_mtxB,
								bool abShortestPath);

	/**
	 * Matrix mulitplication,  A * B = R. This means that B is applied BEFORE A.
	 */
	static cMatrixf MatrixMul(const cMatrixf &a_mtxA, const cMatrixf &a_mtxB);
	/**
	 * Multiply and matrix and a 3d vector
	 */
	static cVector3f MatrixMul(const cMatrixf &a_mtxA, const cVector3f &avB);
	/**
	 * Multiply and matrix and a 3d vector and devide the result with W.
	 */
	static cVector3f MatrixMulDivideW(const cMatrixf &a_mtxA, const cVector3f &avB);

	/**
	 * Multiply matrix and a float.
	 */
	static cMatrixf MatrixMulScalar(const cMatrixf &a_mtxA, float afB);

	/**
	 * Creates a rotation matrix along all axes according to order.
	 */
	static cMatrixf MatrixRotate(cVector3f avRot, eEulerRotationOrder aOrder);
	/**
	 * Create a rotation matrix around the X-axis according to the right hand rule
	 */
	static cMatrixf MatrixRotateX(float afAngle);
	/**
	 * Create a rotation matrix around the Y-axis according to the right hand rule
	 */
	static cMatrixf MatrixRotateY(float afAngle);
	/**
	 * Create a rotation matrix around the Z-axis according to the right hand rule
	 */
	static cMatrixf MatrixRotateZ(float afAngle);
	static cMatrixf MatrixScale(cVector3f avScale);
	static cMatrixf MatrixTranslate(cVector3f avTrans);

	/**
	 * Creates a matrix from a quaternion.
	 * \return
	 */
	static cMatrixf MatrixQuaternion(const cQuaternion &aqRot);

	/**
	 * Get the minor of a matrix.
	 */
	static inline float MatrixMinor(const cMatrixf &a_mtxA,
									const size_t r0, const size_t r1, const size_t r2,
									const size_t c0, const size_t c1, const size_t c2);
	/**
	 * Get the adjoint of a matrix.
	 */
	static inline cMatrixf MatrixAdjoint(const cMatrixf &a_mtxA);

	/**
	 * Get the determinant of a matrix.
	 */
	static inline float MatrixDeterminant(const cMatrixf &a_mtxA);

	/**
	 * Gets the inverse of a matrix.
	 */
	static cMatrixf MatrixInverse(const cMatrixf &a_mtxA);

	/**
	 * Converts the matrix into Euler angles, XYZ only supported at the moment.
	 * \param &a_mtxA
	 * \param aOrder
	 */
	static cVector3f MatrixToEulerAngles(const cMatrixf &a_mtxA, eEulerRotationOrder aOrder);

	/**
	 * Create a char string from the matrix
	 */
	static const char *MatrixToChar(const cMatrixf &a_mtxA);

	//////////////////////////////////////////////////////
	////////// POLYGONS //////////////////////////////////
	//////////////////////////////////////////////////////

	/**
	 * Creates an array with 4d tangent vectors for triangles. alIndexNum % 3 must be 0.
	 * \param apDestArray The destination array, must be 4 * number of vertices large
	 * \param apIndexArray The indices
	 * \param alIndexNum Number of indices
	 * \param apVertexArray Vertices indexed by the indices
	 * \param apTexArray The texture coords
	 * \param apNormalArray The normals
	 * \param  alVertexNum Number of vertex, normals and texcoords.
	 * \return true if success, else false
	 */
	static bool CreateTriTangentVectors(float *apDestArray,
										const unsigned int *apIndexArray, int alIndexNum,
										const float *apVertexArray, int alVtxStride,
										const float *apTexArray,
										const float *apNormalArray, int alVertexNum);

	/**
	 * Creates triangle data for a triangle mesh. alIndexNum % 3 must be 0.
	 * \param avTriangles Where the data will be stored. If empty, this function resizes it.
	 * \param apIndexArray The indices
	 * \param alIndexNum Number of indices
	 * \param apVertexArray Vertices indexed by the indices
	 * \param  alVertexNum Number of vertices
	 * \return true if success, else false
	 */
	static bool CreateTriangleData(tTriangleDataVec &avTriangles,
								   const unsigned int *apIndexArray, int alIndexNum,
								   const float *apVertexArray, int alVtxStride, int alVertexNum);

	/**
	 * Creates edges for a triangle mesh. alIndexNum % 3 must be 0.
	 * \param avEdges An empty edge vector, this is where the edges will be stored.
	 * \param apIndexArray The indices
	 * \param alIndexNum Number of indices
	 * \param apVertexArray Vertices indexed by the indices
	 * \param  alVertexNum Number of vertices
	 * \return true if success, else false
	 */
	static bool CreateEdges(tTriEdgeVec &avEdges,
							const unsigned int *apIndexArray, int alIndexNum,
							const float *apVertexArray, int alVtxStride, int alVertexNum,
							bool *apIsDoubleSided);
};

} // namespace hpl

#endif // HPL_MATH_H
