/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef __NEWTON_H_INCLUDED__
#define __NEWTON_H_INCLUDED__


#define NEWTON_MAJOR_VERSION 2
#define NEWTON_MINOR_VERSION 36


#ifdef _NEWTON_STATIC_LIB
	#define NEWTON_API
#else 
	#ifdef _NEWTON_BUILD_DLL
		#ifdef _WIN32
			#define NEWTON_API __declspec (dllexport)
		#else
			#define NEWTON_API __attribute__ ((visibility("default")))
		#endif
	#else
		#ifdef _WIN32
			#define NEWTON_API __declspec (dllimport)
		#else
			#define NEWTON_API
		#endif
	#endif
#endif


#ifdef __USE_DOUBLE_PRECISION__
  #define dFloat double
#else
  #define dFloat float
#endif

#define dFloat64 double


#ifdef __cplusplus
extern "C" {
#endif

#define NEWTON_PROFILER_WORLD_UPDATE					0

#define NEWTON_PROFILER_COLLISION_UPDATE				1
#define NEWTON_PROFILER_COLLISION_UPDATE_BROAD_PHASE	2
#define NEWTON_PROFILER_COLLISION_UPDATE_NARROW_PHASE	3

#define NEWTON_PROFILER_DYNAMICS_UPDATE					4
#define NEWTON_PROFILER_DYNAMICS_CONSTRAINT_GRAPH		5
#define NEWTON_PROFILER_DYNAMICS_SOLVE_CONSTRAINT_GRAPH	6

#define NEWTON_PROFILER_FORCE_CALLBACK_UPDATE			7


typedef struct NewtonMesh{} NewtonMesh;
typedef struct NewtonBody{} NewtonBody;
typedef struct NewtonWorld{} NewtonWorld;
typedef struct NewtonJoint{} NewtonJoint;
typedef struct NewtonMaterial{} NewtonMaterial;
typedef struct NewtonCollision{} NewtonCollision;
typedef struct NewtonSceneProxy{} NewtonSceneProxy;
typedef struct NewtonbreakableComponentMesh{} NewtonbreakableComponentMesh;

//	typedef struct NewtonRagDoll{} NewtonRagDoll;
//	typedef struct NewtonRagDollBone{} NewtonRagDollBone;

#define SERIALIZE_ID_BOX					0
#define SERIALIZE_ID_CONE					1
#define SERIALIZE_ID_SPHERE					2
#define SERIALIZE_ID_CAPSULE				3
#define SERIALIZE_ID_CYLINDER				4
#define SERIALIZE_ID_COMPOUND				5
#define SERIALIZE_ID_CONVEXHULL				6
#define SERIALIZE_ID_CONVEXMODIFIER			7
#define SERIALIZE_ID_CHAMFERCYLINDER		8
#define SERIALIZE_ID_TREE					9
#define SERIALIZE_ID_NULL					10
#define SERIALIZE_ID_HEIGHTFIELD			11
#define SERIALIZE_ID_USERMESH				12
#define SERIALIZE_ID_SCENE					13
#define SERIALIZE_ID_COMPOUND_BREAKABLE		14

typedef struct NewtonBoxParam NewtonBoxParam;
typedef struct NewtonConeParam NewtonConeParam;
typedef struct NewtonSphereParam NewtonSphereParam;
typedef struct NewtonCapsuleParam NewtonCapsuleParam;
typedef struct NewtonCylinderParam NewtonCylinderParam;
typedef struct NewtonConvexHullParam NewtonConvexHullParam;
typedef struct NewtonCollisionTreeParam NewtonCollisionTreeParam;
typedef struct NewtonSceneCollisionParam NewtonSceneCollisionParam;
typedef struct NewtonChamferCylinderParam NewtonChamferCylinderParam;
typedef struct NewtonCompoundCollisionParam NewtonCompoundCollisionParam;
typedef struct NewtonConvexHullModifierParam NewtonConvexHullModifierParam;
typedef struct NewtonHeightFieldCollisionParam NewtonHeightFieldCollisionParam;

typedef struct NewtonCollisionInfoRecord NewtonCollisionInfoRecord;

struct NewtonBoxParam
{
  dFloat m_x;
  dFloat m_y;
  dFloat m_z;
};

struct NewtonSphereParam
{
  dFloat m_r0;
  dFloat m_r1;
  dFloat m_r2;
};

struct NewtonCylinderParam
{
  dFloat m_r0;
  dFloat m_r1;
  dFloat m_height;
};

struct NewtonCapsuleParam
{
  dFloat m_r0;
  dFloat m_r1;
  dFloat m_height;
};

struct NewtonConeParam
{
  dFloat m_r;
  dFloat m_height;
};

struct NewtonChamferCylinderParam
{
  dFloat m_r;
  dFloat m_height;
};

struct NewtonConvexHullParam
{
  int m_vertexCount;
  int m_vertexStrideInBytes;
  int m_faceCount;
  dFloat* m_vertex;
};

struct NewtonConvexHullModifierParam
{
  NewtonCollision* m_chidren;
};


struct NewtonCompoundCollisionParam
{
  int m_chidrenCount;
  NewtonCollision** m_chidren;
};

struct NewtonCollisionTreeParam
{
  int m_vertexCount;
  int m_indexCount;
};

struct NewtonHeightFieldCollisionParam
{
  int m_width;
  int m_height;
  int m_gridsDiagonals;
  dFloat m_horizonalScale;
  dFloat m_verticalScale;
  unsigned short *m_elevation;
  char *m_atributes;
};

struct NewtonSceneCollisionParam
{
  int m_childrenProxyCount;
};

struct NewtonCollisionInfoRecord
{
  dFloat m_offsetMatrix[4][4];
  int m_collisionType;				// tag id to identify the collision primitive
  int m_referenceCount;				// the current reference count for this collision
  int m_collisionUserID;

  union {
    NewtonBoxParam m_box;
    NewtonConeParam m_cone;
    NewtonSphereParam m_sphere;
    NewtonCapsuleParam m_capsule;
    NewtonCylinderParam m_cylinder;
    NewtonChamferCylinderParam m_chamferCylinder;
    NewtonConvexHullParam m_convexHull;
    NewtonCompoundCollisionParam m_compoundCollision;
    NewtonConvexHullModifierParam m_convexHullModifier;
    NewtonCollisionTreeParam m_collisionTree;
    NewtonHeightFieldCollisionParam m_heightField;
    NewtonSceneCollisionParam m_sceneCollision;
    dFloat m_paramArray[64];		    // user define collision can use this to store information
  };
};


typedef struct NewtonJointRecord NewtonJointRecord;
typedef struct NewtonHingeSliderUpdateDesc NewtonHingeSliderUpdateDesc;
typedef struct NewtonUserMeshCollisionRayHitDesc NewtonUserMeshCollisionRayHitDesc;
typedef struct NewtonUserMeshCollisionCollideDesc NewtonUserMeshCollisionCollideDesc;
typedef struct NewtonWorldConvexCastReturnInfo NewtonWorldConvexCastReturnInfo;


struct NewtonJointRecord
{
  dFloat m_attachmenMatrix_0[4][4];
  dFloat m_attachmenMatrix_1[4][4];
  dFloat m_minLinearDof[3];
  dFloat m_maxLinearDof[3];
  dFloat m_minAngularDof[3];
  dFloat m_maxAngularDof[3];
  const NewtonBody* m_attachBody_0;
  const NewtonBody* m_attachBody_1;
  dFloat m_extraParameters[16];
  int	m_bodiesCollisionOn;
  char m_descriptionType[32];
} ;



struct NewtonUserMeshCollisionCollideDesc
{
  dFloat m_boxP0[4];						// lower bounding box of intersection query in local space
  dFloat m_boxP1[4];						// upper bounding box of intersection query in local space
  int m_threadNumber;						// current thread executing this query
  int	m_faceCount;                        // the application should set here how many polygons intersect the query box
  int m_vertexStrideInBytes;              // the application should set here the size of each vertex
  void* m_userData;                       // user data passed to the collision geometry at creation time
  dFloat* m_vertex;                       // the application should the pointer to the vertex array.
   int* m_userAttribute;                   // the application should set here the pointer to the user data, one for each face
  int* m_faceIndexCount;                  // the application should set here the pointer to the vertex count of each face.
  int* m_faceVertexIndex;                 // the application should set here the pointer index array for each vertex on a face.
  NewtonBody* m_objBody;                  // pointer to the colliding body
  NewtonBody* m_polySoupBody;             // pointer to the rigid body owner of this collision tree
};

struct NewtonWorldConvexCastReturnInfo
{
  dFloat m_point[4];						// collision point in global space
  dFloat m_normal[4];						// surface normal at collision point in global space
  dFloat m_normalOnHitPoint[4];           // surface normal at the surface of the hit body,
  // is the same as the normal calculated by a ray cast hitting the body at the hit point
  dFloat m_penetration;                   // contact penetration at collision point
  int    m_contactID;	                    // collision ID at contact point
  const  NewtonBody* m_hitBody;			// body hit at contact point
};

struct NewtonUserMeshCollisionRayHitDesc
{
  dFloat m_p0[4];							// ray origin in collision local space
  dFloat m_p1[4];                         // ray destination in collision local space
  dFloat m_normalOut[4];					// copy here the normal at the ray intersection
  int m_userIdOut;                        // copy here a user defined id for further feedback
  void* m_userData;                       // user data passed to the collision geometry at creation time
};

struct NewtonHingeSliderUpdateDesc
{
  dFloat m_accel;
  dFloat m_minFriction;
  dFloat m_maxFriction;
  dFloat m_timestep;
} ;


// Newton callback functions
typedef void* (*NewtonAllocMemory) (int sizeInBytes);
typedef void (*NewtonFreeMemory) (void* const ptr, int sizeInBytes);
typedef void (*NewtonDestroyWorld) (const NewtonWorld* const newtonWorld);

typedef unsigned (*NewtonGetTicksCountCallback) ();

typedef void (*NewtonSerialize) (void* const serializeHandle, const void* buffer, int size);
typedef void (*NewtonDeserialize) (void* const serializeHandle, void* buffer, int size);

// user collision callbacks
typedef void (*NewtonUserMeshCollisionDestroyCallback) (void* const userData);
typedef void (*NewtonUserMeshCollisionCollideCallback) (NewtonUserMeshCollisionCollideDesc* const collideDescData);
typedef dFloat (*NewtonUserMeshCollisionRayHitCallback) (NewtonUserMeshCollisionRayHitDesc* const lineDescData);
typedef void (*NewtonUserMeshCollisionGetCollisionInfo) (void* const userData, NewtonCollisionInfoRecord* const infoRecord);
typedef int (*NewtonUserMeshCollisionGetFacesInAABB) (void* const userData, const dFloat* const p0, const dFloat* const p1,
                            const dFloat** const vertexArray, int* const vertexCount, int* const vertexStrideInBytes,
                            const int* const indexList, int maxIndexCount, const int* const userDataList);

typedef dFloat (*NewtonCollisionTreeRayCastCallback) (const NewtonBody* const body, const NewtonCollision* const treeCollision, dFloat interception, dFloat* normal, int faceId, void* usedData);
typedef dFloat (*NewtonHeightFieldRayCastCallback) (const NewtonBody* const body, const NewtonCollision* const heightFieldCollision, dFloat interception, int row, int col, dFloat* normal, int faceId, void* usedData);


// collision tree call back (obsoleted no recommended)
typedef void (*NewtonTreeCollisionCallback) (const NewtonBody* const bodyWithTreeCollision, const NewtonBody* const body, int faceID,
                       int vertexCount, const dFloat* const vertex, int vertexStrideInBytes);

typedef void (*NewtonBodyDestructor) (const NewtonBody* const body);
typedef void (*NewtonApplyForceAndTorque) (const NewtonBody* const body, dFloat timestep, int threadIndex);
typedef void (*NewtonSetTransform) (const NewtonBody* const body, const dFloat* const matrix, int threadIndex);

typedef int (*NewtonIslandUpdate) (const NewtonWorld* const newtonWorld, const void* islandHandle, int bodyCount);
typedef void (*NewtonBodyLeaveWorld) (const NewtonBody* const body, int threadIndex);
typedef void (*NewtonDestroyBodyByExeciveForce) (const NewtonBody* const body, const NewtonJoint* const contact);
typedef void (*NewtonCollisionDestructor) (const NewtonWorld* const newtonWorld, const NewtonCollision* const collision);

typedef int (*NewtonCollisionCompoundBreakableCallback) (NewtonMesh* const mesh, void* const userData, dFloat* const planeMatrixOut);

typedef int (*NewtonGetBuoyancyPlane) (const int collisionID, void* const context, const dFloat* const globalSpaceMatrix, dFloat* const globalSpacePlane);
typedef unsigned (*NewtonWorldRayPrefilterCallback)(const NewtonBody* const body, const NewtonCollision* const collision, void* const userData);
typedef dFloat (*NewtonWorldRayFilterCallback)(const NewtonBody* const body, const dFloat* const hitNormal, int collisionID, void* const userData, dFloat intersectParam);


typedef int  (*NewtonOnAABBOverlap) (const NewtonMaterial* const material, const NewtonBody* const body0, const NewtonBody* const body1, int threadIndex);
typedef void (*NewtonContactsProcess) (const NewtonJoint* const contact, dFloat timestep, int threadIndex);

typedef void (*NewtonBodyIterator) (const NewtonBody* const body, void* const userData);
typedef void (*NewtonJointIterator) (const NewtonJoint* const joint, void* const userData);
typedef void (*NewtonCollisionIterator) (void* const userData, int vertexCount, const dFloat* const faceArray, int faceId);

typedef void (*NewtonBallCallBack) (const NewtonJoint* const ball, dFloat timestep);
typedef unsigned (*NewtonHingeCallBack) (const NewtonJoint* const hinge, NewtonHingeSliderUpdateDesc* const desc);
typedef unsigned (*NewtonSliderCallBack) (const NewtonJoint* const slider, NewtonHingeSliderUpdateDesc* const desc);
typedef unsigned (*NewtonUniversalCallBack) (const NewtonJoint* const universal, NewtonHingeSliderUpdateDesc* const desc);
typedef unsigned (*NewtonCorkscrewCallBack) (const NewtonJoint* const corkscrew, NewtonHingeSliderUpdateDesc* const desc);

typedef void (*NewtonUserBilateralCallBack) (const NewtonJoint* const userJoint, dFloat timestep, int threadIndex);
typedef void (*NewtonUserBilateralGetInfoCallBack) (const NewtonJoint* const userJoint, NewtonJointRecord* const info);

typedef void (*NewtonConstraintDestructor) (const NewtonJoint*const  me);

//	typedef void (*NewtonSetRagDollTransform) (const NewtonRagDollBone* const bone);
//	typedef void (*NewtonBodyActivationState) (const NewtonBody* const body, unsigned state);
//	typedef void (*NewtonVehicleTireUpdate) (const NewtonJoint* const vehicle, dFloat timestep);


// **********************************************************************************************
//
// world control functions
//
// **********************************************************************************************
NEWTON_API int NewtonWorldGetVersion ();
NEWTON_API int NewtonWorldFloatSize ();

NEWTON_API int NewtonGetMemoryUsed ();
NEWTON_API void NewtonSetMemorySystem (NewtonAllocMemory malloc, NewtonFreeMemory mfree);

NEWTON_API NewtonWorld* NewtonCreate ();
NEWTON_API void NewtonDestroy (const NewtonWorld* const newtonWorld);
NEWTON_API void NewtonDestroyAllBodies (const NewtonWorld* const newtonWorld);

NEWTON_API void NewtonUpdate (const NewtonWorld* const newtonWorld, dFloat timestep);
NEWTON_API void NewtonInvalidateCache (const NewtonWorld* const newtonWorld);
NEWTON_API void NewtonCollisionUpdate (const NewtonWorld* const newtonWorld);

NEWTON_API void NewtonSetSolverModel (const NewtonWorld* const newtonWorld, int model);
NEWTON_API void NewtonSetPlatformArchitecture (const NewtonWorld* const newtonWorld, int mode);
NEWTON_API int NewtonGetPlatformArchitecture(const NewtonWorld* const newtonWorld, char* description);
NEWTON_API void NewtonSetMultiThreadSolverOnSingleIsland (const NewtonWorld* const newtonWorld, int mode);
NEWTON_API int NewtonGetMultiThreadSolverOnSingleIsland (const NewtonWorld* const newtonWorld);

NEWTON_API void NewtonSetPerformanceClock (const NewtonWorld* const newtonWorld, NewtonGetTicksCountCallback callback);
NEWTON_API unsigned NewtonReadPerformanceTicks (const NewtonWorld* const newtonWorld, unsigned performanceEntry);
NEWTON_API unsigned NewtonReadThreadPerformanceTicks (const NewtonWorld* newtonWorld, unsigned threadIndex);

NEWTON_API void NewtonWorldCriticalSectionLock (const NewtonWorld* const newtonWorld);
NEWTON_API void NewtonWorldCriticalSectionUnlock (const NewtonWorld* const newtonWorld);
NEWTON_API void NewtonSetThreadsCount (const NewtonWorld* const newtonWorld, int threads);
NEWTON_API int NewtonGetThreadsCount(const NewtonWorld* const newtonWorld);
NEWTON_API int NewtonGetMaxThreadsCount(const NewtonWorld* const newtonWorld);

NEWTON_API void NewtonSetFrictionModel (const NewtonWorld* const newtonWorld, int model);
NEWTON_API void NewtonSetMinimumFrameRate (const NewtonWorld* const newtonWorld, dFloat frameRate);
NEWTON_API void NewtonSetBodyLeaveWorldEvent (const NewtonWorld* const newtonWorld, NewtonBodyLeaveWorld callback);
NEWTON_API void NewtonSetWorldSize (const NewtonWorld* const newtonWorld, const dFloat* const minPoint, const dFloat* const maxPoint);
NEWTON_API void NewtonSetIslandUpdateEvent (const NewtonWorld* const newtonWorld, NewtonIslandUpdate islandUpdate);
NEWTON_API void NewtonSetCollisionDestructor (const NewtonWorld* const newtonWorld, NewtonCollisionDestructor callback);
NEWTON_API void NewtonSetDestroyBodyByExeciveForce (const NewtonWorld* const newtonWorld, NewtonDestroyBodyByExeciveForce callback);


//	NEWTON_API void NewtonWorldForEachBodyDo (const NewtonWorld* const newtonWorld, NewtonBodyIterator callback);
NEWTON_API void NewtonWorldForEachJointDo (const NewtonWorld* const newtonWorld, NewtonJointIterator callback, void* const userData);
NEWTON_API void NewtonWorldForEachBodyInAABBDo (const NewtonWorld* const newtonWorld, const dFloat* const p0, const dFloat* const p1, NewtonBodyIterator callback, void* const userData);


NEWTON_API void NewtonWorldSetUserData (const NewtonWorld* const newtonWorld, void* const userData);
NEWTON_API void* NewtonWorldGetUserData (const NewtonWorld* const newtonWorld);
NEWTON_API void NewtonWorldSetDestructorCallBack (const NewtonWorld* const newtonWorld, NewtonDestroyWorld destructor);
NEWTON_API NewtonDestroyWorld NewtonWorldGetDestructorCallBack (const NewtonWorld* const newtonWorld);

NEWTON_API void NewtonWorldRayCast (const NewtonWorld* const newtonWorld, const dFloat* const p0, const dFloat* const p1, NewtonWorldRayFilterCallback filter, void* const userData,
                  NewtonWorldRayPrefilterCallback prefilter);
NEWTON_API int NewtonWorldConvexCast (const NewtonWorld* const newtonWorld, const dFloat* const matrix, const dFloat* const target, const NewtonCollision* shape, dFloat* const hitParam, void* const userData,
                    NewtonWorldRayPrefilterCallback prefilter, NewtonWorldConvexCastReturnInfo* info, int maxContactsCount, int threadIndex);


// world utility functions
NEWTON_API int NewtonWorldGetBodyCount(const NewtonWorld* const newtonWorld);
NEWTON_API int NewtonWorldGetConstraintCount(const NewtonWorld* const newtonWorld);

// NEWTON_API int NewtonGetActiveBodiesCount();
// NEWTON_API int NewtonGetActiveConstraintsCount();
// NEWTON_API dFloat NewtonGetGlobalScale (const NewtonWorld* const newtonWorld);



// **********************************************************************************************
//
// Simulation islands
//
// **********************************************************************************************
NEWTON_API NewtonBody* NewtonIslandGetBody (const void* const island, int bodyIndex);
NEWTON_API void NewtonIslandGetBodyAABB (const void* const island, int bodyIndex, dFloat* const p0, dFloat* const p1);

// **********************************************************************************************
//
// Physics Material Section
//
// **********************************************************************************************
NEWTON_API int NewtonMaterialCreateGroupID(const NewtonWorld* const newtonWorld);
NEWTON_API int NewtonMaterialGetDefaultGroupID(const NewtonWorld* const newtonWorld);
NEWTON_API void NewtonMaterialDestroyAllGroupID(const NewtonWorld* const newtonWorld);

// material definitions that can not be overwritten in function callback
NEWTON_API void* NewtonMaterialGetUserData (const NewtonWorld* const newtonWorld, int id0, int id1);
NEWTON_API void NewtonMaterialSetSurfaceThickness (const NewtonWorld* const newtonWorld, int id0, int id1, dFloat thickness);
NEWTON_API void NewtonMaterialSetContinuousCollisionMode (const NewtonWorld* const newtonWorld, int id0, int id1, int state);
NEWTON_API void NewtonMaterialSetCollisionCallback (const NewtonWorld* const newtonWorld, int id0, int id1, void* const userData,
                          NewtonOnAABBOverlap aabbOverlap, NewtonContactsProcess process);

NEWTON_API void NewtonMaterialSetDefaultSoftness (const NewtonWorld* const newtonWorld, int id0, int id1, dFloat value);
NEWTON_API void NewtonMaterialSetDefaultElasticity (const NewtonWorld* const newtonWorld, int id0, int id1, dFloat elasticCoef);
NEWTON_API void NewtonMaterialSetDefaultCollidable (const NewtonWorld* const newtonWorld, int id0, int id1, int state);
NEWTON_API void NewtonMaterialSetDefaultFriction (const NewtonWorld* const newtonWorld, int id0, int id1, dFloat staticFriction, dFloat kineticFriction);

NEWTON_API NewtonMaterial* NewtonWorldGetFirstMaterial (const NewtonWorld* const newtonWorld);
NEWTON_API NewtonMaterial* NewtonWorldGetNextMaterial (const NewtonWorld* const newtonWorld, const NewtonMaterial* const material);

NEWTON_API NewtonBody* NewtonWorldGetFirstBody (const NewtonWorld* const newtonWorld);
NEWTON_API NewtonBody* NewtonWorldGetNextBody (const NewtonWorld* const newtonWorld, const NewtonBody* const curBody);


// **********************************************************************************************
//
// Physics Contact control functions
//
// **********************************************************************************************
NEWTON_API void *NewtonMaterialGetMaterialPairUserData (const NewtonMaterial* const material);
NEWTON_API unsigned NewtonMaterialGetContactFaceAttribute (const NewtonMaterial* const material);
NEWTON_API NewtonCollision* NewtonMaterialGetBodyCollidingShape (const NewtonMaterial* const material, const NewtonBody* const body);
//NEWTON_API unsigned NewtonMaterialGetBodyCollisionID (const NewtonMaterial* const material, const NewtonBody* const body);
NEWTON_API dFloat NewtonMaterialGetContactNormalSpeed (const NewtonMaterial* const material);
NEWTON_API void NewtonMaterialGetContactForce (const NewtonMaterial* const material, NewtonBody* const body, dFloat* const force);
NEWTON_API void NewtonMaterialGetContactPositionAndNormal (const NewtonMaterial* const material, NewtonBody* const body, dFloat* const posit, dFloat* const normal);
NEWTON_API void NewtonMaterialGetContactTangentDirections (const NewtonMaterial* const material, NewtonBody* const body, dFloat* const dir0, dFloat* const dir1);
NEWTON_API dFloat NewtonMaterialGetContactTangentSpeed (const NewtonMaterial* const material, int index);

NEWTON_API void NewtonMaterialSetContactSoftness (const NewtonMaterial* const material, dFloat softness);
NEWTON_API void NewtonMaterialSetContactElasticity (const NewtonMaterial* const material, dFloat restitution);
NEWTON_API void NewtonMaterialSetContactFrictionState (const NewtonMaterial* const material, int state, int index);
NEWTON_API void NewtonMaterialSetContactFrictionCoef (const NewtonMaterial* const material, dFloat staticFrictionCoef, dFloat kineticFrictionCoef, int index);

NEWTON_API void NewtonMaterialSetContactNormalAcceleration (const NewtonMaterial* const material, dFloat accel);
NEWTON_API void NewtonMaterialSetContactNormalDirection (const NewtonMaterial* const material, const dFloat* const directionVector);

NEWTON_API void NewtonMaterialSetContactTangentAcceleration (const NewtonMaterial* const material, dFloat accel, int index);
NEWTON_API void NewtonMaterialContactRotateTangentDirections (const NewtonMaterial* const material, const dFloat* const directionVector);



// **********************************************************************************************
//
// convex collision primitives creation functions
//
// **********************************************************************************************
NEWTON_API NewtonCollision* NewtonCreateNull (const NewtonWorld* const newtonWorld);
NEWTON_API NewtonCollision* NewtonCreateSphere (const NewtonWorld* const newtonWorld, dFloat radiusX, dFloat radiusY, dFloat radiusZ, int shapeID, const dFloat* const offsetMatrix);
NEWTON_API NewtonCollision* NewtonCreateBox (const NewtonWorld* const newtonWorld, dFloat dx, dFloat dy, dFloat dz, int shapeID, const dFloat* const offsetMatrix);
NEWTON_API NewtonCollision* NewtonCreateCone (const NewtonWorld* const newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat* const offsetMatrix);
NEWTON_API NewtonCollision* NewtonCreateCapsule (const NewtonWorld* const newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat* const offsetMatrix);
NEWTON_API NewtonCollision* NewtonCreateCylinder (const NewtonWorld* const newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat* const offsetMatrix);
NEWTON_API NewtonCollision* NewtonCreateChamferCylinder (const NewtonWorld* const newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat* const offsetMatrix);
NEWTON_API NewtonCollision* NewtonCreateConvexHull (const NewtonWorld* const newtonWorld, int count, const dFloat* const vertexCloud, int strideInBytes, dFloat tolerance, int shapeID, const dFloat* const offsetMatrix);
NEWTON_API NewtonCollision* NewtonCreateConvexHullFromMesh (const NewtonWorld* const newtonWorld, const NewtonMesh* const mesh, dFloat tolerance, int shapeID);

NEWTON_API NewtonCollision* NewtonCreateConvexHullModifier (const NewtonWorld* const newtonWorld, const NewtonCollision* const convexHullCollision, int shapeID);
NEWTON_API void NewtonConvexHullModifierGetMatrix (const NewtonCollision* const convexHullCollision, dFloat* const matrix);
NEWTON_API void NewtonConvexHullModifierSetMatrix (const NewtonCollision* const convexHullCollision, const dFloat* const matrix);

NEWTON_API int NewtonCollisionIsTriggerVolume(const NewtonCollision* const convexCollision);
NEWTON_API void NewtonCollisionSetAsTriggerVolume(const NewtonCollision* convexCollision, int trigger);

NEWTON_API void NewtonCollisionSetMaxBreakImpactImpulse(const NewtonCollision* const convexHullCollision, dFloat maxImpactImpulse);
NEWTON_API dFloat NewtonCollisionGetMaxBreakImpactImpulse(const NewtonCollision* const convexHullCollision);

NEWTON_API void NewtonCollisionSetUserID (const NewtonCollision* convexCollision, unsigned id);
NEWTON_API unsigned NewtonCollisionGetUserID (const NewtonCollision* const convexCollision);

NEWTON_API int NewtonConvexHullGetFaceIndices (const NewtonCollision* const convexHullCollision, int face, int* const faceIndices);
NEWTON_API dFloat NewtonConvexCollisionCalculateVolume (const NewtonCollision* const convexCollision);
NEWTON_API void NewtonConvexCollisionCalculateInertialMatrix (const NewtonCollision* convexCollision, dFloat* const inertia, dFloat* const origin);


NEWTON_API void NewtonCollisionMakeUnique (const NewtonWorld* const newtonWorld, const NewtonCollision* const collision);
NEWTON_API void NewtonReleaseCollision (const NewtonWorld* const newtonWorld, const NewtonCollision* const collision);
NEWTON_API int NewtonAddCollisionReference (const NewtonCollision* const collision);


// **********************************************************************************************
//
// mass/spring/damper collision shape
//
// **********************************************************************************************
//	NEWTON_API NewtonCollision* NewtonCreateSoftShape (const NewtonWorld* const newtonWorld);
//	NEWTON_API void NewtonSoftBodySetMassCount (const NewtonCollision* convexCollision, int count);
//	NEWTON_API void NewtonSoftBodySetSpringCount (const NewtonCollision* convexCollision, int count);

//	NEWTON_API void NewtonSoftBodySetMass (const NewtonCollision* convexCollision, int index, dFloat mass, dFloat* position);
//	NEWTON_API int NewtonSoftBodySetSpring (const NewtonCollision* convexCollision, int index, int mass0, int mass1, dFloat stiffness, dFloat damper);
//	NEWTON_API int NewtonSoftBodyGetMassArray (const NewtonCollision* convexCollision, dFloat* masses, dFloat** positions);


// **********************************************************************************************
//
// complex collision primitives creation functions
//
// **********************************************************************************************
NEWTON_API NewtonCollision* NewtonCreateCompoundCollision (const NewtonWorld* const newtonWorld, int count, NewtonCollision* const collisionPrimitiveArray[], int shapeID);
NEWTON_API NewtonCollision* NewtonCreateCompoundCollisionFromMesh (const NewtonWorld* const newtonWorld, const NewtonMesh* const mesh, dFloat hullTolerance, int shapeID, int subShapeID);
//NEWTON_API NewtonCollision* NewtonCreateCompoundCollisionFromMesh(const NewtonWorld* const newtonWorld, const NewtonMesh* const mesh, int maxSubShapesCount, int shapeID, int subShapeID);

// **********************************************************************************************
//
// complex breakable collision primitives interface
//
// **********************************************************************************************
//	NEWTON_API NewtonCollision* NewtonCreateCompoundBreakable (const NewtonWorld* const newtonWorld, int meshCount,
//															   NewtonMesh* const solids[], NewtonMesh* const splitePlanes[],
//															   dFloat* const matrixPallete, int* const shapeIDArray, dFloat* const densities,
//															   int shapeID, int debriID, NewtonCollisionCompoundBreakableCallback callback, void* buildUsedData);

NEWTON_API NewtonCollision* NewtonCreateCompoundBreakable (const NewtonWorld* const newtonWorld, int meshCount,
                               const NewtonMesh** const solids, const int* const shapeIDArray,
                               const dFloat* const densities, const int* const internalFaceMaterial,
                               int shapeID, int debriID, dFloat debriSeparationGap);


NEWTON_API void NewtonCompoundBreakableResetAnchoredPieces (const NewtonCollision* const compoundBreakable);
NEWTON_API void NewtonCompoundBreakableSetAnchoredPieces (const NewtonCollision* const compoundBreakable, int fixShapesCount, dFloat* const matrixPallete, NewtonCollision** const fixedShapesArray);

NEWTON_API int NewtonCompoundBreakableGetVertexCount (const NewtonCollision* const compoundBreakable);
NEWTON_API void NewtonCompoundBreakableGetVertexStreams (const NewtonCollision* const compoundBreakable, int vertexStrideInByte, dFloat* const vertex,
                             int normalStrideInByte, dFloat* const normal,	int uvStrideInByte, dFloat* const uv);


NEWTON_API NewtonbreakableComponentMesh* NewtonBreakableGetMainMesh (const NewtonCollision* const compoundBreakable);
NEWTON_API NewtonbreakableComponentMesh* NewtonBreakableGetFirstComponent (const NewtonCollision* const compoundBreakable);
NEWTON_API NewtonbreakableComponentMesh* NewtonBreakableGetNextComponent (const NewtonbreakableComponentMesh* const component);

NEWTON_API void NewtonBreakableBeginDelete (const NewtonCollision* const compoundBreakable);
NEWTON_API NewtonBody* NewtonBreakableCreateDebrieBody (const NewtonCollision* const compoundBreakable, const NewtonbreakableComponentMesh* const component);
NEWTON_API void NewtonBreakableDeleteComponent (const NewtonCollision* const compoundBreakable, const NewtonbreakableComponentMesh* const component);
NEWTON_API void NewtonBreakableEndDelete (const NewtonCollision* const compoundBreakable);


NEWTON_API int NewtonBreakableGetComponentsInRadius (const NewtonCollision* const compoundBreakable, const dFloat* position, dFloat radius, NewtonbreakableComponentMesh** const segments, int maxCount);

NEWTON_API void* NewtonBreakableGetFirstSegment (const NewtonbreakableComponentMesh* const breakableComponent);
NEWTON_API void* NewtonBreakableGetNextSegment (const void* const segment);

NEWTON_API int NewtonBreakableSegmentGetMaterial (const void* const segment);
NEWTON_API int NewtonBreakableSegmentGetIndexCount (const void* const segment);
NEWTON_API int NewtonBreakableSegmentGetIndexStream (const NewtonCollision* const compoundBreakable, const NewtonbreakableComponentMesh* const meshOwner, const void* const segment, int* const index);
NEWTON_API int NewtonBreakableSegmentGetIndexStreamShort (const NewtonCollision* const compoundBreakable, const NewtonbreakableComponentMesh* const meshOwner, const void* const segment, short int* const index);


NEWTON_API NewtonCollision* NewtonCreateUserMeshCollision (const NewtonWorld* const newtonWorld, const dFloat* const minBox,
                               const dFloat* const maxBox, void* const userData, NewtonUserMeshCollisionCollideCallback collideCallback,
                               NewtonUserMeshCollisionRayHitCallback rayHitCallback, NewtonUserMeshCollisionDestroyCallback destroyCallback,
                               NewtonUserMeshCollisionGetCollisionInfo getInfoCallback, NewtonUserMeshCollisionGetFacesInAABB facesInAABBCallback, int shapeID);


NEWTON_API NewtonCollision* NewtonCreateSceneCollision (const NewtonWorld* const newtonWorld, int shapeID);
NEWTON_API NewtonSceneProxy* NewtonSceneCollisionCreateProxy (NewtonCollision* const scene, NewtonCollision* const collision, const dFloat* const matrix);

NEWTON_API void NewtonSceneCollisionDestroyProxy (NewtonCollision* const scene, NewtonSceneProxy* Proxy);
NEWTON_API void NewtonSceneProxySetMatrix (NewtonSceneProxy* const proxy, const dFloat* matrix);
NEWTON_API void NewtonSceneProxyGetMatrix (NewtonSceneProxy* const proxy, dFloat* matrix);
NEWTON_API void NewtonSceneSetProxyUserData (NewtonSceneProxy* const proxy, void* userData);
NEWTON_API void* NewtonSceneGetProxyUserData (NewtonSceneProxy* const proxy);

NEWTON_API NewtonSceneProxy* NewtonSceneGetFirstProxy (NewtonCollision* const scene);
NEWTON_API NewtonSceneProxy* NewtonSceneGetNextProxy (NewtonCollision* const scene, NewtonSceneProxy* const proxy);

NEWTON_API void NewtonSceneCollisionOptimize (NewtonCollision* scene);

//  ***********************************************************************************************************
//
//	Collision serialization functions
//
// ***********************************************************************************************************
NEWTON_API NewtonCollision* NewtonCreateCollisionFromSerialization (const NewtonWorld* const newtonWorld, NewtonDeserialize deserializeFunction, void* const serializeHandle);
NEWTON_API void NewtonCollisionSerialize (const NewtonWorld* const newtonWorld, const NewtonCollision* const collision, NewtonSerialize serializeFunction, void* const serializeHandle);
NEWTON_API void NewtonCollisionGetInfo (const NewtonCollision* const collision, NewtonCollisionInfoRecord* const collisionInfo);

// **********************************************************************************************
//
// Static collision shapes functions
//
// **********************************************************************************************
NEWTON_API NewtonCollision* NewtonCreateHeightFieldCollision (const NewtonWorld* const newtonWorld, int width, int height, int gridsDiagonals,
                                const unsigned short* const elevationMap, const char* const attributeMap,
                                dFloat horizontalScale, dFloat verticalScale, int shapeID);
NEWTON_API void NewtonHeightFieldSetUserRayCastCallback (const NewtonCollision* const treeCollision, NewtonHeightFieldRayCastCallback rayHitCallback);


NEWTON_API NewtonCollision* NewtonCreateTreeCollision (const NewtonWorld* const newtonWorld, int shapeID);
NEWTON_API NewtonCollision* NewtonCreateTreeCollisionFromMesh (const NewtonWorld* const newtonWorld, const NewtonMesh* const mesh, int shapeID);

NEWTON_API void NewtonTreeCollisionSetUserRayCastCallback (const NewtonCollision* const treeCollision, NewtonCollisionTreeRayCastCallback rayHitCallback);

NEWTON_API void NewtonTreeCollisionBeginBuild (const NewtonCollision* treeCollision);
NEWTON_API void NewtonTreeCollisionAddFace (const NewtonCollision* const treeCollision, int vertexCount, const dFloat* const vertexPtr, int strideInBytes, int faceAttribute);
NEWTON_API void NewtonTreeCollisionEndBuild (const NewtonCollision* const treeCollision, int optimize);

NEWTON_API int NewtonTreeCollisionGetFaceAtribute (const NewtonCollision* const treeCollision, const int* const faceIndexArray);
NEWTON_API void NewtonTreeCollisionSetFaceAtribute (const NewtonCollision* const treeCollision, const int* const faceIndexArray, int attribute);
NEWTON_API int NewtonTreeCollisionGetVertexListIndexListInAABB (const NewtonCollision* const treeCollision, const dFloat* const p0, const dFloat* const p1, const dFloat** const vertexArray, int* const vertexCount, int* const vertexStrideInBytes, const int* const indexList, int maxIndexCount, const int* const faceAttribute);


NEWTON_API void NewtonStaticCollisionSetDebugCallback (const NewtonCollision* const staticCollision, NewtonTreeCollisionCallback userCallback);

// **********************************************************************************************
//
// General purpose collision library functions
//
// **********************************************************************************************
NEWTON_API int NewtonCollisionPointDistance (const NewtonWorld* const newtonWorld, const dFloat* const point,
                       const NewtonCollision* const collision, const dFloat* const matrix, dFloat* const contact, dFloat* const normal, int threadIndex);

NEWTON_API int NewtonCollisionClosestPoint (const NewtonWorld* const newtonWorld,
                      const NewtonCollision* const collisionA, const dFloat* const matrixA, const NewtonCollision* const collisionB, const dFloat* const matrixB,
                      dFloat* const contactA, dFloat* const contactB, dFloat* const normalAB, int threadIndex);

NEWTON_API int NewtonCollisionCollide (const NewtonWorld* const newtonWorld, int maxSize,
                     const NewtonCollision* const collisionA, const dFloat* const matrixA,
                     const NewtonCollision* const collisionB, const dFloat* const matrixB,
                     dFloat* const contacts, dFloat* const normals, dFloat* const penetration, int threadIndex);

NEWTON_API int NewtonCollisionCollideContinue (const NewtonWorld* const newtonWorld, int maxSize, const dFloat timestep,
                         const NewtonCollision* const collisionA, const dFloat* const matrixA, const dFloat* const velocA, const dFloat* omegaA,
                         const NewtonCollision* const collisionB, const dFloat* const matrixB, const dFloat* const velocB, const dFloat* const omegaB,
                         dFloat* const timeOfImpact, dFloat* const contacts, dFloat* const normals, dFloat* const penetration, int threadIndex);

NEWTON_API void NewtonCollisionSupportVertex (const NewtonCollision* const collision, const dFloat* const dir, dFloat* const vertex);
NEWTON_API dFloat NewtonCollisionRayCast (const NewtonCollision* const collision, const dFloat* const p0, const dFloat* const p1, dFloat* const normal, int* const attribute);
NEWTON_API void NewtonCollisionCalculateAABB (const NewtonCollision* const collision, const dFloat* const matrix, dFloat* const p0, dFloat* const p1);
NEWTON_API void NewtonCollisionForEachPolygonDo (const NewtonCollision* const collision, const dFloat* const matrix, NewtonCollisionIterator callback, void* const userData);


// **********************************************************************************************
//
// transforms utility functions
//
// **********************************************************************************************
NEWTON_API void NewtonGetEulerAngle (const dFloat* const matrix, dFloat* const eulersAngles);
NEWTON_API void NewtonSetEulerAngle (const dFloat* const eulersAngles, dFloat* const matrix);
NEWTON_API dFloat NewtonCalculateSpringDamperAcceleration (dFloat dt, dFloat ks, dFloat x, dFloat kd, dFloat s);

// **********************************************************************************************
//
// body manipulation functions
//
// **********************************************************************************************
NEWTON_API NewtonBody* NewtonCreateBody (const NewtonWorld* const newtonWorld, const NewtonCollision* const collision, const dFloat* const matrix);
NEWTON_API void  NewtonDestroyBody(const NewtonWorld* const newtonWorld, const NewtonBody* const body);

NEWTON_API void  NewtonBodyAddForce (const NewtonBody* const body, const dFloat* const force);
NEWTON_API void  NewtonBodyAddTorque (const NewtonBody* const body, const dFloat* const torque);
NEWTON_API void  NewtonBodyCalculateInverseDynamicsForce (const NewtonBody* const body, dFloat timestep, const dFloat* const desiredVeloc, dFloat* const forceOut);

NEWTON_API void  NewtonBodySetMatrix (const NewtonBody* const body, const dFloat* const matrix);
NEWTON_API void  NewtonBodySetMatrixRecursive (const NewtonBody* const body, const dFloat* const matrix);
NEWTON_API void  NewtonBodySetMassMatrix (const NewtonBody* const body, dFloat mass, dFloat Ixx, dFloat Iyy, dFloat Izz);
NEWTON_API void  NewtonBodySetMaterialGroupID (const NewtonBody* const body, int id);
NEWTON_API void  NewtonBodySetContinuousCollisionMode (const NewtonBody* const body, unsigned state);
NEWTON_API void  NewtonBodySetJointRecursiveCollision (const NewtonBody* const body, unsigned state);
NEWTON_API void  NewtonBodySetOmega (const NewtonBody* const body, const dFloat* const omega);
NEWTON_API void  NewtonBodySetVelocity (const NewtonBody* const body, const dFloat* const velocity);
NEWTON_API void  NewtonBodySetForce (const NewtonBody* const body, const dFloat* const force);
NEWTON_API void  NewtonBodySetTorque (const NewtonBody* const body, const dFloat* const torque);

NEWTON_API void  NewtonBodySetCentreOfMass  (const NewtonBody* const body, const dFloat* const com);
NEWTON_API void  NewtonBodySetLinearDamping (const NewtonBody* const body, dFloat linearDamp);
NEWTON_API void  NewtonBodySetAngularDamping (const NewtonBody* const body, const dFloat* const angularDamp);
NEWTON_API void  NewtonBodySetUserData (const NewtonBody* const body, void* const userData);
NEWTON_API void  NewtonBodySetCollision (const NewtonBody* const body, const NewtonCollision* const collision);



NEWTON_API int  NewtonBodyGetSleepState (const NewtonBody* const body);
NEWTON_API int  NewtonBodyGetAutoSleep (const NewtonBody* const body);
NEWTON_API void NewtonBodySetAutoSleep (const NewtonBody* const body, int state);

NEWTON_API int  NewtonBodyGetFreezeState(const NewtonBody* const body);
NEWTON_API void NewtonBodySetFreezeState (const NewtonBody* const body, int state);


//	NEWTON_API void NewtonBodySetAutoFreeze(const NewtonBody* const body, int state);
//	NEWTON_API void NewtonBodyCoriolisForcesMode (const NewtonBody* const body, int mode);
//	NEWTON_API void NewtonBodySetGyroscopicForcesMode (const NewtonBody* const body, int mode);
//	NEWTON_API int  NewtonBodyGetGyroscopicForcesMode (const NewtonBody* const body);
//	NEWTON_API int  NewtonBodyGetFreezeState (const NewtonBody* const body);
//	NEWTON_API void NewtonBodySetFreezeState  (const NewtonBody* const body, int state);
//	NEWTON_API void NewtonBodyGetFreezeTreshold (const NewtonBody* const body, dFloat* freezeSpeed2, dFloat* freezeOmega2);
//	NEWTON_API void NewtonBodySetFreezeTreshold (const NewtonBody* const body, dFloat freezeSpeed2, dFloat freezeOmega2, int framesCount);
//	NEWTON_API void NewtonBodySetAutoactiveCallback (const NewtonBody* const body, NewtonBodyActivationState callback);


NEWTON_API void  NewtonBodySetDestructorCallback (const NewtonBody* const body, NewtonBodyDestructor callback);

NEWTON_API void  NewtonBodySetTransformCallback (const NewtonBody* const body, NewtonSetTransform callback);
NEWTON_API NewtonSetTransform NewtonBodyGetTransformCallback (const NewtonBody* const body);

NEWTON_API void  NewtonBodySetForceAndTorqueCallback (const NewtonBody* const body, NewtonApplyForceAndTorque callback);
NEWTON_API NewtonApplyForceAndTorque NewtonBodyGetForceAndTorqueCallback (const NewtonBody* const body);
NEWTON_API void* NewtonBodyGetUserData (const NewtonBody* const body);
NEWTON_API NewtonWorld* NewtonBodyGetWorld (const NewtonBody* const body);
NEWTON_API NewtonCollision* NewtonBodyGetCollision (const NewtonBody* const body);
NEWTON_API int   NewtonBodyGetMaterialGroupID (const NewtonBody* const body);

NEWTON_API int   NewtonBodyGetContinuousCollisionMode (const NewtonBody* const body);
NEWTON_API int   NewtonBodyGetJointRecursiveCollision (const NewtonBody* const body);

NEWTON_API void  NewtonBodyGetMatrix(const NewtonBody* const body, dFloat* const matrix);
NEWTON_API void  NewtonBodyGetRotation(const NewtonBody* const body, dFloat* const rotation);
NEWTON_API void  NewtonBodyGetMassMatrix (const NewtonBody* const body, dFloat* mass, dFloat* const Ixx, dFloat* const Iyy, dFloat* const Izz);
NEWTON_API void  NewtonBodyGetInvMass(const NewtonBody* const body, dFloat* const invMass, dFloat* const invIxx, dFloat* const invIyy, dFloat* const invIzz);
NEWTON_API void  NewtonBodyGetOmega(const NewtonBody* const body, dFloat* const vector);
NEWTON_API void  NewtonBodyGetVelocity(const NewtonBody* const body, dFloat* const vector);
NEWTON_API void  NewtonBodyGetForce(const NewtonBody* const body, dFloat* const vector);
NEWTON_API void  NewtonBodyGetTorque(const NewtonBody* const body, dFloat* const vector);
NEWTON_API void  NewtonBodyGetForceAcc(const NewtonBody* const body, dFloat* const vector);
NEWTON_API void  NewtonBodyGetTorqueAcc(const NewtonBody* const body, dFloat* const vector);
NEWTON_API void  NewtonBodyGetCentreOfMass (const NewtonBody* const body, dFloat* const com);

NEWTON_API dFloat NewtonBodyGetLinearDamping (const NewtonBody* const body);
NEWTON_API void  NewtonBodyGetAngularDamping (const NewtonBody* const body, dFloat* const vector);
NEWTON_API void  NewtonBodyGetAABB (const NewtonBody* const body, dFloat* const p0, dFloat* const p1);
NEWTON_API NewtonJoint* NewtonBodyGetFirstJoint (const NewtonBody* const body);
NEWTON_API NewtonJoint* NewtonBodyGetNextJoint (const NewtonBody* const body, const NewtonJoint* const joint);
NEWTON_API NewtonJoint* NewtonBodyGetFirstContactJoint (const NewtonBody* const body);
NEWTON_API NewtonJoint* NewtonBodyGetNextContactJoint (const NewtonBody* const body, const NewtonJoint* const contactJoint);

NEWTON_API void* NewtonContactJointGetFirstContact (const NewtonJoint* const contactJoint);
NEWTON_API void* NewtonContactJointGetNextContact (const NewtonJoint* const contactJoint, void* const contact);

NEWTON_API int NewtonContactJointGetContactCount(const NewtonJoint* const contactJoint);
NEWTON_API void NewtonContactJointRemoveContact(const NewtonJoint* const contactJoint, void* const contact);

NEWTON_API NewtonMaterial* NewtonContactGetMaterial (const void* const contact);


NEWTON_API void  NewtonBodyAddBuoyancyForce (const NewtonBody* const body, dFloat fluidDensity,
                       dFloat fluidLinearViscosity, dFloat fluidAngularViscosity,
                       const dFloat* const gravityVector, NewtonGetBuoyancyPlane buoyancyPlane, void* const context);

//	NEWTON_API void NewtonBodyForEachPolygonDo (const NewtonBody* const body, NewtonCollisionIterator callback);
NEWTON_API void NewtonBodyAddImpulse (const NewtonBody* const body, const dFloat* const pointDeltaVeloc, const dFloat* const pointPosit);


NEWTON_API void NewtonBodyApplyImpulseArray (const NewtonBody* const body, int impuleCount, int strideInByte, const dFloat* const impulseArray, const dFloat* const pointArray);


// **********************************************************************************************
//
// Common joint functions
//
// **********************************************************************************************
NEWTON_API void* NewtonJointGetUserData (const NewtonJoint* const joint);
NEWTON_API void NewtonJointSetUserData (const NewtonJoint* const joint, void* const userData);

NEWTON_API NewtonBody* NewtonJointGetBody0 (const NewtonJoint* const joint);
NEWTON_API NewtonBody* NewtonJointGetBody1 (const NewtonJoint* const joint);

NEWTON_API void NewtonJointGetInfo  (const NewtonJoint* const joint, NewtonJointRecord* const info);
NEWTON_API int NewtonJointGetCollisionState (const NewtonJoint* const joint);
NEWTON_API void NewtonJointSetCollisionState (const NewtonJoint* const joint, int state);

NEWTON_API dFloat NewtonJointGetStiffness (const NewtonJoint* const joint);
NEWTON_API void NewtonJointSetStiffness (const NewtonJoint* const joint, dFloat state);

NEWTON_API void NewtonDestroyJoint(const NewtonWorld* const newtonWorld, const NewtonJoint* const joint);
NEWTON_API void NewtonJointSetDestructor (const NewtonJoint* const joint, NewtonConstraintDestructor destructor);




// **********************************************************************************************
//
// Ball and Socket joint functions
//
// **********************************************************************************************
NEWTON_API NewtonJoint* NewtonConstraintCreateBall (const NewtonWorld* const newtonWorld, const dFloat* pivotPoint,
                          const NewtonBody* const childBody, const NewtonBody* const parentBody);
NEWTON_API void NewtonBallSetUserCallback (const NewtonJoint* const ball, NewtonBallCallBack callback);
NEWTON_API void NewtonBallGetJointAngle (const NewtonJoint* const ball, dFloat* angle);
NEWTON_API void NewtonBallGetJointOmega (const NewtonJoint* const ball, dFloat* omega);
NEWTON_API void NewtonBallGetJointForce (const NewtonJoint* const ball, dFloat* const force);
NEWTON_API void NewtonBallSetConeLimits (const NewtonJoint* const ball, const dFloat* pin, dFloat maxConeAngle, dFloat maxTwistAngle);

// **********************************************************************************************
//
// Hinge joint functions
//
// **********************************************************************************************
NEWTON_API NewtonJoint* NewtonConstraintCreateHinge (const NewtonWorld* const newtonWorld,
                           const dFloat* pivotPoint, const dFloat* pinDir,
                           const NewtonBody* const childBody, const NewtonBody* const parentBody);

NEWTON_API void NewtonHingeSetUserCallback (const NewtonJoint* const hinge, NewtonHingeCallBack callback);
NEWTON_API dFloat NewtonHingeGetJointAngle (const NewtonJoint* const hinge);
NEWTON_API dFloat NewtonHingeGetJointOmega (const NewtonJoint* const hinge);
NEWTON_API void NewtonHingeGetJointForce (const NewtonJoint* const hinge, dFloat* const force);
NEWTON_API dFloat NewtonHingeCalculateStopAlpha (const NewtonJoint* const hinge, const NewtonHingeSliderUpdateDesc* const desc, dFloat angle);

// **********************************************************************************************
//
// Slider joint functions
//
// **********************************************************************************************
NEWTON_API NewtonJoint* NewtonConstraintCreateSlider (const NewtonWorld* const newtonWorld,
                            const dFloat* pivotPoint, const dFloat* pinDir,
                            const NewtonBody* const childBody, const NewtonBody* const parentBody);
NEWTON_API void NewtonSliderSetUserCallback (const NewtonJoint* const slider, NewtonSliderCallBack callback);
NEWTON_API dFloat NewtonSliderGetJointPosit (const NewtonJoint* slider);
NEWTON_API dFloat NewtonSliderGetJointVeloc (const NewtonJoint* slider);
NEWTON_API void NewtonSliderGetJointForce (const NewtonJoint* const slider, dFloat* const force);
NEWTON_API dFloat NewtonSliderCalculateStopAccel (const NewtonJoint* const slider, const NewtonHingeSliderUpdateDesc* const desc, dFloat position);


// **********************************************************************************************
//
// Corkscrew joint functions
//
// **********************************************************************************************
NEWTON_API NewtonJoint* NewtonConstraintCreateCorkscrew (const NewtonWorld* const newtonWorld,
                             const dFloat* pivotPoint, const dFloat* pinDir,
                             const NewtonBody* const childBody, const NewtonBody* const parentBody);
NEWTON_API void NewtonCorkscrewSetUserCallback (const NewtonJoint* const corkscrew, NewtonCorkscrewCallBack callback);
NEWTON_API dFloat NewtonCorkscrewGetJointPosit (const NewtonJoint* const corkscrew);
NEWTON_API dFloat NewtonCorkscrewGetJointAngle (const NewtonJoint* const corkscrew);
NEWTON_API dFloat NewtonCorkscrewGetJointVeloc (const NewtonJoint* const corkscrew);
NEWTON_API dFloat NewtonCorkscrewGetJointOmega (const NewtonJoint* const corkscrew);
NEWTON_API void NewtonCorkscrewGetJointForce (const NewtonJoint* const corkscrew, dFloat* const force);
NEWTON_API dFloat NewtonCorkscrewCalculateStopAlpha (const NewtonJoint* const corkscrew, const NewtonHingeSliderUpdateDesc* const desc, dFloat angle);
NEWTON_API dFloat NewtonCorkscrewCalculateStopAccel (const NewtonJoint* const corkscrew, const NewtonHingeSliderUpdateDesc* const desc, dFloat position);


// **********************************************************************************************
//
// Universal joint functions
//
// **********************************************************************************************
NEWTON_API NewtonJoint* NewtonConstraintCreateUniversal (const NewtonWorld* const newtonWorld,
                             const dFloat* pivotPoint, const dFloat* pinDir0, const dFloat* pinDir1,
                             const NewtonBody* const childBody, const NewtonBody* const parentBody);
NEWTON_API void NewtonUniversalSetUserCallback (const NewtonJoint* const universal, NewtonUniversalCallBack callback);
NEWTON_API dFloat NewtonUniversalGetJointAngle0 (const NewtonJoint* const universal);
NEWTON_API dFloat NewtonUniversalGetJointAngle1 (const NewtonJoint* const universal);
NEWTON_API dFloat NewtonUniversalGetJointOmega0 (const NewtonJoint* const universal);
NEWTON_API dFloat NewtonUniversalGetJointOmega1 (const NewtonJoint* const universal);
NEWTON_API void NewtonUniversalGetJointForce (const NewtonJoint* const universal, dFloat* const force);
NEWTON_API dFloat NewtonUniversalCalculateStopAlpha0 (const NewtonJoint* const universal, const NewtonHingeSliderUpdateDesc* const desc, dFloat angle);
NEWTON_API dFloat NewtonUniversalCalculateStopAlpha1 (const NewtonJoint* const universal, const NewtonHingeSliderUpdateDesc* const desc, dFloat angle);


// **********************************************************************************************
//
// Up vector joint functions
//
// **********************************************************************************************
NEWTON_API NewtonJoint* NewtonConstraintCreateUpVector (const NewtonWorld* const newtonWorld, const dFloat* pinDir, const NewtonBody* const body);
NEWTON_API void NewtonUpVectorGetPin (const NewtonJoint* const upVector, dFloat *pin);
NEWTON_API void NewtonUpVectorSetPin (const NewtonJoint* const upVector, const dFloat *pin);


// **********************************************************************************************
//
// User defined bilateral Joint
//
// **********************************************************************************************
NEWTON_API NewtonJoint* NewtonConstraintCreateUserJoint (const NewtonWorld* const newtonWorld, int maxDOF,
                             NewtonUserBilateralCallBack callback,
                             NewtonUserBilateralGetInfoCallBack getInfo,
                             const NewtonBody* const childBody, const NewtonBody* const parentBody) ;
NEWTON_API void NewtonUserJointSetFeedbackCollectorCallback (const NewtonJoint* const joint, NewtonUserBilateralCallBack getFeedback);
NEWTON_API void NewtonUserJointAddLinearRow (const NewtonJoint* const joint, const dFloat* const pivot0, const dFloat* const pivot1, const dFloat* const dir);
NEWTON_API void NewtonUserJointAddAngularRow (const NewtonJoint* const joint, dFloat relativeAngle, const dFloat* const dir);
NEWTON_API void NewtonUserJointAddGeneralRow (const NewtonJoint* const joint, const dFloat* const jacobian0, const dFloat* const jacobian1);
NEWTON_API void NewtonUserJointSetRowMinimumFriction (const NewtonJoint* const joint, dFloat friction);
NEWTON_API void NewtonUserJointSetRowMaximumFriction (const NewtonJoint* const joint, dFloat friction);
NEWTON_API void NewtonUserJointSetRowAcceleration (const NewtonJoint* const joint, dFloat acceleration);
NEWTON_API void NewtonUserJointSetRowSpringDamperAcceleration (const NewtonJoint* const joint, dFloat springK, dFloat springD);
NEWTON_API void NewtonUserJointSetRowStiffness (const NewtonJoint* const joint, dFloat stiffness);
NEWTON_API dFloat NewtonUserJointGetRowForce (const NewtonJoint* const joint, int row);


// **********************************************************************************************
//
// Mesh joint functions
//
// **********************************************************************************************
NEWTON_API NewtonMesh* NewtonMeshCreate(const NewtonWorld* const newtonWorld);
NEWTON_API NewtonMesh* NewtonMeshCreateFromMesh(const NewtonMesh* const mesh);
NEWTON_API NewtonMesh* NewtonMeshCreateFromCollision(const NewtonCollision* const collision);
NEWTON_API NewtonMesh* NewtonMeshConvexHull (const NewtonWorld* const newtonWorld, int count, const dFloat* const vertexCloud, int strideInBytes, dFloat tolerance);
NEWTON_API NewtonMesh* NewtonMeshCreatePlane (const NewtonWorld* const newtonWorld, const dFloat* const locationMatrix, dFloat witdth, dFloat breadth, int material, const dFloat* const textureMatrix0, const dFloat* const textureMatrix1);

NEWTON_API void NewtonMeshDestroy(const NewtonMesh* const mesh);

	NEWTON_API void NewtonMesApplyTransform (const NewtonMesh* const mesh, const dFloat* const matrix);
NEWTON_API void NewtonMeshCalculateOOBB(const NewtonMesh* const mesh, dFloat* const matrix, dFloat* const x, dFloat* const y, dFloat* const z);

NEWTON_API void NewtonMeshCalculateVertexNormals(const NewtonMesh* const mesh, dFloat angleInRadians);
NEWTON_API void NewtonMeshApplySphericalMapping(const NewtonMesh* const mesh, int material);
NEWTON_API void NewtonMeshApplyBoxMapping(const NewtonMesh* const mesh, int front, int side, int top);
NEWTON_API void NewtonMeshApplyCylindricalMapping(const NewtonMesh* const mesh, int cylinderMaterial, int capMaterial);

NEWTON_API int NewtonMeshIsOpenMesh (const NewtonMesh* const mesh);
NEWTON_API void NewtonMeshFixTJoints (const NewtonMesh* const mesh);


NEWTON_API void NewtonMeshPolygonize (const NewtonMesh* const mesh);
NEWTON_API void NewtonMeshTriangulate (const NewtonMesh* const mesh);
NEWTON_API NewtonMesh* NewtonMeshUnion (const NewtonMesh* const mesh, const NewtonMesh* const clipper, const dFloat* const clipperMatrix);
NEWTON_API NewtonMesh* NewtonMeshDifference (const NewtonMesh* const mesh, const NewtonMesh* const clipper, const dFloat* const clipperMatrix);
NEWTON_API NewtonMesh* NewtonMeshIntersection (const NewtonMesh* const mesh, const NewtonMesh* const clipper, const dFloat* const clipperMatrix);
NEWTON_API void NewtonMeshClip (const NewtonMesh* const mesh, const NewtonMesh* const clipper, const dFloat* const clipperMatrix, NewtonMesh** const topMesh, NewtonMesh** const bottomMesh);
NEWTON_API void NewtonMeshPlaneClip (const NewtonMesh* const mesh, dFloat* const planeMatrix, const dFloat* const planeTextureMatrix, int planeMaterial, NewtonMesh** const topMesh, NewtonMesh** const bottomMesh);

NEWTON_API NewtonMesh* NewtonMeshApproximateConvexDecomposition (const NewtonMesh* const mesh, dFloat maxConcavity, int maxCount);
NEWTON_API NewtonMesh* NewtonMeshTetrahedralization (const NewtonMesh* const mesh, int internalMaterial, const dFloat* const textureMatrix);
NEWTON_API NewtonMesh* NewtonMeshVoronoiDecomposition (const NewtonMesh* const mesh, int pointCount, int pointStrideInBytes, const dFloat* const pointCloud, int internalMaterial, const dFloat* const textureMatrix);


NEWTON_API void NewtonRemoveUnusedVertices(const NewtonMesh* const mesh, int* const vertexRemapTable);

NEWTON_API void NewtonMeshBeginFace(const NewtonMesh* const mesh);
NEWTON_API void NewtonMeshAddFace(const NewtonMesh* const mesh, int vertexCount, const dFloat* const vertex, int strideInBytes, int materialIndex);
NEWTON_API void NewtonMeshEndFace(const NewtonMesh* const mesh);

NEWTON_API void NewtonMeshBuildFromVertexListIndexList(const NewtonMesh* const mesh,
                             int faceCount, const int* const faceIndexCount, const int* const faceMaterialIndex,
                             const dFloat* const vertex, int vertexStrideInBytes, const int* const vertexIndex,
                             const dFloat* const normal, int normalStrideInBytes, const int* const normalIndex,
                             const dFloat* const uv0, int uv0StrideInBytes, const int* const uv0Index,
                             const dFloat* const uv1, int uv1StrideInBytes, const int* const uv1Index);



NEWTON_API void NewtonMeshGetVertexStreams (const NewtonMesh* const mesh,
                      int vertexStrideInByte, dFloat* const vertex,
                      int normalStrideInByte, dFloat* const normal,
                      int uvStrideInByte0, dFloat* const uv0,
                      int uvStrideInByte1, dFloat* const uv1);
NEWTON_API void NewtonMeshGetIndirectVertexStreams(const NewtonMesh* const mesh,
                           int vertexStrideInByte, dFloat* const vertex, int* const vertexIndices, int* const vertexCount,
                           int normalStrideInByte, dFloat* const normal, int* const normalIndices, int* const normalCount,
                           int uvStrideInByte0, dFloat* const uv0, int* const uvIndices0, int* const uvCount0,
                           int uvStrideInByte1, dFloat* const uv1, int* const uvIndices1, int* const uvCount1);
NEWTON_API void* NewtonMeshBeginHandle (const NewtonMesh* const mesh);
NEWTON_API void NewtonMeshEndHandle (const NewtonMesh* const mesh, void* const handle);
NEWTON_API int NewtonMeshFirstMaterial (const NewtonMesh* const mesh, void* const handle);
NEWTON_API int NewtonMeshNextMaterial (const NewtonMesh* const mesh, void* const handle, int materialId);
NEWTON_API int NewtonMeshMaterialGetMaterial (const NewtonMesh* const mesh, void* const handle, int materialId);
NEWTON_API int NewtonMeshMaterialGetIndexCount (const NewtonMesh* const mesh, void* const handle, int materialId);
NEWTON_API void NewtonMeshMaterialGetIndexStream (const NewtonMesh* const mesh, void* const handle, int materialId, int* const index);
NEWTON_API void NewtonMeshMaterialGetIndexStreamShort (const NewtonMesh* const mesh, void* const handle, int materialId, short int* const index);

NEWTON_API NewtonMesh* NewtonMeshCreateFirstSingleSegment (const NewtonMesh* const mesh);
NEWTON_API NewtonMesh* NewtonMeshCreateNextSingleSegment (const NewtonMesh* const mesh, const NewtonMesh* const segment);

NEWTON_API NewtonMesh* NewtonMeshCreateFirstLayer (const NewtonMesh* const mesh);
NEWTON_API NewtonMesh* NewtonMeshCreateNextLayer (const NewtonMesh* const mesh, const NewtonMesh* const segment);


NEWTON_API int NewtonMeshGetTotalFaceCount (const NewtonMesh* const mesh);
NEWTON_API int NewtonMeshGetTotalIndexCount (const NewtonMesh* const mesh);
NEWTON_API void NewtonMeshGetFaces (const NewtonMesh* const mesh, int* const faceIndexCount, int* const faceMaterial, void** const faceIndices);


NEWTON_API int NewtonMeshGetPointCount (const NewtonMesh* const mesh);
NEWTON_API int NewtonMeshGetPointStrideInByte (const NewtonMesh* const mesh);
NEWTON_API dFloat64* NewtonMeshGetPointArray (const NewtonMesh* const mesh);
NEWTON_API dFloat64* NewtonMeshGetNormalArray (const NewtonMesh* const mesh);
NEWTON_API dFloat64* NewtonMeshGetUV0Array (const NewtonMesh* const mesh);
NEWTON_API dFloat64* NewtonMeshGetUV1Array (const NewtonMesh* const mesh);

NEWTON_API int NewtonMeshGetVertexCount (const NewtonMesh* const mesh);
NEWTON_API int NewtonMeshGetVertexStrideInByte (const NewtonMesh* const mesh);
NEWTON_API dFloat64* NewtonMeshGetVertexArray (const NewtonMesh* const mesh);


NEWTON_API void* NewtonMeshGetFirstVertex (const NewtonMesh* const mesh);
NEWTON_API void* NewtonMeshGetNextVertex (const NewtonMesh* const mesh, const void* const vertex);
NEWTON_API int NewtonMeshGetVertexIndex (const NewtonMesh* const mesh, const void* const vertex);

NEWTON_API void* NewtonMeshGetFirstPoint (const NewtonMesh* const mesh);
NEWTON_API void* NewtonMeshGetNextPoint (const NewtonMesh* const mesh, const void* const point);
NEWTON_API int NewtonMeshGetPointIndex (const NewtonMesh* const mesh, const void* const point);
NEWTON_API int NewtonMeshGetVertexIndexFromPoint (const NewtonMesh* const mesh, const void* const point);


NEWTON_API void* NewtonMeshGetFirstEdge (const NewtonMesh* const mesh);
NEWTON_API void* NewtonMeshGetNextEdge (const NewtonMesh* const mesh, const void* const edge);
NEWTON_API void NewtonMeshGetEdgeIndices (const NewtonMesh* const mesh, const void* const edge, int* const v0, int* const v1);
//NEWTON_API void NewtonMeshGetEdgePointIndices (const NewtonMesh* const mesh, const void* const edge, int* const v0, int* const v1);

NEWTON_API void* NewtonMeshGetFirstFace (const NewtonMesh* const mesh);
NEWTON_API void* NewtonMeshGetNextFace (const NewtonMesh* const mesh, const void* const face);
NEWTON_API int NewtonMeshIsFaceOpen (const NewtonMesh* const mesh, const void* const face);
NEWTON_API int NewtonMeshGetFaceMaterial (const NewtonMesh* const mesh, const void* const face);
NEWTON_API int NewtonMeshGetFaceIndexCount (const NewtonMesh* const mesh, const void* const face);
NEWTON_API void NewtonMeshGetFaceIndices (const NewtonMesh* const mesh, const void* const face, int* const indices);
NEWTON_API void NewtonMeshGetFacePointIndices (const NewtonMesh* const mesh, const void* const face, int* const indices);


#ifdef __cplusplus
}
#endif

#endif



