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

#ifndef __NEWTON_H__
#define __NEWTON_H__


#define NEWTON_MAJOR_VERSION 3 
#define NEWTON_MINOR_VERSION 13 


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


#ifdef __GNUC__
#	define NEWTON_DEPRECATED_API __attribute__((deprecated))
#elif defined(_MSC_VER)
#	define NEWTON_DEPRECATED_API __declspec(deprecated)
#else
#	warning NEWTON_DEPRECATED_API not implemented for this compiler
#	define NEWTON_DEPRECATED_API
#endif


#ifndef dLong
	#define dLong long long		
#endif

#ifndef dFloat
	#ifdef _NEWTON_USE_DOUBLE
		#define dFloat double
	#else
		#define dFloat float
	#endif
#endif

#ifndef dFloat64
	#define dFloat64 double
#endif


#ifdef __cplusplus 
extern "C" {
#endif
	
	#define NEWTON_BROADPHASE_DEFAULT						0
	#define NEWTON_BROADPHASE_PERSINTENT					1

	#define NEWTON_DYNAMIC_BODY								0
	#define NEWTON_KINEMATIC_BODY							1
	#define NEWTON_DEFORMABLE_BODY							2

	#define SERIALIZE_ID_SPHERE								0
	#define SERIALIZE_ID_CAPSULE							1
	#define SERIALIZE_ID_CHAMFERCYLINDER					2
	#define SERIALIZE_ID_TAPEREDCAPSULE						3
	#define SERIALIZE_ID_CYLINDER							4
	#define SERIALIZE_ID_TAPEREDCYLINDER					5
	#define SERIALIZE_ID_BOX								6	
	#define SERIALIZE_ID_CONE								7
	#define SERIALIZE_ID_CONVEXHULL							8
	#define SERIALIZE_ID_NULL								9
	#define SERIALIZE_ID_COMPOUND							10
	#define SERIALIZE_ID_TREE								11
	#define SERIALIZE_ID_HEIGHTFIELD						12
	#define SERIALIZE_ID_CLOTH_PATCH						13
	#define SERIALIZE_ID_DEFORMABLE_SOLID					14
	#define SERIALIZE_ID_USERMESH							15
	#define SERIALIZE_ID_SCENE								16
	#define SERIALIZE_ID_FRACTURED_COMPOUND					17

#ifdef __cplusplus
	class NewtonMesh;
	class NewtonBody;
	class NewtonWorld;
	class NewtonJoint;
	class NewtonMaterial;
	class NewtonCollision;
	class NewtonAcyclicArticulation;
	class NewtonDeformableMeshSegment;
	class NewtonFracturedCompoundMeshPart;
#else
	typedef struct NewtonMesh{} NewtonMesh;
	typedef struct NewtonBody{} NewtonBody;
	typedef struct NewtonWorld{} NewtonWorld;
	typedef struct NewtonJoint{} NewtonJoint;
	typedef struct NewtonMaterial{} NewtonMaterial;
	typedef struct NewtonCollision{} NewtonCollision;
	typedef struct NewtonAcyclicArticulation{} NewtonAcyclicArticulation;
	typedef struct NewtonDeformableMeshSegment{} NewtonDeformableMeshSegment;
	typedef struct NewtonFracturedCompoundMeshPart{} NewtonFracturedCompoundMeshPart;
#endif


	typedef struct NewtonBoxParam
	{
		dFloat m_x;
		dFloat m_y;
		dFloat m_z;
	} NewtonBoxParam;

	typedef struct NewtonSphereParam
	{
		dFloat m_radio;
	} NewtonSphereParam;

	typedef struct NewtonCylinderParam
	{
		dFloat m_radio;
		dFloat m_height;
	} NewtonCylinderParam;

	typedef struct NewtonCapsuleParam
	{
		dFloat m_radio;
		dFloat m_height;
	} NewtonCapsuleParam;

	typedef struct NewtonConeParam
	{
		dFloat m_radio;
		dFloat m_height;
	} NewtonConeParam;

	typedef struct NewtonTaperedCapsuleParam
	{
		dFloat m_radio0;
		dFloat m_radio1;
		dFloat m_height;
	} NewtonTaperedCapsuleParam;


	typedef struct NewtonTaperedCylinderParam
	{
		dFloat m_radio0;
		dFloat m_radio1;
		dFloat m_height;
	} NewtonTaperedCylinderParam;

	typedef struct NewtonChamferCylinderParam
	{
		dFloat m_radio;
		dFloat m_height;
	} NewtonChamferCylinderParam;

	typedef struct NewtonConvexHullParam
	{
		int m_vertexCount;
		int m_vertexStrideInBytes;
		int m_faceCount;
		dFloat* m_vertex;
	} NewtonConvexHullParam;

	typedef struct NewtonCompoundCollisionParam
	{
		int m_chidrenCount;
	} NewtonCompoundCollisionParam;

	typedef struct NewtonCollisionTreeParam
	{
		int m_vertexCount;
		int m_indexCount;
	} NewtonCollisionTreeParam;

	typedef struct NewtonDeformableMeshParam
	{
		int m_vertexCount;
		int m_triangleCount;
		int m_vrtexStrideInBytes;
		unsigned short *m_indexList;
		dFloat *m_vertexList;
	} NewtonDeformableMeshParam;

	typedef struct NewtonHeightFieldCollisionParam
	{
		int m_width;
		int m_height;
		int m_gridsDiagonals;
		int m_elevationDataType;	// 0 = 32 bit floats, 1 = unsigned 16 bit integers
		dFloat m_horizonalScale;
		dFloat m_verticalScale;
		void* m_elevation;
		char* m_atributes;
	} NewtonHeightFieldCollisionParam;

	typedef struct NewtonSceneCollisionParam
	{
		int m_childrenProxyCount;
	} NewtonSceneCollisionParam;

	typedef struct NewtonCollisionInfoRecord
	{
		dFloat m_offsetMatrix[4][4];
		int m_collisionType;				// tag id to identify the collision primitive
		int m_collisionUserID;				
		
		union {
			NewtonBoxParam m_box;									
			NewtonConeParam m_cone;
			NewtonSphereParam m_sphere;
			NewtonCapsuleParam m_capsule;
			NewtonCylinderParam m_cylinder;
			NewtonTaperedCapsuleParam m_taperedCapsule;
			NewtonTaperedCylinderParam m_taperedCylinder;
			NewtonChamferCylinderParam m_chamferCylinder;
			NewtonConvexHullParam m_convexHull;
			NewtonDeformableMeshParam m_deformableMesh;
			NewtonCompoundCollisionParam m_compoundCollision;
			NewtonCollisionTreeParam m_collisionTree;
			NewtonHeightFieldCollisionParam m_heightField;
			NewtonSceneCollisionParam m_sceneCollision;
			dFloat m_paramArray[64];		    // user define collision can use this to store information
		};
	} NewtonCollisionInfoRecord;

	typedef struct NewtonJointRecord
	{
		dFloat m_attachmenMatrix_0[4][4];
		dFloat m_attachmenMatrix_1[4][4];
		dFloat m_minLinearDof[3];
		dFloat m_maxLinearDof[3];
		dFloat m_minAngularDof[3];
		dFloat m_maxAngularDof[3];
		const NewtonBody* m_attachBody_0;
		const NewtonBody* m_attachBody_1;
		dFloat m_extraParameters[64];
		int	m_bodiesCollisionOn;
		char m_descriptionType[128];
	} NewtonJointRecord;

	typedef struct NewtonUserMeshCollisionCollideDesc
	{
		dFloat m_boxP0[4];							// lower bounding box of intersection query in local space
		dFloat m_boxP1[4];							// upper bounding box of intersection query in local space
		dFloat m_boxDistanceTravel[4];				// max distance that box bpxP0 and boxP1 can travel on this timestep, used this for continue collision mode.
		int m_threadNumber;							// current thread executing this query
		int	m_faceCount;                        	// the application should set here how many polygons intersect the query box
		int m_vertexStrideInBytes;              	// the application should set here the size of each vertex
		dFloat m_skinThickness;                     // this is the minimum skin separation specified by the material between these two colliding shapes
		void* m_userData;                       	// user data passed to the collision geometry at creation time

		NewtonBody* m_objBody;                  	// pointer to the colliding body
		NewtonBody* m_polySoupBody;             	// pointer to the rigid body owner of this collision tree 
		NewtonCollision* m_objCollision;			// collision shape of the colliding body, (no necessarily the collision of m_objBody)
		NewtonCollision* m_polySoupCollision;		// collision shape of the collision tree, (no necessarily the collision of m_polySoupBody)

		dFloat* m_vertex;                       	// the application should set here the pointer to the global vertex of the mesh. 
		int* m_faceIndexCount;                  	// the application should set here the pointer to the vertex count of each face.
		int* m_faceVertexIndex;                 	// the application should set here the pointer index array for each vertex on a face.
													// the format of a face is I0, I1, I2, I3, ..., M, N, E0, E1, E2, ..., A
                                                	// I0, I1, I2, .. are the indices to the vertex, relative to m_vertex pointer
		                                        	// M is the index to the material sub shape id
													// N in the index to the vertex normal relative to m_vertex pointer
													// E0, E1, E2, ... are the indices of the the face normal that is shared to that face edge, when the edge does not share a face normal then the edge index is set to index N, which the index to the face normal    
													// A is and estimate of the largest diagonal of the face, this used internally as a hint to improve floating point accuracy and algorithm performance. 
	} NewtonUserMeshCollisionCollideDesc;

	typedef struct NewtonWorldConvexCastReturnInfo
	{
		dFloat m_point[4];						// collision point in global space
		dFloat m_normal[4];						// surface normal at collision point in global space
		//dFloat m_normalOnHitPoint[4];           // surface normal at the surface of the hit body, 
												// is the same as the normal calculated by a ray cast hitting the body at the hit point
		dLong m_contactID;						// collision ID at contact point
		const NewtonBody* m_hitBody;			// body hit at contact point
		dFloat m_penetration;                   // contact penetration at collision point
	} NewtonWorldConvexCastReturnInfo;
	
	typedef struct NewtonUserMeshCollisionRayHitDesc
	{
		dFloat m_p0[4];							// ray origin in collision local space
		dFloat m_p1[4];                         // ray destination in collision local space   
		dFloat m_normalOut[4];					// copy here the normal at the ray intersection
		dLong m_userIdOut;						// copy here a user defined id for further feedback  
		void* m_userData;                       // user data passed to the collision geometry at creation time
	} NewtonUserMeshCollisionRayHitDesc;

	typedef struct NewtonHingeSliderUpdateDesc
	{
		dFloat m_accel;
		dFloat m_minFriction;
		dFloat m_maxFriction;
		dFloat m_timestep;
	} NewtonHingeSliderUpdateDesc;

	typedef struct NewtonClothPatchMaterial
	{
		dFloat m_damper;
		dFloat m_stiffness;
	} NewtonClothPatchMaterial;


	// Newton callback functions
	typedef void* (*NewtonAllocMemory) (int sizeInBytes);
	typedef void (*NewtonFreeMemory) (void* const ptr, int sizeInBytes);


	typedef void (*NewtonWorldDestructorCallback) (const NewtonWorld* const world);

	typedef void (*NewtonWorldListenerBodyDestroyCallback) (const NewtonWorld* const world, void* const listenerUserData, NewtonBody* const body);
	typedef void (*NewtonWorldUpdateListenerCallback) (const NewtonWorld* const world, void* const listenerUserData, dFloat timestep);
	typedef void (*NewtonWorldDestroyListenerCallback) (const NewtonWorld* const world, void* const listenerUserData);

	typedef unsigned (*NewtonGetTicksCountCallback) ();

	typedef void (*NewtonSerializeCallback) (void* const serializeHandle, const void* const buffer, int size);
	typedef void (*NewtonDeserializeCallback) (void* const serializeHandle, void* const buffer, int size);
	
	typedef void (*NewtonOnBodySerializationCallback) (NewtonBody* const body, void* const userData, NewtonSerializeCallback function, void* const serializeHandle);
	typedef void (*NewtonOnBodyDeserializationCallback) (NewtonBody* const body, void* const userData, NewtonDeserializeCallback function, void* const serializeHandle);

	typedef void (*NewtonOnJointSerializationCallback) (const NewtonJoint* const joint, NewtonSerializeCallback function, void* const serializeHandle);
	typedef void (*NewtonOnJointDeserializationCallback) (NewtonBody* const body0, NewtonBody* const body1, NewtonDeserializeCallback function, void* const serializeHandle);

	typedef void (*NewtonOnUserCollisionSerializationCallback) (void* const userData, NewtonSerializeCallback function, void* const serializeHandle);
	
	// user collision callbacks	
	typedef void (*NewtonUserMeshCollisionDestroyCallback) (void* const userData);
	typedef dFloat (*NewtonUserMeshCollisionRayHitCallback) (NewtonUserMeshCollisionRayHitDesc* const lineDescData);
	typedef void (*NewtonUserMeshCollisionGetCollisionInfo) (void* const userData, NewtonCollisionInfoRecord* const infoRecord);
	typedef int (*NewtonUserMeshCollisionAABBTest) (void* const userData, const dFloat* const boxP0, const dFloat* const boxP1);
	typedef int (*NewtonUserMeshCollisionGetFacesInAABB) (void* const userData, const dFloat* const p0, const dFloat* const p1,
														   const dFloat** const vertexArray, int* const vertexCount, int* const vertexStrideInBytes, 
		                                                   const int* const indexList, int maxIndexCount, const int* const userDataList);
	typedef void (*NewtonUserMeshCollisionCollideCallback) (NewtonUserMeshCollisionCollideDesc* const collideDescData, const void* const continueCollisionHandle);

	typedef int (*NewtonTreeCollisionFaceCallback) (void* const context, const dFloat* const polygon, int strideInBytes, const int* const indexArray, int indexCount);

	typedef dFloat (*NewtonCollisionTreeRayCastCallback) (const NewtonBody* const body, const NewtonCollision* const treeCollision, dFloat intersection, dFloat* const normal, int faceId, void* const usedData);
	typedef dFloat (*NewtonHeightFieldRayCastCallback) (const NewtonBody* const body, const NewtonCollision* const heightFieldCollision, dFloat intersection, int row, int col, dFloat* const normal, int faceId, void* const usedData);

	typedef void (*NewtonCollisionCopyConstructionCallback) (const NewtonWorld* const newtonWorld, NewtonCollision* const collision, const NewtonCollision* const sourceCollision);
	typedef void (*NewtonCollisionDestructorCallback) (const NewtonWorld* const newtonWorld, const NewtonCollision* const collision);

	// collision tree call back (obsoleted no recommended)
	typedef void (*NewtonTreeCollisionCallback) (const NewtonBody* const bodyWithTreeCollision, const NewtonBody* const body, int faceID, 
												 int vertexCount, const dFloat* const vertex, int vertexStrideInBytes); 

	typedef void (*NewtonBodyDestructor) (const NewtonBody* const body);
	typedef void (*NewtonApplyForceAndTorque) (const NewtonBody* const body, dFloat timestep, int threadIndex);
	typedef void (*NewtonSetTransform) (const NewtonBody* const body, const dFloat* const matrix, int threadIndex);

	typedef int (*NewtonIslandUpdate) (const NewtonWorld* const newtonWorld, const void* islandHandle, int bodyCount);
	
	typedef void (*NewtonFractureCompoundCollisionOnEmitCompoundFractured) (NewtonBody* const fracturedBody);
	typedef void (*NewtonFractureCompoundCollisionOnEmitChunk) (NewtonBody* const chunkBody, NewtonFracturedCompoundMeshPart* const fracturexChunkMesh, const NewtonCollision* const fracturedCompountCollision);
	typedef void (*NewtonFractureCompoundCollisionReconstructMainMeshCallBack) (NewtonBody* const body, NewtonFracturedCompoundMeshPart* const mainMesh, const NewtonCollision* const fracturedCompountCollision);

	typedef unsigned (*NewtonWorldRayPrefilterCallback)(const NewtonBody* const body, const NewtonCollision* const collision, void* const userData);
	typedef dFloat (*NewtonWorldRayFilterCallback)(const NewtonBody* const body, const NewtonCollision* const shapeHit, const dFloat* const hitContact, const dFloat* const hitNormal, dLong collisionID, void* const userData, dFloat intersectParam);
	

	typedef void (*NewtonContactsProcess) (const NewtonJoint* const contact, dFloat timestep, int threadIndex);
	typedef int  (*NewtonOnAABBOverlap) (const NewtonMaterial* const material, const NewtonBody* const body0, const NewtonBody* const body1, int threadIndex);
	typedef int  (*NewtonOnCompoundSubCollisionAABBOverlap) (const NewtonMaterial* const material, const NewtonBody* const body0, const void* const collsionNode0, const NewtonBody* const body1, const void* const collsionNode1, int threadIndex);

	typedef int (*NewtonBodyIterator) (const NewtonBody* const body, void* const userData);
	typedef void (*NewtonJointIterator) (const NewtonJoint* const joint, void* const userData);
	typedef void (*NewtonCollisionIterator) (void* const userData, int vertexCount, const dFloat* const faceArray, int faceId);

	typedef void (*NewtonBallCallback) (const NewtonJoint* const ball, dFloat timestep);
	typedef unsigned (*NewtonHingeCallback) (const NewtonJoint* const hinge, NewtonHingeSliderUpdateDesc* const desc);
	typedef unsigned (*NewtonSliderCallback) (const NewtonJoint* const slider, NewtonHingeSliderUpdateDesc* const desc);
	typedef unsigned (*NewtonUniversalCallback) (const NewtonJoint* const universal, NewtonHingeSliderUpdateDesc* const desc);
	typedef unsigned (*NewtonCorkscrewCallback) (const NewtonJoint* const corkscrew, NewtonHingeSliderUpdateDesc* const desc);

	typedef void (*NewtonUserBilateralCallback) (const NewtonJoint* const userJoint, dFloat timestep, int threadIndex);
	typedef void (*NewtonUserBilateralGetInfoCallback) (const NewtonJoint* const userJoint, NewtonJointRecord* const info);

	typedef void (*NewtonConstraintDestructor) (const NewtonJoint* const me);

	typedef void (*NewtonJobTask) (NewtonWorld* const world, void* const userData, int threadIndex);

	typedef bool (*NewtonReportProgress) (dFloat normalizedProgressPercent, void* const userData);


	// **********************************************************************************************
	//
	// world control functions
	//
	// **********************************************************************************************
	NEWTON_API int NewtonWorldGetVersion ();
	NEWTON_API int NewtonWorldFloatSize ();

	NEWTON_API int NewtonGetMemoryUsed ();
	NEWTON_API void NewtonSetMemorySystem (NewtonAllocMemory malloc, NewtonFreeMemory free);

	NEWTON_API NewtonWorld* NewtonCreate ();
	NEWTON_API void NewtonDestroy (const NewtonWorld* const newtonWorld);
	NEWTON_API void NewtonDestroyAllBodies (const NewtonWorld* const newtonWorld);

	NEWTON_API void* NewtonAlloc (int sizeInBytes);
	NEWTON_API void NewtonFree (void* const ptr);

	//	NEWTON_API void NewtonSetPlatformArchitecture (const NewtonWorld* const newtonWorld, int mode);
	//	NEWTON_API int NewtonGetPlatformArchitecture(const NewtonWorld* const newtonWorld, char* description);

	NEWTON_API int NewtonEnumrateDevices (const NewtonWorld* const newtonWorld);
	NEWTON_API int NewtonGetCurrentDevice (const NewtonWorld* const newtonWorld);
	NEWTON_API void NewtonSetCurrentDevice (const NewtonWorld* const newtonWorld, int deviceIndex);
	NEWTON_API void NewtonGetDeviceString (const NewtonWorld* const newtonWorld, int deviceIndex, char* const vendorString, int maxSize);

	NEWTON_API dFloat NewtonGetContactMergeTolerance (const NewtonWorld* const newtonWorld);
	NEWTON_API void NewtonSetContactMergeTolerance (const NewtonWorld* const newtonWorld, dFloat tolerance);

	NEWTON_API void NewtonInvalidateCache (const NewtonWorld* const newtonWorld);
	NEWTON_API void NewtonSetSolverModel (const NewtonWorld* const newtonWorld, int model);

	NEWTON_API void NewtonSetMultiThreadSolverOnSingleIsland (const NewtonWorld* const newtonWorld, int mode);
	NEWTON_API int NewtonGetMultiThreadSolverOnSingleIsland (const NewtonWorld* const newtonWorld);

	//NEWTON_API void NewtonSetPerformanceClock (const NewtonWorld* const newtonWorld, NewtonGetTicksCountCallback callback);
	//NEWTON_API unsigned NewtonReadPerformanceTicks (const NewtonWorld* const newtonWorld, unsigned performanceEntry);
	//NEWTON_API unsigned NewtonReadThreadPerformanceTicks (const NewtonWorld* newtonWorld, unsigned threadIndex);

	NEWTON_API int NewtonGetBroadphaseAlgorithm (const NewtonWorld* const newtonWorld);
	NEWTON_API void NewtonSelectBroadphaseAlgorithm (const NewtonWorld* const newtonWorld, int algorithmType);
	
	NEWTON_API void NewtonUpdate (const NewtonWorld* const newtonWorld, dFloat timestep);
	NEWTON_API void NewtonUpdateAsync (const NewtonWorld* const newtonWorld, dFloat timestep);
	NEWTON_API void NewtonWaitForUpdateToFinish (const NewtonWorld* const newtonWorld);

	NEWTON_API void NewtonSerializeToFile (const NewtonWorld* const newtonWorld, const char* const filename, NewtonOnBodySerializationCallback bodyCallback, void* const bodyUserData);
	NEWTON_API void NewtonDeserializeFromFile (const NewtonWorld* const newtonWorld, const char* const filename, NewtonOnBodyDeserializationCallback bodyCallback, void* const bodyUserData);

	NEWTON_API void NewtonSetJointSerializationCallbacks (const NewtonWorld* const newtonWorld, NewtonOnJointSerializationCallback serializeJoint, NewtonOnJointDeserializationCallback deserializeJoint);
	NEWTON_API void NewtonGetJointSerializationCallbacks (const NewtonWorld* const newtonWorld, NewtonOnJointSerializationCallback* const serializeJoint, NewtonOnJointDeserializationCallback* const deserializeJoint);

	

	// multi threading interface 
	NEWTON_API void NewtonWorldCriticalSectionLock (const NewtonWorld* const newtonWorld, int threadIndex);
	NEWTON_API void NewtonWorldCriticalSectionUnlock (const NewtonWorld* const newtonWorld);
	NEWTON_API void NewtonSetThreadsCount (const NewtonWorld* const newtonWorld, int threads);
	NEWTON_API int NewtonGetThreadsCount(const NewtonWorld* const newtonWorld);
	NEWTON_API int NewtonGetMaxThreadsCount(const NewtonWorld* const newtonWorld);
	NEWTON_API void NewtonDispachThreadJob(const NewtonWorld* const newtonWorld, NewtonJobTask task, void* const usedData);
	NEWTON_API void NewtonSyncThreadJobs(const NewtonWorld* const newtonWorld);

	// atomic operations
	NEWTON_API int NewtonAtomicAdd (int* const ptr, int value);
	NEWTON_API int NewtonAtomicSwap (int* const ptr, int value);
	NEWTON_API void NewtonYield ();
	

	NEWTON_API void NewtonSetFrictionModel (const NewtonWorld* const newtonWorld, int model);
	NEWTON_API void NewtonSetMinimumFrameRate (const NewtonWorld* const newtonWorld, dFloat frameRate);
	NEWTON_API void NewtonSetIslandUpdateEvent (const NewtonWorld* const newtonWorld, NewtonIslandUpdate islandUpdate); 
//	NEWTON_API void NewtonSetDestroyBodyByExeciveForce (const NewtonWorld* const newtonWorld, NewtonDestroyBodyByExeciveForce callback); 
//	NEWTON_API void NewtonWorldForEachBodyDo (const NewtonWorld* const newtonWorld, NewtonBodyIterator callback);
	NEWTON_API void NewtonWorldForEachJointDo (const NewtonWorld* const newtonWorld, NewtonJointIterator callback, void* const userData);
	NEWTON_API void NewtonWorldForEachBodyInAABBDo (const NewtonWorld* const newtonWorld, const dFloat* const p0, const dFloat* const p1, NewtonBodyIterator callback, void* const userData);

	
	NEWTON_API void NewtonWorldSetUserData (const NewtonWorld* const newtonWorld, void* const userData);
	NEWTON_API void* NewtonWorldGetUserData (const NewtonWorld* const newtonWorld);


	NEWTON_API void* NewtonWorldGetListenerUserData (const NewtonWorld* const newtonWorld, void* const listener);
	NEWTON_API NewtonWorldListenerBodyDestroyCallback NewtonWorldListenerGetBodyDestroyCallback (const NewtonWorld* const newtonWorld, void* const listener);
	NEWTON_API void NewtonWorldListenerSetBodyDestroyCallback (const NewtonWorld* const newtonWorld, void* const listener, NewtonWorldListenerBodyDestroyCallback bodyDestroyCallback);

	NEWTON_API void* NewtonWorldGetPreListener (const NewtonWorld* const newtonWorld, const char* const nameId);
	NEWTON_API void* NewtonWorldAddPreListener (const NewtonWorld* const newtonWorld, const char* const nameId, void* const listenerUserData, NewtonWorldUpdateListenerCallback update, NewtonWorldDestroyListenerCallback destroy);
	
	NEWTON_API void*  NewtonWorldGetPostListener (const NewtonWorld* const newtonWorld, const char* const nameId);
	NEWTON_API void* NewtonWorldAddPostListener (const NewtonWorld* const newtonWorld, const char* const nameId, void* const listenerUserData, NewtonWorldUpdateListenerCallback update, NewtonWorldDestroyListenerCallback destroy);

	NEWTON_API void NewtonWorldSetDestructorCallback (const NewtonWorld* const newtonWorld, NewtonWorldDestructorCallback destructor);
	NEWTON_API NewtonWorldDestructorCallback NewtonWorldGetDestructorCallback (const NewtonWorld* const newtonWorld);

	NEWTON_API void NewtonWorldSetCollisionConstructorDestructorCallback (const NewtonWorld* const newtonWorld, NewtonCollisionCopyConstructionCallback constructor, NewtonCollisionDestructorCallback destructor);

	NEWTON_DEPRECATED_API inline void NewtonWorldSetCollisionConstructorDestuctorCallback (const NewtonWorld* const newtonWorld, NewtonCollisionCopyConstructionCallback constructor, NewtonCollisionDestructorCallback destructor)
	{
		NewtonWorldSetCollisionConstructorDestructorCallback(newtonWorld, constructor, destructor);
	}

	NEWTON_API void NewtonWorldRayCast (const NewtonWorld* const newtonWorld, const dFloat* const p0, const dFloat* const p1, NewtonWorldRayFilterCallback filter, void* const userData, NewtonWorldRayPrefilterCallback prefilter, int threadIndex);
	NEWTON_API void NewtonWorldConvexRayCast (const NewtonWorld* const newtonWorld, const NewtonCollision* const shape, const dFloat* const matrix, const dFloat* const p1, NewtonWorldRayFilterCallback filter, void* const userData, NewtonWorldRayPrefilterCallback prefilter, int threadIndex);

	NEWTON_API int NewtonWorldCollide (const NewtonWorld* const newtonWorld, const dFloat* const matrix, const NewtonCollision* const shape, void* const userData,  
									   NewtonWorldRayPrefilterCallback prefilter, NewtonWorldConvexCastReturnInfo* const info, int maxContactsCount, int threadIndex);
	NEWTON_API int NewtonWorldConvexCast (const NewtonWorld* const newtonWorld, const dFloat* const matrix, const dFloat* const target, const NewtonCollision* const shape, dFloat* const hitParam, void* const userData,  
										  NewtonWorldRayPrefilterCallback prefilter, NewtonWorldConvexCastReturnInfo* const info, int maxContactsCount, int threadIndex);


	// world utility functions
	NEWTON_API int NewtonWorldGetBodyCount(const NewtonWorld* const newtonWorld);
	NEWTON_API int NewtonWorldGetConstraintCount(const NewtonWorld* const newtonWorld);


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

//	deprecated, not longer continue collision is set on the material  	
//	NEWTON_API void NewtonMaterialSetContinuousCollisionMode (const NewtonWorld* const newtonWorld, int id0, int id1, int state);
	NEWTON_API void NewtonMaterialSetCollisionCallback (const NewtonWorld* const newtonWorld, int id0, int id1, void* const userData, NewtonOnAABBOverlap aabbOverlap, NewtonContactsProcess process);
	NEWTON_API void NewtonMaterialSetCompoundCollisionCallback(const NewtonWorld* const newtonWorld, int id0, int id1, NewtonOnCompoundSubCollisionAABBOverlap compoundAabbOverlap);

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
	NEWTON_API dFloat NewtonMaterialGetContactNormalSpeed (const NewtonMaterial* const material);
	NEWTON_API void NewtonMaterialGetContactForce (const NewtonMaterial* const material, const NewtonBody* const body, dFloat* const force);
	NEWTON_API void NewtonMaterialGetContactPositionAndNormal (const NewtonMaterial* const material, const NewtonBody* const body, dFloat* const posit, dFloat* const normal);
	NEWTON_API void NewtonMaterialGetContactTangentDirections (const NewtonMaterial* const material, const NewtonBody* const body, dFloat* const dir0, dFloat* const dir1);
	NEWTON_API dFloat NewtonMaterialGetContactTangentSpeed (const NewtonMaterial* const material, int index);

	NEWTON_API dFloat NewtonMaterialGetContactMaxNormalImpact (const NewtonMaterial* const material);
	NEWTON_API dFloat NewtonMaterialGetContactMaxTangentImpact (const NewtonMaterial* const material, int index);
	
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
	NEWTON_API NewtonCollision* NewtonCreateSphere (const NewtonWorld* const newtonWorld, dFloat radius, int shapeID, const dFloat* const offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateBox (const NewtonWorld* const newtonWorld, dFloat dx, dFloat dy, dFloat dz, int shapeID, const dFloat* const offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateCone (const NewtonWorld* const newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat* const offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateCapsule (const NewtonWorld* const newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat* const offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateCylinder (const NewtonWorld* const newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat* const offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateTaperedCapsule (const NewtonWorld* const newtonWorld, dFloat radio0, dFloat radio1, dFloat height, int shapeID, const dFloat* const offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateTaperedCylinder (const NewtonWorld* const newtonWorld, dFloat radio0, dFloat radio1, dFloat height, int shapeID, const dFloat* const offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateChamferCylinder (const NewtonWorld* const newtonWorld, dFloat radius, dFloat height, int shapeID, const dFloat* const offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateConvexHull (const NewtonWorld* const newtonWorld, int count, const dFloat* const vertexCloud, int strideInBytes, dFloat tolerance, int shapeID, const dFloat* const offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateConvexHullFromMesh (const NewtonWorld* const newtonWorld, const NewtonMesh* const mesh, dFloat tolerance, int shapeID);

	NEWTON_API int NewtonCollisionGetMode(const NewtonCollision* const convexCollision);
	NEWTON_API void NewtonCollisionSetMode (const NewtonCollision* const convexCollision, int mode);

	

//	NEWTON_API void NewtonCollisionSetMaxBreakImpactImpulse(const NewtonCollision* const convexHullCollision, dFloat maxImpactImpulse);
//	NEWTON_API dFloat NewtonCollisionGetMaxBreakImpactImpulse(const NewtonCollision* const convexHullCollision);

	NEWTON_API int NewtonConvexHullGetFaceIndices (const NewtonCollision* const convexHullCollision, int face, int* const faceIndices);
	NEWTON_API int NewtonConvexHullGetVertexData (const NewtonCollision* const convexHullCollision, dFloat** const vertexData, int* strideInBytes);
	NEWTON_DEPRECATED_API inline int NewtonConvexHullGetVetexData (const NewtonCollision* const convexHullCollision, dFloat** const vertexData, int* strideInBytes)
	{
		return NewtonConvexHullGetVertexData(convexHullCollision, vertexData, strideInBytes);
	}
	
	NEWTON_API dFloat NewtonConvexCollisionCalculateVolume (const NewtonCollision* const convexCollision);
	NEWTON_API void NewtonConvexCollisionCalculateInertialMatrix (const NewtonCollision* convexCollision, dFloat* const inertia, dFloat* const origin);	
	NEWTON_API void NewtonConvexCollisionCalculateBuoyancyAcceleration (const NewtonCollision* const convexCollision, const dFloat* const matrix, const dFloat* const shapeOrigin, const dFloat* const gravityVector, const dFloat* const fluidPlane, dFloat fluidDensity, dFloat fluidViscosity, dFloat* const accel, dFloat* const alpha);

	NEWTON_API const void* NewtonCollisionDataPointer (const NewtonCollision* const convexCollision);

	// **********************************************************************************************
	//
	// compound collision primitives creation functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonCollision* NewtonCreateCompoundCollision (const NewtonWorld* const newtonWorld, int shapeID);
	NEWTON_API NewtonCollision* NewtonCreateCompoundCollisionFromMesh (const NewtonWorld* const newtonWorld, const NewtonMesh* const mesh, dFloat hullTolerance, int shapeID, int subShapeID);

	NEWTON_API void NewtonCompoundCollisionBeginAddRemove (NewtonCollision* const compoundCollision);	
	NEWTON_API void* NewtonCompoundCollisionAddSubCollision (NewtonCollision* const compoundCollision, const NewtonCollision* const convexCollision);	
	NEWTON_API void NewtonCompoundCollisionRemoveSubCollision (NewtonCollision* const compoundCollision, const void* const collisionNode);	
	NEWTON_API void NewtonCompoundCollisionRemoveSubCollisionByIndex (NewtonCollision* const compoundCollision, int nodeIndex);	
	NEWTON_API void NewtonCompoundCollisionSetSubCollisionMatrix (NewtonCollision* const compoundCollision, const void* const collisionNode, const dFloat* const matrix);	
	NEWTON_API void NewtonCompoundCollisionEndAddRemove (NewtonCollision* const compoundCollision);	

	NEWTON_API void* NewtonCompoundCollisionGetFirstNode (NewtonCollision* const compoundCollision);
	NEWTON_API void* NewtonCompoundCollisionGetNextNode (NewtonCollision* const compoundCollision, const void* const collisionNode);

	NEWTON_API void* NewtonCompoundCollisionGetNodeByIndex (NewtonCollision* const compoundCollision, int index);
	NEWTON_API int NewtonCompoundCollisionGetNodeIndex (NewtonCollision* const compoundCollision, const void* const collisionNode);
	NEWTON_API NewtonCollision* NewtonCompoundCollisionGetCollisionFromNode (NewtonCollision* const compoundCollision, const void* const collisionNode);


	// **********************************************************************************************
	//
	// Fractured compound collision primitives interface
	//
	// **********************************************************************************************
	NEWTON_API NewtonCollision* NewtonCreateFracturedCompoundCollision (const NewtonWorld* const newtonWorld, const NewtonMesh* const solidMesh, int shapeID, int fracturePhysicsMaterialID, int pointcloudCount, const dFloat* const vertexCloud, int strideInBytes, int materialID, const dFloat* const textureMatrix,
																		NewtonFractureCompoundCollisionReconstructMainMeshCallBack regenerateMainMeshCallback, 
																		NewtonFractureCompoundCollisionOnEmitCompoundFractured emitFracturedCompound, NewtonFractureCompoundCollisionOnEmitChunk emitFracfuredChunk);
	NEWTON_API NewtonCollision* NewtonFracturedCompoundPlaneClip (const NewtonCollision* const fracturedCompound, const dFloat* const plane);

	NEWTON_API void NewtonFracturedCompoundSetCallbacks (const NewtonCollision* const fracturedCompound, NewtonFractureCompoundCollisionReconstructMainMeshCallBack regenerateMainMeshCallback, 
														 NewtonFractureCompoundCollisionOnEmitCompoundFractured emitFracturedCompound, NewtonFractureCompoundCollisionOnEmitChunk emitFracfuredChunk);


	NEWTON_API int NewtonFracturedCompoundIsNodeFreeToDetach (const NewtonCollision* const fracturedCompound, void* const collisionNode);
	NEWTON_API int NewtonFracturedCompoundNeighborNodeList (const NewtonCollision* const fracturedCompound, void* const collisionNode, void** const list, int maxCount);

	
	NEWTON_API NewtonFracturedCompoundMeshPart* NewtonFracturedCompoundGetMainMesh (const NewtonCollision* const fracturedCompound);
	NEWTON_API NewtonFracturedCompoundMeshPart* NewtonFracturedCompoundGetFirstSubMesh(const NewtonCollision* const fracturedCompound);
	NEWTON_API NewtonFracturedCompoundMeshPart* NewtonFracturedCompoundGetNextSubMesh(const NewtonCollision* const fracturedCompound, NewtonFracturedCompoundMeshPart* const subMesh);

	NEWTON_API int NewtonFracturedCompoundCollisionGetVertexCount (const NewtonCollision* const fracturedCompound, const NewtonFracturedCompoundMeshPart* const meshOwner); 
	NEWTON_API const dFloat* NewtonFracturedCompoundCollisionGetVertexPositions (const NewtonCollision* const fracturedCompound, const NewtonFracturedCompoundMeshPart* const meshOwner);
	NEWTON_API const dFloat* NewtonFracturedCompoundCollisionGetVertexNormals (const NewtonCollision* const fracturedCompound, const NewtonFracturedCompoundMeshPart* const meshOwner);
	NEWTON_API const dFloat* NewtonFracturedCompoundCollisionGetVertexUVs (const NewtonCollision* const fracturedCompound, const NewtonFracturedCompoundMeshPart* const meshOwner);
	NEWTON_API int NewtonFracturedCompoundMeshPartGetIndexStream (const NewtonCollision* const fracturedCompound, const NewtonFracturedCompoundMeshPart* const meshOwner, const void* const segment, int* const index); 

	NEWTON_API void* NewtonFracturedCompoundMeshPartGetFirstSegment (const NewtonFracturedCompoundMeshPart* const fractureCompoundMeshPart); 
	NEWTON_API void* NewtonFracturedCompoundMeshPartGetNextSegment (const void* const fractureCompoundMeshSegment); 
	NEWTON_API int NewtonFracturedCompoundMeshPartGetMaterial (const void* const fractureCompoundMeshSegment); 
	NEWTON_API int NewtonFracturedCompoundMeshPartGetIndexCount (const void* const fractureCompoundMeshSegment); 


	// **********************************************************************************************
	//
	// scene collision are static compound collision that can take polygonal static collisions
	//
	// **********************************************************************************************
	NEWTON_API NewtonCollision* NewtonCreateSceneCollision (const NewtonWorld* const newtonWorld, int shapeID);

	NEWTON_API void NewtonSceneCollisionBeginAddRemove (NewtonCollision* const sceneCollision);	
	NEWTON_API void* NewtonSceneCollisionAddSubCollision (NewtonCollision* const sceneCollision, const NewtonCollision* const collision);	
	NEWTON_API void NewtonSceneCollisionRemoveSubCollision (NewtonCollision* const compoundCollision, const void* const collisionNode);	
	NEWTON_API void NewtonSceneCollisionRemoveSubCollisionByIndex (NewtonCollision* const sceneCollision, int nodeIndex);
	NEWTON_API void NewtonSceneCollisionSetSubCollisionMatrix (NewtonCollision* const sceneCollision, const void* const collisionNode, const dFloat* const matrix);	
	NEWTON_API void NewtonSceneCollisionEndAddRemove (NewtonCollision* const sceneCollision);	

	NEWTON_API void* NewtonSceneCollisionGetFirstNode (NewtonCollision* const sceneCollision);
	NEWTON_API void* NewtonSceneCollisionGetNextNode (NewtonCollision* const sceneCollision, const void* const collisionNode);

	NEWTON_API void* NewtonSceneCollisionGetNodeByIndex (NewtonCollision* const sceneCollision, int index);
	NEWTON_API int NewtonSceneCollisionGetNodeIndex (NewtonCollision* const sceneCollision, const void* const collisionNode);
	NEWTON_API NewtonCollision* NewtonSceneCollisionGetCollisionFromNode (NewtonCollision* const sceneCollision, const void* const collisionNode);


	//  ***********************************************************************************************************
	//
	//	User Static mesh collision interface
	//
	// ***********************************************************************************************************
	NEWTON_API NewtonCollision* NewtonCreateUserMeshCollision (const NewtonWorld* const newtonWorld, const dFloat* const minBox, 
		const dFloat* const maxBox, void* const userData, NewtonUserMeshCollisionCollideCallback collideCallback, 
		NewtonUserMeshCollisionRayHitCallback rayHitCallback, NewtonUserMeshCollisionDestroyCallback destroyCallback,
		NewtonUserMeshCollisionGetCollisionInfo getInfoCallback, NewtonUserMeshCollisionAABBTest getLocalAABBCallback, 
		NewtonUserMeshCollisionGetFacesInAABB facesInAABBCallback, NewtonOnUserCollisionSerializationCallback serializeCallback, int shapeID);

	NEWTON_API int NewtonUserMeshCollisionContinuousOverlapTest (const NewtonUserMeshCollisionCollideDesc* const collideDescData, const void* const continueCollisionHandle, const dFloat* const minAabb, const dFloat* const maxAabb);
	NEWTON_DEPRECATED_API inline int NewtonUserMeshCollisionContinueOveralapTest (const NewtonUserMeshCollisionCollideDesc* const collideDescData, const void* const continueCollisionHandle, const dFloat* const minAabb, const dFloat* const maxAabb)
	{
		return NewtonUserMeshCollisionContinuousOverlapTest(collideDescData, continueCollisionHandle, minAabb, maxAabb);
	}


	//  ***********************************************************************************************************
	//
	//	Collision serialization functions
	//
	// ***********************************************************************************************************
	NEWTON_API NewtonCollision* NewtonCreateCollisionFromSerialization (const NewtonWorld* const newtonWorld, NewtonDeserializeCallback deserializeFunction, void* const serializeHandle);
	NEWTON_API void NewtonCollisionSerialize (const NewtonWorld* const newtonWorld, const NewtonCollision* const collision, NewtonSerializeCallback serializeFunction, void* const serializeHandle);
	NEWTON_API void NewtonCollisionGetInfo (const NewtonCollision* const collision, NewtonCollisionInfoRecord* const collisionInfo);

	// **********************************************************************************************
	//
	// Static collision shapes functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonCollision* NewtonCreateHeightFieldCollision (const NewtonWorld* const newtonWorld, int width, int height, int gridsDiagonals, int elevationdatType,
																  const void* const elevationMap, const char* const attributeMap, dFloat verticalScale, dFloat horizontalScale, int shapeID);
	NEWTON_API void NewtonHeightFieldSetUserRayCastCallback (const NewtonCollision* const heightfieldCollision, NewtonHeightFieldRayCastCallback rayHitCallback);

	
	NEWTON_API NewtonCollision* NewtonCreateTreeCollision (const NewtonWorld* const newtonWorld, int shapeID);
	NEWTON_API NewtonCollision* NewtonCreateTreeCollisionFromMesh (const NewtonWorld* const newtonWorld, const NewtonMesh* const mesh, int shapeID);

	NEWTON_API void NewtonTreeCollisionSetUserRayCastCallback (const NewtonCollision* const treeCollision, NewtonCollisionTreeRayCastCallback rayHitCallback);

	NEWTON_API void NewtonTreeCollisionBeginBuild (const NewtonCollision* const treeCollision);
	NEWTON_API void NewtonTreeCollisionAddFace (const NewtonCollision* const treeCollision, int vertexCount, const dFloat* const vertexPtr, int strideInBytes, int faceAttribute);
	NEWTON_API void NewtonTreeCollisionEndBuild (const NewtonCollision* const treeCollision, int optimize);

	NEWTON_API int NewtonTreeCollisionGetFaceAttribute (const NewtonCollision* const treeCollision, const int* const faceIndexArray, int indexCount); 
	NEWTON_API void NewtonTreeCollisionSetFaceAttribute (const NewtonCollision* const treeCollision, const int* const faceIndexArray, int indexCount, int attribute);

	NEWTON_DEPRECATED_API inline int NewtonTreeCollisionGetFaceAtribute (const NewtonCollision* const treeCollision, const int* const faceIndexArray, int indexCount)
	{
		return NewtonTreeCollisionGetFaceAttribute(treeCollision, faceIndexArray, indexCount);
	}
	NEWTON_DEPRECATED_API inline void NewtonTreeCollisionSetFaceAtribute (const NewtonCollision* const treeCollision, const int* const faceIndexArray, int indexCount, int attribute)
	{
		NewtonTreeCollisionSetFaceAttribute(treeCollision, faceIndexArray, indexCount, attribute);
	}

	NEWTON_API void NewtonTreeCollisionForEachFace (const NewtonCollision* const treeCollision, NewtonTreeCollisionFaceCallback forEachFaceCallback, void* const context); 

	NEWTON_API int NewtonTreeCollisionGetVertexListTriangleListInAABB (const NewtonCollision* const treeCollision, const dFloat* const p0, const dFloat* const p1, const dFloat** const vertexArray, int* const vertexCount, int* const vertexStrideInBytes, const int* const indexList, int maxIndexCount, const int* const faceAttribute); 

	NEWTON_API void NewtonStaticCollisionSetDebugCallback (const NewtonCollision* const staticCollision, NewtonTreeCollisionCallback userCallback);

	// **********************************************************************************************
	//
	// General purpose collision library functions
	//
	// **********************************************************************************************

	NEWTON_API NewtonCollision* NewtonCollisionCreateInstance (const NewtonCollision* const collision);
	NEWTON_API int NewtonCollisionGetType (const NewtonCollision* const collision);

	// for the end user
	NEWTON_API void NewtonCollisionSetUserData (const NewtonCollision* const collision, void* const userData);
	NEWTON_API void* NewtonCollisionGetUserData (const NewtonCollision* const collision);
	
	// this is used data is used by the joint library
	NEWTON_API void NewtonCollisionSetUserData1 (const NewtonCollision* const collision, void* const userData);
	NEWTON_API void* NewtonCollisionGetUserData1 (const NewtonCollision* const collision);
	
	NEWTON_API void NewtonCollisionSetUserID (const NewtonCollision* const collision, unsigned id);
	NEWTON_API unsigned NewtonCollisionGetUserID (const NewtonCollision* const collision);

	NEWTON_API void* NewtonCollisionGetSubCollisionHandle (const NewtonCollision* const collision);
	NEWTON_API NewtonCollision* NewtonCollisionGetParentInstance (const NewtonCollision* const collision);

	NEWTON_API void NewtonCollisionSetMatrix (const NewtonCollision* const collision, const dFloat* const matrix);
	NEWTON_API void NewtonCollisionGetMatrix (const NewtonCollision* const collision, dFloat* const matrix);

	NEWTON_API void NewtonCollisionSetScale (const NewtonCollision* const collision, dFloat scaleX, dFloat scaleY, dFloat scaleZ);
	NEWTON_API void NewtonCollisionGetScale (const NewtonCollision* const collision, dFloat* const scaleX, dFloat* const scaleY, dFloat* const scaleZ);
	NEWTON_API void NewtonDestroyCollision (const NewtonCollision* const collision);

	NEWTON_API dFloat NewtonCollisionGetSkinThickness (const NewtonCollision* const collision);

	NEWTON_API int NewtonCollisionIntersectionTest (const NewtonWorld* const newtonWorld, 
		const NewtonCollision* const collisionA, const dFloat* const matrixA, 
		const NewtonCollision* const collisionB, const dFloat* const matrixB, int threadIndex);

	NEWTON_API int NewtonCollisionPointDistance (const NewtonWorld* const newtonWorld, const dFloat* const point,
		const NewtonCollision* const collision, const dFloat* const matrix, dFloat* const contact, dFloat* const normal, int threadIndex);

	NEWTON_API int NewtonCollisionClosestPoint (const NewtonWorld* const newtonWorld, 
		const NewtonCollision* const collisionA, const dFloat* const matrixA, 
		const NewtonCollision* const collisionB, const dFloat* const matrixB,
		dFloat* const contactA, dFloat* const contactB, dFloat* const normalAB, int threadIndex);

	NEWTON_API int NewtonCollisionCollide (const NewtonWorld* const newtonWorld, int maxSize,
		const NewtonCollision* const collisionA, const dFloat* const matrixA, 
		const NewtonCollision* const collisionB, const dFloat* const matrixB,
		dFloat* const contacts, dFloat* const normals, dFloat* const penetration, 
		dLong* const attributeA, dLong* const attributeB, int threadIndex);

	NEWTON_API int NewtonCollisionCollideContinue (const NewtonWorld* const newtonWorld, int maxSize, dFloat timestep, 
		const NewtonCollision* const collisionA, const dFloat* const matrixA, const dFloat* const velocA, const dFloat* omegaA, 
		const NewtonCollision* const collisionB, const dFloat* const matrixB, const dFloat* const velocB, const dFloat* const omegaB, 
		dFloat* const timeOfImpact, dFloat* const contacts, dFloat* const normals, dFloat* const penetration, 
		dLong* const attributeA, dLong* const attributeB, int threadIndex);

	NEWTON_API void NewtonCollisionSupportVertex (const NewtonCollision* const collision, const dFloat* const dir, dFloat* const vertex);
	NEWTON_API dFloat NewtonCollisionRayCast (const NewtonCollision* const collision, const dFloat* const p0, const dFloat* const p1, dFloat* const normal, dLong* const attribute);
	NEWTON_API void NewtonCollisionCalculateAABB (const NewtonCollision* const collision, const dFloat* const matrix, dFloat* const p0, dFloat* const p1);
	NEWTON_API void NewtonCollisionForEachPolygonDo (const NewtonCollision* const collision, const dFloat* const matrix, NewtonCollisionIterator callback, void* const userData);

	
	
	// **********************************************************************************************
	//
	// transforms utility functions
	//
	// **********************************************************************************************
	NEWTON_API void NewtonSetEulerAngle (const dFloat* const eulersAngles, dFloat* const matrix);
	NEWTON_API void NewtonGetEulerAngle (const dFloat* const matrix, dFloat* const eulersAngles0, dFloat* const eulersAngles1);
	NEWTON_API dFloat NewtonCalculateSpringDamperAcceleration (dFloat dt, dFloat ks, dFloat x, dFloat kd, dFloat s);

	// **********************************************************************************************
	//
	// body manipulation functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonBody* NewtonCreateDynamicBody (const NewtonWorld* const newtonWorld, const NewtonCollision* const collision, const dFloat* const matrix);
	NEWTON_API NewtonBody* NewtonCreateKinematicBody (const NewtonWorld* const newtonWorld, const NewtonCollision* const collision, const dFloat* const matrix);
	NEWTON_API NewtonBody* NewtonCreateDeformableBody (const NewtonWorld* const newtonWorld, const NewtonCollision* const deformableMesh, const dFloat* const matrix);

	NEWTON_API void  NewtonDestroyBody(const NewtonBody* const body);

	NEWTON_DEPRECATED_API void NewtonBodyEnableSimulation(const NewtonBody* const body);
	NEWTON_DEPRECATED_API void NewtonBodyDisableSimulation(const NewtonBody* const body);
	NEWTON_API int NewtonBodyGetSimulationState(const NewtonBody* const body);
	NEWTON_API void NewtonBodySetSimulationState(const NewtonBody* const bodyPtr, const int state);

	NEWTON_API int NewtonBodyGetType (const NewtonBody* const body);
	NEWTON_API int NewtonBodyGetCollidable (const NewtonBody* const body);
	NEWTON_API void NewtonBodySetCollidable (const NewtonBody* const body, int collidableState);

	NEWTON_API void  NewtonBodyAddForce (const NewtonBody* const body, const dFloat* const force);
	NEWTON_API void  NewtonBodyAddTorque (const NewtonBody* const body, const dFloat* const torque);
	NEWTON_API void  NewtonBodyCalculateInverseDynamicsForce (const NewtonBody* const body, dFloat timestep, const dFloat* const desiredVeloc, dFloat* const forceOut);

	NEWTON_API void  NewtonBodySetCentreOfMass (const NewtonBody* const body, const dFloat* const com);
	NEWTON_API void  NewtonBodySetMassMatrix (const NewtonBody* const body, dFloat mass, dFloat Ixx, dFloat Iyy, dFloat Izz);

	NEWTON_API void  NewtonBodySetMassProperties (const NewtonBody* const body, dFloat mass, const NewtonCollision* const collision);
	NEWTON_API void  NewtonBodySetMatrix (const NewtonBody* const body, const dFloat* const matrix);
	NEWTON_API void  NewtonBodySetMatrixRecursive (const NewtonBody* const body, const dFloat* const matrix);
	
	NEWTON_API void  NewtonBodySetMaterialGroupID (const NewtonBody* const body, int id);
	NEWTON_API void  NewtonBodySetContinuousCollisionMode (const NewtonBody* const body, unsigned state);
	NEWTON_API void  NewtonBodySetJointRecursiveCollision (const NewtonBody* const body, unsigned state);
	NEWTON_API void  NewtonBodySetOmega (const NewtonBody* const body, const dFloat* const omega);
	NEWTON_API void  NewtonBodySetVelocity (const NewtonBody* const body, const dFloat* const velocity);
	NEWTON_API void  NewtonBodySetForce (const NewtonBody* const body, const dFloat* const force);
	NEWTON_API void  NewtonBodySetTorque (const NewtonBody* const body, const dFloat* const torque);
	
	NEWTON_API void  NewtonBodySetLinearDamping (const NewtonBody* const body, dFloat linearDamp);
	NEWTON_API void  NewtonBodySetAngularDamping (const NewtonBody* const body, const dFloat* const angularDamp);
	NEWTON_API void  NewtonBodySetCollision (const NewtonBody* const body, const NewtonCollision* const collision);
	NEWTON_API void  NewtonBodySetCollisionScale (const NewtonBody* const body, dFloat scaleX, dFloat  scaleY, dFloat scaleZ);

	NEWTON_API int  NewtonBodyGetSleepState (const NewtonBody* const body);
	NEWTON_API void NewtonBodySetSleepState (const NewtonBody* const body, int state);

	NEWTON_API int  NewtonBodyGetAutoSleep (const NewtonBody* const body);
	NEWTON_API void NewtonBodySetAutoSleep (const NewtonBody* const body, int state);

	NEWTON_API int  NewtonBodyGetFreezeState(const NewtonBody* const body);
	NEWTON_API void NewtonBodySetFreezeState (const NewtonBody* const body, int state);

	NEWTON_API void NewtonBodySetDestructorCallback (const NewtonBody* const body, NewtonBodyDestructor callback);
	NEWTON_API NewtonBodyDestructor NewtonBodyGetDestructorCallback (const NewtonBody* const body);

	NEWTON_API void  NewtonBodySetTransformCallback (const NewtonBody* const body, NewtonSetTransform callback);
	NEWTON_API NewtonSetTransform NewtonBodyGetTransformCallback (const NewtonBody* const body);
	
	NEWTON_API void  NewtonBodySetForceAndTorqueCallback (const NewtonBody* const body, NewtonApplyForceAndTorque callback);
	NEWTON_API NewtonApplyForceAndTorque NewtonBodyGetForceAndTorqueCallback (const NewtonBody* const body);

	NEWTON_API int NewtonBodyGetID (const NewtonBody* const body);

	NEWTON_API void  NewtonBodySetUserData (const NewtonBody* const body, void* const userData);
	NEWTON_API void* NewtonBodyGetUserData (const NewtonBody* const body);

	NEWTON_API NewtonWorld* NewtonBodyGetWorld (const NewtonBody* const body);
	NEWTON_API NewtonCollision* NewtonBodyGetCollision (const NewtonBody* const body);
	NEWTON_API int   NewtonBodyGetMaterialGroupID (const NewtonBody* const body);

	NEWTON_API int   NewtonBodyGetContinuousCollisionMode (const NewtonBody* const body);
	NEWTON_API int   NewtonBodyGetJointRecursiveCollision (const NewtonBody* const body);

	NEWTON_API void NewtonBodyGetPosition(const NewtonBody* const body, dFloat* const pos);
	NEWTON_API void  NewtonBodyGetMatrix(const NewtonBody* const body, dFloat* const matrix);
	NEWTON_API void  NewtonBodyGetRotation(const NewtonBody* const body, dFloat* const rotation);
	NEWTON_API void  NewtonBodyGetMassMatrix (const NewtonBody* const body, dFloat* mass, dFloat* const Ixx, dFloat* const Iyy, dFloat* const Izz);
	NEWTON_API void  NewtonBodyGetInvMass(const NewtonBody* const body, dFloat* const invMass, dFloat* const invIxx, dFloat* const invIyy, dFloat* const invIzz);
	NEWTON_API void  NewtonBodyGetInertiaMatrix(const NewtonBody* const body, dFloat* const inertiaMatrix);
	NEWTON_API void  NewtonBodyGetInvInertiaMatrix(const NewtonBody* const body, dFloat* const invInertiaMatrix);
	NEWTON_API void  NewtonBodyGetOmega(const NewtonBody* const body, dFloat* const vector);
	NEWTON_API void  NewtonBodyGetVelocity(const NewtonBody* const body, dFloat* const vector);
	NEWTON_API void  NewtonBodyGetForce(const NewtonBody* const body, dFloat* const vector);
	NEWTON_API void  NewtonBodyGetTorque(const NewtonBody* const body, dFloat* const vector);
	NEWTON_API void  NewtonBodyGetForceAcc(const NewtonBody* const body, dFloat* const vector);
	NEWTON_API void  NewtonBodyGetTorqueAcc(const NewtonBody* const body, dFloat* const vector);
	NEWTON_API void  NewtonBodyGetCentreOfMass (const NewtonBody* const body, dFloat* const com);

	NEWTON_API void NewtonBodyGetPointVelocity (const NewtonBody* const body, const dFloat* const point, dFloat* const velocOut);
	NEWTON_API void NewtonBodyAddImpulse (const NewtonBody* const body, const dFloat* const pointDeltaVeloc, const dFloat* const pointPosit);
	NEWTON_API void NewtonBodyApplyImpulseArray (const NewtonBody* const body, int impuleCount, int strideInByte, const dFloat* const impulseArray, const dFloat* const pointArray);

	NEWTON_API void NewtonBodyApplyImpulsePair (const NewtonBody* const body, dFloat* const linearImpulse, dFloat* const angularImpulse);

	NEWTON_API void NewtonBodyIntegrateVelocity (const NewtonBody* const body, dFloat timestep);

	NEWTON_API dFloat NewtonBodyGetLinearDamping (const NewtonBody* const body);
	NEWTON_API void  NewtonBodyGetAngularDamping (const NewtonBody* const body, dFloat* const vector);
	NEWTON_API void  NewtonBodyGetAABB (const NewtonBody* const body, dFloat* const p0, dFloat* const p1);

	
	NEWTON_API NewtonJoint* NewtonBodyGetFirstJoint (const NewtonBody* const body);
	NEWTON_API NewtonJoint* NewtonBodyGetNextJoint (const NewtonBody* const body, const NewtonJoint* const joint);
	NEWTON_API NewtonJoint* NewtonBodyGetFirstContactJoint (const NewtonBody* const body);
	NEWTON_API NewtonJoint* NewtonBodyGetNextContactJoint (const NewtonBody* const body, const NewtonJoint* const contactJoint);
	

	// **********************************************************************************************
	//
	// contact joints interface
	//
	// **********************************************************************************************

	NEWTON_API void* NewtonContactJointGetFirstContact (const NewtonJoint* const contactJoint);
	NEWTON_API void* NewtonContactJointGetNextContact (const NewtonJoint* const contactJoint, void* const contact);

	NEWTON_API int NewtonContactJointGetContactCount(const NewtonJoint* const contactJoint);
	NEWTON_API void NewtonContactJointRemoveContact(const NewtonJoint* const contactJoint, void* const contact); 

	NEWTON_API dFloat NewtonContactJointGetClosestDistance(const NewtonJoint* const contactJoint);

	NEWTON_API NewtonMaterial* NewtonContactGetMaterial (const void* const contact);

	NEWTON_API NewtonCollision* NewtonContactGetCollision0 (const void* const contact);	
	NEWTON_API NewtonCollision* NewtonContactGetCollision1 (const void* const contact);	

	NEWTON_API void* NewtonContactGetCollisionID0 (const void* const contact);	
	NEWTON_API void* NewtonContactGetCollisionID1 (const void* const contact);	
	
	
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

	NEWTON_API int NewtonJointIsActive (const NewtonJoint* const joint);



	// **********************************************************************************************
	//
	// particle system interface (soft bodies, individual, pressure bodies and cloth)   
	//
	// **********************************************************************************************
	NEWTON_API NewtonCollision* NewtonCreateClothPatch (const NewtonWorld* const newtonWorld, NewtonMesh* const mesh, int shapeID, NewtonClothPatchMaterial* const structuralMaterial, NewtonClothPatchMaterial* const bendMaterial);
	NEWTON_API NewtonCollision* NewtonCreateDeformableMesh (const NewtonWorld* const newtonWorld, NewtonMesh* const mesh, int shapeID);

	NEWTON_API void NewtonDeformableMeshCreateClusters (NewtonCollision* const deformableMesh, int clusterCount, dFloat overlapingWidth);
	NEWTON_API void NewtonDeformableMeshSetDebugCallback (NewtonCollision* const deformableMesh, NewtonCollisionIterator callback);

	NEWTON_API int NewtonDeformableMeshGetParticleCount (const NewtonCollision* const deformableMesh); 
	NEWTON_API void NewtonDeformableMeshGetParticlePosition (NewtonCollision* const deformableMesh, int particleIndex, dFloat* const posit);

	NEWTON_API void NewtonDeformableMeshBeginConfiguration (const NewtonCollision* const deformableMesh); 
	NEWTON_API void NewtonDeformableMeshUnconstraintParticle (NewtonCollision* const deformableMesh, int particleIndex);
	NEWTON_API void NewtonDeformableMeshConstraintParticle (NewtonCollision* const deformableMesh, int particleIndex, const dFloat* const posit, const NewtonBody* const body);
	NEWTON_API void NewtonDeformableMeshEndConfiguration (const NewtonCollision* const deformableMesh); 

//	NEWTON_API void NewtonDeformableMeshSetPlasticity (NewtonCollision* const deformableMesh, dFloat plasticity);
//	NEWTON_API void NewtonDeformableMeshSetStiffness (NewtonCollision* const deformableMesh, dFloat stiffness);
	NEWTON_API void NewtonDeformableMeshSetSkinThickness (NewtonCollision* const deformableMesh, dFloat skinThickness);

	NEWTON_API void NewtonDeformableMeshUpdateRenderNormals (const NewtonCollision* const deformableMesh); 
	NEWTON_API int NewtonDeformableMeshGetVertexCount (const NewtonCollision* const deformableMesh); 
	NEWTON_API void NewtonDeformableMeshGetVertexStreams (const NewtonCollision* const deformableMesh, int vertexStrideInByte, dFloat* const vertex, int normalStrideInByte, dFloat* const normal, int uvStrideInByte0, dFloat* const uv0);
	NEWTON_API NewtonDeformableMeshSegment* NewtonDeformableMeshGetFirstSegment (const NewtonCollision* const deformableMesh);
	NEWTON_API NewtonDeformableMeshSegment* NewtonDeformableMeshGetNextSegment (const NewtonCollision* const deformableMesh, const NewtonDeformableMeshSegment* const segment);

	NEWTON_API int NewtonDeformableMeshSegmentGetMaterialID (const NewtonCollision* const deformableMesh, const NewtonDeformableMeshSegment* const segment);
	NEWTON_API int NewtonDeformableMeshSegmentGetIndexCount (const NewtonCollision* const deformableMesh, const NewtonDeformableMeshSegment* const segment);
	NEWTON_API const int* NewtonDeformableMeshSegmentGetIndexList (const NewtonCollision* const deformableMesh, const NewtonDeformableMeshSegment* const segment);




	// **********************************************************************************************
	//
	// Ball and Socket joint functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonJoint* NewtonConstraintCreateBall (const NewtonWorld* const newtonWorld, const dFloat* pivotPoint, 
														const NewtonBody* const childBody, const NewtonBody* const parentBody);
	NEWTON_API void NewtonBallSetUserCallback (const NewtonJoint* const ball, NewtonBallCallback callback);
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

	NEWTON_API void NewtonHingeSetUserCallback (const NewtonJoint* const hinge, NewtonHingeCallback callback);
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
	NEWTON_API void NewtonSliderSetUserCallback (const NewtonJoint* const slider, NewtonSliderCallback callback);
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
	NEWTON_API void NewtonCorkscrewSetUserCallback (const NewtonJoint* const corkscrew, NewtonCorkscrewCallback callback);
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
	NEWTON_API void NewtonUniversalSetUserCallback (const NewtonJoint* const universal, NewtonUniversalCallback callback);
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
															 NewtonUserBilateralCallback callback,
															 NewtonUserBilateralGetInfoCallback getInfo,
															 const NewtonBody* const childBody, const NewtonBody* const parentBody) ; 
	NEWTON_API void NewtonUserJointSetFeedbackCollectorCallback (const NewtonJoint* const joint, NewtonUserBilateralCallback getFeedback);
	NEWTON_API void NewtonUserJointAddLinearRow (const NewtonJoint* const joint, const dFloat* const pivot0, const dFloat* const pivot1, const dFloat* const dir);
	NEWTON_API void NewtonUserJointAddAngularRow (const NewtonJoint* const joint, dFloat relativeAngle, const dFloat* const dir);
	NEWTON_API void NewtonUserJointAddGeneralRow (const NewtonJoint* const joint, const dFloat* const jacobian0, const dFloat* const jacobian1);
	NEWTON_API void NewtonUserJointSetRowMinimumFriction (const NewtonJoint* const joint, dFloat friction);
	NEWTON_API void NewtonUserJointSetRowMaximumFriction (const NewtonJoint* const joint, dFloat friction);
	NEWTON_API void NewtonUserJointSetRowAcceleration (const NewtonJoint* const joint, dFloat acceleration);
	NEWTON_API void NewtonUserJointSetRowSpringDamperAcceleration (const NewtonJoint* const joint, dFloat springK, dFloat springD);
	NEWTON_API void NewtonUserJointSetRowStiffness (const NewtonJoint* const joint, dFloat stiffness);
	NEWTON_API dFloat NewtonUserJointGetRowForce (const NewtonJoint* const joint, int row);

	NEWTON_API void NewtonUserJointSetSolver (const NewtonJoint* const joint, int solver, int maxContactJoints);


	// ************************************************************************************************************************
	// 
	//	Acyclic articulation offer the same level of accuracy that Feather stone reduced coordinate link chains algorithm  
	//	these are goo to make near perfect Rag dolls, physically based Created animated bodies, simple robotic contractions with not internal loops,
	//	Vehicles, Ropes, etc. that all interact seamlessly and natural with the physics world.
	//	
	// ************************************************************************************************************************
	NEWTON_API NewtonAcyclicArticulation* NewtonAcyclicArticulationCreate (NewtonBody* const rootBone);
	NEWTON_API void NewtonAcyclicArticulationDelete (NewtonAcyclicArticulation* const articulation);
	NEWTON_API void* NewtonAcyclicArticulationAttachBone (NewtonAcyclicArticulation* const articulation, NewtonBody* const parentBone, NewtonBody* const childBone);
	NEWTON_API void NewtonAcyclicArticulationDetachBone (NewtonAcyclicArticulation* const articulation, void* const bone);
	NEWTON_API void NewtonAcyclicArticulationAddJoint (NewtonAcyclicArticulation* const articulation, NewtonJoint* const joint);


	// **********************************************************************************************
	//
	// Mesh joint functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonMesh* NewtonMeshCreate(const NewtonWorld* const newtonWorld);
	NEWTON_API NewtonMesh* NewtonMeshCreateFromMesh(const NewtonMesh* const mesh);
	NEWTON_API NewtonMesh* NewtonMeshCreateFromCollision(const NewtonCollision* const collision);
	NEWTON_API NewtonMesh* NewtonMeshCreateConvexHull (const NewtonWorld* const newtonWorld, int pointCount, const dFloat* const vertexCloud, int strideInBytes, dFloat tolerance);
	NEWTON_API NewtonMesh* NewtonMeshCreateDelaunayTetrahedralization (const NewtonWorld* const newtonWorld, int pointCount, const dFloat* const vertexCloud, int strideInBytes, int materialID, const dFloat* const textureMatrix);
	NEWTON_API NewtonMesh* NewtonMeshCreateVoronoiConvexDecomposition (const NewtonWorld* const newtonWorld, int pointCount, const dFloat* const vertexCloud, int strideInBytes, int materialID, const dFloat* const textureMatrix);
	NEWTON_API NewtonMesh* NewtonMeshCreateFromSerialization (const NewtonWorld* const newtonWorld, NewtonDeserializeCallback deserializeFunction, void* const serializeHandle);
	NEWTON_API void NewtonMeshDestroy(const NewtonMesh* const mesh);

	NEWTON_API void NewtonMeshSerialize (const NewtonMesh* const mesh, NewtonSerializeCallback serializeFunction, void* const serializeHandle);
	NEWTON_API void NewtonMeshSaveOFF(const NewtonMesh* const mesh, const char* const filename);
	NEWTON_API NewtonMesh* NewtonMeshLoadOFF(const NewtonWorld* const newtonWorld, const char* const filename);

	NEWTON_API void NewtonMeshApplyTransform (const NewtonMesh* const mesh, const dFloat* const matrix);
	NEWTON_API void NewtonMeshCalculateOOBB(const NewtonMesh* const mesh, dFloat* const matrix, dFloat* const x, dFloat* const y, dFloat* const z);

	NEWTON_API void NewtonMeshCalculateVertexNormals(const NewtonMesh* const mesh, dFloat angleInRadians);
	NEWTON_API void NewtonMeshApplySphericalMapping(const NewtonMesh* const mesh, int material);
	NEWTON_API void NewtonMeshApplyCylindricalMapping(const NewtonMesh* const mesh, int cylinderMaterial, int capMaterial);
	NEWTON_API void NewtonMeshApplyBoxMapping(const NewtonMesh* const mesh, int frontMaterial, int sideMaterial, int topMaterial);
	NEWTON_API void NewtonMeshApplyAngleBasedMapping(const NewtonMesh* const mesh, int material, NewtonReportProgress reportPrograssCallback, void* const reportPrgressUserData);
	
	
	NEWTON_API int NewtonMeshIsOpenMesh (const NewtonMesh* const mesh);
	NEWTON_API void NewtonMeshFixTJoints (const NewtonMesh* const mesh);

	NEWTON_API void NewtonMeshPolygonize (const NewtonMesh* const mesh);
	NEWTON_API void NewtonMeshTriangulate (const NewtonMesh* const mesh);
	NEWTON_API NewtonMesh* NewtonMeshUnion (const NewtonMesh* const mesh, const NewtonMesh* const clipper, const dFloat* const clipperMatrix);
	NEWTON_API NewtonMesh* NewtonMeshDifference (const NewtonMesh* const mesh, const NewtonMesh* const clipper, const dFloat* const clipperMatrix);
	NEWTON_API NewtonMesh* NewtonMeshIntersection (const NewtonMesh* const mesh, const NewtonMesh* const clipper, const dFloat* const clipperMatrix);
	NEWTON_API void NewtonMeshClip (const NewtonMesh* const mesh, const NewtonMesh* const clipper, const dFloat* const clipperMatrix, NewtonMesh** const topMesh, NewtonMesh** const bottomMesh);

	NEWTON_API NewtonMesh* NewtonMeshConvexMeshIntersection (const NewtonMesh* const mesh, const NewtonMesh* const convexMesh);

	NEWTON_API NewtonMesh* NewtonMeshSimplify (const NewtonMesh* const mesh, int maxVertexCount, NewtonReportProgress reportPrograssCallback, void* const reportPrgressUserData);
	NEWTON_API NewtonMesh* NewtonMeshApproximateConvexDecomposition (const NewtonMesh* const mesh, dFloat maxConcavity, dFloat backFaceDistanceFactor, int maxCount, int maxVertexPerHull, NewtonReportProgress reportProgressCallback, void* const reportProgressUserData);

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
	NEWTON_API void NewtonMeshCalculateFaceNormal (const NewtonMesh* const mesh, const void* const face, dFloat64* const normal);

	NEWTON_API void NewtonMeshSetFaceMaterial (const NewtonMesh* const mesh, const void* const face, int matId);


#ifdef __cplusplus 
}
#endif
#endif



