/*
 * Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
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

#include "Newton.h"
#include "NewtonClass.h"
#include "NewtonStdAfx.h"

void NewtonInitGlobals() {
	dgInitMemoryGlobals();
}

void NewtonDestroyGlobals() {
	dgDestroyMemoryGlobals();
}


#ifdef _WIN32
#ifdef _DEBUG
//#define DG_USED_DEBUG_EXCEPTIONS
#endif // _DEBUG
#endif // _WIN32

#ifdef _DEBUG

void TraceFuntionName(const char *name) {
	// static int trace;
	// dgTrace(("%d %s\n", trace, name));
	dgTrace(("%s\n", name));
}

//#define TRACE_FUNTION(name) TraceFuntionName (name)
#define TRACE_FUNTION(name)
#else
#define TRACE_FUNTION(name)
#endif // _DEBUG

//#define SAVE_COLLISION
#ifdef SAVE_COLLISION

void SerializeFile(void *serializeHandle, const void *buffer, size_t size) {
	fwrite(buffer, size, 1, (FILE *)serializeHandle);
}

void DeSerializeFile(void *serializeHandle, void *buffer, size_t size) {
	fread(buffer, size, 1, (FILE *)serializeHandle);
}

void SaveCollision(const NewtonCollision *collisionPtr) {
	FILE *file;
	// save the collision file
	file = fopen("collisiontest.bin", "wb");
	// SerializeFile(file, MAGIC_NUMBER, strlen(MAGIC_NUMBER) + 1);
	NewtonCollisionSerialize(collisionPtr, SerializeFile, file);
	fclose(file);
}

#endif // SAVE_COLLISION

/*
 * *dFloat* globalScale - global scale factor that will scale all internal tolerance.
 * Remarks: the physics system in theory should be dimensionless, however in practice the engine have to be implemented with
 * limited precision floating numbers and is also built for real-time simulation, it is inevitable that tolerances have to be used in order to increase performance, and
 * reduce instabilities. These tolerances make the engine dimension dependent, for example let say a rigid body is considered at rest when
 * its velocity is less than 0.01 units per second for some frames. For a program using meters as unit this translate to 0.01 meters per second,
 * however for a program using centimeter this translate to 0.0001 meter per second, since the precession of speed is independent of the unit system,
 * this means that in the second system the engine has to work much harder to bring the body to rest. A solution for this is to scale all tolerances
 * to match the unit system. The argument *globalScale* must be a constant to convert the unit system in the game to meters, for example if in your game you are using 39 units is a meter,
 * the *globaScale* must be 39. The exact conversion factor does not have to be exact, but the closer it is to reality the better performance the application will get.
 * Applications that are already using meters as the unit system must pass 1.0 as *globalSscale*.
 */

/*!
 * Return the exact amount of memory use by the engine and any given time time.
 *
 * @return
 *   Total memory use by the engine.
 *
 * @remarks
 *   This function is useful for application to determine if the memory
 *   use by the engine is balanced at all time.
 *
 * @see
 *   NewtonCreate
 */
int NewtonGetMemoryUsed() {
	// Newton* world;
	// dgMemoryAllocator* allocator;

	TRACE_FUNTION(__FUNCTION__);

	// world = (Newton*)newtonWorld;
	// allocator = world->dgWorld::GetAllocator();

	return dgGetMemoryUsed();
}

void NewtonSetMemorySystem(NewtonAllocMemory mallocFnt, NewtonFreeMemory mfreeFnt) {
	dgMemFree _free;
	dgMemAlloc _malloc;

	TRACE_FUNTION(__FUNCTION__);

	if (mallocFnt && mfreeFnt) {
		_malloc = (dgMemAlloc)mallocFnt;
		_free = (dgMemFree)mfreeFnt;
	} else {
		_malloc = (dgMemAlloc)Newton::DefaultAllocMemory;
		_free = (dgMemFree)Newton::DefaultFreeMemory;
	}

	dgSetGlobalAllocators(_malloc, _free);
}

// ***************************************************************************************************************
//
// Name: World interface
//
// ***************************************************************************************************************

/*!
 * Create an instance of the Newton world.
 *
 * @return
 *   A pointer to an instance of the Newton world.
 *
 * @remarks
 *   This function must be called before any of the other API functions.
 *
 * @see
 *   NewtonDestroy,
 *   NewtonDestroyAllBodies
 */
NewtonWorld *NewtonCreate() {
	TRACE_FUNTION(__FUNCTION__);
	dgMemoryAllocator *const allocator = new dgMemoryAllocator();

	dFloat p0[4];
	dFloat p1[4];

	NewtonWorld *const world = (NewtonWorld *)new (allocator) Newton(
	                               dgFloat32(1.0f), allocator);

	p0[0] = -100.0f;
	p0[1] = -100.0f;
	p0[2] = -100.0f;
	p1[0] = 100.0f;
	p1[1] = 100.0f;
	p1[2] = 100.0f;

	NewtonSetWorldSize(world, p0, p1);
	return world;
}

/*!
 * Destroy an instance of the Newton world.
 *
 * @param newtonWorld
 *   Is the pointer to the Newton world.
 *
 * @remarks
 *   This function will destroy the entire Newton world.
 *
 * @see
 *   NewtonCreate,
 *   NewtonDestroyAllBodies
 */
void NewtonDestroy(const NewtonWorld *const newtonWorld) {
	TRACE_FUNTION(__FUNCTION__);

	const Newton *const world = (const Newton *)newtonWorld;
	dgMemoryAllocator *const allocator = world->dgWorld::GetAllocator();

	delete world;
	delete allocator;
}

// Name: NewtonInvalidateCache
// Reset all internal states of the engine.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Remarks: When an application wants to reset the state of all the objects in the world to a predefined initial condition,
// just setting the initial position and velocity is not sufficient to reproduce equal runs since the engine maintain
// there are internal states that in order to take advantage of frame to frame coherence.
// In this cases this function will reset all of the internal states.
//
// Remarks: This function must be call outside of a Newton Update. this function should only be used for special case of synchronization,
// using it as part of the simulation loop will severally affect the engine performance.
//
// See also: NewtonUpdate
void NewtonInvalidateCache(NewtonWorld *const newtonWorld) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;
	world->FlushCache();
}

// Name: NewtonGetGlobalScale
// Get the global scale factor.
//
// Remarks: the physics system in theory should be dimensionless, however in practice the engine have to be implemented with
// limited precision floating numbers and is also built for real-time simulation, it is inevitable that tolerances have to be used in order to increase performance, and
// reduce instabilities. These tolerances make the engine dimension dependent, for example let say a rigid body is considered at rest when
// its velocity is less than 0.01 units per second for some frames. For a program using meters as unit this translate to 0.01 meters per second,
// however for a program using centimeter this translate to 0.0001 meter per second, since the precession of speed is independent of the unit system,
// this means that in the second system the engine has to work much harder to bring the body to rest. A solution for this is to scale all tolerances
// to match the unit system. The argument *globalScale* must be a constant to convert the unit system in the game to meters, for example if in your game you are using 39 units is a meter,
// the *globaScale* must be 39. The exact conversion factor does not have to be exact, but the closer it is to reality the better performance the application will get.
// Applications that are already using meters as the unit system must pass 1.0 as *globalScale*.
//
// See also: NewtonCreate
// dFloat NewtonGetGlobalScale(const NewtonWorld* const newtonWorld)
//{
//	Newton* world;
//	world = (Newton *) newtonWorld;
//	return world->GetGlobalScale();
//	return dgFloat32(1.0f);
//}

// Name: NewtonSetPlatformArchitecture
// Set the current platform hardware architecture.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* model - model of operation 0 = default, 1 = medium, n = best.
//
// Return: Nothing
//
// Remarks: This function allows the application to configure the Newton to take advantage
// for specific hardware architecture in the same platform.
//
// 0: force the hardware lower common denominator for the running platform.
//
// 1: will try to use common floating point enhancement like special instruction set
// on the specific architecture. This mode made lead to result that differ from mode 1 and 2 as the accumulation
// round off errors maybe different.
//
// Remarks: the only hardware mode guarantee to work is mode 0. all other are only
// hints to the engine, for example setting mode 1 will take not effect on CPUs without
// specially floating point instructions set.
//
// See also: NewtonGetPlatformArchitecture
void NewtonSetPlatformArchitecture(NewtonWorld *const newtonWorld,
                                   int mode) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;
	world->SetHardwareMode(mode);
}

// Name: NewtonGetPlatformArchitecture
// Get the current platform hardware architecture.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *char* *description - pointer to a string to contain a description of the current architecture
//
// Return: index indicating the current platform architecture.
//
// Remarks: if *description* is not NULL, then is must be at least 32 characters long.
//
// Remarks: This function allows the application to configure the Newton to take advantage
// for specific hardware architecture in the same platform.
//
// 0: force the hardware lower common denominator for the running platform.
//
// 1: will try to use common floating point enhancement like special instruction set
// on the specific architecture. This mode made lead to result that differ from mode 1 and 2 as the accumulation
// round off errors maybe different.
//
// Remarks: the only hardware mode guarantee to work is mode 0. all other are only
// hints to the engine, for example setting mode 1 will take not effect on CPUs without
// specially floating point instructions set.
//
// See also: NewtonSetPlatformArchitecture
int NewtonGetPlatformArchitecture(const NewtonWorld *const newtonWorld,
                                  char *description) {
	TRACE_FUNTION(__FUNCTION__);
	const Newton *const world = (const Newton *)newtonWorld;
	return world->GetHardwareMode(description);
}

// Name: NewtonWorldCriticalSectionLock
// this function block all other threads from executing the same subsequent code simultaneously.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Remarks: this function should use to present racing conditions when when a call back ins executed form a mutithreaded loop.
// In general most call back are thread safe when they do not write to object outside the scope of the call back.
// this means for example that the application can modify values of object pointed by the arguments and or call that function
// that are allowed to be call for such callback.
// There are cases, however, when the application need to collect data for the client logic, example of such case are collecting
// information to display debug information, of collecting data for feedback.
// In these situations it is possible the the same critical code could be execute at the same time but several thread causing unpredictable side effect.
// so it is necessary to block all of the thread from executing any pieces of critical code.
//
// Remarks: Not calling function *NewtonWorldCriticalSectionUnlock* will result on the engine going into an infinite loop.
//
// Remarks: it is important that the critical section wrapped by functions *NewtonWorldCriticalSectionLock* and
// *NewtonWorldCriticalSectionUnlock* be keep small if the application is using the multi threaded functionality of the engine
// no doing so will lead to serialization of the parallel treads since only one thread can run the a critical section at a time.
//
// Return: Nothing.
//
// See also: NewtonWorldCriticalSectionUnlock
void NewtonWorldCriticalSectionLock(NewtonWorld *const newtonWorld) {
	TRACE_FUNTION(__FUNCTION__);

	Newton *const world = (Newton *)newtonWorld;
	world->dgGetUserLock();
}

// Name: NewtonWorldCriticalSectionUnlock
// this function block all other threads from executing the same subsequent code simultaneously.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
//
// Remarks: this function should use to present racing conditions when when a call back ins executed form a mutithreaded loop.
// In general most call back are thread safe when they do not write to object outside the scope of the call back.
// this means for example that the application can modify values of object pointed by the arguments and or call that function
// that are allowed to be call for such callback.
// There are cases, however, when the application need to collect data for the client logic, example of such case are collecting
// information to display debug information, of collecting data for feedback.
// In these situations it is possible the the same critical code could be execute at the same time but several thread causing unpredictable side effect.
// so it is necessary to block all of the thread from executing any pieces of critical code.
//
// Remarks: it is important that the critical section wrapped by functions *NewtonWorldCriticalSectionLock* and
// *NewtonWorldCriticalSectionUnlock* be keep small if the application is using the multi threaded functionality of the engine
// no doing so will lead to serialization of the parallel treads since only one thread can run the a critical section at a time.
//
// Return: Nothing.
//
// See also: NewtonWorldCriticalSectionLock
void NewtonWorldCriticalSectionUnlock(NewtonWorld *const newtonWorld) {
	TRACE_FUNTION(__FUNCTION__);

	Newton *const world = (Newton *)newtonWorld;
	world->dgReleasedUserLock();
}

// Name: NewtonSetThreadsCount
// Set the maximum number of thread the engine is allowed to use by the application.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* threads - max number of threaded allowed
//
// Return: Nothing
//
// Remarks: The maximum number of threaded is set on initialization to the maximum number of CPU in the system.
//
// Remarks: The engine will only allow a maximum number of thread equal are lower to the maximum number of cores visible in the system.
// if *threads* is set to a value larger that then number of logical CPUs in the system, the *threads* will be clamped to the number of logical CPUs.
//
// Remarks: the function is only only have effect on the multi core version of the engine.
//
// See also: NewtonGetThreadNumber, NewtonGetThreadsCount
void NewtonSetThreadsCount(NewtonWorld *const newtonWorld, int threads) {
	TRACE_FUNTION(__FUNCTION__);

	Newton *const world = (Newton *)newtonWorld;
	world->SetThreadsCount(threads);
}

// Name: NewtonGetThreadsCount
// Get the total number of thread running in the engine.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
//
// Return: number threads
//
// Remarks: The maximum number of threaded is set on initialization to the maximum number of CPU in the system.
//
// Remarks: the function will always return 1 on the none multi core version of the library..
//
// See also: NewtonGetThreadNumber, NewtonSetThreadsCount, NewtonSetMultiThreadSolverOnSingleIsland
int NewtonGetThreadsCount(const NewtonWorld *const newtonWorld) {
	TRACE_FUNTION(__FUNCTION__);

	const Newton *const world = (const Newton *)newtonWorld;
	return world->GetThreadsCount();
}

// Name: NewtonGetMaxThreadsCount
// Get the maximu number of thread abialble.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
//
// Return: number threads
//
// Remarks: The maximum number of threaded is set on initialization to the maximum number of CPU in the system.
//
// Remarks: the function will always return 1 on the none multi core version of the library..
//
// See also: NewtonGetThreadNumber, NewtonSetThreadsCount, NewtonSetMultiThreadSolverOnSingleIsland
int NewtonGetMaxThreadsCount(const NewtonWorld *const newtonWorld) {
	TRACE_FUNTION(__FUNCTION__);

	const Newton *const world = (const Newton *)newtonWorld;
	return world->GetMaxThreadsCount();
}

/*
 // Name: NewtonGetThreadNumber
 // Get the current thread the Engine is running
 //
 // Parameters:
 // *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
 //
 // Return: the index to the current workir thread runnin the call back. -1 if the function is called ourxoed of a call back
 //
 // Remarks: This function let application to read the current thread running in a callback
 //
 // Remarks: the function will always return 1 on the none multi core version of the library..
 //
 // See also: NewtonGetThreadsCount, NewtonSetThreadsCount, NewtonSetMultiThreadSolverOnSingleIsland
 int NewtonGetThreadNumber(const NewtonWorld* const newtonWorld)
 {
 Newton* world;
 world = (Newton *)newtonWorld;
 TRACE_FUNTION(__FUNCTION__);
 return world->GetThreadNumber();
 }
 */

// Name: NewtonSetMultiThreadSolverOnSingleIsland
// Enable or disable solver to resolve constraint forces in multi threaded mode when large island configurations. Mode is disabled by default.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* mode - solver mode 1 enable parallel mode 0 disable parallel mode, default
//
// Return: Nothing
//
// Remarks: When running in multi threaded mode it is not always faster to calculate constraint forces in parallel.
// there reasons for this are:
// 1 - there is a significant software cost for setting threads both in memory and instructions overhead.
// 2 - different systems have different cost for running separate threads in a share memory environment
// 3 - numerical algorithms have decreased converge rate when implemented in parallel, typical lost of converge can be as high as half
// of the of the sequential version, for this reason the parallel version require higher number of interaction to achieve similar convergence.
//
// Remarks: It is recommended this option is enabled on system with more than two cores, since the performance gain in a dual core system are marginally better.
// at the very list the application must test the option to verify the performance gains.
//
// Remarks: disabling or enabling this option have not impact on the execution of the any of the other subsystems of the engine.
//
// See also: NewtonGetThreadsCount, NewtonSetThreadsCount
void NewtonSetMultiThreadSolverOnSingleIsland(
    NewtonWorld *const newtonWorld, int mode) {
	Newton *world;
	world = (Newton *)newtonWorld;
	TRACE_FUNTION(__FUNCTION__);
	world->EnableThreadOnSingleIsland(mode);
}

int NewtonGetMultiThreadSolverOnSingleIsland(
    const NewtonWorld *const newtonWorld) {
	const Newton *world;
	world = (const Newton *)newtonWorld;
	TRACE_FUNTION(__FUNCTION__);
	return world->GetThreadOnSingleIsland();
}

// Name: NewtonSetSolverModel
// Set the solver precision mode.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* model - model of operation 0 = exact, 1 = adaptive, n = linear. The default is exact.
//
// Return: Nothing
//
// Remarks: This function allows the application to configure the Newton solver to work in three different modes.
//
// 0: Is the exact mode. This is good for application where precision is more important than speed, ex: realistic simulation.
//
// 1: Is the adaptive mode, the solver is not as exact but the simulation will still maintain a high degree of accuracy.
// This mode is good for applications were a good degree of stability is important but not as important as speed.
//
// n: Linear mode. The solver will not try to reduce the joints relative acceleration errors to below some limit,
// instead it will perform up to n passes over the joint configuration each time reducing the acceleration error,
// but it will terminate when the number of passes is exhausted regardless of the error magnitude.
// In general this is the fastest mode and is is good for applications where speed is the only important factor, ex: video games.
//
// Remarks: the adaptive friction model combined with the linear model make for the fastest possible configuration
// of the Newton solver. This setup is best for games.
// If you need the best realistic behavior, we recommend the use of the exact solver and exact friction model which are the defaults.
//
// See also: NewtonSetFrictionModel, NewtonGetThreadNumber
void NewtonSetSolverModel(NewtonWorld *const newtonWorld, int model) {
	Newton *world;
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	world->SetSolverMode(model);
}

// Name: NewtonSetFrictionModel
// Set coulomb model of friction.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* model - friction model;  0 = exact coulomb, 1 = adaptive coulomb, The default is exact.
//
// Return: Nothing.
//
// Remarks: This function allows the application to chose between and exact or an adaptive coulomb friction model
//
// 0: Is the exact model. Friction forces are calculated in each frame.
// This model is good for applications where precision is more important than speed, ex: realistic simulation.
//
// 1: Is the adaptive model. Here values from previous frames are used to determine the maximum friction values of the current frame.
// This is about 10% faster than the exact model however it may introduce strange friction behaviors. For example a
// bouncing object tumbling down a ramp will act as a friction less object because the contacts do not have continuity.
// In general each time a new contact is generated the friction value is zero, only if the contact persist a non zero
// friction values is used. The second effect is that if a normal force is very strong, and if the contact is suddenly
// destroyed, a very strong friction force will be generated at the contact point making the object react in a non-familiar way.
//
// Remarks: the adaptive friction model combined with the linear model make for the fastest possible configuration
// of the Newton solver. This setup is best for games.
// If you need the best realistic behavior, we recommend the use of the exact solver and exact friction model which are the defaults.
//
// See also: NewtonSetSolverModel
void NewtonSetFrictionModel(NewtonWorld *const newtonWorld, int model) {
	Newton *world;
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	world->SetFrictionMode(model);
}

// Name: NewtonSetPerformanceClock
// Set performance Counter callback.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *NewtonGetTicksCountCallback* callback - application define callback
//
// Remarks: The application can use this function to profile Newton. *NewtonGetTicksCountCallback* callback is a
// function call back that should return and absolute time since the the application started.
//
// Return: Nothing.
//
// See also: NewtonReadPerformanceTicks
void NewtonSetPerformanceClock(NewtonWorld *const newtonWorld,
                               NewtonGetTicksCountCallback callback) {
	Newton *world;
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	world->SetPerfomanceCounter((OnGetPerformanceCountCallback)callback);
}

// Name: NewtonReadPerformanceTicks
// Get the number of ticks used by the engine in one of the major components
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *unsigned* performanceEntry - index to one of the engine internal components.
//
// Remarks: there are 9 hierarchical counters
//
// NEWTON_PROFILER_WORLD_UPDATE: measure total ticks in each update call
// NEWTON_PROFILER_COLLISION_UPDATE: total ticks in each collision and contact calculation
// NEWTON_PROFILER_COLLISION_UPDATE_BROAD_PHASE: measure ticks on colliding pairs.
// NEWTON_PROFILER_COLLISION_UPDATE_NARROW_PHASE: measure ticks calculating contacts for each colliding pair.
// NEWTON_PROFILER_DYNAMICS_UPDATE: measure ticks on solving constraints, contact inter penetration, and integration
// NEWTON_PROFILER_DYNAMICS_CONSTRAINT_GRAPH: measure ticks solving contacts and constraint graph interconnectivity.
// NEWTON_PROFILER_DYNAMICS_BUILD_MASS_MATRIX: measure ticks solving preparing constraint matrix
// NEWTON_PROFILER_DYNAMICS_SOLVE_CONSTRAINT_GRAPH: measure tick solving constraint matrix and calculating reaction forces and integration.
//
// Remarks: This function will return zero unless the application had previous
// set a performance counter callback by calling the function *NewtonSetPerformanceClock*
//
// Return: Ticks count used by application in lass call to *NewtonUpdate*
//
// See also: NewtonReadPerformanceTicks, NewtonUpdate
unsigned NewtonReadPerformanceTicks(const NewtonWorld *const newtonWorld,
                                    unsigned performanceEntry) {
	const Newton *world;
	world = (const Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	return world->GetPerfomanceTicks(performanceEntry);
}

unsigned NewtonReadThreadPerformanceTicks(const NewtonWorld *newtonWorld,
        unsigned threadIndex) {
	const Newton *const world = (const Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	return world->GetThreadPerfomanceTicks(threadIndex);
}

#ifdef DG_USED_DEBUG_EXCEPTIONS
dgInt32 ExecptionHandler(void *exceptPtr) {
	EXCEPTION_RECORD *record;

	record = ((EXCEPTION_POINTERS *)exceptPtr)->ExceptionRecord;
	//  if (record->ExceptionCode != EXCEPTION_ACCESS_VIOLATION) {
	//      return 0;
	//  }
	return 1;
}
#endif

// Name: NewtonUpdate
// Advance the simulation by an amount of time.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *dFloat* timestep - time step in seconds
//
// Return: Nothing
//
// Remarks: This function will advance the simulation by the amount of time specified by *timestep*. The Newton Engine does
// not perform sub-steps, and does not need tuning parameters. It is the responsibility of the application to
// ensure that *timestep* is small enough to guarantee physics stability.
//
// Return: This function call NewtonCollisionUpdate at the lower level to get the colliding contacts.
//
// See also: NewtonInvalidateCache, NewtonCollisionUpdate
void NewtonUpdate(NewtonWorld *const newtonWorld, dFloat timestep) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;

	// timestep = ClampValue (timestep, MIN_TIMESTEP, MAX_TIMESTEP * 2.0f);
	timestep = ClampValue(dgFloat32(timestep), dgFloat32(MIN_TIMESTEP),
	                      dgFloat32(dgFloat32(1.0f / 60.0f)));
	dgInt32 count = dgInt32(
	                    dgCeil(timestep / (world->g_maxTimeStep + dgFloat32(1.0e-10f))));
	dgFloat32 time = timestep / count;

	for (dgInt32 i = 0; i < count; i++) {
#ifdef DG_USED_DEBUG_EXCEPTIONS
		__try {
			world->UpdatePhysics(time);
		} __except (ExecptionHandler(_exception_info())) {
			NEWTON_ASSERT(0);
			//              world->UpdatePhysics (time);
		}
#else
		world->UpdatePhysics(time);
#endif
	}
}

// Name: NewtonCollisionUpdate
// Update the collision state of all object in eh world.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
//
// Return: Nothing
//
// Remarks: This function will update all of the collision for all body in the physics world.
// the function is meant for application that are using the engine as collision system only,
// sample of these are legacy application that use collision but no physics, level editor, plug in for modeling packages
// and even physics application that some time do not need to execute and dynamics simulation step for what every reason
//
// Return: when calling this function the application do not need to call NetwonUpdate
//
// See also: NewtonUpdate, NewtonInvalidateCache
void NewtonCollisionUpdate(NewtonWorld *const newtonWorld) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;

#ifdef DG_USED_DEBUG_EXCEPTIONS
	__try {
		world->UpdateCollision();
	} __except (ExecptionHandler(_exception_info())) {
		world->UpdateCollision();
	}
#else
	world->UpdateCollision();
#endif
}

// Name: NewtonSetMinimumFrameRate
// Set the minimum frame rate at which the simulation can run.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *dFloat* frameRate - minimum frame rate of the simulation in frame per second. This value is clamped between 60fps and 1000fps
//
// Return: nothing
//
// Remarks: the default minimum frame rate of the simulation is 60 frame per second. When the simulation falls below the specified minimum frame, Newton will
// perform sub steps in order to meet the desired minimum FPS.
void NewtonSetMinimumFrameRate(NewtonWorld *const newtonWorld,
                               dFloat frameRate) {
	Newton *world;

	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	NEWTON_ASSERT(
	    dgFloat32(1.0f) / dgFloat32(MAX_TIMESTEP) < dgFloat32(1.0f) / dgFloat32(MIN_TIMESTEP));
	frameRate = ClampValue(frameRate, dgFloat32(1.0f) / dgFloat32(MAX_TIMESTEP),
	                       dgFloat32(1.0f) / dgFloat32(MIN_TIMESTEP));
	world->g_maxTimeStep = dgFloat32(1.0f) / frameRate;
}

/*
 // Name: NewtonGetTimeStep
 // Return the correct time step for this simulation update.
 //
 // Parameters:
 // *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
 //
 // Remark: This application can used this function to get the current simulation time step.
 //
 // Return: correct update timestep.
 dFloat NewtonGetTimeStep(const NewtonWorld* const newtonWorld)
 {
 Newton* world;
 world = (Newton *)newtonWorld;
 return world->GetTimeStep();
 }
 */

// Name: NewtonDestroyAllBodies
// Remove all bodies and joints from the newton world.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is a pointer to the Newton world
//
// Return: Nothing
//
// Remarks: This function will destroy all bodies and all joints in the Newton world, but it will retain group IDs.
// Use this function for when you want to clear the world but preserve all the group IDs and material pairs.
//
// See also: NewtonMaterialDestroyAllGroupID
void NewtonDestroyAllBodies(NewtonWorld *const newtonWorld) {
	dFloat p0[4];
	dFloat p1[4];
	Newton *world;

	p0[0] = -100.0f;
	p0[1] = -100.0f;
	p0[2] = -100.0f;
	p1[0] = 100.0f;
	p1[1] = 100.0f;
	p1[2] = 100.0f;

	TRACE_FUNTION(__FUNCTION__);
	NewtonSetWorldSize(newtonWorld, p0, p1);

	world = (Newton *)newtonWorld;

	//  world->RagDollList::DestroyAll();
	world->DestroyAllBodies();
}

// Name: NewtonSetWorldSize
// Set the size of the Newton world.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *const dFloat* *minPtr - is the minimum point of the world bounding box
// *const dFloat* *maxPtr - is the maximum point of the world bounding box
//
// Return: Nothing.
//
// Remarks: The Newton world must have a finite size. The size of the world is set to a box +- 100 units in all three dimensions
// at creation time and after a call to the function _NewtonRemoveAllBodies_
//
// See also: NewtonSetBodyLeaveWorldEvent
void NewtonSetWorldSize(NewtonWorld *const newtonWorld,
                        const dFloat *const minPtr, const dFloat *const maxPtr) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;
	dgVector p0(minPtr[0], minPtr[1], minPtr[2], dgFloat32(1.0f));
	dgVector p1(maxPtr[0], maxPtr[1], maxPtr[2], dgFloat32(1.0f));
	world->SetWorldSize(p0, p1);
}

// Name: NewtonSetIslandUpdateEvent
// Set a function callback to be call on each island update.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *NewtonIslandUpdate* slandUpdate - application defined callback
//
// Return: Nothing.
//
// Remarks: The application can set a function callback to be called just after the array of all bodies making an island of articulated and colliding bodies are collected for resolution.
// This function will be called just before the array is accepted for solution and integration.
// The function callback may return one to validate the array or zero to skip the resolution of this array of bodies on this frame only.
// This functionality can be used by the application to implement in game physics LOD. For example the application can determine the AABB of the
// island and check against the view frustum, if the entire island AABB is invisible then the application can suspend simulation even if they are not in equilibrium.
// another functionality is the implementation of visual debuggers, and also the implementation of auto frozen bodies under arbitrary condition set by the logic of the application.
//
// Remarks: The application should not modify the position, velocity, or it create or destroy any body or joint during this function call. Doing so will result in unpredictable malfunctions.
//
// See also: NewtonIslandGetBody
void NewtonSetIslandUpdateEvent(NewtonWorld *const newtonWorld,
                                NewtonIslandUpdate islandUpdate) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	world->SetIslandUpdateCallback((OnIslandUpdate)islandUpdate);
}

// Name: NewtonSetCollisionDestructor
// Set a function callback to be call on each island update.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *NewtonIslandUpdate* slandUpdate - application defined callback
//
// Return: Nothing.
void NewtonSetCollisionDestructor(NewtonWorld *const newtonWorld,
                                  NewtonCollisionDestructor callback) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	world->SetDestroyCollisionCallback((OnDestroyCollision)callback);
}

// Name: NewtonSetDestroyBodyByExeciveForce
// Set a function callback to be call when the force applied at a contact point exceed the max force allowed for that convex shape
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *NewtonDestroyBodyByExeciveForce* callback - application defined callback
//
// Remarks: Only convex Hull shapes can have max brak force parameter.
//
// Return: Nothing.
void NewtonSetDestroyBodyByExeciveForce(NewtonWorld *const newtonWorld,
                                        NewtonDestroyBodyByExeciveForce callback) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	world->SetBodyDestructionByExeciveForce(
	    (OnBodyDestructionByExeciveForce)callback);
}

// Name: NewtonSetBodyLeaveWorldEvent
// Set the event callback function to be called in the event a body is escaping the limits of the world
// during simulation time.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *NewtonBodyLeaveWorld* callback - is the pointer to the function callback
//
// Return: Nothing
//
// Remarks: When a body moves outside the bounding box that defines the world space the body is automatically disabled
// and Newton calls the application defined callback function *NewtonBodyLeaveWorld callback*.
// The application should decide how to handle the event, because Newton will make the callback once.
// The only options available to the application are: do nothing or destroy the object.
//
// See also: NewtonSetWorldSize, NewtonBodyGetFirstJoint, NewtonBodyGetNextJoint
void NewtonSetBodyLeaveWorldEvent(NewtonWorld *const newtonWorld,
                                  NewtonBodyLeaveWorld callback) {
	Newton *world;
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	world->SetLeavingWorldCallback((OnLeavingWorldAction)callback);
}

// Name: NewtonWorldGetFirstBody
// get th firt body in the body in the world body list.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Return: nothing
//
// Remarks: The application can call this function to iterate thought every body in the world.
//
// Remarks: The application call this function for debugging purpose
// See also: NewtonWorldGetNextBody, NewtonBodyForEachPolygonDo, NewtonWorldForEachBodyInAABBDo, NewtonWorldForEachJointDo
NewtonBody *NewtonWorldGetFirstBody(const NewtonWorld *const newtonWorld) {
	const Newton *world;
	dgBodyMasterList::dgListNode *node;

	world = (const Newton *)newtonWorld;
	const dgBodyMasterList &masterList = *world;

	TRACE_FUNTION(__FUNCTION__);
	NEWTON_ASSERT(
	    masterList.GetFirst()->GetInfo().GetBody() == world->GetSentinelBody());
	node = masterList.GetFirst()->GetNext();
	//      body = node->GetInfo().GetBody();
	//      node = node->GetNext();
	//      callback ((const NewtonBody*) body);
	//  }
	if (node) {
		return (NewtonBody *)node->GetInfo().GetBody();
	} else {
		return NULL;
	}
}

// Name: NewtonWorldGetFirstBody
// get the fixt body in the general body.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Return: nothing
//
// Remarks: The application can call this function to iterate thought every body in the world.
//
// Remarks: The application call this function for debugging purpose
// See also: NewtonWorldGetFirstBody, NewtonBodyForEachPolygonDo, NewtonWorldForEachBodyInAABBDo, NewtonWorldForEachJointDo
NewtonBody *NewtonWorldGetNextBody(const NewtonWorld *const newtonWorld,
                                   const NewtonBody *const curBody) {
	const dgBody *const body = (const dgBody *)curBody;

	TRACE_FUNTION(__FUNCTION__);

	dgBodyMasterList::dgListNode *const node = body->GetMasterList()->GetNext();
	if (node) {
		return (NewtonBody *)node->GetInfo().GetBody();
	} else {
		return NULL;
	}
}

// Name: NewtonWorldForEachJointDo
// Iterate thought every joint in the world calling the function callback.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *NewtonJointIterator* callback - application define callback
// *void* callback - application define userdata
//
// Return: nothing
//
// Remarks: The application can call this function to iterate thought every joint in the world.
// the application should provide the function *NewtonJointIterator callback* to be called by Newton for every joint in the world
//
// Remarks: this function affect severally the performance of Newton. The application should call this function only for debugging
// or for serialization purposes.
//
// See also: NewtonBodyForEachPolygonDo, NewtonWorldForEachBodyInAABBDo, NewtonWorldGetFirstBody
void NewtonWorldForEachJointDo(const NewtonWorld *const newtonWorld,
                               NewtonJointIterator callback, void *const userData) {
	const Newton *world;
	dgBodyMasterList::dgListNode *node;
	dgBodyMasterListRow::dgListNode *jointNode;

	world = (const Newton *)newtonWorld;
	const dgBodyMasterList &masterList = *world;

	TRACE_FUNTION(__FUNCTION__);
	dgTree<dgConstraint *, dgConstraint *> jointMap(world->dgWorld::GetAllocator());
	for (node = masterList.GetFirst()->GetNext(); node; node = node->GetNext()) {
		dgBodyMasterListRow &row = node->GetInfo();
		for (jointNode = row.GetFirst(); jointNode;
		        jointNode = jointNode->GetNext()) {
			const dgBodyMasterListCell &cell = jointNode->GetInfo();
			if (cell.m_joint->GetId() != dgContactConstraintId) {
				if (!jointMap.Find(cell.m_joint)) {
					jointMap.Insert(cell.m_joint, cell.m_joint);
					callback((const NewtonJoint *)cell.m_joint, userData);
				}
			}
		}
	}
}

// Name: NewtonWorldForEachBodyInAABBDo
// Iterate thought every body in the world that intersect the AABB calling the function callback.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const dFloat* *p0 - pointer to an array of at least three floats to hold minimum value for the AABB.
// *const dFloat* *p1 - pointer to an array of at least three floats to hold maximum value for the AABB.
// *NewtonBodyIterator* callback - application define callback
// *void* callback - application define userdata
//
// Return: nothing
//
// Remarks: The application can call this function to iterate thought every body in the world.
// the application should provide the function *NewtonBodyIterator callback* to be called by Newton for every body in the world
//
// Remarks: For relatively small AABB volumes this function is much more inefficients
// that NewtonWorldGetFirstBody, however in case where the AABB contain must of the objects in the scene,
// the overhead of scanning the internal Broad face collision plus the AABB test make this function more expensive.
//
// See also: NewtonBodyForEachPolygonDo, NewtonWorldGetFirstBody
void NewtonWorldForEachBodyInAABBDo(const NewtonWorld *const newtonWorld,
                                    const dFloat *const p0, const dFloat *const p1, NewtonBodyIterator callback,
                                    void *const userData) {
	const Newton *world;

	world = (const Newton *)newtonWorld;
	dgVector q0(p0[0], p0[1], p0[2], dgFloat32(0.0f));
	dgVector q1(p1[0], p1[1], p1[2], dgFloat32(0.0f));

	TRACE_FUNTION(__FUNCTION__);
	world->ForEachBodyInAABB(q0, q1, (OnBodiesInAABB)callback, userData);
}

// Name: NewtonWorldGetVersion
// Return the current library version number.
//
// Parameters:
//
// Return: release decimal three digit value x.xx
// the first digit:  is mayor version number (interface changes among other things)
// the second digit: is mayor patch number (new features, and bug fixes)
// third digit: is minor bug fixed patch.
int NewtonWorldGetVersion() {
	TRACE_FUNTION(__FUNCTION__);
	return NEWTON_MAJOR_VERSION * 100 + NEWTON_MINOR_VERSION;
}

// Name: NewtonWorldFloatSize
// Return the current sizeof of float value in bytes.
//
// Parameters:
//
// Return: sizeof of float value in bytes
int NewtonWorldFloatSize() {
	TRACE_FUNTION(__FUNCTION__);
	return sizeof(dFloat);
}

// Name: NewtonWorldSetUserData
// Store a user defined data value with the world.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the newton world.
// *void* *userDataPtr - pointer to the user defined user data value.
//
// Return: Nothing.
//
// Remarks: The application can store a user defined value with the Newton world. The user data is useful for application developing
// object oriented classes based on the Newton API.
//
// See also: NewtonWorldGetUserData
void NewtonWorldSetUserData(NewtonWorld *const newtonWorld,
                            void *const userData) {
	Newton *world;
	world = (Newton *)newtonWorld;

	world->SetUserData(userData);
}

// Name: NewtonWorldGetUserData
// Retrieve a user previously stored user define value with the world.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Return: user data value.
//
// Remarks: The application can store a user defined value with the Newton world. The user data is useful for application developing
// object oriented classes based on the Newton API.
//
// See also: NewtonWorldSetDestructorCallBack, NewtonWorldGetUserData
void *NewtonWorldGetUserData(const NewtonWorld *const newtonWorld) {
	const Newton *world;
	world = (const Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	return world->GetUserData();
}

// Name: NewtonWorldSetDestructorCallBack
// set a function pointer as destructor call back.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *NewtonDestroyWorld* destructor - function poiter callback
//
// Remarks: The application can store a user defined destrutor call back function to be called at the  time the world is to be destruyed
//
// See also: NewtonWorldGetUserData
void NewtonWorldSetDestructorCallBack(NewtonWorld *const newtonWorld,
                                      NewtonDestroyWorld destructor) {
	Newton *world;
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);

	world->m_destructor = destructor;
}

// Name: NewtonWorldSetDestructorCallBack
// Return the function call back Pointer.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Remarks: The application can store a user defined destrutor call back function to be called at the  time the world is to be destruyed
//
// See also: NewtonWorldGetUserData, NewtonWorldSetDestructorCallBack
NewtonDestroyWorld NewtonWorldGetDestructorCallBack(
    const NewtonWorld *const newtonWorld) {
	const Newton *world;
	world = (const Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);

	return world->m_destructor;
}

// Name: NewtonWorldGetBodyCount
// return the total number of rigid bodies in the world.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
//
// Return: number of rigid bodies in this world.
//
int NewtonWorldGetBodyCount(const NewtonWorld *const newtonWorld) {
	const Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (const Newton *)newtonWorld;
	//  dgBodyMasterList &masterList = *world;

	//  NEWTON_ASSERT (masterList.GetFirst()->GetInfo().GetBody() == world->GetSentinelBody());
	//  return masterList.GetCount() - 1;
	return world->GetBodiesCount();
}

// Name: NewtonWorldGetConstraintCount
// return the total number of contsting in th eworld.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
//
// remark: this function will retrun the total numbe of joint including conctats
//
// Return: number of rigid bodies in this world.
//
int NewtonWorldGetConstraintCount(const NewtonWorld *const newtonWorld) {
	const Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (const Newton *)newtonWorld;
	return world->GetConstraintsCount();
}

// Name: NewtonWorldRayCast
// Shoot a ray from p0 to p1 and call the application callback with each ray intersection.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the world.
// *const dFloat* *p0 - pointer to an array of at least three floats containing the beginning of the ray in global space.
// *const dFloat* *p1 - pointer to an array of at least three floats containing the end of the ray in global space.
// *NewtonWorldRayFilterCallback* filter - user define function to be called for each body hit during the ray scan.
// *void* *userData - user data to be passed to the filter callback.
// *NewtonWorldRayPrefilterCallback* prefilter - user define function to be called for each body before intersection.
//
// Return: nothing
//
// Remarks: The ray cast function will call the application with each body intersecting the line segment.
// By writing the callback filter function in different ways the application can implement different flavors of ray casting.
// For example an all body ray cast can be easily implemented by having the filter function always returning 1.0, and copying each
// rigid body into an array of pointers; a closest hit ray cast can be implemented by saving the body with the smaller intersection
// parameter and returning the parameter t; and a report the first body hit can be implemented by having the filter function returning
// zero after the first call and saving the pointer to the rigid body.
//
// Remarks: The most common use for the ray cast function is the closest body hit, In this case it is important, for performance reasons,
// that the filter function returns the intersection parameter. If the filter function returns a value of zero the ray cast will terminate
// immediately.
//
// Remarks: if prefilter is not NULL, Newton will call the application right before executing the intersections between the ray and the primitive.
// if the function returns zero the Newton will not ray cast the primitive. passing a NULL pointer will ray cast the.
// The application can use this implement faster or smarter filters when implementing complex logic, otherwise for normal all ray cast
// this parameter could be NULL.
//
// Remarks: The ray cast function is provided as an utility function, this means that even thought the function is very high performance
// by function standards, it can not by batched and therefore it can not be an incremental function. For example the cost of calling 1000
// ray cast is 1000 times the cost of calling one ray cast. This is much different than the collision system where the cost of calculating
// collision for 1000 pairs in much, much less that the 1000 times the cost of one pair. Therefore this function must be used with care,
// as excessive use of it can degrade performance.
//
// See also: NewtonWorldConvexCast
void NewtonWorldRayCast(NewtonWorld *const newtonWorld,
                        const dFloat *const p0, const dFloat *const p1,
                        NewtonWorldRayFilterCallback filter, void *const userData,
                        NewtonWorldRayPrefilterCallback prefilter) {

	TRACE_FUNTION(__FUNCTION__);
	if (filter) {
		dgVector pp0(p0[0], p0[1], p0[2], dgFloat32(0.0f));
		dgVector pp1(p1[0], p1[1], p1[2], dgFloat32(0.0f));
		Newton *const world = (Newton *)newtonWorld;
		world->RayCast(pp0, pp1, (OnRayCastAction)filter,
		               (OnRayPrecastAction)prefilter, userData);
	}
}

// Name: NewtonWorldConvexCast
// cast a simple convex shape along the ray that goes for the matrix position to the destination and get the firsts contacts of collision.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the world.
// *const dFloat* *matrix - pointer to an array of at least three floats containing the beginning and orienetaion of the shape in global space.
// *const dFloat* *target - pointer to an array of at least three floats containing the end of the ray in global space.
// *const NewtonCollision* shape - collision shap[e use to cat the ray.
// *dFloat* hitParam - pointe to a variable the will contart the time to closet aproah to the collision.
// *void* *userData - user data to be passed to the prefilter callback.
// *NewtonWorldRayPrefilterCallback* prefilter - user define function to be called for each body before intersection.
// *NewtonWorldConvexCastReturnInfo* *info - pointer to an array of contacts at the point of intesections.
// *int* maxContactsCount - maximun number of contacts to be conclaculated, the variable sould be initialized to the capaciaty of *info*
// *int* threadIndex -  thread index from whe thsi function is called, zero if call form outsize a newton update
//
// Return: the number of contact at the intesection point (a value equal o lower than maxContactsCount.
// variable *hitParam* will be set the uintesation parameter an the momen of impact.
//
// Remarks: passing and value of NULL in *info* an dzero in maxContactsCount will turn thos function into a spcial Ray cast
// where the function will only calculate the *hitParam* at the momenet of contacts. tshi si one of the most effiecnet way to use thsio function.
//
// Remarks: these function is similar to *NewtonWorldRayCast* but instead of casting a point it cast a simple convex shape along a ray for maoprix.m_poit
// to target position. the shape is global orientation and position is set to matrix and then is swept along the segment to target and it will stop at the very first intersession contact.
//
// Remarks: for case where the application need to cast solid short to medium rays, it is better to use this function instead of casting and array of parallel rays segments.
// examples of these are: implementation of ray cast cars with cylindrical tires, foot placement of character controllers, kinematic motion of objects, user controlled continue collision, etc.
// this function may not be as efficient as sampling ray for long segment, for these cases try using parallel ray cast.
//
// Remarks: The most common use for the ray cast function is the closest body hit, In this case it is important, for performance reasons,
// that the filter function returns the intersection parameter. If the filter function returns a value of zero the ray cast will terminate
// immediately.
//
// Remarks: if prefilter is not NULL, Newton will call the application right before executing the intersections between the ray and the primitive.
// if the function returns zero the Newton will not ray cast the primitive.
// The application can use this callback to implement faster or smarter filters when implementing complex logic, otherwise for normal all ray cast
// this parameter could be NULL.
//
// See also: NewtonWorldRayCast
int NewtonWorldConvexCast(NewtonWorld *const newtonWorld,
                          const dFloat *const matrix, const dFloat *const target,
                          NewtonCollision *const shape, dFloat *const hitParam,
                          void *const userData, NewtonWorldRayPrefilterCallback prefilter,
                          NewtonWorldConvexCastReturnInfo *const info, int maxContactsCount,
                          int threadIndex) {
	TRACE_FUNTION(__FUNCTION__);
	dgVector destination(target[0], target[1], target[2], dgFloat32(0.0f));
	Newton *const world = (Newton *)newtonWorld;
	return world->ConvexCast((dgCollision *)shape, *((const dgMatrix *)matrix),
	                         destination, *(dgFloat32 *)hitParam, (OnRayPrecastAction)prefilter,
	                         userData, (dgConvexCastReturnInfo *)info, maxContactsCount, threadIndex);
}

// Name: NewtonIslandGetBody
// Get the body indexed by bodyIndex form and island.
//
// Parameters:
// *const void* *island - is the pointer to current island
// *int* bodyIndex - index to the body in current island
//
// Return: body at location bodtIndex.
//
// Remarks: This function can only be called from an island update callback.
//
// Remarks: The application can set a function callback to be called just after the array of all bodies making an island of connected bodies are collected.
// This function will be called just before the array is accepted for solution and integration.
// The function callback may return one to validate the array of zero to freeze it.
// This functionality can be used by the application to implement in game physics LOD. For example the application can determine the AABB of the
// island and check against the view frustum, if the entire island AABB is invisible then the application can suspend simulation even if they are not in equilibrium.
// another functionality is the implementation of visual debuggers, and also the implementation of auto frozen bodies under arbitrary condition set by the logic of the application.
//
// Remarks: The application should not modify any parameter of the origin body when the callback is called, nor it should create or destroy any body or joint. Do so will result in unpredictable malfunction.
//
// See also: NewtonSetIslandUpdateEvent
NewtonBody *NewtonIslandGetBody(const void *const island, int bodyIndex) {
	const dgWorld *world;

	TRACE_FUNTION(__FUNCTION__);
	world = *(const dgWorld * const *)island;
	return (NewtonBody *)world->GetIslandBody(island, bodyIndex);
}

// Name: NewtonIslandGetBodyAABB
// Return the AABB of the body on this island
//
// Parameters:
// *const void* *island - is the pointer to current island
// *int* bodyIndex - index to the body in current island
//
// Remarks: This function can only be called from an island update callback.
//
// Remarks: The application can set a function callback to be called just after the array of all bodies making an island of connected bodies are collected.
// This function will be called just before the array is accepted for solution and integration.
// The function callback may return one to validate the array of zero to freeze it.
// This functionality can be used by the application to implement in game physics LOD. For example the application can determine the AABB of the
// island and check against the view frustum, if the entire island AABB is invisible then the application can suspend simulation even if they are not in equilibrium.
// another functionality is the implementation of visual debuggers, and also the implementation of auto frozen bodies under arbitrary condition set by the logic of the application.
//
// Remarks: The application should not modify any parameter of the origin body when the callback is called, nor it should create or destroy any body or joint. Do so will result in unpredictable malfunction.
//
// See also: NewtonSetIslandUpdateEvent
void NewtonIslandGetBodyAABB(const void *const island, int bodyIndex,
                             dFloat *const p0, dFloat *const p1) {
	TRACE_FUNTION(__FUNCTION__);
	const dgBody *const body = (const dgBody *)NewtonIslandGetBody(island, bodyIndex);
	if (body) {
		body->GetAABB((dgVector &)*p0, (dgVector &)*p1);
	}
}

// ***************************************************************************************************************
//
// Name: GroupID interface
//
// ***************************************************************************************************************

// Name: NewtonMaterialGetDefaultGroupID
// Get the value of the default MaterialGroupID.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
//
// Return: The ID number for the default Group ID.
//
// Remarks: Group IDs can be interpreted as the nodes of a dense graph. The edges of the graph are the physics materials.
// When the Newton world is created, the default Group ID is created by the engine.
// When bodies are created the application assigns a group ID to the body.
int NewtonMaterialGetDefaultGroupID(const NewtonWorld *const newtonWorld) {
	const Newton *const world = (const Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	return int(world->GetDefualtBodyGroupID());
}

// Name: NewtonMaterialCreateGroupID
// Create a new MaterialGroupID.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
//
// Return: The ID of a new GroupID.
//
// Remarks: Group IDs can be interpreted as the nodes of a dense graph. The edges of the graph are the physics materials.
// When the Newton world is created, the default Group ID is created by the engine.
// When bodies are created the application assigns a group ID to the body.
//
// Note: The only way to destroy a Group ID after its creation is by destroying all the bodies and calling the function  *NewtonMaterialDestroyAllGroupID*.
//
// See also: NewtonMaterialDestroyAllGroupID
int NewtonMaterialCreateGroupID(NewtonWorld *const newtonWorld) {
	Newton *const world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	return int(world->CreateBodyGroupID());
}

// Name: NewtonMaterialDestroyAllGroupID
// Remove all groups ID from the Newton world.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
//
// Return: Nothing.
//
// Remarks: This function removes all groups ID from the Newton world.
// This function must be called after there are no more rigid bodies in the word.
//
// See also: NewtonDestroyAllBodies
void NewtonMaterialDestroyAllGroupID(NewtonWorld *const newtonWorld) {
	Newton *world;

	//  NEWTON_ASSERT (0);
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	world->RemoveAllGroupID();
}

// int NewtonGetActiveBodiesCount()
//{
//	NEWTON_ASSERT (0);
//	return 0;
// }

// int NewtonGetActiveConstraintsCount()
//{
//	NEWTON_ASSERT (0);
//	return 0;
// }

// ***************************************************************************************************************
//
// Name: Material setup interface
//
// ***************************************************************************************************************

// Name: NewtonMaterialSetDefaultCollidable
// Set the material interaction between two physics materials  to be collidable or non-collidable by default.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* id0 - group id0
// *int* id1 - group id1
// *int* state - state for this material: 1 = collidable; 0 = non collidable
//
// Return: Nothing.
void NewtonMaterialSetDefaultCollidable(const NewtonWorld *const newtonWorld,
                                        int id0, int id1, int state) {
	const Newton *const world = (const Newton *)newtonWorld;
	dgContactMaterial *const material = world->GetMaterial(dgUnsigned32(id0),
	                                    dgUnsigned32(id1));

	TRACE_FUNTION(__FUNCTION__);
	//  material->m_collisionEnable = state ? true : false;
	if (state) {
		material->m_flags |= dgContactMaterial::m_collisionEnable__;
	} else {
		material->m_flags &= ~dgContactMaterial::m_collisionEnable__;
	}
}

// Name: NewtonMaterialSetContinuousCollisionMode
// Set the material interaction between two physics materials to enable or disable continue collision.
// continue collision is on by defaults.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* id0 - group id0
// *int* id1 - group id1
// *int* state - state for this material: 1 = continue collision on; 0 = continue collision off, default mode is on
//
// Return: Nothing.
//
// Remarks: continue collision mode enable allow the engine to predict colliding contact on rigid bodies
// Moving at high speed of subject to strong forces.
//
// Remarks: continue collision mode does not prevent rigid bodies from inter penetration instead it prevent bodies from
// passing trough each others by extrapolating contact points when the bodies normal contact calculation determine the bodies are not colliding.
//
// Remarks: for performance reason the bodies angular velocities is only use on the broad face of the collision,
// but not on the contact calculation.
//
// Remarks: continue collision does not perform back tracking to determine time of contact, instead it extrapolate contact by incrementally
// extruding the collision geometries of the two colliding bodies along the linear velocity of the bodies during the time step,
// if during the extrusion colliding contact are found, a collision is declared and the normal contact resolution is called.
//
// Remarks: for continue collision to be active the continue collision mode must on the material pair of the colliding bodies as well as on at least one of the two colliding bodies.
//
// Remarks: Because there is penalty of about 40% to 80% depending of the shape complexity of the collision geometry, this feature is set
// off by default. It is the job of the application to determine what bodies need this feature on. Good guidelines are: very small objects,
// and bodies that move a height speed.
//
// See also: NewtonBodySetContinuousCollisionMode
void NewtonMaterialSetContinuousCollisionMode(
    const NewtonWorld *const newtonWorld, int id0, int id1, int state) {
	const Newton *const world = (const Newton *)newtonWorld;
	dgContactMaterial *const material = world->GetMaterial(dgUnsigned32(id0),
	                                    dgUnsigned32(id1));

	TRACE_FUNTION(__FUNCTION__);
	// material->m_collisionContinueCollisionEnable = state ? true : false;
	if (state) {
		material->m_flags |=
		    dgContactMaterial::m_collisionContinueCollisionEnable__;
	} else {
		material->m_flags &=
		    ~dgContactMaterial::m_collisionContinueCollisionEnable__;
	}
}

// Name: NewtonMaterialSetSurfaceThickness
// Set an imaginary thickness between the collision geometry of two colliding bodies who�s physics
// properties are defined by this material pair
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* id0 - group id0
// *int* id1 - group id1
// *dFloat* thickness - material thickness a value form 0.0 to 0.125; the default surface value is 0.0
//
// Return: Nothing.
//
// Remarks: when two bodies collide the engine resolve contact inter penetration by applying a small restoring
// velocity at each contact point. By default this restoring velocity will stop when the two contacts are
// at zero inter penetration distance. However by setting a non zero thickness the restoring velocity will
// continue separating the contacts until the distance between the two point of the collision geometry is equal
// to the surface thickness.
//
// Remark: Surfaces thickness can improve the behaviors of rolling objects on flat surfaces.
//
// Remarks: Surface thickness does not alter the performance of contact calculation.
void NewtonMaterialSetSurfaceThickness(const NewtonWorld *const newtonWorld,
                                       int id0, int id1, dFloat thickness) {

	TRACE_FUNTION(__FUNCTION__);
	const Newton *const world = (const Newton *)newtonWorld;
	dgContactMaterial *const material = world->GetMaterial(dgUnsigned32(id0),
	                                    dgUnsigned32(id1));

	material->m_penetrationPadding = GetMin(GetMax(thickness, dgFloat32(0.0)),
	                                        dgFloat32(DG_MAX_COLLISION_PADDING));
}

// Name: NewtonMaterialSetDefaultFriction
// Set the default coefficients of friction for the material interaction between two physics materials .
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* id0 - group id0
// *int* id1 - group id1
// *dFloat* staticFriction -  static friction coefficients
// *dFloat* kineticFriction - dynamic coefficient of friction
//
// Return: Nothing.
//
// Remarks: *staticFriction* and *kineticFriction* must be positive values. *kineticFriction* must be lower than *staticFriction*.
// It is recommended that *staticFriction* and *kineticFriction* be set to a value lower or equal to 1.0, however because some synthetic materials
// can have higher than one coefficient of friction Newton allows for the coefficient of friction to be as high as 2.0.
void NewtonMaterialSetDefaultFriction(const NewtonWorld *const newtonWorld,
                                      int id0, int id1, dFloat staticFriction, dFloat kineticFriction) {
	dFloat stat;
	dFloat kine;
	const Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (const Newton *)newtonWorld;
	dgContactMaterial *const material = world->GetMaterial(dgUnsigned32(id0),
	                                    dgUnsigned32(id1));

	staticFriction = dgAbsf(staticFriction);
	kineticFriction = dgAbsf(kineticFriction);

	if (material) {
		if (staticFriction >= dgFloat32(1.e-2f)) {
			stat = ClampValue(staticFriction, dFloat(0.01f), dFloat(2.0f));
			kine = ClampValue(kineticFriction, dFloat(0.01f), dFloat(2.0f));
			stat = GetMax(stat, kine);
			material->m_staticFriction0 = stat;
			material->m_staticFriction1 = stat;
			material->m_dynamicFriction0 = kine;
			material->m_dynamicFriction1 = kine;
		} else {
			// material->m_friction0Enable = false;
			// material->m_friction1Enable = false;
			material->m_flags &= ~(dgContactMaterial::m_friction0Enable__ | dgContactMaterial::m_friction1Enable__);
		}
	}
}

// Name: NewtonMaterialSetDefaultElasticity
// Set the default coefficients of restitution (elasticity) for the material interaction between two physics materials .
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* id0 - group id0
// *int* id1 - group id1
// *dFloat* elasticCoef - static friction coefficients
//
// Return: Nothing.
//
// Remarks: *elasticCoef* must be a positive value.
// It is recommended that *elasticCoef* be set to a value lower or equal to 1.0
void NewtonMaterialSetDefaultElasticity(const NewtonWorld *const newtonWorld,
                                        int id0, int id1, dFloat elasticCoef) {
	const Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (const Newton *)newtonWorld;
	dgContactMaterial *const material = world->GetMaterial(dgUnsigned32(id0),
	                                    dgUnsigned32(id1));

	material->m_restitution = ClampValue(elasticCoef, dFloat(0.01f),
	                                     dFloat(2.0f));
}

// Name: NewtonMaterialSetDefaultSoftness
// Set the default softness coefficients for the material interaction between two physics materials .
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world
// *int* id0 - group id0
// *int* id1 - group id1
// *dFloat* softnessCoef - softness coefficient
//
// Return: Nothing.
//
// Remarks: *softnessCoef* must be a positive value.
// It is recommended that *softnessCoef* be set to value lower or equal to 1.0
// A low value for *softnessCoef* will make the material soft. A typical value for *softnessCoef* is 0.15
void NewtonMaterialSetDefaultSoftness(const NewtonWorld *const newtonWorld,
                                      int id0, int id1, dFloat softnessCoef) {
	const Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (const Newton *)newtonWorld;
	dgContactMaterial *const material = world->GetMaterial(dgUnsigned32(id0),
	                                    dgUnsigned32(id1));

	material->m_softness = ClampValue(softnessCoef, dFloat(0.01f),
	                                  dFloat(dgFloat32(1.0f)));
}

// Name: NewtonMaterialSetCollisionCallback
// Set userData and the functions event handlers for the material interaction between two physics materials .
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *int* id0 - group id0.
// *int* id1 - group id1.
// *void* *userData - user data value.
// *NewtonOnAABBOverlap* aabbOverlap - address of the event function called when the AABB of tow bodyes overlap. This parameter can be NULL.
// *NewtonContactsProcess* processCallback - address of the event function called for every contact resulting from contact calculation. This parameter can be NULL.
//
// Return: Nothing.
//
// Remarks: When the AABB extend of the collision geometry of two bodies overlap, Newton collision system retrieves the material
// interaction that defines the behavior between the pair of bodies. The material interaction is collected from a database of materials,
//  indexed by the material gruopID assigned to the bodies. If the material is tagged as non collidable,
// then no action is taken and the simulation continues.
// If the material is tagged as collidable, and a *aabbOverlap* was set for this material, then the *aabbOverlap* function is called.
// If the function  *aabbOverlap* returns 0, no further action is taken for this material (this can be use to ignore the interaction under
// certain conditions). If the function  *aabbOverlap* returns 1, Newton proceeds to calculate the array of contacts for the pair of
// colliding bodies. If the function *processCallback* was set, the application receives a callback for every contact found between the
// two colliding bodies. Here the application can perform fine grain control over the behavior of the collision system. For example,
// rejecting the contact, making the contact frictionless, applying special effects to the surface etc.
// After all contacts are processed and if the function *endCallback* was set, Newton calls *endCallback*.
// Here the application can collect information gathered during the contact-processing phase and provide some feedback to the player.
// A typical use for the material callback is to play sound effects. The application passes the address of structure in the *userData* along with
// three event function callbacks. When the function *aabbOverlap* is called by Newton, the application resets a variable say *maximumImpactSpeed*.
// Then for every call to the function *processCallback*, the application compares the impact speed for this contact with the value of
// *maximumImpactSpeed*, if the value is larger, then the application stores the new value along with the position, and any other quantity desired.
// When the application receives the call to *endCallback* the application plays a 3d sound based in the position and strength of the contact.
//
// See also: NewtonMaterialAsThreadSafe
void NewtonMaterialSetCollisionCallback(const NewtonWorld *const newtonWorld,
                                        int id0, int id1, void *userData, NewtonOnAABBOverlap aabbOverlap,
                                        NewtonContactsProcess processCallback) {
	const Newton *world;

	world = (const Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	dgContactMaterial *const material = world->GetMaterial(dgUnsigned32(id0),
	                                    dgUnsigned32(id1));

	material->SetUserData(userData);
	material->SetCollisionCallback((OnAABBOverlap)aabbOverlap,
	                               (OnContactCallback)processCallback);
}

// Name: NewtonMaterialSetCollisionCallback
// Set userData and the functions event handlers for the material interaction between two physics materials .
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *int* id0 - group id0.
// *int* id1 - group id1.
// *void* *userData - user data value.
// *NewtonOnAABBOverlap* aabbOverlap - address of the event function called before contact calculation for collision. This parameter can be NULL.
// *NewtonContactsProcess* processCallback - address of the event function called for every contact resulting from contact calculation. This parameter can be NULL.
// *NewtonContactEnd* endCallback - address of the event function called after all contacts are processed. This parameter can be NULL.
//
// Return: Nothing.
//
// Remarks: When the AABB extend of the collision geometry of two bodies overlap, Newton collision system retrieves the material
// interaction that defines the behavior between the pair of bodies. The material interaction is collected from a database of materials,
//  indexed by the material gruopID assigned to the bodies. If the material is tagged as non collidable,
// then no action is taken and the simulation continues.
// If the material is tagged as collidable, and a *aabbOverlap* was set for this material, then the *aabbOverlap* function is called.
// If the function  *aabbOverlap* returns 0, no further action is taken for this material (this can be use to ignore the interaction under
// certain conditions). If the function  *aabbOverlap* returns 1, Newton proceeds to calculate the array of contacts for the pair of
// colliding bodies. If the function *processCallback* was set, the application receives a callback for every contact found between the
// two colliding bodies. Here the application can perform fine grain control over the behavior of the collision system. For example,
// rejecting the contact, making the contact frictionless, applying special effects to the surface etc.
// After all contacts are processed and if the function *endCallback* was set, Newton calls *endCallback*.
// Here the application can collect information gathered during the contact-processing phase and provide some feedback to the player.
// A typical use for the material callback is to play sound effects. The application passes the address of structure in the *userData* along with
// three event function callbacks. When the function *aabbOverlap* is called by Newton, the application resets a variable say *maximumImpactSpeed*.
// Then for every call to the function *processCallback*, the application compares the impact speed for this contact with the value of
// *maximumImpactSpeed*, if the value is larger, then the application stores the new value along with the position, and any other quantity desired.
// When the application receives the call to *endCallback* the application plays a 3d sound based in the position and strength of the contact.
void NewtonMaterialSetCompondCollisionCallback(
    const NewtonWorld *const newtonWorld, int id0, int id1,
    NewtonOnAABBOverlap aabbOverlap) {
	const Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (const Newton *)newtonWorld;
	dgContactMaterial *const material = world->GetMaterial(dgUnsigned32(id0),
	                                    dgUnsigned32(id1));

	material->SetCompoundCollisionCallback((OnAABBOverlap)aabbOverlap);
}

// Name: NewtonMaterialGetUserData
// Get userData associated with this material.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *int* id0 - group id0.
// *int* id1 - group id1.
//
// Return: Nothing.
void *NewtonMaterialGetUserData(const NewtonWorld *const newtonWorld, int id0,
                                int id1) {
	const Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (const Newton *)newtonWorld;
	dgContactMaterial *const material = world->GetMaterial(dgUnsigned32(id0),
	                                    dgUnsigned32(id1));

	return material->GetUserData();
}

// Name: NewtonWorldGetFirstMaterial
// Get the first Material pair from the material array.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Return: the first material.
//
// See also: NewtonWorldGetNextMaterial
NewtonMaterial *NewtonWorldGetFirstMaterial(
    const NewtonWorld *const newtonWorld) {
	const Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (const Newton *)newtonWorld;
	return (NewtonMaterial *)world->GetFirstMaterial();
}

// Name: NewtonWorldGetNextMaterial
// Get the next Material pair from the material array.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonMaterial* *material - corrent material
//
// Return: next material in material array or NULL if material is the last material in the list.
//
// See also: NewtonWorldGetFirstMaterial
NewtonMaterial *NewtonWorldGetNextMaterial(const NewtonWorld *const newtonWorld,
        NewtonMaterial *const material) {
	const Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (const Newton *)newtonWorld;

	return (NewtonMaterial *)world->GetNextMaterial((dgContactMaterial *)material);
}

// ***************************************************************************************************************
//
// Name: Contact behavior control interface
//
// ***************************************************************************************************************
/*
 // Name: NewtonMaterialDisableContact
 // Disable processing for the contact.
 //
 // Parameters:
 // *const NewtonMaterial* materialHandle - pointer to a material pair
 //
 // Return: Nothing.
 //
 // Remarks: This function can only be called from a material callback event handler.
 //
 // See also: NewtonMaterialSetCollisionCallback
 void NewtonMaterialDisableContact(const NewtonMaterial* const materialHandle)
 {
 material = (dgContactMaterial*) materialHandle;
 dgContactMaterial* const material->m_collisionEnable = false;
 }
 */

// Name: NewtonMaterialGetMaterialPairUserData
// Get the userData set by the application when it created this material pair.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair
//
// Return: Application user data.
//
// Remarks: This function can only be called from a material callback event handler.
//
// See also: NewtonMaterialSetCollisionCallback
void *NewtonMaterialGetMaterialPairUserData(
    const NewtonMaterial *const materialHandle) {
	TRACE_FUNTION(__FUNCTION__);
	const dgContactMaterial *const material = (const dgContactMaterial *)materialHandle;
	return material->GetUserData();
}

// Name: NewtonMaterialGetContactFaceAttribute
// Return the face attribute assigned to this face when for a user defined collision or a Newton collision tree.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair
//
// Return: face attribute for collision trees. Zero if the contact was generated by two convex collisions.
//
// Remarks: This function can only be called from a material callback event handler.
//
// Remarks: this function can be used by the application to retrieve the face id of a polygon for a collision tree.
//
// See also: NewtonMaterialSetCollisionCallback
unsigned NewtonMaterialGetContactFaceAttribute(
    const NewtonMaterial *const materialHandle) {
	TRACE_FUNTION(__FUNCTION__);
	const dgContactMaterial *const material = (const dgContactMaterial *)materialHandle;
	return (unsigned)material->m_userId;
}

/*
 // Name: NewtonMaterialGetCurrentTimestep
 // Get the current time step.
 //
 // Parameters:
 // *const NewtonMaterial* materialHandle - pointer to a material pair
 //
 // Return: the current time step.
 //
 // Remarks: This function can only be called from a material callback event handler. The function can be useful for the implementation of powered contacts.
 //
 // See also: NewtonMaterialSetCollisionCallback
 dFloat NewtonMaterialGetCurrentTimestep(const NewtonMaterial* const materialHandle)
 {
 dgContactMaterial* const material = (dgContactMaterial*) materialHandle;
 // return material->m_currTimestep;

 NEWTON_ASSERT (material->m_body0);
 return material->m_body0->GetWorld()->GetTimeStep();
 }
 */

// Name: NewtonMaterialGetContactNormalSpeed
// Calculate the speed of this contact along the normal vector of the contact.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair
//
// Return: Contact speed. A positive value means the contact is repulsive.
//
// Remarks: This function can only be called from a material callback event handler.
//
// See also: NewtonMaterialSetCollisionCallback
dFloat NewtonMaterialGetContactNormalSpeed(
    const NewtonMaterial *const materialHandle) {
	//  NEWTON_ASSERT (0);

	TRACE_FUNTION(__FUNCTION__);
	const dgContactMaterial *const material = (const dgContactMaterial *)materialHandle;
	//  contact = (dgContact*) contactlHandle;

	dgBody *const body0 = material->m_body0;
	dgBody *const body1 = material->m_body1;

	dgVector p0(material->m_point - body0->GetPosition());
	dgVector p1(material->m_point - body1->GetPosition());

	dgVector v0(body0->GetVelocity() + body0->GetOmega() * p0);
	dgVector v1(body1->GetVelocity() + body1->GetOmega() * p1);

	dgVector dv(v1 - v0);

	dFloat speed = dv % material->m_normal;
	return speed;
}

// Name: NewtonMaterialGetContactTangentSpeed
// Calculate the speed of this contact along the tangent vector of the contact.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *int* index - index to the tangent vector. This value can be 0 for primary tangent direction or 1 for the secondary tangent direction.
//
// Return: Contact tangent speed.
//
// Remarks: This function can only be called from a material callback event handler.
//
// See also: NewtonMaterialSetCollisionCallback
dFloat NewtonMaterialGetContactTangentSpeed(
    const NewtonMaterial *const materialHandle, int index) {
	//  NEWTON_ASSERT (0);

	TRACE_FUNTION(__FUNCTION__);
	const dgContactMaterial *const material = (const dgContactMaterial *)materialHandle;
	//  contact = (dgContact*) contactlHandle;

	dgBody *const body0 = material->m_body0;
	dgBody *const body1 = material->m_body1;

	dgVector p0(material->m_point - body0->GetPosition());
	dgVector p1(material->m_point - body1->GetPosition());

	dgVector v0(body0->GetVelocity() + body0->GetOmega() * p0);
	dgVector v1(body1->GetVelocity() + body1->GetOmega() * p1);

	dgVector dv(v1 - v0);

	dgVector dir;
	if (index) {
		dir = material->m_dir1;
	} else {
		dir = material->m_dir0;
	}
	dFloat speed = dv % dir;
	return -speed;
}

// Name: NewtonMaterialGetContactPositionAndNormal
// Get the contact position and normal in global space.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *dFloat* *positPtr   - pointer to an array of at least three floats to hold the contact position.
// *dFloat* *normalPtr  - pointer to an array of at least three floats to hold the contact normal.
//
// Return: Nothing.
//
// Remarks: This function can only be called from a material callback event handle.
//
// See also: NewtonMaterialSetCollisionCallback
void NewtonMaterialGetContactPositionAndNormal(
    const NewtonMaterial *const materialHandle, NewtonBody *const body,
    dFloat *const positPtr, dFloat *const normalPtr) {
	TRACE_FUNTION(__FUNCTION__);
	const dgContactMaterial *const material = (const dgContactMaterial *)materialHandle;

	positPtr[0] = material->m_point.m_x;
	positPtr[1] = material->m_point.m_y;
	positPtr[2] = material->m_point.m_z;

	normalPtr[0] = material->m_normal.m_x;
	normalPtr[1] = material->m_normal.m_y;
	normalPtr[2] = material->m_normal.m_z;

	if ((dgBody *)body != material->m_body0) {
		normalPtr[0] *= dgFloat32(-1.0f);
		normalPtr[1] *= dgFloat32(-1.0f);
		normalPtr[2] *= dgFloat32(-1.0f);
	}
}

// Name: NewtonMaterialGetContactForce
// Get the contact force vector in global space.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *dFloat* *forcePtr  - pointer to an array of at least three floats to hold the force vector in global space.
//
// Return: Nothing.
//
// Remarks: The contact force value is only valid when calculating resting contacts. This means if two bodies collide with
// non zero relative velocity, the reaction force will be an impulse, which is not a reaction force, this will return zero vector.
// this function will only return meaningful values when the colliding bodies are at rest.
//
// Remarks: This function can only be called from a material callback event handler.
//
// See also: NewtonMaterialSetCollisionCallback
void NewtonMaterialGetContactForce(const NewtonMaterial *const materialHandle,
                                   NewtonBody *const body, dFloat *const forcePtr) {

	TRACE_FUNTION(__FUNCTION__);
	const dgContactMaterial *const material = (const dgContactMaterial *)materialHandle;

	dgVector force(
	    material->m_normal.Scale(material->m_normal_Force) + material->m_dir0.Scale(material->m_dir0_Force) + material->m_dir1.Scale(material->m_dir1_Force));

	forcePtr[0] = force.m_x;
	forcePtr[1] = force.m_y;
	forcePtr[2] = force.m_z;

	if ((dgBody *)body != material->m_body0) {
		forcePtr[0] *= dgFloat32(-1.0f);
		forcePtr[1] *= dgFloat32(-1.0f);
		forcePtr[2] *= dgFloat32(-1.0f);
	}
}

// Name: NewtonMaterialGetContactTangentDirections
// Get the contact tangent vector to the contact point.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *dFloat* *dir0  - pointer to an array of at least three floats to hold the contact primary tangent vector.
// *dFloat* *dir1  - pointer to an array of at least three floats to hold the contact secondary tangent vector.
//
// Return: Nothing.
//
// Remarks: This function can only be called from a material callback event handler.
//
// See also: NewtonMaterialSetCollisionCallback
void NewtonMaterialGetContactTangentDirections(
    const NewtonMaterial *const materialHandle, NewtonBody *const body,
    dFloat *const dir0, dFloat *const dir1) {
	TRACE_FUNTION(__FUNCTION__);
	const dgContactMaterial *const material = (const dgContactMaterial *)materialHandle;

	dir0[0] = material->m_dir0.m_x;
	dir0[1] = material->m_dir0.m_y;
	dir0[2] = material->m_dir0.m_z;

	dir1[0] = material->m_dir1.m_x;
	dir1[1] = material->m_dir1.m_y;
	dir1[2] = material->m_dir1.m_z;

	if ((dgBody *)body != material->m_body0) {
		dir0[0] *= dgFloat32(-1.0f);
		dir0[1] *= dgFloat32(-1.0f);
		dir0[2] *= dgFloat32(-1.0f);

		dir1[0] *= dgFloat32(-1.0f);
		dir1[1] *= dgFloat32(-1.0f);
		dir1[2] *= dgFloat32(-1.0f);
	}
}

// Name: NewtonMaterialGetBodyCollisionID
// Retrieve a user defined value stored with a convex collision primitive.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *const NewtonBody* *bodyPtr - is the pointer to the body.
//
// Return: a user defined value. Zero if not id was stored with the collision primitive.
//
// Remarks: the application can store an id with any collision primitive. This id can be used to identify what type of collision primitive generated a contact.
// This function can only be called from a contact callback,
//
// Remarks: this function can only be called from a contact process callback. If called from contact callback begin this function will crash the application.
//
// See also: NewtonCollisionSetUserID, NewtonCreateBox, NewtonCreateSphere
/*
unsigned NewtonMaterialGetBodyCollisionID(const NewtonMaterial* const material, const NewtonBody* const body)
{
  TRACE_FUNTION(__FUNCTION__);
  dgBody* const bodyPtr = (dgBody*) body;
  dgContactMaterial* const materialPtr = (dgContactMaterial*) material;

  dgCollision* collision = materialPtr->m_collision0;
    if (bodyPtr == materialPtr->m_body1) {
    collision = materialPtr->m_collision1;
  }

  return collision->SetUserDataID();
}
*/

NewtonCollision *NewtonMaterialGetBodyCollidingShape(const NewtonMaterial *const material, const NewtonBody *const body) {
	TRACE_FUNTION(__FUNCTION__);
	const dgBody *const bodyPtr = (const dgBody *)body;
	const dgContactMaterial *const materialPtr = (const dgContactMaterial *)material;

	dgCollision *collision = materialPtr->m_collision0;
	if (bodyPtr == materialPtr->m_body1) {
		collision = materialPtr->m_collision1;
	}
	return (NewtonCollision *)collision;
}

// Name: NewtonMaterialSetContactSoftness
// Override the default softness value for the contact.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *dFloat* softness  - softness value, must be positive.
//
// Return: Nothing.
//
// Remarks: This function can only be called from a material callback event handler.
//
// See also: NewtonMaterialSetCollisionCallback, NewtonMaterialSetDefaultSoftness
void NewtonMaterialSetContactSoftness(
    NewtonMaterial *const materialHandle, dFloat softness) {
	TRACE_FUNTION(__FUNCTION__);
	dgContactMaterial *const material = (dgContactMaterial *)materialHandle;
	material->m_softness = ClampValue(softness, dFloat(0.01f), dFloat(0.7f));
}

// Name: NewtonMaterialSetContactElasticity
// Override the default elasticity (coefficient of restitution) value for the contact.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *dFloat* restitution  - elasticity value, must be positive.
//
// Return: Nothing.
//
// Remarks: This function can only be called from a material callback event handler.
//
// See also: NewtonMaterialSetCollisionCallback, NewtonMaterialSetDefaultElasticity
void NewtonMaterialSetContactElasticity(
    NewtonMaterial *const materialHandle, dFloat restitution) {

	TRACE_FUNTION(__FUNCTION__);
	dgContactMaterial *const material = (dgContactMaterial *)materialHandle;

	material->m_restitution = ClampValue(restitution, dFloat(0.01f),
	                                     dFloat(2.0f));
}

// Name: NewtonMaterialSetContactFrictionState
// Enable or disable friction calculation for this contact.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *int* state* - new state. 0 makes the contact frictionless along the index tangent vector.
// *int* index - index to the tangent vector. 0 for primary tangent vector or 1 for the secondary tangent vector.
//
// Return: Nothing.
//
// Remarks: This function can only be called from a material callback event handler.
//
// See also: NewtonMaterialSetCollisionCallback
void NewtonMaterialSetContactFrictionState(
    NewtonMaterial *const materialHandle, int state, int index) {

	TRACE_FUNTION(__FUNCTION__);
	dgContactMaterial *const material = (dgContactMaterial *)materialHandle;

	if (index) {
		//      material->m_friction1Enable = state ? true : false;
		if (state) {
			material->m_flags |= dgContactMaterial::m_friction1Enable__;
		} else {
			material->m_flags &= ~dgContactMaterial::m_friction1Enable__;
		}
	} else {
		//      material->m_friction0Enable = state ? true : false;
		if (state) {
			material->m_flags |= dgContactMaterial::m_friction0Enable__;
		} else {
			material->m_flags &= ~dgContactMaterial::m_friction0Enable__;
		}
	}
}

// Name: NewtonMaterialSetContactFrictionCoef
// Override the default value of the kinetic and static coefficient of friction for this contact.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *dFloat* staticFrictionCoef - static friction coefficient. Must be positive.
// *dFloat* kineticFrictionCoef - static friction coefficient. Must be positive.
// *int* index - index to the tangent vector. 0 for primary tangent vector or 1 for the secondary tangent vector.
//
// Return: Nothing.
//
// Remarks: This function can only be called from a material callback event handler.
//
// Remarks: It is recommended that *coef* be set to a value lower or equal to 1.0, however because some synthetic materials
// can have hight than one coefficient of friction Newton allows for the coefficient of friction to be as high as 2.0.
//
// Remarks: the value *staticFrictionCoef* and *kineticFrictionCoef* will be clamped between 0.01f and 2.0.
// If the application wants to set a kinetic friction higher than the current static friction it must increase the static friction first.
//
// See also: NewtonMaterialSetCollisionCallback, NewtonMaterialSetDefaultFriction, NewtonMaterialSetContactStaticFrictionCoef
void NewtonMaterialSetContactFrictionCoef(
    NewtonMaterial *const materialHandle, dFloat staticFrictionCoef,
    dFloat kineticFrictionCoef, int index) {

	TRACE_FUNTION(__FUNCTION__);
	dgContactMaterial *const material = (dgContactMaterial *)materialHandle;

	if (staticFrictionCoef < kineticFrictionCoef) {
		staticFrictionCoef = kineticFrictionCoef;
	}

	if (index) {
		material->m_staticFriction1 = ClampValue(staticFrictionCoef, dFloat(0.01f),
		                              dFloat(2.0f));
		material->m_dynamicFriction1 = ClampValue(kineticFrictionCoef,
		                               dFloat(0.01f), dFloat(2.0f));
	} else {
		material->m_staticFriction0 = ClampValue(staticFrictionCoef, dFloat(0.01f),
		                              dFloat(2.0f));
		material->m_dynamicFriction0 = ClampValue(kineticFrictionCoef,
		                               dFloat(0.01f), dFloat(2.0f));
	}
}

// Name: NewtonMaterialSetContactNormalAcceleration
// Force the contact point to have a non-zero acceleration aligned this the contact normal.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *dFloat* accel - desired contact acceleration, Must be a positive value
//
// Return: Nothing.
//
// Remarks: This function can only be called from a material callback event handler.
//
// Remarks: This function can be used for spacial effects like implementing jump, of explosive contact in a call back.
//
// See also: NewtonMaterialSetCollisionCallback
void NewtonMaterialSetContactNormalAcceleration(
    NewtonMaterial *const materialHandle, dFloat accel) {

	TRACE_FUNTION(__FUNCTION__);
	dgContactMaterial *const material = (dgContactMaterial *)materialHandle;

	//  if (accel > dFloat (0.0f)) {
	material->m_normal_Force = accel;
	//      material->m_overrideNormalAccel = true;
	material->m_flags |= dgContactMaterial::m_overrideNormalAccel__;
	//  }
}

// Name: NewtonMaterialSetContactTangentAcceleration
// Force the contact point to have a non-zero acceleration along the surface plane.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *dFloat* accel - desired contact acceleration.
// *int* index - index to the tangent vector. 0 for primary tangent vector or 1 for the secondary tangent vector.
//
// Return: Nothing.
//
// Remarks: This function can only be called from a material callback event handler.
//
// See also: NewtonMaterialSetCollisionCallback, NewtonMaterialContactRotateTangentDirections
void NewtonMaterialSetContactTangentAcceleration(
    NewtonMaterial *const materialHandle, dFloat accel, int index) {

	TRACE_FUNTION(__FUNCTION__);
	dgContactMaterial *const material = (dgContactMaterial *)materialHandle;
	if (index) {
		material->m_dir1_Force = accel;
		//      material->m_override1Accel = true;
		material->m_flags |= dgContactMaterial::m_override1Accel__;
	} else {
		material->m_dir0_Force = accel;
		//      material->m_override0Accel = true;
		material->m_flags |= dgContactMaterial::m_override0Accel__;
	}
}

// Name: NewtonMaterialSetContactNormalDirection
// Set the new direction of the for this contact point.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *const dFloat* *direction  - pointer to an array of at least three floats holding the direction vector.
//
// Return: Nothing.
//
// Remarks: This function can only be called from a material callback event handler.
// This function changes the basis of the contact point to one where the contact normal is aligned to the new direction vector
// and the tangent direction are recalculated to be perpendicular to the new contact normal.
//
// Remarks: In 99.9% of the cases the collision system can calculates a very good contact normal.
// however this algorithm that calculate the contact normal use as criteria the normal direction
// that will resolve the inter penetration with the least amount on motion.
// There are situations however when this solution is not the best. Take for example a rolling
// ball over a tessellated floor, when the ball is over a flat polygon, the contact normal is always
// perpendicular to the floor and pass by the origin of the sphere, however when the sphere is going
// across two adjacent polygons, the contact normal is now perpendicular to the polygons edge and this does
// not guarantee they it will pass bay the origin of the sphere, but we know that the best normal is always
// the one passing by the origin of the sphere.
//
// See also: NewtonMaterialSetCollisionCallback, NewtonMaterialContactRotateTangentDirections
void NewtonMaterialSetContactNormalDirection(
    NewtonMaterial *const materialHandle, const dFloat *const direction) {
	TRACE_FUNTION(__FUNCTION__);
	dgContactMaterial *const material = (dgContactMaterial *)materialHandle;
	dgVector normal(direction[0], direction[1], direction[2], dgFloat32(0.0f));

	NEWTON_ASSERT(
	    (dgAbsf(normal % material->m_normal) - dgFloat32(1.0f)) < dgFloat32(0.01f));
	if ((normal % material->m_normal) < dgFloat32(0.0f)) {
		normal = normal.Scale(-dgFloat32(1.0f));
	}
	material->m_normal = normal;

	dgMatrix matrix(normal);
	material->m_dir1 = matrix.m_up;
	material->m_dir0 = matrix.m_right;
	//  NewtonMaterialContactRotateTangentDirections(materialHandle, &material->m_dir0[0]);
}

// Name: NewtonMaterialContactRotateTangentDirections
// Rotate the tangent direction of the contacts until the primary direction is aligned with the alignVector.
//
// Parameters:
// *const NewtonMaterial* materialHandle - pointer to a material pair.
// *const dFloat* *alignVector  - pointer to an array of at least three floats holding the aligning vector.
//
// Return: Nothing.
//
// Remarks: This function can only be called from a material callback event handler.
// This function rotates the tangent vectors of the contact point until the primary tangent vector and the align vector
// are perpendicular (ex. when the dot product between the primary tangent vector and the alignVector is 1.0). This
// function can be used in conjunction with NewtonMaterialSetContactTangentAcceleration in order to
// create special effects. For example, conveyor belts, cheap low LOD vehicles, slippery surfaces, etc.
//
// See also: NewtonMaterialSetCollisionCallback, NewtonMaterialSetContactNormalDirection
void NewtonMaterialContactRotateTangentDirections(
    NewtonMaterial *const materialHandle, const dFloat *const alignVector) {
	TRACE_FUNTION(__FUNCTION__);
	dgContactMaterial *const material = (dgContactMaterial *)materialHandle;

	const dgVector dir0(alignVector[0], alignVector[1], alignVector[2],
	                    dgFloat32(0.0f));

	dgVector dir1 = material->m_normal * dir0;
	dFloat mag2 = dir1 % dir1;
	if (mag2 > 1.0e-6f) {
		material->m_dir1 = dir1.Scale(dgRsqrt(mag2));
		material->m_dir0 = material->m_dir1 * material->m_normal;
	}
}

// **********************************************************************************************
//
// Name: Convex collision primitives interface
//
// **********************************************************************************************

// Name: NewtonCreateNull
// Create a transparent collision primitive.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Return: Pointer to the collision object.
//
// Remarks: Some times the application needs to create helper rigid bodies that will never collide with other bodies,
// for example the neck of a rag doll, or an internal part of an articulated structure. This can be done by using the material system
// but it too much work and it will increase unnecessarily the material count, and therefore the project complexity. The Null collision
// is a collision object that satisfy all this conditions without having to change the engine philosophy.
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the objects.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
// Collision primitives can be reused with more than one body. This will reduce the amount of memory used by the engine, as well
// as speed up some calculations.
//
// See also: NewtonReleaseCollision
NewtonCollision *NewtonCreateNull(NewtonWorld *const newtonWorld) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	return (NewtonCollision *)world->CreateNull();
}

// Name: NewtonCreateBox
// Create a box primitive for collision.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *dFloat* dx  - box side one x dimension.
// *dFloat* dy  - box side one y dimension.
// *dFloat* dz  - box side one z dimension.
// *const dFloat* *offsetMatrix - pointer to an array of 16 floats containing the offset matrix of the box relative to the body. If this parameter is NULL, then the primitive is centered at the origin of the body.
//
// Return: Pointer to the box
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
// Collision primitives can be reused with more than one body. This will reduce the amount of memory used be the engine, as well
// as speed up some calculations.
//
// See also: NewtonAddCollisionReference, NewtonReleaseCollision, NewtonCreateConvexHullModifier
NewtonCollision *NewtonCreateBox(NewtonWorld *const newtonWorld,
                                 dFloat dx, dFloat dy, dFloat dz, int shapeID,
                                 const dFloat *const offsetMatrix) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	dgMatrix matrix(dgGetIdentityMatrix());
	if (offsetMatrix) {
		matrix = *((const dgMatrix *)offsetMatrix);
	}
	return (NewtonCollision *)world->CreateBox(dx, dy, dz, shapeID, matrix);
}

// Name: NewtonCreateSphere
// Create a generalized ellipsoid primitive..
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *dFloat* radiusX  - sphere radius along x axis.
// *dFloat* radiusY  - sphere radius along x axis.
// *dFloat* radiusZ  - sphere radius along x axis.
// *const dFloat* *offsetMatrix - pointer to an array of 16 floats containing the offset matrix of the sphere relative to the body. If this parameter is NULL then the sphere is centered at the origin of the body.
//
// Return: Pointer to the generalized sphere.
//
// Remarks: Sphere collision are generalized ellipsoids, the application can create many different kind of objects by just playing with dimensions of the radius.
// for example to make a sphere set all tree radius to the same value, to make a ellipse of revolution just set two of the tree radius to the same value.
//
// Remarks: General ellipsoids are very good hull geometries to represent the outer shell of avatars in a game.
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
// Collision primitives can be reused with more than one body. This will reduce the amount of memory used be the engine, as well
// as speed up some calculations.
//
// See also: NewtonAddCollisionReference, NewtonReleaseCollision, NewtonCreateConvexHullModifier
NewtonCollision *NewtonCreateSphere(NewtonWorld *const newtonWorld,
                                    dFloat radiusX, dFloat radiusY, dFloat radiusZ, int shapeID,
                                    const dFloat *const offsetMatrix) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	dgMatrix matrix(dgGetIdentityMatrix());
	if (offsetMatrix) {
		matrix = *((const dgMatrix *)offsetMatrix);
	}

	radiusX = dgAbsf(radiusX);
	radiusY = dgAbsf(radiusY);
	radiusZ = dgAbsf(radiusZ);

	if ((dgAbsf(radiusX - radiusY) < 1.0e-5f) && (dgAbsf(radiusX - radiusZ) < 1.0e-5f)) {
		return (NewtonCollision *)world->CreateSphere(radiusX, shapeID, matrix);
	}

	return (NewtonCollision *)world->CreateEllipse(radiusX, radiusY, radiusZ,
	        shapeID, matrix);
}

// Name: NewtonCreateCone
// Create a cone primitive for collision.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *dFloat* radius  - cone radius at the base.
// *dFloat* height  - cone height along the x local axis from base to tip.
// *const dFloat* *offsetMatrix - pointer to an array of 16 floats containing the offset matrix of the box relative to the body. If this parameter is NULL, then the primitive is centered at the origin of the body.
//
// Return: Pointer to the box
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
// Collision primitives can be reused with more than one body. This will reduce the amount of memory used be the engine, as well
// as speed up some calculations.
//
// See also: NewtonAddCollisionReference, NewtonReleaseCollision, NewtonCreateConvexHullModifier
NewtonCollision *NewtonCreateCone(NewtonWorld *const newtonWorld,
                                  dFloat radius, dFloat height, int shapeID, const dFloat *const offsetMatrix) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	dgMatrix matrix(dgGetIdentityMatrix());
	if (offsetMatrix) {
		matrix = *((const dgMatrix *)offsetMatrix);
	}
	return (NewtonCollision *)world->CreateCone(radius, height, shapeID, matrix);
}

// Name: NewtonCreateCapsule
// Create a capsule primitive for collision.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *dFloat* radius  - capsule radius at the base.
// *dFloat* height  - capsule height along the x local axis from tip to tip.
// *const dFloat* *offsetMatrix - pointer to an array of 16 floats containing the offset matrix of the box relative to the body. If this parameter is NULL, then the primitive is centered at the origin of the body.
//
// Return: Pointer to the box
//
// Remark: the capsule height must equal of larger than the sum of the cap radius. If this is not the case the height will be clamped the 2 * radius.
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
// Collision primitives can be reused with more than one body. This will reduce the amount of memory used be the engine, as well
// as speed up some calculations.
//
// See also: NewtonAddCollisionReference, NewtonReleaseCollision, NewtonCreateConvexHullModifier
NewtonCollision *NewtonCreateCapsule(NewtonWorld *const newtonWorld,
                                     dFloat radius, dFloat height, int shapeID, const dFloat *const offsetMatrix) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	dgMatrix matrix(dgGetIdentityMatrix());
	if (offsetMatrix) {
		matrix = *((const dgMatrix *)offsetMatrix);
	}
	return (NewtonCollision *)world->CreateCapsule(radius, height, shapeID,
	        matrix);
}

// Name: NewtonCreateCylinder
// Create a cylinder primitive for collision.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *dFloat* radius  - cylinder radius at the base.
// *dFloat* height  - cylinder height along the x local axis.
// *const dFloat* *offsetMatrix - pointer to an array of 16 floats containing the offset matrix of the box relative to the body. If this parameter is NULL, then the primitive is centered at the origin of the body.
//
// Return: Pointer to the box
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
// Collision primitives can be reused with more than one body. This will reduce the amount of memory used be the engine, as well
// as speed up some calculations.
//
// See also: NewtonAddCollisionReference, NewtonReleaseCollision, NewtonCreateConvexHullModifier
NewtonCollision *NewtonCreateCylinder(NewtonWorld *const newtonWorld,
                                      dFloat radius, dFloat height, int shapeID, const dFloat *const offsetMatrix) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	dgMatrix matrix(dgGetIdentityMatrix());
	if (offsetMatrix) {
		matrix = *((const dgMatrix *)offsetMatrix);
	}
	return (NewtonCollision *)world->CreateCylinder(radius, height, shapeID,
	        matrix);
}

NewtonCollision *NewtonCreateChamferCylinder(
    const NewtonWorld *const newtonWorld, dFloat radius, dFloat height,
    const dFloat *const offsetMatrix);

// Name: NewtonCreateChamferCylinder
// Create a ChamferCylinder primitive for collision.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *dFloat* radius  - ChamferCylinder radius at the base.
// *dFloat* height  - ChamferCylinder height along the x local axis.
// *const dFloat* *offsetMatrix - pointer to an array of 16 floats containing the offset matrix of the box relative to the body. If this parameter is NULL, then the primitive is centered at the origin of the body.
//
// Return: Pointer to the box
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
// Collision primitives can be reused with more than one body. This will reduce the amount of memory used be the engine, as well
// as speed up some calculations.
//
// See also: NewtonAddCollisionReference, NewtonReleaseCollision, NewtonCreateConvexHullModifier
NewtonCollision *NewtonCreateChamferCylinder(
    NewtonWorld *const newtonWorld, dFloat radius, dFloat height,
    int shapeID, const dFloat *const offsetMatrix) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	dgMatrix matrix(dgGetIdentityMatrix());
	if (offsetMatrix) {
		matrix = *((const dgMatrix *)offsetMatrix);
	}
	return (NewtonCollision *)world->CreateChamferCylinder(radius, height,
	        shapeID, matrix);
}

// Name: NewtonCreateConvexHull
// Create a ConvexHull primitive from collision from a cloud of points.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *int* count  - number of consecutive point to follow must be at least 4.
// *const dFloat* *vertexCloud - pointer to and array of point.
// *int* strideInBytes - vertex size in bytes, must be at least 12.
// *dFloat* tolerance - tolerance value for the hull generation.
// *const dFloat* *offsetMatrix - pointer to an array of 16 floats containing the offset matrix of the box relative to the body. If this parameter is NULL, then the primitive is centered at the origin of the body.
//
// Return: Pointer to the collision mesh, NULL if the function fail to generate convex shape
//
// Remarks: Convex hulls are the solution to collision primitive that can not be easily represented by an implicit solid.
// The implicit solid primitives (spheres, cubes, cylinders, capsules, cones, etc.), have constant time complexity for contact calculation
// and are also extremely efficient on memory usage, therefore the application get perfect smooth behavior.
// However for cases where the shape is too difficult or a polygonal representation is desired, convex hulls come closest to the to the model shape.
// For example it is a mistake to model a 10000 point sphere as a convex hull when the perfect sphere is available, but it is better to represent a
// pyramid by a convex hull than with a sphere or a box.
//
// Remarks: There is not upper limit as to how many vertex the application can pass to make a hull shape,
// however for performance and memory usage concern it is the application responsibility to keep the max vertex at the possible minimum.
// The minimum number of vertex should be equal or larger than 4 and it is the application responsibility that the points are part of a solid geometry.
// Unpredictable results will occur if all points happen to be collinear or coplanar.
//
// remark: The performance of collision with convex hull proxies is sensitive to the vertex count of the hull. Since a the convex hull
// of a visual geometry is already an approximation of the mesh, for visual purpose there is not significant difference between the
// appeal of a exact hull and one close to the exact hull but with but with a smaller vertex count.
// It just happens that sometime complex meshes lead to generation of convex hulls with lots of small detail that play not
// roll of the quality of the simulation but that have a significant impact on the performance because of a large vertex count.
// For this reason the application have the option to set a *tolerance* parameter.
// *tolerance* is use to post process the final geometry in the following faction, a point on the surface of the hull can
// be remove if the distance of all of the surrounding vertex immediately adjacent to the average plane equation formed the
// faces adjacent to that point, is smaller than the tolerance. A value of zero in *tolerance* will generate an exact hull and a value langer that zero
// will generate a loosely fitting hull and it willbe faster to generate.
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
// Collision primitives can be reused with more than one body. This will reduce the amount of memory used be the engine, as well
// as speed up some calculations.
//
// See also: NewtonAddCollisionReference, NewtonReleaseCollision, NewtonCreateConvexHullModifier
NewtonCollision *NewtonCreateConvexHull(NewtonWorld *const newtonWorld,
                                        int count, const dFloat *const vertexCloud, int strideInBytes,
                                        dgFloat32 tolerance, int shapeID, const dFloat *const offsetMatrix) {
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	dgMatrix matrix(dgGetIdentityMatrix());
	if (offsetMatrix) {
		matrix = *((const dgMatrix *)offsetMatrix);
	}
	tolerance = ClampValue(tolerance, dgFloat32(0.0f), dgFloat32(0.125f));
	return (NewtonCollision *)world->CreateConvexHull(count, vertexCloud,
	        strideInBytes, tolerance, shapeID, matrix);
}

// Name: NewtonCreateConvexHullFromMesh
// Create a ConvexHull primitive from a special effect mesh.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonMesh* *mesh - special effect mesh
// *dFloat* tolerance - tolerance value for the hull generation.
//
// Return: Pointer to the collision mesh, NULL if the function fail to generate convex shape
//
// Remark: Because the in general this function is used for runtime special effect like debris and or solid particles
// it is recommended that the source mesh complexity is kept small.
//
// See also: NewtonCreateConvexHull, NewtonMeshCreate
NewtonCollision *NewtonCreateConvexHullFromMesh(
    const NewtonWorld *const newtonWorld, const NewtonMesh *const mesh,
    dFloat tolerance, int shapeID) {
	TRACE_FUNTION(__FUNCTION__);
	const dgMeshEffect *meshEffect = (const dgMeshEffect *)mesh;
	return (NewtonCollision *)meshEffect->CreateConvexCollision(tolerance,
	        shapeID);
}

// Name: NewtonCreateConvexHullModifier
// Create a collision modifier for any convex collision part.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *NewtonCollision* convexHullCollision.
//
// Return: Pointer to the collision modifier
//
// Remarks: The matrix should be arranged in row-major order.
// a collision modifier can take any type of transformation matrix, as long as the matrix can be invertible by straight
// Gaussian elimination process. Typical uses are non-uniform scaling, translation and skewing.
//
// Remarks: Collision modifier can be used by the application to achieve effects like animating collision geometry at run time,
// however care must taken as animation of a collision primitive could result in unwanted penetrations.
//
// See also: NewtonAddCollisionReference, NewtonReleaseCollision, NewtonConvexHullModifierSetMatrix, NewtonConvexHullModifierGetMatrix
NewtonCollision *NewtonCreateConvexHullModifier(
    NewtonWorld *const newtonWorld,
    NewtonCollision *const convexHullCollision, int shapeID) {
	Newton *world;
	dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;

	collision = world->CreateConvexModifier((dgCollision *)convexHullCollision);

	collision->SetUserDataID(dgUnsigned32(shapeID));

	return (NewtonCollision *)collision;
}

// Name: NewtonConvexHullModifierGetMatrix
// Get the transformation matrix of a convex hull modifier collision.
//
// Parameters:
// *const NewtonCollision* *convexHullModifier - pointer to the body.
// *dFloat* *matrixPtr - pointer to an array of 16 floats containing the global matrix of the collision modifier.
//
// Return: Nothing.
//
// Remarks: The matrix should be arranged in row-major order.
// a collision modifier can take any type of transformation matrix, as long as the matrix can be invertible by straight
// Gaussian elimination process. Typical uses are non-uniform scaling, translation and skewing.
//
// Remarks: Collision modifier can be used by the application to achieve effects like animating collision geometry at run time,
// however care must taken as animation of a collision primitive could result into unwanted penetrations.
//
// See also: NewtonCreateConvexHullModifier, NewtonConvexHullModifierSetMatrix
void NewtonConvexHullModifierGetMatrix(
    const NewtonCollision *convexHullModifier, dFloat *matrixPtr) {
	const dgCollision *collision;
	collision = (const dgCollision *)convexHullModifier;

	TRACE_FUNTION(__FUNCTION__);
	dgMatrix &matrix = (*((dgMatrix *)matrixPtr));
	matrix = collision->ModifierGetMatrix();
}

// Name: NewtonConvexHullModifierSetMatrix
// Set the transformation matrix of a convex hull modifier collision.
//
// Parameters:
// *const NewtonCollision* *convexHullModifier - pointer to the body.
// *const dFloat* *matrixPtr - pointer to an array of 16 floats containing the global matrix of the collision modifier.
//
// Return: Nothing.
//
// Remarks: The matrix should be arranged in row-major order.
// a collision modifier can take any type of transformation matrix, as long as the matrix can be invertible by straight
// Gaussian elimination process. Typical uses are non-uniform scaling, translation and skewing.
//
// Remarks: Collision modifier can be used by the application to achieve effects like animating collision geometry at run time,
// however care must taken as animation of a collision primitive could result into unwanted penetrations.
//
// See also: NewtonCreateConvexHullModifier, NewtonConvexHullModifierGetMatrix
void NewtonConvexHullModifierSetMatrix(
    NewtonCollision *convexHullModifier, const dFloat *const matrixPtr) {
	dgCollision *collision;
	collision = (dgCollision *)convexHullModifier;

	TRACE_FUNTION(__FUNCTION__);
	const dgMatrix &matrix = (*((const dgMatrix *)matrixPtr));
	collision->ModifierSetMatrix(matrix);
}

// Name: NewtonCreateCompoundCollision
// Create a container to hold an array of convex collision primitives.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *int* count - number of primitives in the array.
// *const NewtonCollision* **collisionPrimitiveArray - pointer to an array of convex collision primitives. This array must be filled with convex collision primitives before this function is called.
// *int* shapeID
//
// Return: Pointer to the compound collision.
//
// Remarks: Compound collision primitives can only be made of convex collision primitives and they can not contain compound collision. Therefore they are treated as convex primitives.
//
// Remarks: Compound collision primitives are treated as instance collision objects that can not shared by multiples rigid bodies.
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
//
// See also: NewtonReleaseCollision
NewtonCollision *NewtonCreateCompoundCollision(
    NewtonWorld *const newtonWorld, int count,
    NewtonCollision *const collisionPrimitiveArray[], int shapeID) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *world = (Newton *)newtonWorld;
	dgCollision *collision = world->CreateCollisionCompound(count,
	                         (dgCollision * const *)collisionPrimitiveArray);
	collision->SetUserDataID(dgUnsigned32(shapeID));
	return (NewtonCollision *)collision;
}

// Name: NewtonCreateCompoundCollisionFromMesh
// Create a compound collision from a concave mesh by an approximate convex partition
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonMesh* *mesh - pointed concave mesh.
// *int* maxsubShapesCount,
// *int* shapeID
// *int* subShapeId
//
//
// Return: Pointer to the compound collision.
//
// Remarks: The algorithm will separated the the original mesh into a series of sub meshes until either
// the worse concave point is smaller than the specified min concavity or the max number convex shapes is reached.
//
// Remarks: is is recommended that convex approximation are made by person with a graphics toll by physically overlaying collision primitives over the concave mesh.
// but for quit test of maybe for simple meshes and algorithm approximations can be used.
//
// Remarks: is is recommended that for best performance this function is used in an off line toll and serialize the output.
//
// Remarks: Compound collision primitives are treated as instanced collision objects that cannot be shared by multiples rigid bodies.
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
//
// See also: NewtonCreateCompoundCollision
NEWTON_API NewtonCollision *NewtonCreateCompoundCollisionFromMesh(NewtonWorld *const newtonWorld, const NewtonMesh *const convexAproximation, dFloat hullTolerance, int shapeID, int subShapeID) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;
	//  dgMeshEffect* const convexAproximation = (dgMeshEffect*) mesh;

	dgList<NewtonCollision *> list(world->dgWorld::GetAllocator());
	NewtonMesh *nextSegment = NULL;
	for (NewtonMesh *segment = NewtonMeshCreateFirstSingleSegment(convexAproximation); segment; segment = nextSegment) {
		nextSegment = NewtonMeshCreateNextSingleSegment(convexAproximation, segment);

		NewtonCollision *convexHull = NewtonCreateConvexHullFromMesh(newtonWorld, segment, 0.01f, subShapeID);
		if (convexHull) {
			list.Append(convexHull);
		}
		NewtonMeshDestroy(segment);
	}

	dgInt32 count = 0;
	dgStack<NewtonCollision *> array(list.GetCount());
	for (dgList<NewtonCollision *>::dgListNode *node = list.GetFirst(); node; node = node->GetNext()) {
		array[count] = node->GetInfo();
		count++;
	}

	NewtonCollision *const collision = NewtonCreateCompoundCollision(newtonWorld, count, &array[0], shapeID);
	for (dgInt32 i = 0; i < count; i++) {
		NewtonReleaseCollision(newtonWorld, array[i]);
	}
	return collision;
}

NEWTON_API NewtonCollision *NewtonCreateCompoundBreakable(
    NewtonWorld *const newtonWorld, int meshCount,
	NewtonMesh **const solids, const int32 *const shapeIDArray,
    const dFloat *const densities, const int32 *const internalFaceMaterial,
    int shapeID, int debriID, dFloat debriSeparationGap) {
	Newton *world;
	dgCollision *collision;

	world = (Newton *)newtonWorld;
	TRACE_FUNTION(__FUNCTION__);

	collision = world->CreateCollisionCompoundBreakable(meshCount,
	            (dgMeshEffect **)solids, shapeIDArray, densities, internalFaceMaterial,
	            debriID, debriSeparationGap);

	collision->SetUserDataID(dgUnsigned32(shapeID));
	return (NewtonCollision *)collision;
}

void NewtonCompoundBreakableResetAnchoredPieces(NewtonCollision *const compoundBreakable) {
	dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	collision = (dgCollision *)compoundBreakable;

	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		dgCollisionCompoundBreakable *compound;
		compound = (dgCollisionCompoundBreakable *)collision;
		compound->ResetAnchor();
	}
}

void NewtonCompoundBreakableSetAnchoredPieces(
    NewtonCollision *const compoundBreakable, int fixShapesCount,
    dFloat *const matrixPallete, NewtonCollision **const fixedShapesArray) {
	dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	collision = (dgCollision *)compoundBreakable;

	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		dgCollisionCompoundBreakable *compound;
		compound = (dgCollisionCompoundBreakable *)collision;
		compound->SetAnchoredParts(fixShapesCount, (dgMatrix *)matrixPallete,
		                           (dgCollision * const *)fixedShapesArray);
	}
}

NewtonbreakableComponentMesh *NewtonBreakableGetMainMesh(
    const NewtonCollision *const compoundBreakable) {
	const dgCollision *collision;
	NewtonbreakableComponentMesh *mesh;

	TRACE_FUNTION(__FUNCTION__);
	collision = (const dgCollision *)compoundBreakable;

	mesh = NULL;
	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		const dgCollisionCompoundBreakable *compound;
		compound = (const dgCollisionCompoundBreakable *)collision;
		mesh = (NewtonbreakableComponentMesh *)compound->GetMainMesh();
	}
	return mesh;
}

void NewtonBreakableBeginDelete(NewtonCollision *const compoundBreakable) {
	dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	collision = (dgCollision *)compoundBreakable;

	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		dgCollisionCompoundBreakable *compound;
		compound = (dgCollisionCompoundBreakable *)collision;
		compound->DeleteComponentBegin();
	}
}

NewtonBody *NewtonBreakableCreateDebrieBody(
    NewtonCollision *const compoundBreakable,
    NewtonbreakableComponentMesh *const component) {
	dgBody *body;
	dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	collision = (dgCollision *)compoundBreakable;

	body = NULL;
	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		dgCollisionCompoundBreakable *compound;
		compound = (dgCollisionCompoundBreakable *)collision;
		body = compound->CreateComponentBody(
		           (dgCollisionCompoundBreakable::dgDebriGraph::dgListNode *)component);
	}

	return (NewtonBody *)body;
}

void NewtonBreakableDeleteComponent(
    NewtonCollision *const compoundBreakable,
    NewtonbreakableComponentMesh *const component) {
	dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	collision = (dgCollision *)compoundBreakable;

	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		dgCollisionCompoundBreakable *compound;
		compound = (dgCollisionCompoundBreakable *)collision;
		compound->DeleteComponent(
		    (dgCollisionCompoundBreakable::dgDebriGraph::dgListNode *)component);
	}
}

void NewtonBreakableEndDelete(NewtonCollision *const compoundBreakable) {
	dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	collision = (dgCollision *)compoundBreakable;

	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		dgCollisionCompoundBreakable *compound;
		compound = (dgCollisionCompoundBreakable *)collision;
		compound->DeleteComponentEnd();
	}
}

NewtonbreakableComponentMesh *NewtonBreakableGetFirstComponent(
    const NewtonCollision *const compoundBreakable) {
	const dgCollision *collision;
	NewtonbreakableComponentMesh *mesh;

	TRACE_FUNTION(__FUNCTION__);
	collision = (const dgCollision *)compoundBreakable;

	mesh = NULL;
	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		const dgCollisionCompoundBreakable *compound;
		compound = (const dgCollisionCompoundBreakable *)collision;
		mesh = (NewtonbreakableComponentMesh *)compound->GetFirstComponentMesh();
	}
	return mesh;
}

const NewtonbreakableComponentMesh *NewtonBreakableGetNextComponent(
    const NewtonbreakableComponentMesh *const component) {
	const dgCollisionCompoundBreakable::dgDebriGraph::dgListNode *node;

	TRACE_FUNTION(__FUNCTION__);
	node = (const dgCollisionCompoundBreakable::dgDebriGraph::dgListNode *)component;
	node = node->GetNext();

	return node->GetNext() ? (const NewtonbreakableComponentMesh *)node : NULL;
}

int NewtonCompoundBreakableGetVertexCount(
    const NewtonCollision *const compoundBreakable) {
	dgInt32 count;
	const dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	collision = (const dgCollision *)compoundBreakable;

	count = 0;
	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		const dgCollisionCompoundBreakable *compound;
		compound = (const dgCollisionCompoundBreakable *)collision;
		count = compound->GetVertecCount();
	}
	return count;
}

void NewtonCompoundBreakableGetVertexStreams(
    const NewtonCollision *const compoundBreakable, int vertexStrideInByte,
    dFloat *const vertex, int normalStrideInByte, dFloat *const normal,
    int uvStrideInByte, dFloat *const uv) {
	const dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	collision = (const dgCollision *)compoundBreakable;

	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		const dgCollisionCompoundBreakable *compound;
		compound = (const dgCollisionCompoundBreakable *)collision;
		compound->GetVertexStreams(vertexStrideInByte, vertex, normalStrideInByte,
		                           normal, uvStrideInByte, uv);
	}
}

void *NewtonBreakableGetFirstSegment(
    NewtonbreakableComponentMesh *const breakableComponent) {
	dgCollisionCompoundBreakable::dgDebriGraph::dgListNode *node;
	TRACE_FUNTION(__FUNCTION__);

	node = (dgCollisionCompoundBreakable::dgDebriGraph::dgListNode *)breakableComponent;
	return node->GetInfo().m_nodeData.m_mesh->GetFirst();
}

void *NewtonBreakableGetNextSegment(const void *const segment) {
	const dgCollisionCompoundBreakable::dgMesh::dgListNode *node;
	TRACE_FUNTION(__FUNCTION__);

	node = (const dgCollisionCompoundBreakable::dgMesh::dgListNode *)segment;
	return node->GetNext();
}

int NewtonBreakableGetComponentsInRadius(
    const NewtonCollision *const compoundBreakable, const dFloat *position,
    dFloat radius, NewtonbreakableComponentMesh **const segments, int maxCount) {
	dgInt32 count;
	const dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);

	count = 0;
	collision = (const dgCollision *)compoundBreakable;
	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		const dgCollisionCompoundBreakable *compound;
		compound = (const dgCollisionCompoundBreakable *)collision;
		dgVector origin(position[0], position[1], position[2], dgFloat32(0.0f));
		count = compound->GetSegmentsInRadius(origin, radius,
		                                      (dgCollisionCompoundBreakable::dgDebriGraph::dgListNode **)segments,
		                                      maxCount);
	}

	return count;
}

int NewtonBreakableSegmentGetMaterial(void *const segment) {
	dgCollisionCompoundBreakable::dgMesh::dgListNode *node;
	TRACE_FUNTION(__FUNCTION__);

	node = (dgCollisionCompoundBreakable::dgMesh::dgListNode *)segment;
	return node->GetInfo().m_material;
}

int NewtonBreakableSegmentGetIndexCount(void *const segment) {
	dgCollisionCompoundBreakable::dgMesh::dgListNode *node;
	TRACE_FUNTION(__FUNCTION__);

	node = (dgCollisionCompoundBreakable::dgMesh::dgListNode *)segment;
	return node->GetInfo().m_faceCount * 3;
}

int NewtonBreakableSegmentGetIndexStream(
    const NewtonCollision *const compoundBreakable,
    NewtonbreakableComponentMesh *const meshOwner,
	void *const segment, int32 *const index) {
	int count;
	const dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);

	count = 0;
	collision = (const dgCollision *)compoundBreakable;
	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		const dgCollisionCompoundBreakable *compound;
		compound = (const dgCollisionCompoundBreakable *)collision;
		count = compound->GetSegmentIndexStream(
		            (dgCollisionCompoundBreakable::dgDebriGraph::dgListNode *)meshOwner,
		            (dgCollisionCompoundBreakable::dgMesh::dgListNode *)segment, index);
	}
	return count;
}

int NewtonBreakableSegmentGetIndexStreamShort(
    const NewtonCollision *const compoundBreakable,
    const NewtonbreakableComponentMesh *const meshOwner,
    void *const segment, short int *const index) {
	int count;
	const dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);

	count = 0;
	collision = (const dgCollision *)compoundBreakable;
	if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
		const dgCollisionCompoundBreakable *compound;
		compound = (const dgCollisionCompoundBreakable *)collision;
		count = compound->GetSegmentIndexStreamShort(
		            (const dgCollisionCompoundBreakable::dgDebriGraph::dgListNode *)meshOwner,
		            (dgCollisionCompoundBreakable::dgMesh::dgListNode *)segment, index);
	}
	return count;
}

// Name: NewtonCollisionSetAsTriggerVolume
// Return the trigger volume flag of this shape.
//
// Parameters:
// *const NewtonCollision* convexCollision - is the pointer to a convex collision primitive.
//
// Return: 0 if collison shape is solid, non zero is collision shspe is a trigger volume.
//
// Remarks: this function can be used to place collision triggers in the scene.
// Setting this flag is not really a neessesary to place a collision trigger however this option hint the egine that
// this particular shape is a trigger volume and no contact calculation is desired.
//
// See also: NewtonCollisionIsTriggerVolume
int NewtonCollisionIsTriggerVolume(const NewtonCollision *const convexCollision) {
	const dgCollision *collision;
	collision = (const dgCollision *)convexCollision;

	TRACE_FUNTION(__FUNCTION__);

	return collision->IsTriggerVolume() ? 1 : 0;
}

// Name: NewtonCollisionSetAsTriggerVolume
// Set a flag on a convex collision shape to indicate that no contacts should calculated for this shape.
//
// Parameters:
// *const NewtonCollision* convexCollision - is the pointer to a convex collision primitive.
// *unsigned* triggerMode - 1 set diecable contact calculation 0 enable contact calculation.
//
// Return: nothing
//
// Remarks: this function can be used to place collision triggers in the scene.
// Setting this flag is not really a nessesary to place a collision trigger however this option hint the egine that
// this particular shape is a trigger volume and no contact calculation is desired.
//
// See also: NewtonCollisionIsTriggerVolume
void NewtonCollisionSetAsTriggerVolume(NewtonCollision *convexCollision,
                                       int trigger) {
	dgCollision *collision;
	collision = (dgCollision *)convexCollision;

	TRACE_FUNTION(__FUNCTION__);

	collision->SetAsTriggerVolume(trigger ? true : false);
}

void NewtonCollisionSetMaxBreakImpactImpulse(
    NewtonCollision *const convexHullCollision, dFloat maxImpactImpulse) {
	dgCollision *collision;
	collision = (dgCollision *)convexHullCollision;

	TRACE_FUNTION(__FUNCTION__);
	collision->SetBreakImpulse(dgFloat32(maxImpactImpulse));
}

dFloat NewtonCollisionGetMaxBreakImpactImpulse(
    const NewtonCollision *const convexHullCollision) {
	const dgCollision *collision;
	collision = (const dgCollision *)convexHullCollision;

	TRACE_FUNTION(__FUNCTION__);
	return dgFloat32(collision->GetBreakImpulse());
}

// Name: NewtonCollisionSetUserID
// Store a user defined value with a convex collision primitive.
//
// Parameters:
// *const NewtonCollision* collision - is the pointer to a collision primitive.
// *unsigned* id - value to store with the collision primitive.
//
// Return: nothing
//
// Remarks: the application can store an id with any collision primitive. This id can be used to identify what type of collision primitive generated a contact.
//
// See also: NewtonMaterialGetBodyCollisionID, NewtonCollisionGetUserID, NewtonCreateBox, NewtonCreateSphere
void NewtonCollisionSetUserID(NewtonCollision *const collision,
                              unsigned id) {

	dgCollision *coll;
	coll = (dgCollision *)collision;

	TRACE_FUNTION(__FUNCTION__);
	coll->SetUserDataID(id);
}

// Name: NewtonCollisionGetUserID
// Return a user define value with a convex collision primitive.
//
// Parameters:
// *const NewtonCollision* collision - is the pointer to a convex collision primitive.
//
// Return: user id
//
// Remarks: the application can store an id with any collision primitive. This id can be used to identify what type of collision primitive generated a contact.
//
// See also: NewtonMaterialGetBodyCollisionID, NewtonMaterialGetBodyCollisionID, NewtonCreateBox, NewtonCreateSphere
unsigned NewtonCollisionGetUserID(NewtonCollision *const collision) {
	dgCollision *coll;

	TRACE_FUNTION(__FUNCTION__);
	coll = (dgCollision *)collision;
	//  //return unsigned (dgUnsigned64 (collision->GetUserData()));
	//  return unsigned (PointerToInt (collision->GetUserData()));
	return coll->SetUserDataID();
}

// Name: NewtonConvexHullGetFaceIndices
// Return the number of vertices of face and copy each index into array faceIndices.
//
// Parameters:
// *const NewtonCollision* convexHullCollision - is the pointer to a convex collision hull primitive.
//
// Return: user face count of face.
//
// Remarks: this function will return zero on all shapes other than a convex full collision shape.
//
// Remarks: To get the number of faces of a convex hull shape see function *NewtonCollisionGetInfo*
//
// See also: NewtonCollisionGetInfo, NewtonCreateConvexHull
int NewtonConvexHullGetFaceIndices(
    const NewtonCollision *const convexHullCollision, int face,
    int32 *const faceIndices) {
	const dgCollision *coll;

	TRACE_FUNTION(__FUNCTION__);
	coll = (const dgCollision *)convexHullCollision;

	if (coll->IsType(dgCollision::dgCollisionConvexHull_RTTI)) {
		return ((const dgCollisionConvexHull *)coll)->GetFaceIndices(face, faceIndices);
	} else {
		return 0;
	}
}

// Name: NewtonConvexCollisionCalculateVolume
// calculate the total volume defined by a convex collision geometry.
//
// Parameters:
// *const NewtonCollision* *convexCollision - pointer to the collision.
//
// Return: collision geometry volume. This function will return zero if the body collision geometry is no convex.
//
// Remarks: The total volume calculated by the function is only an approximation of the ideal volume. This is not an error, it is a fact resulting from the polygonal representation of convex solids.
//
// Remarks: This function can be used to assist the application in calibrating features like fluid density weigh factor when calibrating buoyancy forces for more realistic result.
//
// See also: NewtonBodyAddBuoyancyForce
dFloat NewtonConvexCollisionCalculateVolume(
    const NewtonCollision *const convexCollision) {
	const dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	collision = (const dgCollision *)convexCollision;
	return collision->GetVolume();
}

// Name: NewtonConvexCollisionCalculateInertialMatrix
// Calculate the three principal axis and the the values of the inertia matrix of a convex collision objects.
//
// Parameters:
// *const NewtonCollision* convexCollision - is the pointer to a convex collision primitive.
// *dFloat* *inertia - pointer to and array of a least 3 floats to hold the values of the principal inertia.
// *dFloat* *origin - pointer to and array of a least 3 floats to hold the values of the center of mass for the principal inertia.
//
// Remarks: This function calculate a general inertial matrix for arbitrary convex collision including compound collisions.
//
// See also: NewtonBodySetMassMatrix, NewtonBodyGetMassMatrix, NewtonBodySetCentreOfMass, NewtonBodyGetCentreOfMass
void NewtonConvexCollisionCalculateInertialMatrix(
    const NewtonCollision *convexCollision, dFloat *const inertia,
    dFloat *const origin) {
	const dgCollision *collision;
	collision = (const dgCollision *)convexCollision;

	dgVector tmpInertia;
	dgVector tmpOringin;

	TRACE_FUNTION(__FUNCTION__);
	collision->CalculateInertia(tmpInertia, tmpOringin);

	inertia[0] = tmpInertia[0];
	inertia[1] = tmpInertia[1];
	inertia[2] = tmpInertia[2];
	origin[0] = tmpOringin[0];
	origin[1] = tmpOringin[1];
	origin[2] = tmpOringin[2];
}

// **********************************************************************************************
//
// Name: Complex collision primitives interface
//
// **********************************************************************************************

// Name: NewtonCreateUserMeshCollision
// Create a complex collision geometry to be controlled by the application.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const dFloat* *minBox - pointer to an array of at least three floats to hold minimum value for the box relative to the collision.
// *const dFloat* *maxBox - pointer to an array of at least three floats to hold maximum value for the box relative to the collision.
// *void* *userData - pointer to user data to be used as context for event callback.
// *NewtonUserMeshCollisionCollideCallback* collideCallback - pointer to an event function for providing Newton with the polygon inside a given box region.
// *NewtonUserMeshCollisionRayHitCallback* rayHitCallBack   - pointer to an event function for providing Newton with ray intersection information.
// *NewtonUserMeshCollisionDestroyCallback* destroyCallback - pointer to an event function for destroying any data allocated for use by the application.
// *NewtonUserMeshCollisionGetCollisionInfo* getInfoCallback -  xxxxx
// *NewtonUserMeshCollisionGetFacesInAABB* facesInAABBCallback - xxxxxxxxxx
//
// Return: Pointer to the user collision.
//
// Remarks: *UserMeshCollision* provides the application with a method of overloading the built-in collision system for background objects.
// UserMeshCollision can be used for implementing collisions with height maps, collisions with BSP, and any other collision structure the application
// supports and wishes to preserve.
// However, *UserMeshCollision* can not take advantage of the efficient and sophisticated algorithms and data structures of the
// built-in *TreeCollision*. We suggest you experiment with both methods and use the method best suited to your situation.
//
// Remarks: When a *UserMeshCollision* is assigned to a body, the mass of the body is ignored in all dynamics calculations.
// This make the body behave as a static body.
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
// Collision primitives can be reused with more than one body. This will reduce the amount of memory used be the engine, as well
// as speed up some calculations.
//
// See also: NewtonReleaseCollision
NewtonCollision *NewtonCreateUserMeshCollision(
    NewtonWorld *const newtonWorld, const dFloat *const minBox,
    const dFloat *const maxBox, void *const userData,
    NewtonUserMeshCollisionCollideCallback collideCallback,
    NewtonUserMeshCollisionRayHitCallback rayHitCallBack,
    NewtonUserMeshCollisionDestroyCallback destroyCallback,
    NewtonUserMeshCollisionGetCollisionInfo getInfoCallback,
    NewtonUserMeshCollisionGetFacesInAABB facesInAABBCallback, int shapeID) {
	Newton *world;
	dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	dgVector p0(minBox[0], minBox[1], minBox[2], dgFloat32(1.0f));
	dgVector p1(maxBox[0], maxBox[1], maxBox[2], dgFloat32(1.0f));

	world = (Newton *)newtonWorld;

	dgUserMeshCreation data;
	data.m_userData = userData;
	data.m_collideCallback = (OnUserMeshCollideCallback)collideCallback;
	data.m_rayHitCallBack = (OnUserMeshRayHitCallback)rayHitCallBack;
	data.m_destroyCallback = (OnUserMeshDestroyCallback)destroyCallback;
	data.m_getInfo = (UserMeshCollisionInfo)getInfoCallback;
	data.m_faceInAabb = (UserMeshFacesInAABB)facesInAABBCallback;

	collision = world->CreateStaticUserMesh(p0, p1, data);
	collision->SetUserDataID(dgUnsigned32(shapeID));
	return (NewtonCollision *)collision;
}

// Name: NewtonCreateTreeCollision
// Create an empty complex collision geometry tree.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Return: Pointer to the collision tree.
//
// Remarks: *TreeCollision* is the preferred method within Newton for collision with polygonal meshes of arbitrary complexity.
// The mesh must be made of flat non-intersecting polygons, but they do not explicitly need to be triangles.
// *TreeCollision* can be serialized by the application to/from an arbitrary storage device.
//
// Remarks: When a *TreeCollision* is assigned to a body the mass of the body is ignored in all dynamics calculations.
// This makes the body behave as a static body.
//
// Remarks: Collision primitives are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
// Collision primitives can be reused with more than one body. This will reduce the amount of memory used by the engine, as well
// as speed up some calculations.
//
// See also: NewtonTreeCollisionBeginBuild, NewtonTreeCollisionAddFace, NewtonTreeCollisionEndBuild, NewtonStaticCollisionSetDebugCallback, NewtonTreeCollisionGetFaceAtribute, NewtonTreeCollisionSetFaceAtribute, NewtonReleaseCollision
NewtonCollision *NewtonCreateTreeCollision(NewtonWorld *const newtonWorld,
        int shapeID) {
	Newton *world;
	dgCollision *collision;
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	collision = world->CreateBVH();
	collision->SetUserDataID(dgUnsigned32(shapeID));
	return (NewtonCollision *)collision;
}

NewtonCollision *NewtonCreateTreeCollisionFromMesh(const NewtonWorld *const newtonWorld, const NewtonMesh *const mesh, int shapeID) {
	TRACE_FUNTION(__FUNCTION__);
	// Newton* const world = (Newton *)newtonWorld;
	const dgMeshEffect *meshEffect = (const dgMeshEffect *)mesh;
	dgCollision *const collision = meshEffect->CreateCollisionTree(shapeID);
	//  dgCollision* const collision =  world->CreateBVH ();
	//  collision->SetUserDataID(dgUnsigned32 (shapeID));
	return (NewtonCollision *)collision;
}

// Name: NewtonStaticCollisionSetDebugCallback
// set a function call back to be call during the face query of a collision tree.
//
// Parameters:
// *const NewtonCollision* *staticCollision - is the pointer to the static collision (a CollisionTree of a HeightFieldCollision)
// *NewtonTreeCollisionCallback *userCallback - pointer to an event function to call before Newton evaluates the polygons colliding with a body. This parameter can be NULL.
//
// Remarks: because debug display display report all the faces of a collision primitive, it could get slow on very large static collision.
// this function can be used for debugging purpose to just report only faces intersetion the collision AABB of the collision shape colliding with the polyginal mesh collision.
//
// Remarks: this function is not recommended to use for production code only for debug purpose.
//
// See also: NewtonTreeCollisionGetFaceAtribute, NewtonTreeCollisionSetFaceAtribute
void NewtonStaticCollisionSetDebugCallback(
    NewtonCollision *const staticCollision,
    NewtonTreeCollisionCallback userCallback) {
	/*
	 dgCollision* collision;

	 TRACE_FUNTION(__FUNCTION__);

	 collision = (dgCollision*) staticCollision;
	 if (collision->IsType(dgCollision::dgCollisionPolygonalSoup_RTTI)) {
	 dgCollisionPolygonalSoup* collisionTree;
	 collisionTree = (dgCollisionPolygonalSoup*) collision;
	 if (collisionTree->GetCallBack___() ==  NewtonCollisionTree::GetIntersectingPolygons) {
	 NewtonCollisionTree& tree = *((NewtonCollisionTree*) collisionTree->GetUserData());
	 tree.SetCollisionCallback (userCallback);

	 } else if (collisionTree->GetCallBack___() == NewtonHeightFieldCollision::GetIntersectingPolygons) {
	 NewtonHeightFieldCollision& heightField = *((NewtonHeightFieldCollision*) collisionTree->GetUserData());
	 heightField.SetCollisionCallback (userCallback);
	 }
	 }
	 */

	dgCollision *collision;
	//  dgCollisionMesh* collision;

	TRACE_FUNTION(__FUNCTION__);
	collision = (dgCollision *)staticCollision;
	if (collision->IsType(dgCollision::dgCollisionMesh_RTTI)) {
		dgCollisionMesh *mesh;
		mesh = (dgCollisionMesh *)staticCollision;
		mesh->SetCollisionCallback((dgCollisionMeshCollisionCallback)userCallback);
	} else if (collision->IsType(dgCollision::dgCollisionScene_RTTI)) {
		dgCollisionScene *scene;
		scene = (dgCollisionScene *)staticCollision;
		scene->SetCollisionCallback(
		    (dgCollisionMeshCollisionCallback)userCallback);
	}
}

// Name: NewtonTreeCollisionSetUserRayCastCallback
// set a function call back to be called during the face query of a collision tree.
//
// Parameters:
// *const NewtonCollision* *treeCollision - is the pointer to the collision tree.
// *NewtonCollisionTreeRayCastCallback *userCallback - pointer to an event function to call before Newton evaluates the polygons colliding with a body. This parameter can be NULL.
//
// Remarks: In general a ray cast on a collision tree will stops at the first interceptions with the closest face in the tree
// that was hit by the ray. In some cases the application may be interested in the intesation with faces other than the fiorst hit.
// In this cases the application can set this alternate callback and the ray scanner will notify the application of each face hit by the ray scan.
//
// Remarks: since this function faces the ray scanner to visit all of the potential faces intersected by the ray,
// setting the function call back make the ray casting on collision tree less efficient than the default behavior.
// So it is this functionality is only recommended for cases were the application is using especial effects like transparencies, or other effects
//
// Remarks: calling this function with *rayHitCallback* = NULL will rest the collision tree to it default raycast mode, which is return with the closest hit.
//
// Remarks: when *rayHitCallback* is not null then the callback is dalled with the follwing arguments
// *const NetwonCollisio* collision - pointer to the collision tree
// *dFloat* interseption - inetstion parameters of the ray
// *dFloat* *normal - unnormalized face mormal in the space fo eth parent of the collision.
// *int* faceId -  id of this face in the collision tree.
//
// See also: NewtonTreeCollisionGetFaceAtribute, NewtonTreeCollisionSetFaceAtribute
void NewtonTreeCollisionSetUserRayCastCallback(
    NewtonCollision *const treeCollision,
    NewtonCollisionTreeRayCastCallback rayHitCallback) {
	TRACE_FUNTION(__FUNCTION__);
	dgCollisionBVH *const collision = (dgCollisionBVH *)treeCollision;
	if (collision->IsType(dgCollision::dgCollisionBVH_RTTI)) {
		collision->SetCollisionRayCastCallback(
		    (dgCollisionBVHUserRayCastCallback)rayHitCallback);
	}
}

void NewtonHeightFieldSetUserRayCastCallback(
    NewtonCollision *treeCollision,
    NewtonHeightFieldRayCastCallback rayHitCallback) {
	TRACE_FUNTION(__FUNCTION__);
	dgCollisionHeightField *const collision =
	    (dgCollisionHeightField *)treeCollision;
	if (collision->IsType(dgCollision::dgCollisionHeightField_RTTI)) {
		collision->SetCollisionRayCastCallback(
		    (dgCollisionHeightFieldRayCastCallback)rayHitCallback);
	}
}

// Name: NewtonTreeCollisionBeginBuild
// Prepare a *TreeCollision* to begin to accept the polygons that comprise the collision mesh.
//
// Parameters:
// *const NewtonCollision* *treeCollision - is the pointer to the collision tree.
//
// Return: Nothing.
//
// See also: NewtonTreeCollisionAddFace, NewtonTreeCollisionEndBuild
void NewtonTreeCollisionBeginBuild(NewtonCollision *treeCollision) {

	TRACE_FUNTION(__FUNCTION__);
	dgCollisionBVH *const collision = (dgCollisionBVH *)treeCollision;
	NEWTON_ASSERT(collision->IsType(dgCollision::dgCollisionBVH_RTTI));

	collision->BeginBuild();
}

// Name: NewtonTreeCollisionAddFace
// Add an individual polygon to a *TreeCollision*.
//
// Parameters:
// *const NewtonCollision* *treeCollision - is the pointer to the collision tree.
// *int* vertexCount - number of vertex in *vertexPtr*
// *const dFloat* *vertexPtr - pointer to an array of vertex. The vertex should consist of at least 3 floats each.
// *int* strideInBytes - size of each vertex in bytes. This value should be 12 or larger.
// *int* faceAttribute - id that identifies the polygon. The application can use this value to customize the behavior of the collision geometry.
//
// Return: Nothing.
//
// Remarks: After the call to *NewtonTreeCollisionBeginBuild* the *TreeCollision* is ready to accept polygons. The application should iterate
// through the application's mesh, adding the mesh polygons to the *TreeCollision* one at a time.
// The polygons must be flat and non-self intersecting.
//
// See also: NewtonTreeCollisionAddFace, NewtonTreeCollisionEndBuild
void NewtonTreeCollisionAddFace(NewtonCollision *const treeCollision,
                                int vertexCount, const dFloat *const vertexPtr, int strideInBytes,
                                int faceAttribute) {
	TRACE_FUNTION(__FUNCTION__);
	dgCollisionBVH *const collision = (dgCollisionBVH *)treeCollision;
	NEWTON_ASSERT(collision->IsType(dgCollision::dgCollisionBVH_RTTI));
	collision->AddFace(vertexCount, vertexPtr, strideInBytes, faceAttribute);
}

// Name: NewtonTreeCollisionEndBuild
// Finalize the construction of the polygonal mesh.
//
// Parameters:
// *const NewtonCollision* *treeCollision - is the pointer to the collision tree.
// *int* optimize - flag that indicates to Newton whether it should optimize this mesh. Set to 1 to optimize the mesh, otherwise 0.
//
// Return: Nothing.
//
//
// Remarks: After the application has finished adding polygons to the *TreeCollision*, it must call this function to finalize the construction of the collision mesh.
// If concave polygons are added to the *TreeCollision*, the application must call this function with the parameter *optimize* set to 1.
// With the *optimize* parameter set to 1, Newton will optimize the collision mesh by removing non essential edges from adjacent flat polygons.
// Newton will not change the topology of the mesh but significantly reduces the number of polygons in the mesh. The reduction factor of the number of polygons in the mesh depends upon the irregularity of the mesh topology.
// A reduction factor of 1.5 to 2.0 is common.
// Calling this function with the parameter *optimize* set to zero, will leave the mesh geometry unaltered.
//
// See also: NewtonTreeCollisionAddFace, NewtonTreeCollisionEndBuild
void NewtonTreeCollisionEndBuild(NewtonCollision *const treeCollision,
                                 int optimize) {
	TRACE_FUNTION(__FUNCTION__);
	dgCollisionBVH *const collision = (dgCollisionBVH *)treeCollision;
	NEWTON_ASSERT(collision->IsType(dgCollision::dgCollisionBVH_RTTI));
	collision->EndBuild(optimize);
}

// Name: NewtonTreeCollisionGetFaceAtribute
// Get the user defined collision attributes stored with each face of the collision mesh.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const int* *faceIndexArray - pointer to the face index list passed to the function *NewtonTreeCollisionCallback userCallback*
//
// Return: User id of the face.
//
// Remarks: This function is used to obtain the user data stored in faces of the collision geometry.
// The application can use this user data to achieve per polygon material behavior in large static collision meshes.
//
// See also: NewtonTreeCollisionSetFaceAtribute, NewtonCreateTreeCollision, NewtonCreateTreeCollisionFromSerialization
int NewtonTreeCollisionGetFaceAtribute(
    const NewtonCollision *const treeCollision, const int32 *const faceIndexArray) {
	TRACE_FUNTION(__FUNCTION__);
	const dgCollisionBVH *const collision = (const dgCollisionBVH *)treeCollision;
	NEWTON_ASSERT(collision->IsType(dgCollision::dgCollisionBVH_RTTI));

	return int(collision->GetTagId(faceIndexArray));
}

// Name: NewtonTreeCollisionSetFaceAtribute
// Change the user defined collision attribute stored with faces of the collision mesh.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const int* *faceIndexArray - pointer to the face index list passed to the function *NewtonTreeCollisionCallback userCallback*
// *int* attribute - value of the user defined attribute to be stored with the face.
//
// Return: User id of the face.
//
// Remarks: This function is used to obtain the user data stored in faces of the collision geometry.
// The application can use this user data to achieve per polygon material behavior in large static collision meshes.
// By changing the value of this user data the application can achieve modifiable surface behavior with the collision geometry.
// For example, in a driving game, the surface of a polygon that represents the street can changed from pavement to oily or wet after
// some collision event occurs.
//
// See also: NewtonTreeCollisionGetFaceAtribute, NewtonCreateTreeCollision, NewtonCreateTreeCollisionFromSerialization
void NewtonTreeCollisionSetFaceAtribute(
    NewtonCollision *const treeCollision, const int32 *const faceIndexArray,
    int attribute) {
	TRACE_FUNTION(__FUNCTION__);
	dgCollisionBVH *const collision = (dgCollisionBVH *)treeCollision;
	NEWTON_ASSERT(collision->IsType(dgCollision::dgCollisionBVH_RTTI));

	collision->SetTagId(faceIndexArray, dgUnsigned32(attribute));
}

// Name: NewtonTreeCollisionGetVertexListIndexListInAABB
// collect the vertex list index list mesh intersecting the AABB in collision mesh.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *dFloat* *p0 - pointer to an array of at least three floats representing the ray origin in the local space of the geometry.
// *dFloat* *p1 - pointer to an array of at least three floats representing the ray end in the local space of the geometry.
// *const dFloat* **vertexArray - pointer to a the vertex array of vertex.
// *int* *vertexCount - pointer int to return the number of vertex in vertexArray.
// *int* *vertexStrideInBytes - pointer to int to return the size of each vertex in vertexArray.
// *const int* *indexList - pointer to array on integers containing the triangles intersection the aabb.
// *const int* maxIndexCount - maximum number of indices the function will copy to indexList.
// *const int* *faceAttribute - pointer to array on integers top contain the face containing the .
//
// Return: the number of triangles in indexList.
//
// Remarks: indexList should be a list 3 * maxIndexCount the number of elements.
//
// Remarks: faceAttributet should be a list maxIndexCount the number of elements.
//
// Remarks: this function could be used by the application for many purposes.
// for example it can be used to draw the collision geometry intersecting a collision primitive instead
// of drawing the entire collision tree in debug mode.
// Another use for this function is to to efficient draw projective texture shadows.
int NewtonTreeCollisionGetVertexListIndexListInAABB(
    const NewtonCollision *const treeCollision, const dFloat *const p0,
    const dFloat *const p1, const dFloat **const vertexArray,
    int32 *const vertexCount, int *const vertexStrideInBytes,
    int32 *const indexList, int maxIndexCount,
    int32 *const faceAttribute) {
	dgInt32 count;
	const dgCollision *meshColl;

	count = 0;

	TRACE_FUNTION(__FUNCTION__);

	meshColl = (const dgCollision *)treeCollision;

	if (meshColl->IsType(dgCollision::dgCollisionMesh_RTTI)) {
		const dgCollisionMesh *collision;
		collision = (const dgCollisionMesh *)meshColl;

		// NewtonCollisionTree& tree = *((NewtonCollisionTree*) collisionTree->GetUserData());
		dgVector pmin(p0[0], p0[1], p0[2], dgFloat32(0.0f));
		dgVector pmax(p1[0], p1[1], p1[2], dgFloat32(0.0f));

		dgCollisionMesh::dgGetVertexListIndexList data;
		data.m_indexList = (dgInt32 *)indexList;
		data.m_userDataList = (dgInt32 *)faceAttribute;
		data.m_maxIndexCount = maxIndexCount;
		data.m_triangleCount = 0;
		collision->GetVertexListIndexList(pmin, pmax, data);

		count = data.m_triangleCount;
		*vertexArray = data.m_veterxArray;
		*vertexCount = data.m_vertexCount;
		*vertexStrideInBytes = data.m_vertexStrideInBytes;
	}

	return count;
}

// Name: NewtonCreateHeightFieldCollision
// Create a height field collision geometry.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *int* width -
// *int* height -
// *int* cellsDiagonals -
// *unsigned short* elevationMap -
// *char* atributeMap -
// *dFloat* horizontalScale -
// *dFloat* verticalScale -
//
// Return: Pointer to the collision.
//
// Remarks:
// Remarks:
// Remarks:
//
// See also: NewtonCreateTreeCollision, NewtonReleaseCollision
NewtonCollision *NewtonCreateHeightFieldCollision(
    NewtonWorld *const newtonWorld, int width, int height,
    int cellsDiagonals, const unsigned short *const elevationMap,
    const int8 *const atributeMap, dFloat horizontalScale, dFloat verticalScale,
    int shapeID) {
	Newton *world;
	dgCollision *collision;
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);

	collision = world->CreateBVHFieldCollision(width, height, cellsDiagonals,
	            elevationMap, atributeMap, horizontalScale, verticalScale);
	collision->SetUserDataID(dgUnsigned32(shapeID));
	return (NewtonCollision *)collision;
}

// Name: NewtonCreateSceneCollision
// Create a height field collision geometry.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Return: Pointer to the collision.
//
// Remarks:
// Remarks:
// Remarks:
//
// See also: NewtonCreateTreeCollision, NewtonReleaseCollision
NewtonCollision *NewtonCreateSceneCollision(NewtonWorld *const newtonWorld, int shapeID) {
	Newton *world;
	dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;

	collision = world->CreateScene();

	collision->SetUserDataID(dgUnsigned32(shapeID));
	return (NewtonCollision *)collision;
}

NewtonSceneProxy *NewtonSceneCollisionCreateProxy(NewtonCollision *const scene,
        NewtonCollision *const collision, dFloat *const matrixPtr) {
	dgMatrix matrix(*((dgMatrix *)matrixPtr));
	matrix.m_front.m_w = dgFloat32(0.0f);
	matrix.m_up.m_w = dgFloat32(0.0f);
	matrix.m_right.m_w = dgFloat32(0.0f);
	matrix.m_posit.m_w = dgFloat32(1.0f);

	dgCollisionScene *const newtonScene = (dgCollisionScene *)scene;
	NEWTON_ASSERT(newtonScene->IsType(dgCollision::dgCollisionScene_RTTI));
	return (NewtonSceneProxy *)newtonScene->AddProxy((dgCollision *)collision,
	        matrix);
}

void NewtonSceneCollisionDestroyProxy(NewtonCollision *const scene,
                                      NewtonSceneProxy *const proxy) {
	dgCollisionScene *const newtonScene = (dgCollisionScene *)scene;
	NEWTON_ASSERT(newtonScene->IsType(dgCollision::dgCollisionScene_RTTI));
	newtonScene->RemoveProxy(proxy);
}

void NewtonSceneProxySetMatrix(NewtonSceneProxy *const proxy,
                               const dFloat *const matrix) {
	dgList<dgCollisionScene::dgProxy *>::dgListNode *const node = (dgList <
	        dgCollisionScene::dgProxy * >::dgListNode *)proxy;
	dgCollisionScene *const newtonScene = node->GetInfo()->m_owner;
	const dgMatrix &offset = *((const dgMatrix *)matrix);
	newtonScene->SetProxyMatrix(node, offset);
}

void NewtonSceneProxyGetMatrix(NewtonSceneProxy *const proxy,
                               dFloat *const matrix) {
	dgList<dgCollisionScene::dgProxy *>::dgListNode *const node = (dgList <
	        dgCollisionScene::dgProxy * >::dgListNode *)proxy;
	dgCollisionScene *const newtonScene = node->GetInfo()->m_owner;

	dgMatrix &offset = *((dgMatrix *)matrix);
	offset = newtonScene->GetProxyMatrix(node);
}

void NewtonSceneSetProxyUserData(NewtonSceneProxy *const proxy, void *userData) {
	dgList<dgCollisionScene::dgProxy *>::dgListNode *const node = (dgList <
	        dgCollisionScene::dgProxy * >::dgListNode *)proxy;
	dgCollisionScene *const newtonScene = node->GetInfo()->m_owner;

	newtonScene->SetProxyUserData(node, userData);
}

void *NewtonSceneGetProxyUserData(NewtonSceneProxy *const proxy) {
	dgList<dgCollisionScene::dgProxy *>::dgListNode *const node = (dgList <
	        dgCollisionScene::dgProxy * >::dgListNode *)proxy;
	dgCollisionScene *const newtonScene = node->GetInfo()->m_owner;

	return newtonScene->GetProxyUserData(node);
}

NewtonSceneProxy *NewtonSceneGetFirstProxy(NewtonCollision *const scene) {
	dgCollisionScene *const newtonScene = (dgCollisionScene *)scene;
	return (NewtonSceneProxy *)newtonScene->GetFirstProxy();
}

NewtonSceneProxy *NewtonSceneGetNextProxy(NewtonCollision *const scene,
        NewtonSceneProxy *const proxy) {
	dgList<dgCollisionScene::dgProxy *>::dgListNode *const node = (dgList <
	        dgCollisionScene::dgProxy * >::dgListNode *)proxy;
	dgCollisionScene *const newtonScene = node->GetInfo()->m_owner;

	return (NewtonSceneProxy *)newtonScene->GetNextProxy(proxy);
}

void NewtonSceneCollisionOptimize(NewtonCollision *const scene) {
	dgCollisionScene *const newtonScene = (dgCollisionScene *)scene;
	NEWTON_ASSERT(newtonScene->IsType(dgCollision::dgCollisionScene_RTTI));
	newtonScene->ImproveTotalFitness();
}

// **********************************************************************************************
//
// Name: Generic collision library functions
//
// **********************************************************************************************

// Name: NewtonCollisionPointDistance
// Calculate the closest point between a point and convex collision primitive.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *dFloat* *point - pointer to and array of a least 3 floats representing the origin.
// *const NewtonCollision* *collision -  pointer to collision primitive.
// *const dFloat* *matrix - pointer to an array of 16 floats containing the offset matrix of collision primitiveA.
// *dFloat* *contact - pointer to and array of a least 3 floats to contain the closest point to collisioA.
// *dFloat* *normal - pointer to and array of a least 3 floats to contain the separating vector normal.
// *int* threadIndex -Thread index form where the call is made from, zeor otherwize
//
// Return: one if the two bodies are disjoint and the closest point could be found,
// zero if the point is inside the convex primitive.
//
// Remarks: This function can be used as a low-level building block for a stand-alone collision system.
// Applications that have already there own physics system, and only want and quick and fast collision solution,
// can use Newton advanced collision engine as the low level collision detection part.
// To do this the application only needs to initialize Newton, create the collision primitives at application discretion,
// and just call this function when the objects are in close proximity. Applications using Newton as a collision system
// only, are responsible for implementing their own broad phase collision determination, based on any high level tree structure.
// Also the application should implement their own trivial aabb test, before calling this function .
//
// Remarks: the current implementation of this function do work on collision trees, or user define collision.
//
// See also: NewtonCollisionCollideContinue, NewtonCollisionClosestPoint, NewtonCollisionCollide, NewtonCollisionRayCast, NewtonCollisionCalculateAABB
int NewtonCollisionPointDistance(NewtonWorld *const newtonWorld,
                                 const dFloat *const point, NewtonCollision *const collision,
                                 const dFloat *const matrix, dFloat *const contact, dFloat *const normal,
                                 int threadIndex) {
	Newton *world;
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	return world->ClosestPoint(*((const dgTriplex *)point), (dgCollision *)collision,
	                           *((const dgMatrix *)matrix), *((dgTriplex *)contact), *((dgTriplex *)normal),
	                           threadIndex);
}

// Name: NewtonCollisionClosestPoint
// Calculate the closest points between two disjoint convex collision primitive.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonCollision* *collisionA -  pointer to collision primitive A.
// *const dFloat* *matrixA - pointer to an array of 16 floats containing the offset matrix of collision primitiveA.
// *const NewtonCollision* *collisionB - pointer to collision primitive B.
// *const dFloat* *matrixB - pointer to an array of 16 floats containing the offset matrix of collision primitiveB.
// *dFloat* *contactA - pointer to and array of a least 3 floats to contain the closest point to collisionA.
// *dFloat* *contactB - pointer to and array of a least 3 floats to contain the closest point to collisionB.
// *dFloat* *normalAB - pointer to and array of a least 3 floats to contain the separating vector normal.
// *int* threadIndex -Thread index form where the call is made from, zeor otherwize
//
// Return: one if the tow bodies are disjoint and he closest point could be found,
// zero if the two collision primitives are intersecting.
//
// Remarks: This function can be used as a low-level building block for a stand-alone collision system.
// Applications that have already there own physics system, and only want and quick and fast collision solution,
// can use Newton advanced collision engine as the low level collision detection part.
// To do this the application only needs to initialize Newton, create the collision primitives at application discretion,
// and just call this function when the objects are in close proximity. Applications using Newton as a collision system
// only, are responsible for implementing their own broad phase collision determination, based on any high level tree structure.
// Also the application should implement their own trivial aabb test, before calling this function .
//
// Remarks: the current implementation of this function does not work on collision trees, or user define collision.
//
// See also: NewtonCollisionCollideContinue, NewtonCollisionPointDistance, NewtonCollisionCollide, NewtonCollisionRayCast, NewtonCollisionCalculateAABB
int NewtonCollisionClosestPoint(NewtonWorld *const newtonWorld,
                                NewtonCollision *const collisionA, const dFloat *const matrixA,
                                NewtonCollision *const collisionB, const dFloat *const matrixB,
                                dFloat *const contactA, dFloat *const contactB, dFloat *const normalAB,
                                int threadIndex) {
	Newton *world;
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	return world->ClosestPoint((dgCollision *)collisionA, *((const dgMatrix *)matrixA),
	                           (dgCollision *)collisionB, *((const dgMatrix *)matrixB),
	                           *((dgTriplex *)contactA), *((dgTriplex *)contactB),
	                           *((dgTriplex *)normalAB), threadIndex);
}

// Name: NewtonCollisionCollide
// Calculate contact points between two collision primitive.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *int* maxSize - size of maximum number of elements in contacts, normals, and penetration.
// *const NewtonCollision* *collisionA -  pointer to collision primitive A.
// *const dFloat* *matrixA - pointer to an array of 16 floats containing the offset matrix of collision primitiveA.
// *const NewtonCollision* *collisionB - pointer to collision primitive B.
// *const dFloat* *matrixB - pointer to an array of 16 floats containing the offset matrix of collision primitiveB.
// *dFloat* *contacts - pointer to and array of a least 3 times maxSize floats to contain the collision contact points.
// *dFloat* *normals - pointer to and array of a least 3 times maxSize floats to contain the collision contact normals.
// *dFloat* *penetration - pointer to and array of a least maxSize floats to contain the collision penetration at each contact.
// *int* threadIndex -Thread index form where the call is made from, zeor otherwize
//
// Return: the number of contact points.
//
// Remarks: This function can be used as a low-level building block for a stand-alone collision system.
// Applications that have already there own physics system, and only want and quick and fast collision solution,
// can use Newton advanced collision engine as the low level collision detection part.
// To do this the application only needs to initialize Newton, create the collision primitives at application discretion,
// and just call this function when the objects are in close proximity. Applications using Newton as a collision system
// only, are responsible for implementing their own broad phase collision determination, based on any high level tree structure.
// Also the application should implement their own trivial aabb test, before calling this function .
//
// See also: NewtonCollisionCollideContinue, NewtonCollisionClosestPoint, NewtonCollisionPointDistance, NewtonCollisionRayCast, NewtonCollisionCalculateAABB
int NewtonCollisionCollide(NewtonWorld *const newtonWorld, int maxSize,
                           NewtonCollision *const collisionA, const dFloat *const matrixA,
                           NewtonCollision *const collisionB, const dFloat *const matrixB,
                           dFloat *const contacts, dFloat *const normals, dFloat *const penetration,
                           int threadIndex) {
	Newton *world;
	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	return world->Collide((dgCollision *)collisionA, *((const dgMatrix *)matrixA),
	                      (dgCollision *)collisionB, *((const dgMatrix *)matrixB), (dgTriplex *)contacts,
	                      (dgTriplex *)normals, penetration, maxSize, threadIndex);
}

// Name: NewtonCollisionCollideContinue
// Calculate time of impact of impact and contact points between two collision primitive.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *int* maxSize - size of maximum number of elements in contacts, normals, and penetration.
// *const dFloat* timestep - maximum time interval considered for the continue collision calculation.
// *const NewtonCollision* *collisionA -  pointer to collision primitive A.
// *const dFloat* *matrixA - pointer to an array of 16 floats containing the offset matrix of collision primitiveA.
// *const dFloat* *velocA - pointer to and array of a least 3 times maxSize floats containing the linear velocity of collision primitiveA.
// *const dFloat* *omegaA - pointer to and array of a least 3 times maxSize floats containing the angular velocity of collision primitiveA.
// *const NewtonCollision* *collisionB - pointer to collision primitive B.
// *const dFloat* *matrixB - pointer to an array of 16 floats containing the offset matrix of collision primitiveB.
// *const dFloat* *velocB - pointer to and array of a least 3 times maxSize floats containing the linear velocity of collision primitiveB.
// *const dFloat* *omegaB - pointer to and array of a least 3 times maxSize floats containing the angular velocity of collision primitiveB.
// *dFloat* *timeOfImpact - pointer to least 1 float variable to contain the time of the intersection.
// *dFloat* *contacts - pointer to and array of a least 3 times maxSize floats to contain the collision contact points.
// *dFloat* *normals - pointer to and array of a least 3 times maxSize floats to contain the collision contact normals.
// *dFloat* *penetration - pointer to and array of a least maxSize floats to contain the collision penetration at each contact.
// *int* threadIndex -Thread index form where the call is made from, zeor otherwize
//
// Return: the number of contact points.
//
// Remarks: by passing zero as *maxSize* not contact will be calculated and the function will just determine the time of impact is any.
//
// Remarks: if the body are inter penetrating the time of impact will be zero.
//
// Remarks: if the bodies do not collide time of impact will be set to *timestep*
//
// Remarks: This function can be used as a low-level building block for a stand-alone collision system.
// Applications that have already there own physics system, and only want and quick and fast collision solution,
// can use Newton advanced collision engine as the low level collision detection part.
// To do this the application only needs to initialize Newton, create the collision primitives at application discretion,
// and just call this function when the objects are in close proximity. Applications using Newton as a collision system
// only, are responsible for implementing their own broad phase collision determination, based on any high level tree structure.
// Also the application should implement their own trivial aabb test, before calling this function .
//
// See also: NewtonCollisionCollide, NewtonCollisionClosestPoint, NewtonCollisionPointDistance, NewtonCollisionRayCast, NewtonCollisionCalculateAABB
int NewtonCollisionCollideContinue(NewtonWorld *const newtonWorld,
                                   int maxSize, const dFloat timestep, NewtonCollision *const collisionA,
                                   const dFloat *const matrixA, const dFloat *const velocA,
                                   const dFloat *const omegaA, NewtonCollision *const collisionB,
                                   const dFloat *const matrixB, const dFloat *const velocB,
                                   const dFloat *const omegaB, dFloat *const timeOfImpact,
                                   dFloat *const contacts, dFloat *const normals, dFloat *const penetration,
                                   int threadIndex) {

	Newton *const world = (Newton *)newtonWorld;

	*timeOfImpact = timestep;

	TRACE_FUNTION(__FUNCTION__);
	return world->CollideContinue((dgCollision *)collisionA,
	                              *((const dgMatrix *)matrixA), *((const dgVector *)velocA), *((const dgVector *)omegaA),
	                              (dgCollision *)collisionB, *((const dgMatrix *)matrixB), *((const dgVector *)velocB),
	                              *((const dgVector *)omegaB), *timeOfImpact, (dgTriplex *)contacts,
	                              (dgTriplex *)normals, penetration, maxSize, threadIndex);
}

// Name: NewtonCollisionSupportVertex
// Calculate the most extreme point of a convex collision shape along the given direction.
//
// Parameters:
// *const NewtonCollision* *collisionPtr - pointer to the collision object.
// *const dFloat* *dir - pointer to an array of at least three floats representing the search direction.
// *dFloat* *vertex - pointer to an array of at least three floats to hold the collision most extreme vertex along the search direction.
//
// Return: nothing.
//
// Remarks: the search direction must be in the space of the collision shape.
//
// See also: NewtonCollisionRayCast, NewtonCollisionClosestPoint, NewtonCollisionPointDistance
void NewtonCollisionSupportVertex(const NewtonCollision *collisionPtr,
                                  const dFloat *const dir, dFloat *const vertex) {
	const dgCollisionConvex *collision;

	TRACE_FUNTION(__FUNCTION__);

	collision = (const dgCollisionConvex *)collisionPtr;
	//  NEWTON_ASSERT (collision->IsType (dgCollision::dgConvexCollision_RTTI));

	const dgMatrix &matrix = collision->GetOffsetMatrix();
	dgVector searchDir(
	    matrix.UnrotateVector(dgVector(dir[0], dir[1], dir[2], dgFloat32(0.0f))));
	searchDir = searchDir.Scale(dgRsqrt(searchDir % searchDir));

	dgVector vertexOut(
	    matrix.TransformVector(collision->SupportVertex(searchDir)));

	vertex[0] = vertexOut[0];
	vertex[1] = vertexOut[1];
	vertex[2] = vertexOut[2];
}

// Name: NewtonCollisionRayCast
// Ray cast specific collision object.
//
// Parameters:
// *const NewtonCollision* *collisionPtr - pointer to the collision object.
// *const dFloat* *p0 - pointer to an array of at least three floats representing the ray origin in the local space of the geometry.
// *const dFloat* *p1 - pointer to an array of at least three floats representing the ray end in the local space of the geometry.
// *dFloat* *normal - pointer to an array of at least three floats to hold the normal at the intersection point.
// *int* *attribute - pointer to an array of at least one floats to hold the ID of the face hit by the ray.
//
// Return: the parametric value of the intersection, between 0.0 and 1.0, an value larger than 1.0 if the ray miss.
//
// Remarks: This function is intended for applications using newton collision system separate from the dynamics system, also for applications
// implementing any king of special purpose logic like sensing distance to another object.
//
// Remarks: the ray most be local to the collisions geometry, for example and application ray casting the collision geometry of
// of a rigid body, must first take the points p0, and p1 to the local space of the rigid body by multiplying the points by the
// inverse of he rigid body transformation matrix.
//
// See also: NewtonCollisionClosestPoint, NewtonCollisionSupportVertex, NewtonCollisionPointDistance, NewtonCollisionCollide, NewtonCollisionCalculateAABB
dFloat NewtonCollisionRayCast(const NewtonCollision *collisionPtr,
                              const dFloat *const p0, const dFloat *const p1, dFloat *const normal,
                              int *const attribute) {
	dFloat t;
	const dgCollision *collision;

	collision = (const dgCollision *)collisionPtr;

	TRACE_FUNTION(__FUNCTION__);
	const dgMatrix &matrix = collision->GetOffsetMatrix();

	dgVector q0(
	    matrix.UntransformVector(dgVector(p0[0], p0[1], p0[2], dgFloat32(0.0f))));
	dgVector q1(
	    matrix.UntransformVector(dgVector(p1[0], p1[1], p1[2], dgFloat32(0.0f))));
	dgContactPoint contact;

	t = collision->RayCast(q0, q1, contact, NULL, NULL, NULL);
	if (t >= dFloat(0.0f) && t <= dFloat(dgFloat32(1.0f))) {
		attribute[0] = (int)contact.m_userId;

		dgVector n(matrix.RotateVector(contact.m_normal));
		normal[0] = n[0];
		normal[1] = n[1];
		normal[2] = n[2];
	}
	return t;
}

// Name: NewtonCollisionCalculateAABB
// Calculate an axis-aligned bounding box for this collision, the box is calculated relative to *offsetMatrix*.
//
// Parameters:
// *const NewtonCollision* *collisionPtr - pointer to the collision object.
// *const dFloat* *offsetMatrix - pointer to an array of 16 floats containing the offset matrix used as the coordinate system and center of the AABB.
// *dFloat* *p0 - pointer to an array of at least three floats to hold minimum value for the AABB.
// *dFloat* *p1 - pointer to an array of at least three floats to hold maximum value for the AABB.
//
// Return: Nothing.
//
// See also: NewtonCollisionClosestPoint, NewtonCollisionPointDistance, NewtonCollisionCollide, NewtonCollisionRayCast
void NewtonCollisionCalculateAABB(const NewtonCollision *collisionPtr,
                                  const dFloat *const offsetMatrix, dFloat *const p0, dFloat *const p1) {
	const dgCollision *collision;
	collision = (const dgCollision *)collisionPtr;
	// const dgMatrix& matrix = *((dgMatrix*) offsetMatrix);
	dgMatrix matrix(collision->GetOffsetMatrix() * (*((const dgMatrix *)offsetMatrix)));

	dgVector q0;
	dgVector q1;

	TRACE_FUNTION(__FUNCTION__);
	collision->CalcAABB(matrix, q0, q1);
	p0[0] = q0.m_x;
	p0[1] = q0.m_y;
	p0[2] = q0.m_z;

	p1[0] = q1.m_x;
	p1[1] = q1.m_y;
	p1[2] = q1.m_z;
}

// Name: NewtonCollisionForEachPolygonDo
// Iterate thought polygon of the collision geometry of a body calling the function callback.
//
// Parameters:
// *const NewtonBody* *collisionPtr - is the pointer to the collision objects.
// *const dFloat32* *matrix - is the pointer to the collision objects.
// *NewtonCollisionIterator* callback - application define callback
// *void* *userDataPtr - pointer to the user defined user data value.
//
// Return: nothing
//
// Remarks: This function used to be a member of the rigid body, but to making it a member of the collision object provides better
// low lever display capabilities. The application can still call this function to show the collision of a rigid body by
// getting the collision and the transformation matrix from the rigid, and then calling this functions.
//
// Remarks: This function can be called by the application in order to show the collision geometry. The application should provide a pointer to the function *NewtonCollisionIterator*,
// Newton will convert the collision geometry into a polygonal mesh, and will call *callback* for every polygon of the mesh
//
// Remarks: this function affect severely the performance of Newton. The application should call this function only for debugging purpose
//
// Remarks: This function will ignore user define collision mesh
// See also: NewtonWorldGetFirstBody, NewtonWorldForEachBodyInAABBDo
void NewtonCollisionForEachPolygonDo(const NewtonCollision *collisionPtr,
                                     const dFloat *const matrixPtr, NewtonCollisionIterator callback,
                                     void *const userDataPtr) {
	TRACE_FUNTION(__FUNCTION__);
	const dgCollision *const collision = (const dgCollision *)(collisionPtr);
	const dgMatrix matrix = *((const dgMatrix *)matrixPtr);

	collision->DebugCollision(matrix, (OnDebugCollisionMeshCallback)callback,
	                          userDataPtr);
}

// Name: NewtonCollisionMakeUnique
// Convert a collision primitive to a unique instance by removing it for the collision cache.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonCollision* *collisionPtr - pointer to the collision object
//
// Return: Nothing.
//
// Remarks:
// This function will not make preexisting collision object unique instances, so for best result this function should be call immediately after the
// creation of the collision object.
//
// Remarks:
// Collision objects are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
void NewtonCollisionMakeUnique(NewtonWorld *const newtonWorld,
                               NewtonCollision *const collisionPtr) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;
	dgCollision *const collision = (dgCollision *)collisionPtr;
	world->RemoveFromCache(collision);
}

// Name: NewtonAddCollisionReference
// Increase the reference count of this collision object.
//
// Parameters:
// *const NewtonCollision* *collisionPtr - pointer to the collision object
//
// Return: the new refCount.
//
// Remarks: to get the correct reference count of a collision primitive the application can call fution *NewtonCollisionGetInfo*
//
// Remarks:
// Collision objects are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
//
// See also: NewtonReleaseCollision , NewtonCollisionGetInfo, NewtonCollisionSerialize
int NewtonAddCollisionReference(NewtonCollision *collisionPtr) {
	TRACE_FUNTION(__FUNCTION__);
	dgCollision *const collision = (dgCollision *)collisionPtr;
	collision->AddRef();
	return collision->GetRefCount();
}

// Name: NewtonReleaseCollision
// Release a reference from this collision object returning control to Newton.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonCollision* *collisionPtr - pointer to the collision object
//
// Return: Nothing.
//
// Remarks: to get the correct reference count of a collision primitive the application can call function *NewtonCollisionGetInfo*
//
// Remarks:
// Collision objects are reference counted objects. The application should call *NewtonReleaseCollision* in order to release references to the object.
// Neglecting to release references to collision primitives is a common cause of memory leaks.
//
// See also: NewtonAddCollisionReference, NewtonCollisionGetInfo, NewtonCollisionSerialize
void NewtonReleaseCollision(NewtonWorld *const newtonWorld,
                            NewtonCollision *const collisionPtr) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;
	dgCollision *const collision = (dgCollision *)collisionPtr;
	world->ReleaseCollision(collision);
}

// Name: NewtonCollisionSerialize
// Serialize a * general collision shape.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonCollision* *collision - is the pointer to the collision tree shape.
// *NewtonSerialize* serializeFunction - pointer to the event function that will do the serialization.
// *void* *serializeHandle  - user data that will be passed to the *NewtonSerialize* callback.
//
// Return: Nothing.
//
// Remarks: Small and medium collision shapes like *TreeCollision* (under 50000 polygons) small convex hulls or compude collision can be constructed at application
// startup without significant processing overhead.
//
//
// See also: NewtonCollisionGetInfo
void NewtonCollisionSerialize(const NewtonWorld *const newtonWorld,
                              const NewtonCollision *const collision, NewtonSerialize serializeFunction,
                              void *const serializeHandle) {
	TRACE_FUNTION(__FUNCTION__);
	const Newton *const world = (const Newton *)newtonWorld;
	world->Serialize((const dgCollision *)collision, (dgSerialize)serializeFunction,
	                 serializeHandle);
}

// Name: NewtonCreateCollisionFromSerialization
// Create a collision shape via a serialization function.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *NewtonTreeCollisionCallback *userCallback - pointer to an event function to call before Newton is begins collecting polygons that are colliding with a body. This parameter can be NULL.
// *NewtonSerialize* callback - pointer to the callback function that will handle the serialization.
// *void* *userData - user data that will be passed as the argument to *NewtonSerialize* callback.
//
// Return: Nothing.
//
// Remarks: this function is useful to to load collision primitive for and archive file. In the case of complex shapes like convex hull and compound collision the
// it save a significant amount of construction time.
//
// Remarks: if this function is called to load a serialized tree collision, the tree collision will be loaded, but the function pointer callback will be set to NULL.
// for this operation see function *NewtonCreateTreeCollisionFromSerialization*
//
// See also: NewtonAddCollisionReference, NewtonReleaseCollision, NewtonCollisionSerialize, NewtonCollisionGetInfo
NewtonCollision *NewtonCreateCollisionFromSerialization(
    NewtonWorld *const newtonWorld, NewtonDeserialize deserializeFunction,
    void *const serializeHandle) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;
	return (NewtonCollision *)world->CreateFromSerialization(
	           (dgDeserialize)deserializeFunction, serializeHandle);
}

/*
 // Name: NewtonCreateTreeCollisionFromSerialization
 // Create a tree collision and load the polygon mesh via a serialization function.
 //
 // Parameters:
 // *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
 // *NewtonTreeCollisionCallback *userCallback - pointer to an event function to call before Newton is begins collecting polygons that are colliding with a body. This parameter can be NULL.
 // *NewtonSerialize* callback - pointer to the callback function that will handle the serialization.
 // *void* *userData    - user data that will be passed as the argument to *NewtonSerialize* callback.
 //
 // Return: Nothing.
 //
 // Remarks: if this function is call on a non tree collision, the results are unpredictable.
 //
 // Remarks: Small and medium size *TreeCollision* objects (under 50000 polygons) can be constructed at application startup without significant processing overhead.
 // However, for very large polygons sets (over 50000 polygons) it is recommended that the application use *NewtonCreateTreeCollision*
 // in an off-line tool. Then the application can call this function to store the *TreeCollision* to a file or
 // any file packer system the application is using. At run time the application can use the function *NewtonCreateTreeCollisionFromSerialization*
 // to create and load a pre-made *TreeCollision*
 //
 // See also: NewtonAddCollisionReference, NewtonReleaseCollision, NewtonCollisionSerialize, NewtonCollisionGetInfo
 NewtonCollision* NewtonCreateTreeCollisionFromSerialization(const NewtonWorld* const newtonWorld, NewtonTreeCollisionCallback userCallback, NewtonDeserialize deserializeFunction, void* const serializeHandle)
 {
 Newton* world;
 dgCollision* collision;
 NewtonCollisionTree *dataBase;

 world = (Newton *)newtonWorld;

 dataBase = new NewtonCollisionTree (world, userCallback);
 dataBase->Deserialize (deserializeFunction, serializeHandle);

 dgVector p0;
 dgVector p1;

 dataBase->GetAABB (p0, p1);

 world = (Newton *)newtonWorld;

 collision = world->CreatePolygonSoup (dataBase,
 NewtonCollisionTree::GetIntersectingPolygons,
 NewtonCollisionTree::RayHit,
 NewtonCollisionTree::Destroy, NULL, NULL);

 collision->SetCollisionBBox(p0, p1);
 return (NewtonCollision*)collision;
 }
 */

// Name: NewtonCollisionGetInfo
// Get creation parameters for this collision objects.
//
// Parameters:
// *const NewtonCollision* collision - is the pointer to a convex collision primitive.
// *NewtonCollisionInfoRecord* *collisionInfo - pointer to a collision information record.
//
// Remarks: This function can be used by the application for writing file format and for serialization.
//
// See also: NewtonCollisionGetInfo, NewtonCollisionSerialize
void NewtonCollisionGetInfo(const NewtonCollision *const collision,
                            NewtonCollisionInfoRecord *const collisionInfo) {
	const dgCollision *coll;
	coll = (const dgCollision *)collision;

	TRACE_FUNTION(__FUNCTION__);

	NEWTON_ASSERT(sizeof(dgCollisionInfo) <= sizeof(NewtonCollisionInfoRecord));
	coll->GetCollisionInfo((dgCollisionInfo *)collisionInfo);
}

// **********************************************************************************************
//
// Name: Transform utility functions
//
// **********************************************************************************************

// Name: NewtonGetEulerAngle
// Get the three Euler angles from a 4x4 rotation matrix arranged in row-major order.
//
// Parameters:
// *const dFloat* matrix - pointer to the 4x4 rotation matrix.
// *dFloat* angles - pointer to an array of at least three floats to hold the Euler angles.
//
// Return: Nothing.
//
// Remarks: The motivation for this function is that many graphics engines still use Euler angles to represent the orientation
// of graphics entities.
// The angles are expressed in radians and represent:
// *angle[0]* - rotation about first matrix row
// *angle[1]* - rotation about second matrix row
// *angle[2]* - rotation about third matrix row
//
// See also: NewtonSetEulerAngle
void NewtonGetEulerAngle(const dFloat *const matrix, dFloat *const angles) {
	const dgMatrix &mat = *((const dgMatrix *)matrix);

	TRACE_FUNTION(__FUNCTION__);
	dgVector eulers(mat.CalcPitchYawRoll());
	angles[0] = eulers.m_x;
	angles[1] = eulers.m_y;
	angles[2] = eulers.m_z;
}

// Name: NewtonSetEulerAngle
// Build a rotation matrix from the Euler angles in radians.
//
// Parameters:
// *dFloat* matrix - pointer to the 4x4 rotation matrix.
// *const dFloat* angles - pointer to an array of at least three floats to hold the Euler angles.
//
// Return: Nothing.
//
// Remarks: The motivation for this function is that many graphics engines still use Euler angles to represent the orientation
// of graphics entities.
// The angles are expressed in radians and represent:
// *angle[0]* - rotation about first matrix row
// *angle[1]* - rotation about second matrix row
// *angle[2]* - rotation about third matrix row
//
// See also: NewtonGetEulerAngle
void NewtonSetEulerAngle(const dFloat *const angles, dFloat *const matrix) {
	dgInt32 i;
	dgInt32 j;

	TRACE_FUNTION(__FUNCTION__);
	dgMatrix mat(
	    dgPitchMatrix(angles[0]) * dgYawMatrix(angles[1]) * dgRollMatrix(angles[2]));
	dgMatrix &retMatrix = *((dgMatrix *)matrix);

	retMatrix[3][3] = dgFloat32(1.0f);
	for (i = 0; i < 3; i++) {
		retMatrix[3][i] = 0.0f;
		for (j = 0; j < 4; j++) {
			retMatrix[i][j] = mat[i][j];
		}
	}
}

// Name: NewtonCalculateSpringDamperAcceleration
// Calculates the acceleration to satisfy the specified the spring damper system.
//
// Parameters:
// *dFloat* dt - integration time step.
// *dFloat* ks - spring stiffness, it must be a positive value.
// *dFloat* x - spring position.
// *dFloat* kd - desired spring damper, it must be a positive value.
// *dFloat* s - spring velocity.
//
// return: the spring acceleration.
//
// Remark: the acceleration calculated by this function represent the mass, spring system of the form
// a = -ks * x - kd * v.
dFloat NewtonCalculateSpringDamperAcceleration(dFloat dt, dFloat ks, dFloat x,
        dFloat kd, dFloat s) {
	dFloat accel;
	//  accel = - (ks * x + kd * s);

	TRACE_FUNTION(__FUNCTION__);
	// at =  [- ks (x2 - x1) - kd * (v2 - v1) - dt * ks * (v2 - v1)] / [1 + dt * kd + dt * dt * ks]
	dgFloat32 ksd = dt * ks;
	dgFloat32 num = ks * x + kd * s + ksd * s;
	dgFloat32 den = dgFloat32(1.0f) + dt * kd + dt * ksd;
	NEWTON_ASSERT(den > 0.0f);
	accel = -num / den;
	//  dgCheckFloat (accel);
	return accel;
}

// **********************************************************************************************
//
// Name: Rigid body interface
//
// **********************************************************************************************

// Name: NewtonCreateBody
// Create a rigid body.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonCollision* *collisionPtr - pointer to the collision object.
//
// Return: Pointer to the rigid body.
//
// Remarks: This function creates a Newton rigid body and assigns a *collisionPtr* as the collision geometry representing the rigid body.
// This function increments the reference count of the collision geometry.
// All event functions are set to NULL and the material gruopID of the body is set to the default GroupID.
//
// See also: NewtonDestroyBody
NewtonBody *NewtonCreateBody(NewtonWorld *const newtonWorld,
                             NewtonCollision *const collisionPtr, const dFloat *const matrixPtr) {

	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;
	dgCollision *const collision = (dgCollision *)collisionPtr;

#ifdef SAVE_COLLISION
	SaveCollision(collisionPtr);
#endif

	dgMatrix matrix(*((const dgMatrix *)matrixPtr));
#ifdef _DEBUG
	//  matrix.m_front = matrix.m_front.Scale (dgRsqrt (matrix.m_front % matrix.m_front));
	//  matrix.m_right = matrix.m_front * matrix.m_up;
	//  matrix.m_right = matrix.m_right.Scale (dgRsqrt (matrix.m_right % matrix.m_right));
	//  matrix.m_up = matrix.m_right * matrix.m_front;
#endif

	matrix.m_front.m_w = dgFloat32(0.0f);
	matrix.m_up.m_w = dgFloat32(0.0f);
	matrix.m_right.m_w = dgFloat32(0.0f);
	matrix.m_posit.m_w = dgFloat32(1.0f);

	return (NewtonBody *)world->CreateBody(collision, matrix);
}

// Name: NewtonDestroyBody
// Destroy a rigid body.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonBody* *bodyPtr - pointer to the body to be destroyed.
//
// Return: Nothing.
//
// Remarks: If this function is called from inside a simulation step the destruction of the body will be delayed until end of the time step.
// This function will decrease the reference count of the collision geometry by one. If the reference count reaches zero, then the collision
// geometry will be destroyed. This function will destroy all joints associated with this body.
//
// See also: NewtonCreateBody
void NewtonDestroyBody(NewtonWorld *const newtonWorld,
                       NewtonBody *const bodyPtr) {
	dgBody *body;
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	body = (dgBody *)bodyPtr;
	world = (Newton *)newtonWorld;
	world->DestroyBody(body);
}

// Name: NewtonBodySetUserData
// Store a user defined data value with the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *void* *userDataPtr - pointer to the user defined user data value.
//
// Return: Nothing.
//
// Remarks: The application can store a user defined value with the Body. This value can be the pointer to a structure containing some application data for special effect.
// if the application allocate some resource to store the user data, the application can register a joint destructor to get rid of the allocated resource when the body is destroyed
//
// See also: NewtonBodyGetUserData, NewtonBodySetDestructorCallback
void NewtonBodySetUserData(NewtonBody *const bodyPtr,
                           void *const userDataPtr) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	body->SetUserData(userDataPtr);
}

// Name: NewtonBodyGetUserData
// Retrieve a user defined data value stored with the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
//
// Return: The user defined data.
//
// Remarks: The application can store a user defined value with a rigid body. This value can be the pointer
// to a structure which is the graphical representation of the rigid body.
//
// See also: NewtonBodySetUserData
void *NewtonBodyGetUserData(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return body->GetUserData();
}

// Name: NewtonBodyGetWorld
// Retrieve get the pointer to the world from the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
//
// Return: the world that own this body.
//
// Remarks: The application can use this function to determine what world own this body. If the application
// have to get the world from a joint, it can do so by getting one of the bodies attached to the joint and getting the world from
// that body.
//
NewtonWorld *NewtonBodyGetWorld(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return (NewtonWorld *)body->GetWorld();
}

// Name: NewtonBodySetTransformCallback
// Assign a transformation event function to the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *NewtonSetTransform callback - pointer to a function callback in used to update the transformation matrix of the visual object that represents the rigid body.
//
// Return: Nothing.
//
// Remarks: The function *NewtonSetTransform callback* is called by the Newton engine every time a visual object that represents the rigid body has changed.
// The application can obtain the pointer user data value that points to the visual object.
// The Newton engine does not call the *NewtonSetTransform callback* function for bodies that are inactive or have reached a state of stable equilibrium.
//
// Remarks: The matrix should be organized in row-major order (this is the way directX and OpenGL stores matrices).
//
// See also: NewtonBodyGetUserData, NewtonBodyGetUserData
void NewtonBodySetTransformCallback(NewtonBody *const bodyPtr,
                                    NewtonSetTransform callback) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	body->SetMatrixUpdateCallback((OnMatrixUpdateCallback)callback);
}

// Name: NewtonBodyGetTransformCallback
// Assign a transformation event function to the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *NewtonSetTransform callback - pointer to a function callback in used to update the transformation matrix of the visual object that represents the rigid body.
//
// Return: Nothing.
//
// Remarks: The function *NewtonSetTransform callback* is called by the Newton engine every time a visual object that represents the rigid body has changed.
// The application can obtain the pointer user data value that points to the visual object.
// The Newton engine does not call the *NewtonSetTransform callback* function for bodies that are inactive or have reached a state of stable equilibrium.
//
// Remarks: The matrix should be organized in row-major order (this is the way directX and OpenGL stores matrices).
//
// See also: NewtonBodyGetUserData, NewtonBodyGetUserData
NewtonSetTransform NewtonBodyGetTransformCallback(
			    const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return (NewtonSetTransform)body->GetMatrixUpdateCallback();
}

// Name: NewtonBodySetForceAndTorqueCallback
// Assign an event function for applying external force and torque to a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *NewtonApplyForceAndTorque callback - pointer to a function callback used to apply force and torque to a rigid body.
//
// Return: Nothing.
//
// Remarks: Before the *NewtonApplyForceAndTorque callback* is called for a body, Newton first clears the net force and net torque for the body.
//
// Remarks: The function *NewtonApplyForceAndTorque callback* is called by the Newton Engine every time an active body is going to be simulated.
// The Newton Engine does not call the *NewtonApplyForceAndTorque callback* function for bodies that are inactive or have reached a state of stable equilibrium.
//
// See also: NewtonBodyGetUserData, NewtonBodyGetUserData, NewtonBodyGetForceAndTorqueCallback
void NewtonBodySetForceAndTorqueCallback(NewtonBody *const bodyPtr,
        NewtonApplyForceAndTorque callback) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	body->SetExtForceAndTorqueCallback((OnApplyExtForceAndTorque)callback);
}

// Name: NewtonBodyGetForceAndTorqueCallback
// Return the pointer to the current force and torque call back function.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
//
// Return: pointer to the force call back.
//
// Remarks: This function can be used to concatenate different force calculation components making more modular the
// design of function components dedicated to apply special effect. For example a body may have a basic force a force that
// only apply the effect of gravity, but that application can place a region in where there can be a fluid volume, or another gravity field.
// we this function the application can read the correct function and save into a local variable, and set a new one.
// this new function will firs call the save function pointer and upon return apply the correct effect.
// this similar to the concept of virtual methods on objected oriented languages.
//
// Remarks: The function *NewtonApplyForceAndTorque callback* is called by the Newton Engine every time an active body is going to be simulated.
// The Newton Engine does not call the *NewtonApplyForceAndTorque callback* function for bodies that are inactive or have reached a state of stable equilibrium.
//
// See also: NewtonBodyGetUserData, NewtonBodyGetUserData, NewtonBodySetForceAndTorqueCallback
NewtonApplyForceAndTorque NewtonBodyGetForceAndTorqueCallback(
    const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return (NewtonApplyForceAndTorque)body->GetExtForceAndTorqueCallback();
}

// Name: NewtonBodySetDestructorCallback
// Assign an event function to be called when this body is about to be destroyed.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body to be destroyed.
// *NewtonBodyDestructor* callback - pointer to a function callback.
//
// Return: Nothing.
//
// Remarks:
// This function *NewtonBodyDestructor callback* acts like a destruction function in CPP. This function
// is called when the body and all data joints associated with the body are about to be destroyed.
// The application could use this function to destroy or release any resource associated with this body.
// The application should not make reference to this body after this function returns.
//
// Remarks:
// The destruction of a body will destroy all joints associated with the body.
//
// See also: NewtonBodyGetUserData, NewtonBodyGetUserData
void NewtonBodySetDestructorCallback(NewtonBody *const bodyPtr,
                                     NewtonBodyDestructor callback) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	body->SetDestructorCallback((OnBodyDestroy)callback);
}

// Name: NewtonBodySetMassMatrix
// Set the mass matrix of a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *dFloat* mass - mass value.
// *dFloat* Ixx - moment of inertia of the first principal axis of inertia of the body.
// *dFloat* Iyy - moment of inertia of the first principal axis of inertia of the body.
// *dFloat* Izz - moment of inertia of the first principal axis of inertia of the body.
//
// Return: Nothing.
//
// Remarks: Newton algorithms have no restriction on the values for the mass, but due to floating point dynamic
// range (24 bit precision) it is best if the ratio between the heaviest and the lightest body in the scene is limited to 200.
// There are no special utility functions in Newton to calculate the moment of inertia of common primitives.
// The application should specify the inertial values, keeping in mind that realistic inertia values are necessary for
// realistic physics behavior.
//
// See also: NewtonConvexCollisionCalculateInertialMatrix, NewtonBodyGetMassMatrix, NewtonBodyGetInvMass
void NewtonBodySetMassMatrix(NewtonBody *const bodyPtr, dFloat mass,
                             dFloat Ixx, dFloat Iyy, dFloat Izz) {
	dgBody *body;
	dFloat Ixx1;
	dFloat Iyy1;
	dFloat Izz1;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	mass = dgAbsf(mass);
	Ixx = dgAbsf(Ixx);
	Iyy = dgAbsf(Iyy);
	Izz = dgAbsf(Izz);

	Ixx1 = ClampValue(Ixx, dgFloat32(0.001f) * mass, dgFloat32(100.0f) * mass);
	Iyy1 = ClampValue(Iyy, dgFloat32(0.001f) * mass, dgFloat32(100.0f) * mass);
	Izz1 = ClampValue(Izz, dgFloat32(0.001f) * mass, dgFloat32(100.0f) * mass);
	if (mass < dgFloat32(1.0e-3f)) {
		mass = DG_INFINITE_MASS * dgFloat32(1.5f);
	}

	body->SetMassMatrix(mass, Ixx1, Iyy1, Izz1);
	body->SetAparentMassMatrix(dgVector(Ixx, Iyy, Izz, mass));
}

// Name: NewtonBodyGetMassMatrix
// Get the mass matrix of a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *dFloat* *mass - pointer to a variable that will hold the mass value of the body.
// *dFloat* *Ixx - pointer to a variable that will hold the moment of inertia of the first principal axis of inertia of the body.
// *dFloat* *Iyy - pointer to a variable that will hold the moment of inertia of the first principal axis of inertia of the body.
// *dFloat* *Izz - pointer to a variable that will hold the moment of inertia of the first principal axis of inertia of the body.
//
// Return: Nothing.
//
// See also: NewtonBodySetMassMatrix, NewtonBodyGetInvMass
void NewtonBodyGetMassMatrix(const NewtonBody *const bodyPtr,
                             dFloat *const mass, dFloat *const Ixx, dFloat *const Iyy, dFloat *const Izz) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);

	//  dgVector vector (body->GetMass ());
	dgVector vector(body->GetAparentMass());
	Ixx[0] = vector.m_x;
	Iyy[0] = vector.m_y;
	Izz[0] = vector.m_z;
	mass[0] = vector.m_w;
	if (vector.m_w > DG_INFINITE_MASS * 0.5f) {
		Ixx[0] = 0.0f;
		Iyy[0] = 0.0f;
		Izz[0] = 0.0f;
		mass[0] = 0.0f;
	}
}

// Name: NewtonBodyGetInvMass
// Get the inverse mass matrix of a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *dFloat* *invMass - pointer to a variable that will hold the mass inverse value of the body.
// *dFloat* *invIxx - pointer to a variable that will hold the moment of inertia inverse of the first principal axis of inertia of the body.
// *dFloat* *invIyy - pointer to a variable that will hold the moment of inertia inverse of the first principal axis of inertia of the body.
// *dFloat* *invIzz - pointer to a variable that will hold the moment of inertia inverse of the first principal axis of inertia of the body.
//
// Return: Nothing.
//
// See also: NewtonBodySetMassMatrix, NewtonBodyGetMassMatrix
void NewtonBodyGetInvMass(const NewtonBody *const bodyPtr,
                          dFloat *const invMass, dFloat *const invIxx, dFloat *const invIyy,
                          dFloat *const invIzz) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	//  dgVector vector (body->GetInvMass ());
	//  invIxx[0] = vector.m_x;
	//  invIyy[0] = vector.m_y;
	//  invIzz[0] = vector.m_z;
	//  invMass[0] = vector.m_w;

	dgVector vector1(body->GetAparentMass());
	invIxx[0] = dgFloat32(1.0f) / (vector1.m_x + dgFloat32(1.0e-8f));
	invIyy[0] = dgFloat32(1.0f) / (vector1.m_y + dgFloat32(1.0e-8f));
	invIzz[0] = dgFloat32(1.0f) / (vector1.m_z + dgFloat32(1.0e-8f));
	invMass[0] = dgFloat32(1.0f) / (vector1.m_w + dgFloat32(1.0e-8f));
}

// Name: NewtonBodySetMatrix
// Set the transformation matrix of a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *matrixPtr - pointer to an array of 16 floats containing the global matrix of the rigid body.
//
// Return: Nothing.
//
// Remarks: The matrix should be arranged in row-major order.
// If you are using OpenGL matrices (column-major) you will need to transpose you matrices into a local array, before
// passing them to Newton.
//
// Remarks: That application should make sure the transformation matrix has not scale, otherwise unpredictable result will occur.
//
// See also: NewtonBodyGetMatrix
void NewtonBodySetMatrix(NewtonBody *const bodyPtr,
                         dFloat *const matrixPtr) {
	dgBody *body;
	body = (dgBody *)bodyPtr;
	dgMatrix matrix(*((dgMatrix *)matrixPtr));

	TRACE_FUNTION(__FUNCTION__);

#ifdef _DEBUG
//	matrix.m_front = matrix.m_front.Scale (dgRsqrt (matrix.m_front % matrix.m_front));
//	matrix.m_right = matrix.m_front * matrix.m_up;
//	matrix.m_right = matrix.m_right.Scale (dgRsqrt (matrix.m_right % matrix.m_right));
//	matrix.m_up = matrix.m_right * matrix.m_front;
#endif

	matrix.m_front.m_w = dgFloat32(0.0f);
	matrix.m_up.m_w = dgFloat32(0.0f);
	matrix.m_right.m_w = dgFloat32(0.0f);
	matrix.m_posit.m_w = dgFloat32(1.0f);
	body->SetMatrixIgnoreSleep(matrix);
}

// Name: NewtonBodySetMatrixRecursive
// Apply hierarchical transformation to a body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *matrixPtr - pointer to an array of 16 floats containing the global matrix of the rigid body.
//
// Return: Nothing.
//
// Remarks: This function applies the transformation matrix to the *body* and also applies the appropriate transformation matrix to
// set of articulated bodies. If the body is in contact with another body the other body is not transformed.
//
// Remarks: this function should not be used to transform set of articulated bodies that are connected to a static body.
// doing so will result in unpredictables results. Think for example moving a chain attached to a ceiling from one place to another,
// to do that in real life a person first need to disconnect the chain (destroy the joint), move the chain (apply the transformation to the
// entire chain), the reconnect it in the new position (recreate the joint again).
//
// Remarks: this function will set to zero the linear and angular velocity of all bodies that are part of the set of articulated body array.
//
// Remarks: The matrix should be arranged in row-major order (this is the way direct x stores matrices).
// If you are using OpenGL matrices (column-major) you will need to transpose you matrices into a local array, before
// passing them to Newton.
//
// See also: NewtonBodySetMatrix
void NewtonBodySetMatrixRecursive(NewtonBody *const bodyPtr,
                                  const dFloat *const matrixPtr) {
	dgBody *body;
	Newton *world;

	body = (dgBody *)bodyPtr;
	world = (Newton *)body->GetWorld();
	const dgMatrix matrix(*((const dgMatrix *)matrixPtr));

	world->BodySetMatrix(body, matrix);
}

// Name: NewtonBodyGetMatrix
// Get the transformation matrix of a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *matrixPtr - pointer to an array of 16 floats that will hold the global matrix of the rigid body.
//
// Return: Nothing.
//
// Remarks: The matrix should be arranged in row-major order (this is the way direct x stores matrices).
// If you are using OpenGL matrices (column-major) you will need to transpose you matrices into a local array, before
// passing them to Newton.
//
// See also: NewtonBodySetMatrix, NewtonBodyGetRotation
void NewtonBodyGetMatrix(const NewtonBody *const bodyPtr,
                         dFloat *const matrixPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgMatrix &matrix = *((dgMatrix *)matrixPtr);
	matrix = body->GetMatrix();
}

// Name: NewtonBodyGetRotation
// Get the rotation part of the transformation matrix of a body, in form of a unit quaternion.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *rotPtr - pointer to an array of 4 floats that will hold the global rotation of the rigid body.
//
// Return: Nothing.
//
// Remarks: The rotation matrix is written set in the form of a unit quaternion in the format Rot (q0, q1, q1, q3)
//
// Remarks: The rotation quaternion is the same as what the application would get by using at function to extract a quaternion form a matrix.
// however since the rigid body already contained the rotation in it, it is more efficient to just call this function avoiding expensive conversion.
//
// Remarks: this function could be very useful for the implementation of pseudo frame rate independent simulation.
// by running the simulation at a fix rate and using linear interpolation between the last two simulation frames.
// to determine the exact fraction of the render step.
//
// See also: NewtonBodySetMatrix, NewtonBodyGetMatrix
void NewtonBodyGetRotation(const NewtonBody *const bodyPtr, dFloat *rotPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgQuaternion &rot = *((dgQuaternion *)rotPtr);
	rot = body->GetRotation();
}

// Name: NewtonBodySetForce
// Set the net force applied to a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *vectorPtr - pointer to an array of 3 floats containing the net force to be applied to the body.
//
// Return: Nothing.
//
// Remarks: This function is only effective when called from *NewtonApplyForceAndTorque callback*
//
// See also: NewtonBodyAddForce, NewtonBodyGetForce, NewtonBodyGetForceAcc
void NewtonBodySetForce(NewtonBody *const bodyPtr,
                        const dFloat *vectorPtr) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(vectorPtr[0], vectorPtr[1], vectorPtr[2], dgFloat32(0.0f));
	body->SetForce(vector);
}

// Name: NewtonBodyAddForce
// Add the net force applied to a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body to be destroyed.
// *const dFloat* *vectorPtr - pointer to an array of 3 floats containing the net force to be applied to the body.
//
// Return: Nothing.
//
// Remarks: This function is only effective when called from *NewtonApplyForceAndTorque callback*
//
// See also: NewtonBodySetForce, NewtonBodyGetForce, NewtonBodyGetForceAcc
void NewtonBodyAddForce(NewtonBody *const bodyPtr,
                        const dFloat *const vectorPtr) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(vectorPtr[0], vectorPtr[1], vectorPtr[2], dgFloat32(0.0f));

	body->AddForce(vector);
}

// Name: NewtonBodyGetForceAcc
// Get the force applied on the last call to apply force and torque callback.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *vectorPtr - pointer to an array of 3 floats to hold the net force of the body.
//
// Remark: this function can be useful to modify force from joint callback
//
// Return: Nothing.
//
// See also: NewtonBodyAddForce, NewtonBodyGetForce, NewtonBodyGetForce
void NewtonBodyGetForceAcc(const NewtonBody *const bodyPtr,
                           dFloat *const vectorPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(body->GetForce());
	vectorPtr[0] = vector.m_x;
	vectorPtr[1] = vector.m_y;
	vectorPtr[2] = vector.m_z;
}

// Name: NewtonBodyGetForce
// Get the net force applied to a rigid body after the last NewtonUpdate.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *vectorPtr - pointer to an array of 3 floats to hold the net force of the body.
//
// Return: Nothing.
//
// See also: NewtonBodyAddForce, NewtonBodyGetForce, NewtonBodyGetForceAcc
void NewtonBodyGetForce(const NewtonBody *const bodyPtr,
                        dFloat *const vectorPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(body->GetNetForce());
	vectorPtr[0] = vector.m_x;
	vectorPtr[1] = vector.m_y;
	vectorPtr[2] = vector.m_z;
}

// Name: NewtonBodyCalculateInverseDynamicsForce
// Calculate the next force that net to be applied to the body to archive the desired velocity in the current time step.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *dFloat* timestep - time step that the force will be applyed.
// *const dFloat* *desiredVeloc - pointer to an array of 3 floats containing the desired velocity.
// *dFloat* *forceOut - pointer to an array of 3 floats to hold the calculated net force.
//
// Remark: this function can be useful when creating object for game play.
//
// remark: this treat the body as a point mass and is uses the solver to calculates the net force that need to be applied to the body
// such that is reach the desired velocity in the net time step.
// In general the force should be calculated by the expression f = M * (dsiredVeloc - bodyVeloc) / timestep
// however due to algorithmic optimization and limitations if such equation is used then the solver will generate a different desired velocity.
//
// Return: Nothing.
//
// See also: NewtonBodySetForce, NewtonBodyAddForce, NewtonBodyGetForce, NewtonBodyGetForceAcc
void NewtonBodyCalculateInverseDynamicsForce(const NewtonBody *const bodyPtr,
        dFloat timestep, const dFloat *const desiredVeloc, dFloat *const forceOut) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector veloc(desiredVeloc[0], desiredVeloc[1], desiredVeloc[2],
	               dgFloat32(0.0f));

	dgVector force(body->CalculateInverseDynamicForce(veloc, timestep));
	forceOut[0] = force[0];
	forceOut[1] = force[1];
	forceOut[2] = force[2];
}

// Name: NewtonBodySetTorque
// Set the net torque applied to a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *vectorPtr - pointer to an array of 3 floats containing the net torque to be applied to the body.
//
// Return: Nothing.
//
// Remarks: This function is only effective when called from *NewtonApplyForceAndTorque callback*
//
// See also: NewtonBodyAddTorque, NewtonBodyGetTorque, NewtonBodyGetTorqueAcc
void NewtonBodySetTorque(NewtonBody *bodyPtr,
                         const dFloat *const vectorPtr) {
	dgBody *body;

	TRACE_FUNTION(__FUNCTION__);
	body = (dgBody *)bodyPtr;
	dgVector vector(vectorPtr[0], vectorPtr[1], vectorPtr[2], dgFloat32(0.0f));
	body->SetTorque(vector);
}

// Name: NewtonBodyAddTorque
// Add the net torque applied to a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *vectorPtr - pointer to an array of 3 floats containing the net torque to be applied to the body.
//
// Return: Nothing.
//
// Remarks: This function is only effective when called from *NewtonApplyForceAndTorque callback*
//
// See also: NewtonBodySetTorque, NewtonBodyGetTorque, NewtonBodyGetTorqueAcc
void NewtonBodyAddTorque(NewtonBody *const bodyPtr,
                         const dFloat *const vectorPtr) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(vectorPtr[0], vectorPtr[1], vectorPtr[2], dgFloat32(0.0f));
	body->AddTorque(vector);
}

// Name: NewtonBodyGetTorque
// Get the net torque applied to a rigid body after the last NewtonUpdate.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *vectorPtr - pointer to an array of 3 floats to hold the net torque of the body.
//
// Return: Nothing.
//
// See also: NewtonBodyAddTorque, NewtonBodyGetTorque, NewtonBodyGetTorqueAcc
void NewtonBodyGetTorque(const NewtonBody *const bodyPtr,
                         dFloat *const vectorPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(body->GetNetTorque());
	vectorPtr[0] = vector.m_x;
	vectorPtr[1] = vector.m_y;
	vectorPtr[2] = vector.m_z;
}

// Name: NewtonBodyGetTorqueAcc
// Get the torque applied on the last call to apply force and torque callback.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *vectorPtr - pointer to an array of 3 floats to hold the net force of the body.
//
// Remark: this function can be useful to modify torque form joint callback
//
// Return: Nothing.
//
// See also: NewtonBodyAddTorque, NewtonBodyGetTorque, NewtonBodyGetTorque
void NewtonBodyGetTorqueAcc(const NewtonBody *const bodyPtr,
                            dFloat *const vectorPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(body->GetTorque());
	vectorPtr[0] = vector.m_x;
	vectorPtr[1] = vector.m_y;
	vectorPtr[2] = vector.m_z;
}

// Name: NewtonBodySetCentreOfMass
// Set the relative position of the center of mass of a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const dFloat* *comPtr - pointer to an array of 3 floats containing the relative offset of the center of mass of the body.
//
// Return: Nothing.
//
// Remarks: This function can be used to set the relative offset of the center of mass of a rigid body.
// when a rigid body is created the center of mass is set the the point c(0, 0, 0), and normally this is
// the best setting for a rigid body. However the are situations in which and object does not have symmetry or
// simple some kind of special effect is desired, and this origin need to be changed.
//
// Remarks: Care must be taken when offsetting the center of mass of a body.
// The application must make sure that the external torques resulting from forces applied at at point
// relative to the center of mass are calculated appropriately.
// this could be done Transform and Torque callback function as the follow pseudo code fragment shows:
//
// Matrix matrix;
// Vector center;
//
// NewtonGatMetrix(body, matrix)
// NewtonGetCentreOfMass(body, center);
//
// for global space torque.
// Vector localForce (fx, fy, fz);
// Vector localPosition (x, y, z);
// Vector localTroque (crossproduct ((localPosition - center). localForce);
// Vector globalTroque (matrix.RotateVector (localTroque));
//
// for global space torque.
// Vector globalCentre (matrix.TranformVector (center));
// Vector globalPosition (x, y, z);
// Vector globalForce (fx, fy, fz);
// Vector globalTroque (crossproduct ((globalPosition - globalCentre). globalForce);
//
// See also: NewtonConvexCollisionCalculateInertialMatrix, NewtonBodyGetCentreOfMass
void NewtonBodySetCentreOfMass(NewtonBody *bodyPtr,
                               const dFloat *const comPtr) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(comPtr[0], comPtr[1], comPtr[2], dgFloat32(1.0f));
	body->SetCentreOfMass(vector);
}

// Name: NewtonBodyGetCentreOfMass
// Get the relative position of the center of mass of a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *dFloat* *comPtr - pointer to an array of 3 floats to hold the relative offset of the center of mass of the body.
//
// Return: Nothing.
//
// Remarks: This function can be used to set the relative offset of the center of mass of a rigid body.
// when a rigid body is created the center of mass is set the the point c(0, 0, 0), and normally this is
// the best setting for a rigid body. However the are situations in which and object does not have symmetry or
// simple some kind of special effect is desired, and this origin need to be changed.
//
// Remarks: This function can be used in conjunction with *NewtonConvexCollisionCalculateInertialMatrix*
//
// See also: NewtonConvexCollisionCalculateInertialMatrix, NewtonBodySetCentreOfMass
void NewtonBodyGetCentreOfMass(const NewtonBody *const bodyPtr,
                               dFloat *const comPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(body->GetCentreOfMass());
	comPtr[0] = vector.m_x;
	comPtr[1] = vector.m_y;
	comPtr[2] = vector.m_z;
}

// Name: NewtonBodyGetFirstJoint
// Return a pointer to the first joint attached to this rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
//
// Return: Joint if at least one is attached to the body, NULL if not joint is attached
//
// Remarks: this function will only return the pointer to user defined joints, older build in constraints will be skipped by this function.
//
// Remark: this function can be used to implement recursive walk of complex articulated arrangement of rodid bodies.
//
// See also: NewtonBodyGetNextJoint
NewtonJoint *NewtonBodyGetFirstJoint(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return (NewtonJoint *)body->GetFirstJoint();
}

// Name: NewtonBodyGetNextJoint
// Return a pointer to the next joint attached to this body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const NewtonJoint* *joint - pointer to current joint.
//
// Return: Joint is at least one joint is attached to the body, NULL if not joint is attached
//
// Remarks: this function will only return the pointer to User defined joint, older build in constraints will be skipped by this function.
//
// Remark: this function can be used to implement recursive walk of complex articulated arrangement of rodid bodies.
//
// See also: NewtonBodyGetFirstJoint
NewtonJoint *NewtonBodyGetNextJoint(const NewtonBody *const bodyPtr,
                                    const NewtonJoint *const jointPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return (NewtonJoint *)body->GetNextJoint((const dgConstraint *)jointPtr);
}

// Name: NewtonBodyGetFirstContactJoint
// Return a pointer to the first contact joint attached to this rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
//
// Return: Contact if the body is colliding with anther body, NULL otherwise
//
// See also: NewtonBodyGetNextContactJoint, NewtonContactJointGetFirstContact, NewtonContactJointGetNextContact, NewtonContactJointRemoveContact
NewtonJoint *NewtonBodyGetFirstContactJoint(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return (NewtonJoint *)body->GetFirstContact();
}

// Name: NewtonBodyGetNextContactJoint
// Return a pointer to the next contactjoint attached to this rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const NewtonJoint* *contactJoint - pointer to corrent contact joint.
//
// Return: Contact if the body is colliding with anther body, NULL otherwise
//
// See also: NewtonBodyGetFirstContactJoint, NewtonContactJointGetFirstContact, NewtonContactJointGetNextContact, NewtonContactJointRemoveContact
NewtonJoint *NewtonBodyGetNextContactJoint(const NewtonBody *const bodyPtr,
        const NewtonJoint *const contactPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return (NewtonJoint *)body->GetNextContact((const dgConstraint *)contactPtr);
}

// Name: NewtonContactJointGetContactCount
// Return to number of contact int thsi contact joint.
//
// Parameters:
// *const NewtonJoint* *contactJoint - pointer to corrent contact joint.
//
// Return: numbet of contacts.
//
// See also: NewtonContactJointGetFirstContact, NewtonContactJointGetNextContact, NewtonContactJointRemoveContact
int NewtonContactJointGetContactCount(const NewtonJoint *const contactJoint) {
	const dgContact *joint;
	joint = (const dgContact *)contactJoint;

	TRACE_FUNTION(__FUNCTION__);

	if ((joint->GetId() == dgContactConstraintId) && joint->GetCount()) {
		return joint->GetCount();
	} else {
		return 0;
	}
}

// Name: NewtonContactJointGetFirstContact
// Return to the next contact from the cantact array of the contact joint.
//
// Parameters:
// *const NewtonJoint* *contactJoint - pointer to corrent contact joint.
//
// Return: first contact contact array of the joint contact exist, NULL otherwise
//
// See also: NewtonContactJointGetNextContact, NewtonContactGetMaterial, NewtonContactJointRemoveContact
void *NewtonContactJointGetFirstContact(const NewtonJoint *const contactJoint) {
	const dgContact *joint;
	joint = (const dgContact *)contactJoint;

	TRACE_FUNTION(__FUNCTION__);

	if ((joint->GetId() == dgContactConstraintId) && joint->GetCount()) {
		return joint->GetFirst();
	} else {
		return NULL;
	}
}

// Name: NewtonContactJointGetNextContact
// Return to the first contact fromm the cantact array of the contact joint.
//
// Parameters:
// *const NewtonJoint* *contactJoint - pointer to corrent contact joint.
// *void* *contact - pointer to current contact.
//
// Return: a handle to the next contact contact in the contact array if contact exist, NULL otherwise.
//
// See also: NewtonContactJointGetFirstContact, NewtonContactGetMaterial, NewtonContactJointRemoveContact
void *NewtonContactJointGetNextContact(const NewtonJoint *const contactJoint,
                                       void *const contact) {
	const dgContact *joint;
	joint = (const dgContact *)contactJoint;

	TRACE_FUNTION(__FUNCTION__);

	if ((joint->GetId() == dgContactConstraintId) && joint->GetCount()) {
		dgList<dgContactMaterial>::dgListNode *node;
		node = (dgList<dgContactMaterial>::dgListNode *)contact;
		return node->GetNext();
	} else {
		return NULL;
	}
}

// Name: NewtonContactJointRemoveContact
// Return to the next contact from the cantact array of the contact joint.
//
// Parameters:
// *const NewtonJoint* *contactJoint - pointer to corrent contact joint.
//
// Return: first contact contact array of the joint contact exist, NULL otherwise
//
// See also: NewtonBodyGetFirstContactJoint, NewtonBodyGetNextContactJoint, NewtonContactJointGetFirstContact, NewtonContactJointGetNextContact
void NewtonContactJointRemoveContact(NewtonJoint *contactJoint,
                                     void *const contact) {
	dgContact *joint;
	joint = (dgContact *)contactJoint;

	TRACE_FUNTION(__FUNCTION__);

	if ((joint->GetId() == dgContactConstraintId) && joint->GetCount()) {
		dgList<dgContactMaterial>::dgListNode *node;
		node = (dgList<dgContactMaterial>::dgListNode *)contact;
		joint->Remove(node);
	}
}

// Name: NewtonContactGetMaterial
// Return to the next contact from the cantact array of the contact joint.
//
// Parameters:
// *const NewtonJoint* *contactJoint - pointer to corrent contact joint.
//
// Return: first contact contact array of the joint contact exist, NULL otherwise
//
// See also: NewtonContactJointGetFirstContact, NewtonContactJointGetNextContact
NewtonMaterial *NewtonContactGetMaterial(void *const contact) {
	dgList<dgContactMaterial>::dgListNode *node;

	TRACE_FUNTION(__FUNCTION__);

	node = (dgList<dgContactMaterial>::dgListNode *)contact;
	dgContactMaterial &contactMaterial = node->GetInfo();
	return (NewtonMaterial *)&contactMaterial;
}

// Name: NewtonBodyAddBuoyancyForce
// Add buoyancy force and torque for bodies immersed in a fluid.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *dFloat* fluidDensity - fluid density.
// *dFloat* fluidLinearViscosity - fluid linear viscosity (resistance to linear translation).
// *dFloat* fluidAngularViscosity - fluid angular viscosity (resistance to rotation).
// *const dFloat* *gravityVector - pointer to an array of floats containing the gravity vector.
// *NewtonGetBuoyancyPlane* *buoyancyPlane - pointer to an array of at least 4 floats containing the plane equation of the surface of the fluid. This parameter can be NULL
//
// Return: Nothing.
//
// Remarks: This function is only effective when called from *NewtonApplyForceAndTorque callback*
//
// Remarks: This function adds buoyancy force and torque to a body when it is immersed in a fluid.
// The force is calculated according to Archimedes� Buoyancy Principle. When the parameter *buoyancyPlane* is set to NULL, the body is considered
// to completely immersed in the fluid. This can be used to simulate boats and lighter than air vehicles etc..
//
// Remarks: If *buoyancyPlane* return 0 buoyancy calculation for this collision primitive is ignored, this could be used to filter buoyancy calculation
// of compound collision geometry with different IDs.
//
// See also: NewtonConvexCollisionCalculateVolume
void NewtonBodyAddBuoyancyForce(NewtonBody *const bodyPtr,
                                dFloat fluidDensity, dFloat fluidLinearViscosity,
                                dFloat fluidAngularViscosity, const dFloat *const gravityVector,
                                NewtonGetBuoyancyPlane buoyancyPlane, void *const context) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector gravity(gravityVector[0], gravityVector[1], gravityVector[2],
	                 dgFloat32(0.0f));
	body->AddBuoyancyForce(fluidDensity, fluidLinearViscosity,
	                       fluidAngularViscosity, gravity, (GetBuoyancyPlane)buoyancyPlane,
	                       context);
}

// Name: NewtonBodySetCollision
// Assign a collision primitive to the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *const collisionPtr* *collisionPtr - pointer to the new collision geometry.
//
// Return: Nothing.
//
// Remarks: This function replaces a collision geometry of a body with the new collision geometry.
// This function increments the reference count of the collision geometry and decrements the reference count
// of the old collision geometry. If the reference count of the old collision geometry reaches zero, the old collision geometry is destroyed.
// This function can be used to swap the collision geometry of bodies at runtime.
//
// See also: NewtonCreateBody, NewtonBodyGetCollision
void NewtonBodySetCollision(NewtonBody *bodyPtr,
                            NewtonCollision *collisionPtr) {
	dgBody *body;
	dgCollision *collision;

	TRACE_FUNTION(__FUNCTION__);
	body = (dgBody *)bodyPtr;
	collision = (dgCollision *)collisionPtr;
	body->AttachCollision(collision);
}

/*
 // Name: NewtonBodySetGyroscopicForcesMode
 // Enable or disable Coriolis and gyroscopic force calculation for this body.
 //
 // Parameters:
 // *const NewtonBody* *bodyPtr - pointer to the body.
 // *int* mode - force mode zero indicate not gyroscopic force calculation.
 //
 // Return: Nothing.
 //
 // Remarks: Gyroscopic forces internal forces generated as a result of an asymmetric tensor. They are a pure mathematical consequence that the physics have to comply in order to agree with the math. As Gyroscopic forces are not real forces but the result of net unbalance of the changing inertia tensor or a rigid body when its angular velocity is measured on a reference frame different than the body�s own.
 // Gyroscopic forces are extremely non linear by nature, therefore a first order implicit integrator will have a extremely hard time at dealing with this kind of forces, however because the fact that they are not real forces they do not make much difference in the outcome of the integration.
 // Fortunately due to the fact that the magnitude of gyroscopic forces is proportional to the unbalance of the inertia tensor, it is possible to disregard the effect of this forces by assuming their inertial tensor is symmetric for the purpose of this calculation. For most cases an ordinary person is not capable to distinguish the motion of a body subject to gyroscopic forces and one that is not, especially when the motion is constrained.
 // Because of this fact gyroscopic force are turned off by default in Newton, however there are cases when the desire effect is precisely to simulate these forces like a spinning top, or the design of a space navigational system, etc. The most important feature of gyroscopic forces is that they make the rigid body to process.
 void  NewtonBodySetGyroscopicForcesMode(const NewtonBody* const bodyPtr, int mode)
 {
 dgBody *body;

 body = (dgBody *)bodyPtr;
 body->SetGyroscopicTorqueMode (mode ? true : false);
 }


 // Name: NewtonBodyGetGyroscopicForcesMode
 // get a values indicating if Coriolis and gyroscopic force calculation for this body are enable.
 //
 // Parameters:
 // *const NewtonBody* *bodyPtr - pointer to the body.
 //
 // Return: force mode 1 means Gyro copy force are on.
 //
 // Remarks: Gyroscopic forces are internal forces generated as a result of an asymmetric tensor. They are a pure mathematical consequence that the physics have to comply in order to agree with the math. As Gyroscopic forces are not real forces but the result of net unbalance of the changing inertia tensor or a rigid body when its angular velocity is measured on a reference frame different than the body�s own.
 // Gyroscopic forces are extremely non linear by nature, therefore a first order implicit integrator will have a extremely hard time at dealing with this kind of forces, however because the fact that they are not real forces they do not make much difference in the outcome of the integration.
 // Fortunately due to the fact that the magnitude of gyroscopic forces is proportional to the unbalance of the inertia tensor, it is possible to disregard the effect of this forces by assuming their inertial tensor is symmetric for the purpose of this calculation. For most cases an ordinary person is not capable to distinguish the motion of a body subject to gyroscopic forces and one that is not, especially when the motion is constrained.
 // Because of this fact gyroscopic force are turned off by default in Newton, however there are cases when the desire effect is precisely to simulate these forces like a spinning top, or the design of a space navigational system, etc. The most important feature of gyroscopic forces is that they make the rigid body to process.
 int NewtonBodyGetGyroscopicForcesMode(const NewtonBody* const bodyPtr)
 {
 dgBody *body;
 body = (dgBody *)bodyPtr;
 return body->GetGyroscopicTorqueMode () ? 1 : 0;
 }
 */

// Name: NewtonBodyGetCollision
// Get the collision primitive of a body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
//
// Return: Pointer to body collision geometry.
//
// Remarks: This function does not increment the reference count of the collision geometry.
//
// See also: NewtonCreateBody, NewtonBodySetCollision
NewtonCollision *NewtonBodyGetCollision(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return (NewtonCollision *)body->GetCollision();
}

// Name: NewtonBodySetMaterialGroupID
// Assign a material group id to the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *int* id - id of a previously created material group.
//
// Return: Nothing.
//
// Remarks: When the application creates a body, the default material group, *defaultGroupId*, is applied by default.
//
// See also: NewtonBodyGetMaterialGroupID, NewtonMaterialCreateGroupID, NewtonMaterialGetDefaultGroupID
void NewtonBodySetMaterialGroupID(NewtonBody *bodyPtr, int id) {
	dgBody *const body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	body->SetGroupID(dgUnsigned32(id));
}

// Name: NewtonBodyGetMaterialGroupID
// Get the material group id of the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *int* id - id of a previously created material group.
//
// Return: Nothing.
//
// See also: NewtonBodySetMaterialGroupID
int NewtonBodyGetMaterialGroupID(const NewtonBody *const bodyPtr) {
	const dgBody *const body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return int(body->GetGroupID());
}

// Name: NewtonBodySetContinuousCollisionMode
// Set the continuous collision state mode for this rigid body.
// continue collision flag is off by default in when bodies are created.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *int* state - collision state. 1 indicates this body may tunnel through other objects while moving at high speed. 0 ignore high speed collision checks.
//
// Return: Nothing.
//
// Remarks: continue collision mode enable allow the engine to predict colliding contact on rigid bodies
// Moving at high speed of subject to strong forces.
//
// Remarks: continue collision mode does not prevent rigid bodies from inter penetration instead it prevent bodies from
// passing trough each others by extrapolating contact points when the bodies normal contact calculation determine the bodies are not colliding.
//
// Remarks: for performance reason the bodies angular velocities is only use on the broad face of the collision,
// but not on the contact calculation.
//
// Remarks: continue collision does not perform back tracking to determine time of contact, instead it extrapolate contact by incrementally
// extruding the collision geometries of the two colliding bodies along the linear velocity of the bodies during the time step,
// if during the extrusion colliding contact are found, a collision is declared and the normal contact resolution is called.
//
// Remarks: for continue collision to be active the continue collision mode must on the material pair of the colliding bodies as well as on at least one of the two colliding bodies.
//
// Remarks: Because there is penalty of about 40% to 80% depending of the shape complexity of the collision geometry, this feature is set
// off by default. It is the job of the application to determine what bodies need this feature on. Good guidelines are: very small objects,
// and bodies that move a height speed.
//
// See also: NewtonBodyGetContinuousCollisionMode, NewtonBodySetContinuousCollisionMode
void NewtonBodySetContinuousCollisionMode(NewtonBody *bodyPtr,
        unsigned state) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	body->SetContinuesCollisionMode(state ? true : false);
}

// Name: NewtonBodyGetContinuousCollisionMode
// Get the continuous collision state mode for this rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
//
// Return: Nothing.
//
// Remarks:
// Remark: Because there is there is penalty of about 3 to 5 depending of the shape complexity of the collision geometry, this feature is set
// off by default. It is the job of the application to determine what bodies need this feature on. Good guidelines are: very small objects,
// and bodies that move a height speed.
//
// Remark: this feature is currently disabled:
//
// See also: NewtonBodySetContinuousCollisionMode, NewtonBodySetContinuousCollisionMode
int NewtonBodyGetContinuousCollisionMode(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return body->GetContinuesCollisionMode() ? 1 : false;
}

// Name: NewtonBodySetJointRecursiveCollision
// Set the collision state flag of this body when the body is connected to another body by a hierarchy of joints.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
// *int* state - collision state. 1 indicates this body will collide with any linked body. 0 disable collision with body connected to this one by joints.
//
// Return: Nothing.
//
// Remarks: sometimes when making complicated arrangements of linked bodies it is possible the collision geometry of these bodies is in the way of the
// joints work space. This could be a problem for the normal operation of the joints. When this situation happens the application can determine which bodies
// are the problem and disable collision for those bodies while they are linked by joints. For the collision to be disable for a pair of body,
// both bodies must have the collision disabled. If the joints connecting the bodies are destroyed these bodies become collidable automatically.
// This feature can also be achieved by making special material for the whole configuration of jointed bodies, however it is a lot easier just to set collision disable
// for jointed bodies.
//
// See also: NewtonBodySetMaterialGroupID
void NewtonBodySetJointRecursiveCollision(NewtonBody *bodyPtr,
        unsigned state) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	body->SetCollisionWithLinkedBodies(state ? true : false);
}

// Name: NewtonBodyGetJointRecursiveCollision
// Get the collision state flag when the body is joint.
//
// Parameters:
// *const NewtonBody* *bodyPtr - pointer to the body.
//
// Return: return the collision state flag for this body.
//
// See also: NewtonBodySetMaterialGroupID
int NewtonBodyGetJointRecursiveCollision(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return body->GetCollisionWithLinkedBodies() ? 1 : 0;
}

// Name: NewtonBodyGetFreezeState
// get the freeze state of this body
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body to be frozen
//
// Return: 1 id the bode is frozen, 0 if bode is unfrozen.
//
// Remarks: When a body is created it is automatically placed in the active simulation list. As an optimization
// for large scenes, you may use this function to put background bodies in an inactive equilibrium state.
//
// Remarks: This function tells Newton that this body does not currently need to be simulated.
// However, if the body is part of a larger configuration it may be affected indirectly by the reaction forces
// of objects that it is connected to.
//
// See also: NewtonBodySetAutoSleep, NewtonBodyGetAutoSleep
int NewtonBodyGetFreezeState(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return body->GetFreeze() ? 1 : 0;
}

// Name: NewtonBodySetFreezeState
// This function tells Newton to simulate or suspend simulation of this body and all other bodies in contact with it
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body to be activated
// *int* state - 1 teels newton to freeze the bode and allconceted bodiesm, 0 to unfreze it
//
// Return: Nothing
//
// Remarks: This function to no activate the body, is just lock or unlock the body for physics simulation.
//
// See also: NewtonBodyGetFreezeState, NewtonBodySetAutoSleep, NewtonBodyGetAutoSleep
void NewtonBodySetFreezeState(NewtonBody *bodyPtr, int state) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	body->SetFreeze(state ? true : false);
}

// Name: NewtonBodySetAutoSleep
// Set the auto-activation mode for this body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
// *int* state - active mode: 1 = auto-activation on (controlled by Newton). 0 = auto-activation off and body is active all the time.
//
// Return: Nothing.
//
// Remarks: Bodies are created with auto-activation on by default.
//
// Remarks: Auto activation enabled is the default state for the majority of bodies in a large scene.
// However, for player control, ai control or some other special circumstance, the application may want to control
// the activation/deactivation of the body.
// In that case, the application may call NewtonBodySetAutoSleep (body, 0) followed by
// NewtonBodySetFreezeState(body), this will make the body active forever.
//
// See also: NewtonBodyGetFreezeState, NewtonBodySetFreezeState, NewtonBodyGetAutoSleep, NewtonBodySetFreezeTreshold
void NewtonBodySetAutoSleep(NewtonBody *bodyPtr, int state) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	//  body->SetAutoSleep (state ? false : true);
	body->SetAutoSleep(state ? true : false);
}

// Name: NewtonBodyGetAutoSleep
// Get the auto-activation state of the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
//
// Return: Auto activation state: 1 = auto-activation on. 0 = auto-activation off.
//
// See also: NewtonBodySetAutoSleep, NewtonBodyGetSleepState
int NewtonBodyGetAutoSleep(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return body->GetAutoSleep() ? 1 : 0;
}

// Name: NewtonBodyGetSleepState
// Return the sleep mode of a rigid body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
//
// Return: Sleep state: 1 = active. 0 = sleeping.
//
// See also: NewtonBodySetAutoSleep
int NewtonBodyGetSleepState(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;
	return body->GetSleepState() ? 1 : 0;
}

/*
 // Name: NewtonBodySetFreezeTreshold
 // Set the minimum values for velocity of a body that will be considered at rest.
 //
 // Parameters:
 // *const NewtonBody* *bodyPtr - is the pointer to the body.
 // *dFloat* freezeSpeedMag2 - magnitude squared of the velocity threshold.
 // *dFloat* freezeOmegaMag2 - magnitude squared of angular velocity threshold.
 // *int* framesCount - number of frames the body velocity and angular will not exceed freezeSpeedMag and freezeOmegaMag.
 //
 // Remarks: Ideally, a body should be deactivated when it reaches a state of stable equilibrium. However, because of floating point
 // inaccuracy, discrete time step simulation and other factors it is virtually impossible for a body to reach that state
 // in a real-time simulation. Therefore, in the Newton World, a body is considered to be in stable equilibrium when its
 // velocity and angular velocity fall below some threshold for a consecutive number of frames.
 //
 // Remarks: The default and minimum values for the thresholds is 0.01 for speed and 10 for frames count.
 // These values are tuned for single objects colliding under the influence of gravity. It is possible that for complex configuration
 // of bodies like multiples pendulums, rag dolls, etc. these values may need to be increased. This is because joints have the property that they
 // add a small amount of energy to the system in order to reduce the separation error. This may cause the bodies reach a state of unstable
 // equilibrium. That is, when a body oscillates between two different positions because the energy added to the body is equal to the energy
 // dissipated by the integrator. This is a situation that is hard to predict, and the best solution is to tweak these values for specific cases.
 //
 // See also: NewtonBodySetAutoSleep, NewtonBodyGetFreezeTreshold
 void NewtonBodySetFreezeTreshold(const NewtonBody* const bodyPtr, dFloat freezeSpeedMag2, dFloat freezeOmegaMag2, int framesCount)
 {
 // dFloat alpha;
 // dFloat accel;
 dgBody *body;

 body = (dgBody *)bodyPtr;
 // framesCount =  GetMin (framesCount * 2, 30);
 // alpha = 60.0f * dgSqrt (freezeOmegaMag2) / framesCount;
 // accel = 60.0f * dgSqrt (freezeSpeedMag2) / framesCount;
 // body->SetFreezeTreshhold (accel * accel, alpha * alpha, freezeSpeedMag2, freezeOmegaMag2);
 // dFloat scale;
 // slace = dgFloat32(1.0f) / (framesCount

 body->SetFreezeTreshhold (freezeSpeedMag2, freezeOmegaMag2, freezeSpeedMag2 * dgFloat32(0.1f), freezeOmegaMag2 * dgFloat32(0.1f));
 }

 // Name: NewtonBodyGetFreezeTreshold
 // Get the minimum values for velocity of a body the will be considered at rest.
 //
 // Parameters:
 // *const NewtonBody* *bodyPtr - is the pointer to the body.
 // *dFloat* freezeSpeedMag2 - point the to a dFloat to hold the velocity threshold
 // *dFloat* freezeOmegaMag2 - point the to a dFloat to hold the angular velocity threshold
 //
 // See also: NewtonBodySetFreezeTreshold
 void NewtonBodyGetFreezeTreshold(const NewtonBody* const bodyPtr, dFloat* freezeSpeedMag2, dFloat* freezeOmegaMag2)
 {
 dgBody *body;
 dFloat alpha;
 dFloat accel;

 body = (dgBody *)bodyPtr;
 body->GetFreezeTreshhold (accel, alpha, *freezeSpeedMag2, *freezeOmegaMag2);

 freezeSpeedMag2[0] *= 10.0f;
 freezeOmegaMag2[0] *= 10.0f;
 }
 */

// Name: NewtonBodyGetAABB
// Get the world axis aligned bounding box (AABB) of the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
// *dFloat* *p0 - pointer to an array of at least three floats to hold minimum value for the AABB.
// *dFloat* *p1 - pointer to an array of at least three floats to hold maximum value for the AABB.
//
void NewtonBodyGetAABB(const NewtonBody *const bodyPtr, dFloat *const p0,
                       dFloat *const p1) {
	const dgBody *body;

	dgVector vector0;
	dgVector vector1;

	TRACE_FUNTION(__FUNCTION__);
	body = (const dgBody *)bodyPtr;
	body->GetAABB(vector0, vector1);

	p0[0] = vector0.m_x;
	p0[1] = vector0.m_y;
	p0[2] = vector0.m_z;

	p1[0] = vector1.m_x;
	p1[1] = vector1.m_y;
	p1[2] = vector1.m_z;
}

// Name: NewtonBodySetVelocity
// Set the global linear velocity of the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
// *const dFloat* *velocity - pointer to an array of at least three floats containing the velocity vector.
//
// See also: NewtonBodyGetVelocity
void NewtonBodySetVelocity(NewtonBody *bodyPtr,
                           const dFloat *const velocity) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(velocity[0], velocity[1], velocity[2], dgFloat32(0.0f));
	body->SetVelocity(vector);
}

// Name: NewtonBodyGetVelocity
// Get the global linear velocity of the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
// *const dFloat* *velocity - pointer to an array of at least three floats to hold the velocity vector.
//
// See also: NewtonBodySetVelocity
void NewtonBodyGetVelocity(const NewtonBody *const bodyPtr,
                           dFloat *const velocity) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	//  dgVector& vector = *((dgVector*) velocity);
	//  vector = body->GetVelocity();

	dgVector vector(body->GetVelocity());
	velocity[0] = vector.m_x;
	velocity[1] = vector.m_y;
	velocity[2] = vector.m_z;
}

// Name: NewtonBodySetOmega
// Set the global angular velocity of the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
// *const dFloat* *omega - pointer to an array of at least three floats containing the angular velocity vector.
//
// See also: NewtonBodyGetOmega
void NewtonBodySetOmega(NewtonBody *bodyPtr,
                        const dFloat *const omega) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(omega[0], omega[1], omega[2], dgFloat32(0.0f));
	body->SetOmega(vector);
}

// Name: NewtonBodyGetOmega
// Get the global angular velocity of the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body
// *dFloat* *omega - pointer to an array of at least three floats to hold the angular velocity vector.
//
// See also: NewtonBodySetOmega
void NewtonBodyGetOmega(const NewtonBody *const bodyPtr, dFloat *const omega) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	//  dgVector& vector = *((dgVector*) omega);
	//  vector = body->GetOmega();

	dgVector vector(body->GetOmega());
	omega[0] = vector.m_x;
	omega[1] = vector.m_y;
	omega[2] = vector.m_z;
}

// Name: NewtonBodySetLinearDamping
// Apply the linear viscous damping coefficient to the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
// *dFloat* linearDamp - linear damping coefficient.
//
// Remarks: the default value of *linearDamp* is clamped to a value between 0.0 and 1.0; the default value is 0.1,
// There is a non zero implicit attenuation value of 0.0001 assume by the integrator.
//
// Remarks: The dampening viscous friction force is added to the external force applied to the body every frame before going to the solver-integrator.
// This force is proportional to the square of the magnitude of the velocity to the body in the opposite direction of the velocity of the body.
// An application can set *linearDamp* to zero when the application takes control of the external forces and torque applied to the body, should the application
// desire to have absolute control of the forces over that body. However, it is recommended that the *linearDamp* coefficient is set to a non-zero
// value for the majority of background bodies. This saves the application from having to control these forces and also prevents the integrator from
// adding very large velocities to a body.
//
// See also: NewtonBodyGetLinearDamping
void NewtonBodySetLinearDamping(NewtonBody *bodyPtr,
                                dFloat linearDamp) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	body->SetLinearDamping(linearDamp);
}

// Name: NewtonBodyGetLinearDamping
// Get the linear viscous damping of the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
//
// Return: The linear damping coefficient.
//
// See also: NewtonBodySetLinearDamping
dFloat NewtonBodyGetLinearDamping(const NewtonBody *const bodyPtr) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	return body->GetLinearDamping();
}

// Name: NewtonBodySetAngularDamping
// Apply the angular viscous damping coefficient to the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
// *dFloat* *angularDamp - pointer to an array of at least three floats containing the angular damping coefficients for the principal axis of the body.
//
// Remarks: the default value of *angularDamp* is clamped to a value between 0.0 and 1.0; the default value is 0.1,
// There is a non zero implicit attenuation value of 0.0001 assumed by the integrator.
//
// Remarks: The dampening viscous friction torque is added to the external torque applied to the body every frame before going to the solver-integrator.
// This torque is proportional to the square of the magnitude of the angular velocity to the body in the opposite direction of the angular velocity of the body.
// An application can set *angularDamp* to zero when the to take control of the external forces and torque applied to the body, should the application
// desire to have absolute control of the forces over that body. However, it is recommended that the *linearDamp* coefficient be set to a non-zero
// value for the majority of background bodies. This saves the application from needing to control these forces and also prevents the integrator from
// adding very large velocities to a body.
//
// See also: NewtonBodyGetAngularDamping
void NewtonBodySetAngularDamping(NewtonBody *bodyPtr,
                                 const dFloat *angularDamp) {
	dgBody *body;
	body = (dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	dgVector vector(angularDamp[0], angularDamp[1], angularDamp[2],
	                dgFloat32(0.0f));
	body->SetAngularDamping(vector);
}

// Name: NewtonBodyGetAngularDamping
// Get the linear viscous damping of the body.
//
// Parameters:
// *const NewtonBody* *bodyPtr - is the pointer to the body.
// *dFloat* *angularDamp - pointer to an array of at least three floats to hold the angular damping coefficient for the principal axis of the body.
//
// See also: NewtonBodySetAngularDamping
void NewtonBodyGetAngularDamping(const NewtonBody *const bodyPtr,
                                 dFloat *angularDamp) {
	const dgBody *body;
	body = (const dgBody *)bodyPtr;

	TRACE_FUNTION(__FUNCTION__);
	//  dgVector& vector = *((dgVector*) angularDamp);
	//  vector = body->GetAngularDamping();

	dgVector vector(body->GetAngularDamping());
	angularDamp[0] = vector.m_x;
	angularDamp[1] = vector.m_y;
	angularDamp[2] = vector.m_z;
}

/*
 // Name: NewtonBodyForEachPolygonDo
 // Iterate thought polygon of the collision geometry of a body calling the function callback.
 //
 // Parameters:
 // *const NewtonBody* *bodyPtr - is the pointer to the body.
 // *NewtonCollisionIterator* callback - application define callback
 //
 // Return: nothing
 //
 // Remarks: This function can be called by the application in order to show the collision geometry. The application should provide a pointer to the function *NewtonCollisionIterator*,
 // Newton will convert the collision geometry into a polygonal mesh, and will call *callback* for every polygon of the mesh
 //
 // Remarks: this function affect severely the performance of Newton. The application should call this function only for debugging purpose
 //
 // Remarks: This function will ignore user define collision mesh
 // See also: NewtonWorldGetFirstBody, NewtonWorldForEachBodyInAABBDo, NewtonCollisionForEachPolygonDo
 void NewtonBodyForEachPolygonDo(const NewtonBody* const bodyPtr, NewtonCollisionIterator callback)
 {
 dgBody *body;
 void *saveCallBack;
 dgCollision *collision;
 dgCollisionPolygonalSoup *treeCollision;

 body = (dgBody *) bodyPtr;
 collision = body->GetCollision();
 if (collision->IsType (dgCollision::dgCollisionPolygonalSoup_RTTI)) {
 treeCollision = (dgCollisionPolygonalSoup*) collision;
 saveCallBack = treeCollision->GetCallBack();
 if (saveCallBack == NewtonCollisionTree::GetIntersectingPolygons) {
 treeCollision->SetCallBack ((void*)NewtonCollisionTree::IteratePolygonMesh);
 collision->DebugCollision (*body, (DebugCollisionMeshCallback) callback);
 treeCollision->SetCallBack (saveCallBack);
 }
 } else {
 collision->DebugCollision (*body, (DebugCollisionMeshCallback) callback);
 }
 }
 */

// Name: NewtonBodyAddImpulse
// Add an impulse to a specific point on a body.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - pointer to the Newton world.
// *const NewtonBody* *bodyPtr - is the pointer to the body.
// *const dFloat* pointDeltaVeloc - pointer to an array of at least three floats containing the desired change in velocity to point pointPosit.
// *const dFloat* pointPosit    - pointer to an array of at least three floats containing the center of the impulse in global space.
//
// Return: Nothing.
//
// Remarks: This function will activate the body.
//
// Remarks: *pointPosit* and *pointDeltaVeloc* must be specified in global space.
//
// Remarks: *pointDeltaVeloc* represent a change in velocity. For example, a value of *pointDeltaVeloc* of (1, 0, 0) changes the velocity
// of *bodyPtr* in such a way that the velocity of point *pointDeltaVeloc* will increase by (1, 0, 0)
//
// Remarks: Because *pointDeltaVeloc* represents a change in velocity, this function must be used with care. Repeated calls
// to this function will result in an increase of the velocity of the body and may cause to integrator to lose stability.
void NewtonBodyAddImpulse(NewtonBody *bodyPtr,
                          const dFloat *const pointDeltaVeloc, const dFloat *const pointPosit) {
	dgBody *body;
	dgWorld *world;

	TRACE_FUNTION(__FUNCTION__);
	body = (dgBody *)bodyPtr;
	world = body->GetWorld();

	dgVector p(pointPosit);
	dgVector v(pointDeltaVeloc);

	world->AddBodyImpulse(body, v, p);
}

// Name: NewtonBodyAddImpulse
// Add an train of impulses to a specific point on a body.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - pointer to the Newton world.
// *const NewtonBody* *bodyPtr - is the pointer to the body.
// int impulseCount - number of impulses and distances in the array distance
// int strideInByte - sized in bytes of vector impulse and
// *const dFloat* impulseArray - pointer to an array containing the desired impulse to apply ate psoition pointarray.
// *const dFloat* pointArray    - pointer to an array of at least three floats containing the center of the impulse in global space.
//
// Return: Nothing.
//
// Remarks: This function will activate the body.
//
// Remarks: *pointPosit* and *pointDeltaVeloc* must be specified in global space.
//
//
// Remarks: this function apply at general impulse to a body a oppose to a desired change on velocity
// this mean that the body mass, and Inertia will determine the gain on velocity.
void NewtonBodyApplyImpulseArray(NewtonBody *bodyPtr,
                                 int impuleCount, int strideInByte, const dFloat *const impulseArray,
                                 const dFloat *const pointArray) {
	dgBody *body;
	dgWorld *world;

	TRACE_FUNTION(__FUNCTION__);
	body = (dgBody *)bodyPtr;
	world = body->GetWorld();

	world->ApplyImpulseArray(body, impuleCount, strideInByte, impulseArray,
	                         pointArray);
}

// ***************************************************************************************************************
//
// Name: Ball and Socket joint interface
//
// ***************************************************************************************************************

// Name: NewtonConstraintCreateBall
// Create a ball an socket joint.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonCollision* *pivotPoint - is origin of ball and socket in global space.
// *const NewtonBody* *childBody - is the pointer to the attached rigid body, this body can not be NULL or it can not have an infinity (zero) mass.
// *const NewtonBody* *parentBody - is the pointer to the parent rigid body, this body can be NULL or any kind of rigid body.
//
// Return: Pointer to the ball and socket joint.
//
// Remarks: This function creates a ball and socket and add it to the world. By default joint disables collision with the linked bodies.
NewtonJoint *NewtonConstraintCreateBall(NewtonWorld *newtonWorld,
                                        const dFloat *pivotPoint, NewtonBody *childBody,
                                        NewtonBody *parentBody) {
	dgBody *body0;
	dgBody *body1;
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	body0 = (dgBody *)childBody;
	body1 = (dgBody *)parentBody;
	dgVector pivot(pivotPoint[0], pivotPoint[1], pivotPoint[2], dgFloat32(0.0f));
	return (NewtonJoint *)world->CreateBallConstraint(pivot, body0, body1);
}

// Name: NewtonBallSetConeLimits
// Set the ball and socket cone and twist limits.
//
// Parameters:
// *const NewtonJoint* *ball - is the pointer to a ball and socket joint.
// *const NewtonCollision* *pin - pointer to a unit vector defining the cone axis in global space.
// *const dFloat* maxConeAngle - max angle in radians the attached body is allow to swing relative to the pin axis, a value of zero will disable this limits.
// *const dFloat* maxTwistAngle - max angle in radians the attached body is allow to twist relative to the pin axis, a value of zero will disable this limits.
//
// Remarks: limits are disabled at creation time. A value of zero for *maxConeAngle* disable the cone limit, a value of zero for *maxTwistAngle* disable the twist limit
// all non-zero value for *maxConeAngle* are clamped between 5 degree and 175 degrees
//
// See also: NewtonConstraintCreateBall
void NewtonBallSetConeLimits(NewtonJoint *ball, const dFloat *pin,
                             dFloat maxConeAngle, dFloat maxTwistAngle) {
	dgBallConstraint *joint;

	TRACE_FUNTION(__FUNCTION__);
	joint = (dgBallConstraint *)ball;

	dgVector coneAxis(pin[0], pin[1], pin[2], dgFloat32(0.0f));

	if ((coneAxis % coneAxis) < 1.0e-3f) {
		coneAxis.m_x = dgFloat32(1.0f);
	}
	dgVector tmp(dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	             dgFloat32(0.0f));
	if (dgAbsf(tmp % coneAxis) > dgFloat32(0.999f)) {
		tmp = dgVector(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f),
		               dgFloat32(0.0f));
		if (dgAbsf(tmp % coneAxis) > dgFloat32(0.999f)) {
			tmp = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f),
			               dgFloat32(0.0f));
			NEWTON_ASSERT(dgAbsf(tmp % coneAxis) < dgFloat32(0.999f));
		}
	}
	dgVector lateral(tmp * coneAxis);
	lateral = lateral.Scale(dgRsqrt(lateral % lateral));
	coneAxis = coneAxis.Scale(dgRsqrt(coneAxis % coneAxis));

	maxConeAngle = dgAbsf(maxConeAngle);
	maxTwistAngle = dgAbsf(maxTwistAngle);
	joint->SetConeLimitState((maxConeAngle > dgDEG2RAD) ? true : false);
	joint->SetTwistLimitState((maxTwistAngle > dgDEG2RAD) ? true : false);
	joint->SetLatealLimitState(false);
	joint->SetLimits(coneAxis, -maxConeAngle, maxConeAngle, maxTwistAngle,
	                 lateral, 0.0f, 0.0f);
}

// Name: NewtonBallSetUserCallback
// Set an update call back to be called when either of the two bodies linked by the joint is active.
//
// Parameters:
// *const NewtonJoint* *ball - pointer to the joint.
// *NewtonBallCallBack* callback - pointer to the joint function call back.
//
// Return: nothing.
//
// Remarks: if the application wants to have some feedback from the joint simulation, the application can register a function
// update callback to be called every time any of the bodies linked by this joint is active. This is useful to provide special
// effects like particles, sound or even to simulate breakable moving parts.
//
// See also: NewtonJointSetUserData
void NewtonBallSetUserCallback(NewtonJoint *ball,
                               NewtonBallCallBack callback) {
	dgBallConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (dgBallConstraint *)ball;
	contraint->SetJointParameterCallBack((dgBallJointFriction)callback);
}

// Name: NewtonBallGetJointAngle
// Get the relative joint angle between the two bodies.
//
// Parameters:
// *const NewtonJoint* *ball - pointer to the joint.
// *dFloat* *angle - pointer to an array of a least three floats to hold the joint relative Euler angles.
//
// Return: nothing.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play a bell sound when the joint angle passes some max value.
//
// See also: NewtonBallSetUserCallback
void NewtonBallGetJointAngle(const NewtonJoint *const ball, dFloat *angle) {
	const dgBallConstraint *contraint;

	contraint = (const dgBallConstraint *)ball;
	dgVector angleVector(contraint->GetJointAngle());

	TRACE_FUNTION(__FUNCTION__);
	angle[0] = angleVector.m_x;
	angle[1] = angleVector.m_y;
	angle[2] = angleVector.m_z;
}

// Name: NewtonBallGetJointOmega
// Get the relative joint angular velocity between the two bodies.
//
// Parameters:
// *const NewtonJoint* *ball - pointer to the joint.
// *dFloat* *omega - pointer to an array of a least three floats to hold the joint relative angular velocity.
//
// Return: nothing.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play the creaky noise of a hanging lamp.
//
// See also: NewtonBallSetUserCallback
void NewtonBallGetJointOmega(const NewtonJoint *const ball, dFloat *omega) {
	const dgBallConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgBallConstraint *)ball;
	dgVector omegaVector(contraint->GetJointOmega());
	omega[0] = omegaVector.m_x;
	omega[1] = omegaVector.m_y;
	omega[2] = omegaVector.m_z;
}

// Name: NewtonBallGetJointForce
// Get the total force asserted over the joint pivot point, to maintain the constraint.
//
// Parameters:
// *const NewtonJoint* *ball - pointer to the joint.
// *dFloat* *force - pointer to an array of a least three floats to hold the force value of the joint.
//
// Return: nothing.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can destroy the joint if the force exceeds some predefined value.
//
// See also: NewtonBallSetUserCallback
void NewtonBallGetJointForce(const NewtonJoint *const ball, dFloat *const force) {
	const dgBallConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgBallConstraint *)ball;
	dgVector forceVector(contraint->GetJointForce());
	force[0] = forceVector.m_x;
	force[1] = forceVector.m_y;
	force[2] = forceVector.m_z;
}

// ***************************************************************************************************************
//
// Name: Hinge joint interface
//
// ***************************************************************************************************************

// Name:  NewtonConstraintCreateHinge
// Create a hinge joint.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const dFloat* *pivotPoint - is origin of the hinge in global space.
// *const dFloat* *pinDir - is the line of action of the hinge in global space.
// *const NewtonBody* *childBody - is the pointer to the attached rigid body, this body can not be NULL or it can not have an infinity (zero) mass.
// *const NewtonBody* *parentBody - is the pointer to the parent rigid body, this body can be NULL or any kind of rigid body.
//
// Return: Pointer to the hinge joint.
//
// Remarks: This function creates a hinge and add it to the world. By default joint disables collision with the linked bodies.
NewtonJoint *NewtonConstraintCreateHinge(NewtonWorld *newtonWorld,
        const dFloat *pivotPoint, const dFloat *pinDir,
        NewtonBody *childBody, NewtonBody *parentBody) {
	dgBody *body0;
	dgBody *body1;
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	body0 = (dgBody *)childBody;
	body1 = (dgBody *)parentBody;
	dgVector pivot(pivotPoint[0], pivotPoint[1], pivotPoint[2], dgFloat32(0.0f));
	dgVector pin(pinDir[0], pinDir[1], pinDir[2], dgFloat32(0.0f));
	return (NewtonJoint *)world->CreateHingeConstraint(pivot, pin, body0, body1);
}

// Name: NewtonHingeSetUserCallback
// Set an update call back to be called when either of the two body linked by the joint is active.
//
// Parameters:
// *const NewtonJoint* *Hinge - pointer to the joint.
// *NewtonHingeCallBack* callback - pointer to the joint function call back.
//
// Return: nothing.
//
// Remarks: if the application wants to have some feedback from the joint simulation, the application can register a function
// update callback to be call every time any of the bodies linked by this joint is active. This is useful to provide special
// effects like particles, sound or even to simulate breakable moving parts.
//
// See also: NewtonJointGetUserData, NewtonJointSetUserData
void NewtonHingeSetUserCallback(NewtonJoint *hinge,
                                NewtonHingeCallBack callback) {
	dgHingeConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (dgHingeConstraint *)hinge;
	contraint->SetJointParameterCallBack((dgHingeJointAcceleration)callback);
}

// Name: NewtonHingeGetJointAngle
// Get the relative joint angle between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Hinge - pointer to the joint.
//
// Return: the joint angle relative to the hinge pin.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play a bell sound when the joint angle passes some max value.
//
// See also: NewtonHingeSetUserCallback
dFloat NewtonHingeGetJointAngle(const NewtonJoint *const hinge) {
	const dgHingeConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgHingeConstraint *)hinge;
	return contraint->GetJointAngle();
}

// Name: NewtonHingeGetJointOmega
// Get the relative joint angular velocity between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Hinge - pointer to the joint.
//
// Return: the joint angular velocity relative to the pin axis.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play the creaky noise of a hanging lamp.
//
// See also: NewtonHingeSetUserCallback
dFloat NewtonHingeGetJointOmega(const NewtonJoint *const hinge) {
	const dgHingeConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgHingeConstraint *)hinge;
	return contraint->GetJointOmega();
}

// Name: NewtonHingeGetJointForce
// Calculate the angular acceleration needed to stop the hinge at the desired angle.
//
// Parameters:
// *const NewtonJoint* *Hinge - pointer to the joint.
// *NewtonHingeSliderUpdateDesc* *desc - is the pointer to and the Hinge or slide structure.
// *dFloat* angle - is the desired hinge stop angle
//
// Return: the relative angular acceleration needed to stop the hinge.
//
// Remarks: this function can only be called from a *NewtonHingeCallBack* and it can be used by the application to implement hinge limits.
//
// See also: NewtonHingeSetUserCallback
dFloat NewtonHingeCalculateStopAlpha(const NewtonJoint *const hinge,
                                     const NewtonHingeSliderUpdateDesc *const desc, dFloat angle) {
	const dgHingeConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgHingeConstraint *)hinge;
	return contraint->CalculateStopAlpha(angle, (const dgJointCallBackParam *)desc);
}

// Name: NewtonHingeGetJointForce
// Get the total force asserted over the joint pivot point, to maintain the constraint.
//
// Parameters:
// *const NewtonJoint* *Hinge - pointer to the joint.
// *dFloat* *force - pointer to an array of a least three floats to hold the force value of the joint.
//
// Return: nothing.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can destroy the joint if the force exceeds some predefined value.
//
// See also: NewtonHingeSetUserCallback
void NewtonHingeGetJointForce(const NewtonJoint *const hinge,
                              dFloat *const force) {
	const dgHingeConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgHingeConstraint *)hinge;
	dgVector forceVector(contraint->GetJointForce());
	force[0] = forceVector.m_x;
	force[1] = forceVector.m_y;
	force[2] = forceVector.m_z;
}

// ***************************************************************************************************************
//
// Name: Slider joint interface
//
// ***************************************************************************************************************

// Name: NewtonConstraintCreateSlider
// Create a slider joint.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const dFloat* *pivotPoint - is origin of the slider in global space.
// *const dFloat* *pinDir - is the line of action of the slider in global space.
// *const NewtonBody* *childBody - is the pointer to the attached rigid body, this body can not be NULL or it can not have an infinity (zero) mass.
// *const NewtonBody* *parentBody - is the pointer to the parent rigid body, this body can be NULL or any kind of rigid body.
//
// Return: Pointer to the slider joint.
//
// Remarks: This function creates a slider and add it to the world. By default joint disables collision with the linked bodies.
NewtonJoint *NewtonConstraintCreateSlider(NewtonWorld *newtonWorld,
        const dFloat *pivotPoint, const dFloat *pinDir,
        NewtonBody *childBody, NewtonBody *parentBody) {
	dgBody *body0;
	dgBody *body1;
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	body0 = (dgBody *)childBody;
	body1 = (dgBody *)parentBody;
	dgVector pin(pinDir[0], pinDir[1], pinDir[2], dgFloat32(0.0f));
	dgVector pivot(pivotPoint[0], pivotPoint[1], pivotPoint[2], dgFloat32(0.0f));
	return (NewtonJoint *)world->CreateSlidingConstraint(pivot, pin, body0, body1);
}

// Name: NewtonSliderSetUserCallback
// Set an update call back to be called when either of the two body linked by the joint is active.
//
// Parameters:
// *const NewtonJoint* *Slider - pointer to the joint.
// *NewtonSliderCallBack* callback - pointer to the joint function call back.
//
// Return: nothing.
//
// Remarks: if the application wants to have some feedback from the joint simulation, the application can register a function
// update callback to be call every time any of the bodies linked by this joint is active. This is useful to provide special
// effects like particles, sound or even to simulate breakable moving parts.
//
// See also: NewtonJointGetUserData, NewtonJointSetUserData
void NewtonSliderSetUserCallback(NewtonJoint *slider,
                                 NewtonSliderCallBack callback) {
	dgSlidingConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (dgSlidingConstraint *)slider;
	contraint->SetJointParameterCallBack((dgSlidingJointAcceleration)callback);
}

// Name: NewtonSliderGetJointPosit
// Get the relative joint angle between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Slider - pointer to the joint.
//
// Return: the joint angle relative to the hinge pin.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play a bell sound when the joint angle passes some max value.
//
// See also: NewtonSliderSetUserCallback
dFloat NewtonSliderGetJointPosit(const NewtonJoint *Slider) {
	const dgSlidingConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgSlidingConstraint *)Slider;
	return contraint->GetJointPosit();
}

// Name: NewtonSliderGetJointVeloc
// Get the relative joint angular velocity between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Slider - pointer to the joint.
//
// Return: the joint angular velocity relative to the pin axis.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play the creaky noise of a hanging lamp.
//
// See also: NewtonSliderSetUserCallback
dFloat NewtonSliderGetJointVeloc(const NewtonJoint *Slider) {
	const dgSlidingConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgSlidingConstraint *)Slider;
	return contraint->GetJointVeloc();
}

// Name: NewtonSliderGetJointForce
// Calculate the angular acceleration needed to stop the slider at the desired angle.
//
// Parameters:
// *const NewtonJoint* *slider - pointer to the joint.
// *NewtonSliderSliderUpdateDesc* *desc - is the pointer to the Slider or slide structure.
// *dFloat* distance - desired stop distance relative to the pivot point
//
// Return: the relative linear acceleration needed to stop the slider.
//
// Remarks: this function can only be called from a *NewtonSliderCallBack* and it can be used by the application to implement slider limits.
//
// See also: NewtonSliderSetUserCallback
dFloat NewtonSliderCalculateStopAccel(const NewtonJoint *const slider,
                                      const NewtonHingeSliderUpdateDesc *const desc, dFloat distance) {
	const dgSlidingConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgSlidingConstraint *)slider;
	return contraint->CalculateStopAccel(distance, (const dgJointCallBackParam *)desc);
}

// Name: NewtonSliderGetJointForce
// Get the total force asserted over the joint pivot point, to maintain the constraint.
//
// Parameters:
// *const NewtonJoint* *Slider - pointer to the joint.
// *dFloat* *force - pointer to an array of a least three floats to hold the force value of the joint.
//
// Return: nothing.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can destroy the joint if the force exceeds some predefined value.
//
// See also: NewtonSliderSetUserCallback
void NewtonSliderGetJointForce(const NewtonJoint *const slider,
                               dFloat *const force) {
	const dgSlidingConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgSlidingConstraint *)slider;
	dgVector forceVector(contraint->GetJointForce());
	force[0] = forceVector.m_x;
	force[1] = forceVector.m_y;
	force[2] = forceVector.m_z;
}

// ***************************************************************************************************************
//
// Name: Corkscrew joint interface
//
// ***************************************************************************************************************

// Name: NewtonConstraintCreateCorkscrew
// Create a corkscrew joint.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const dFloat* *pivotPoint - is origin of the corkscrew in global space.
// *const dFloat* *pinDir - is the line of action of the corkscrew in global space.
// *const NewtonBody* *childBody - is the pointer to the attached rigid body, this body can not be NULL or it can not have an infinity (zero) mass.
// *const NewtonBody* *parentBody - is the pointer to the parent rigid body, this body can be NULL or any kind of rigid body.
//
// Return: Pointer to the corkscrew joint.
//
// Remarks: This function creates a corkscrew and add it to the world. By default joint disables collision with the linked bodies.
NewtonJoint *NewtonConstraintCreateCorkscrew(
    NewtonWorld *newtonWorld, const dFloat *pivotPoint,
    const dFloat *pinDir, NewtonBody *childBody,
    NewtonBody *parentBody) {
	dgBody *body0;
	dgBody *body1;
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	body0 = (dgBody *)childBody;
	body1 = (dgBody *)parentBody;
	dgVector pin(pinDir[0], pinDir[1], pinDir[2], dgFloat32(0.0f));
	dgVector pivot(pivotPoint[0], pivotPoint[1], pivotPoint[2], dgFloat32(0.0f));
	return (NewtonJoint *)world->CreateCorkscrewConstraint(pivot, pin, body0,
	        body1);
}

// Name: NewtonCorkscrewSetUserCallback
// Set an update call back to be called when either of the two body linked by the joint is active.
//
// Parameters:
// *const NewtonJoint* *Corkscrew - pointer to the joint.
// *NewtonCorkscrewCallBack* callback - pointer to the joint function call back.
//
// Return: nothing.
//
// Remarks: if the application wants to have some feedback from the joint simulation, the application can register a function
// update callback to be call every time any of the bodies linked by this joint is active. This is useful to provide special
// effects like particles, sound or even to simulate breakable moving parts.
//
// Remarks: the function *NewtonCorkscrewCallBack callback* should return a bit field code.
// if the application does not want to set the joint acceleration the return code is zero
// if the application only wants to change the joint linear acceleration the return code is 1
// if the application only wants to change the joint angular acceleration the return code is 2
// if the application only wants to change the joint angular and linear acceleration the return code is 3
//
// See also: NewtonJointGetUserData, NewtonJointSetUserData
void NewtonCorkscrewSetUserCallback(NewtonJoint *corkscrew,
                                    NewtonCorkscrewCallBack callback) {
	dgCorkscrewConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (dgCorkscrewConstraint *)corkscrew;
	contraint->SetJointParameterCallBack((dgCorkscrewJointAcceleration)callback);
}

// Name: NewtonCorkscrewGetJointPosit
// Get the relative joint angle between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Corkscrew - pointer to the joint.
//
// Return: the joint angle relative to the hinge pin.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play a bell sound when the joint angle passes some max value.
//
// See also: NewtonCorkscrewSetUserCallback
dFloat NewtonCorkscrewGetJointPosit(const NewtonJoint *const corkscrew) {
	const dgCorkscrewConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgCorkscrewConstraint *)corkscrew;
	return contraint->GetJointPosit();
}

// Name: NewtonCorkscrewGetJointVeloc
// Get the relative joint angular velocity between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Corkscrew - pointer to the joint.
//
// Return: the joint angular velocity relative to the pin axis.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play the creaky noise of a hanging lamp.
//
// See also: NewtonCorkscrewSetUserCallback
dFloat NewtonCorkscrewGetJointVeloc(const NewtonJoint *const corkscrew) {
	const dgCorkscrewConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgCorkscrewConstraint *)corkscrew;
	return contraint->GetJointVeloc();
}

// Name: NewtonCorkscrewGetJointAngle
// Get the relative joint angle between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Corkscrew - pointer to the joint.
//
// Return: the joint angle relative to the corkscrew pin.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play a bell sound when the joint angle passes some max value.
//
// See also: NewtonCorkscrewSetUserCallback
dFloat NewtonCorkscrewGetJointAngle(const NewtonJoint *const corkscrew) {
	const dgCorkscrewConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgCorkscrewConstraint *)corkscrew;
	return contraint->GetJointAngle();
}

// Name: NewtonCorkscrewGetJointOmega
// Get the relative joint angular velocity between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Corkscrew - pointer to the joint.
//
// Return: the joint angular velocity relative to the pin axis.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play the creaky noise of a hanging lamp.
//
// See also: NewtonCorkscrewSetUserCallback
dFloat NewtonCorkscrewGetJointOmega(const NewtonJoint *const corkscrew) {
	const dgCorkscrewConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgCorkscrewConstraint *)corkscrew;
	return contraint->GetJointOmega();
}

// Name: NewtonCorkscrewCalculateStopAlpha
// Calculate the angular acceleration needed to stop the corkscrew at the desired angle.
//
// Parameters:
// *const NewtonJoint* *Corkscrew - pointer to the joint.
// *NewtonCorkscrewSliderUpdateDesc* *desc - is the pointer to the Corkscrew or slide structure.
// *dFloat* angle - is the desired corkscrew stop angle
//
// Return: the relative angular acceleration needed to stop the corkscrew.
//
// Remarks: this function can only be called from a *NewtonCorkscrewCallBack* and it can be used by the application to implement corkscrew limits.
//
// See also: NewtonCorkscrewSetUserCallback
dFloat NewtonCorkscrewCalculateStopAlpha(const NewtonJoint *const corkscrew,
        const NewtonHingeSliderUpdateDesc *const desc, dFloat angle) {
	const dgCorkscrewConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgCorkscrewConstraint *)corkscrew;
	return contraint->CalculateStopAlpha(angle, (const dgJointCallBackParam *)desc);
}

// Name: NewtonCorkscrewGetJointForce
// Calculate the angular acceleration needed to stop the corkscrew at the desired angle.
//
// Parameters:
// *const NewtonJoint* *corkscrew - pointer to the joint.
// *NewtonCorkscrewCorkscrewUpdateDesc* *desc - is the pointer to the Corkscrew or slide structure.
// *dFloat* distance - desired stop distance relative to the pivot point
//
// Return: the relative linear acceleration needed to stop the corkscrew.
//
// Remarks: this function can only be called from a *NewtonCorkscrewCallBack* and it can be used by the application to implement corkscrew limits.
//
// See also: NewtonCorkscrewSetUserCallback
dFloat NewtonCorkscrewCalculateStopAccel(const NewtonJoint *const corkscrew,
        const NewtonHingeSliderUpdateDesc *const desc, dFloat distance) {
	const dgCorkscrewConstraint *contraint;
	contraint = (const dgCorkscrewConstraint *)corkscrew;
	return contraint->CalculateStopAccel(distance, (const dgJointCallBackParam *)desc);
}

// Name: NewtonCorkscrewGetJointForce
// Get the total force asserted over the joint pivot point, to maintain the constraint.
//
// Parameters:
// *const NewtonJoint* *Corkscrew - pointer to the joint.
// *dFloat* *force - pointer to an array of a least three floats to hold the force value of the joint.
//
// Return: nothing.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can destroy the joint if the force exceeds some predefined value.
//
// See also: NewtonCorkscrewSetUserCallback
void NewtonCorkscrewGetJointForce(const NewtonJoint *const corkscrew,
                                  dFloat *const force) {
	const dgCorkscrewConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgCorkscrewConstraint *)corkscrew;
	dgVector forceVector(contraint->GetJointForce());
	force[0] = forceVector.m_x;
	force[1] = forceVector.m_y;
	force[2] = forceVector.m_z;
}

// ***************************************************************************************************************
//
// Name: Universal joint interface
//
// ***************************************************************************************************************

// Name: NewtonConstraintCreateUniversal
// Create a universal joint.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const dFloat* *pivotPoint - is origin of the universal joint in global space.
// *const dFloat* *pinDir0 - first axis of rotation fixed on childBody body and perpendicular to pinDir1.
// *const dFloat* *pinDir1 - second axis of rotation fixed on parentBody body and perpendicular to pinDir0.
// *const NewtonBody* *childBody - is the pointer to the attached rigid body, this body can not be NULL or it can not have an infinity (zero) mass.
// *const NewtonBody* *parentBody - is the pointer to the parent rigid body, this body can be NULL or any kind of rigid body.
//
// Return: Pointer to the universal joint.
//
// Remarks: This function creates a universal joint and add it to the world. By default joint disables collision with the linked bodies.
//
// Remark: a universal joint is a constraint that restricts twp rigid bodies to be connected to a point fixed on both bodies,
// while and allowing one body to spin around a fix axis in is own frame, and the other body to spin around another axis fixes on
// it own frame. Both axis must be mutually perpendicular.
NewtonJoint *NewtonConstraintCreateUniversal(
    NewtonWorld *newtonWorld, const dFloat *pivotPoint,
    const dFloat *pinDir0, const dFloat *pinDir1,
    NewtonBody *childBody, NewtonBody *parentBody) {
	dgBody *body0;
	dgBody *body1;
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	body0 = (dgBody *)childBody;
	body1 = (dgBody *)parentBody;
	dgVector pin0(pinDir0[0], pinDir0[1], pinDir0[2], dgFloat32(0.0f));
	dgVector pin1(pinDir1[0], pinDir1[1], pinDir1[2], dgFloat32(0.0f));
	dgVector pivot(pivotPoint[0], pivotPoint[1], pivotPoint[2], dgFloat32(0.0f));
	return (NewtonJoint *)world->CreateUniversalConstraint(pivot, pin0, pin1,
	        body0, body1);
}

// Name: NewtonUniversalSetUserCallback
// Set an update call back to be called when either of the two body linked by the joint is active.
//
// Parameters:
// *const NewtonJoint* *Universal - pointer to the joint.
// *NewtonUniversalCallBack* callback - pointer to the joint function call back.
//
// Return: nothing.
//
// Remarks: if the application wants to have some feedback from the joint simulation, the application can register a function
// update callback to be called every time any of the bodies linked by this joint is active. This is useful to provide special
// effects like particles, sound or even to simulate breakable moving parts.
//
// Remarks: the function *NewtonUniversalCallBack callback* should return a bit field code.
// if the application does not want to set the joint acceleration the return code is zero
// if the application only wants to change the joint linear acceleration the return code is 1
// if the application only wants to change the joint angular acceleration the return code is 2
// if the application only wants to change the joint angular and linear acceleration the return code is 3
//
// See also: NewtonJointGetUserData, NewtonJointSetUserData
void NewtonUniversalSetUserCallback(NewtonJoint *universal,
                                    NewtonUniversalCallBack callback) {
	dgUniversalConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (dgUniversalConstraint *)universal;
	contraint->SetJointParameterCallBack((dgUniversalJointAcceleration)callback);
}

// Name: NewtonUniversalGetJointAngle0
// Get the relative joint angle between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Universal - pointer to the joint.
//
// Return: the joint angle relative to the universal pin0.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play a bell sound when the joint angle passes some max value.
//
// See also: NewtonUniversalSetUserCallback
dFloat NewtonUniversalGetJointAngle0(const NewtonJoint *const universal) {
	const dgUniversalConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgUniversalConstraint *)universal;
	return contraint->GetJointAngle0();
}

// Name: NewtonUniversalGetJointAngle1
// Get the relative joint angle between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Universal - pointer to the joint.
//
// Return: the joint angle relative to the universal pin1.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play a bell sound when the joint angle passes some max value.
//
// See also: NewtonUniversalSetUserCallback
dFloat NewtonUniversalGetJointAngle1(const NewtonJoint *const universal) {
	const dgUniversalConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgUniversalConstraint *)universal;
	return contraint->GetJointAngle1();
}

// Name: NewtonUniversalGetJointOmega0
// Get the relative joint angular velocity between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Universal - pointer to the joint.
//
// Return: the joint angular velocity relative to the pin0 axis.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play the creaky noise of a hanging lamp.
//
// See also: NewtonUniversalSetUserCallback
dFloat NewtonUniversalGetJointOmega0(const NewtonJoint *const universal) {
	const dgUniversalConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgUniversalConstraint *)universal;
	return contraint->GetJointOmega0();
}

// Name: NewtonUniversalGetJointOmega1
// Get the relative joint angular velocity between the two bodies.
//
// Parameters:
// *const NewtonJoint* *Universal - pointer to the joint.
//
// Return: the joint angular velocity relative to the pin1 axis.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can play the creaky noise of a hanging lamp.
//
// See also: NewtonUniversalSetUserCallback
dFloat NewtonUniversalGetJointOmega1(const NewtonJoint *const universal) {
	const dgUniversalConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgUniversalConstraint *)universal;
	return contraint->GetJointOmega1();
}

// Name: NewtonUniversalCalculateStopAlpha0
// Calculate the angular acceleration needed to stop the universal at the desired angle.
//
// Parameters:
// *const NewtonJoint* *Universal - pointer to the joint.
// *NewtonUniversalSliderUpdateDesc* *desc - is the pointer to the Universal or slide structure.
// *dFloat* angle - is the desired universal stop angle rotation around pin0
//
// Return: the relative angular acceleration needed to stop the universal.
//
// Remarks: this function can only be called from a *NewtonUniversalCallBack* and it can be used by the application to implement universal limits.
//
// See also: NewtonUniversalSetUserCallback
dFloat NewtonUniversalCalculateStopAlpha0(const NewtonJoint *const universal,
        const NewtonHingeSliderUpdateDesc *const desc, dFloat angle) {
	const dgUniversalConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgUniversalConstraint *)universal;
	return contraint->CalculateStopAlpha0(angle, (const dgJointCallBackParam *)desc);
}

// Name: NewtonUniversalCalculateStopAlpha1
// Calculate the angular acceleration needed to stop the universal at the desired angle.
//
// Parameters:
// *const NewtonJoint* *Universal - pointer to the joint.
// *NewtonUniversalSliderUpdateDesc* *desc - is the pointer to and the Universal or slide structure.
// *dFloat* angle - is the desired universal stop angle rotation around pin1
//
// Return: the relative angular acceleration needed to stop the universal.
//
// Remarks: this function can only be called from a *NewtonUniversalCallBack* and it can be used by the application to implement universal limits.
//
// See also: NewtonUniversalSetUserCallback
dFloat NewtonUniversalCalculateStopAlpha1(const NewtonJoint *const universal,
        const NewtonHingeSliderUpdateDesc *const desc, dFloat angle) {
	const dgUniversalConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgUniversalConstraint *)universal;
	return contraint->CalculateStopAlpha1(angle, (const dgJointCallBackParam *)desc);
}

// Name: NewtonUniversalGetJointForce
// Get the total force asserted over the joint pivot point, to maintain the constraint.
//
// Parameters:
// *const NewtonJoint* *Universal - pointer to the joint.
// *dFloat* *force - pointer to an array of a least three floats to hold the force value of the joint.
//
// Return: nothing.
//
// Remarks: this function can be used during a function update call back to provide the application with some special effect.
// for example the application can destroy the joint if the force exceeds some predefined value.
//
// See also: NewtonUniversalSetUserCallback
void NewtonUniversalGetJointForce(const NewtonJoint *const universal,
                                  dFloat *const force) {
	const dgUniversalConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgUniversalConstraint *)universal;
	dgVector forceVector(contraint->GetJointForce());
	force[0] = forceVector.m_x;
	force[1] = forceVector.m_y;
	force[2] = forceVector.m_z;
}

// ***************************************************************************************************************
//
// Name: UpVector joint Interface
//
// ***************************************************************************************************************

// Name: NewtonConstraintCreateUpVector
// Create a UpVector joint.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const dFloat* *pinDir - is the aligning vector.
// *const NewtonBody* *body - is the pointer to the attached rigid body, this body can not be NULL or it can not have an infinity (zero) mass.
//
// Return: Pointer to the up vector joint.
//
// Remarks: This function creates an up vector joint. An up vector joint is a constraint that allows a body to translate freely in 3d space,
// but it only allows the body to rotate around the pin direction vector. This could be use by the application to control a character
// with physics and collision.
//
// Remark: Since the UpVector joint is a unary constraint, there is not need to have user callback or user data assigned to it.
// The application can simple hold to the joint handle and update the pin on the force callback function of the rigid body owning the joint.
NewtonJoint *NewtonConstraintCreateUpVector(
    NewtonWorld *newtonWorld, const dFloat *pinDir,
    NewtonBody *body) {
	dgBody *body0;
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	body0 = (dgBody *)body;
	dgVector pin(pinDir[0], pinDir[1], pinDir[2], dgFloat32(0.0f));
	return (NewtonJoint *)world->CreateUpVectorConstraint(pin, body0);
}

// Name: NewtonUpVectorGetPin
// Get the up vector pin of this joint in global space.
//
// Parameters:
// *const NewtonJoint* *upVector - pointer to the joint.
// *dFloat* *pin - pointer to an array of a least three floats to hold the up vector direction in global space.
//
// Return: nothing.
//
// Remarks: the application ca call this function to read the up vector, this is useful to animate the up vector.
// if the application is going to animated the up vector, it must do so by applying only small rotation,
// too large rotation can cause vibration of the joint.
//
// See also: NewtonUpVectorSetUserCallback, NewtonUpVectorSetPin
void NewtonUpVectorGetPin(const NewtonJoint *const upVector, dFloat *pin) {
	const dgUpVectorConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (const dgUpVectorConstraint *)upVector;

	dgVector pinVector(contraint->GetPinDir());
	pin[0] = pinVector.m_x;
	pin[1] = pinVector.m_y;
	pin[2] = pinVector.m_z;
}

// Name: NewtonUpVectorSetPin
// Set the up vector pin of this joint in global space.
//
// Parameters:
// *const NewtonJoint* *upVector - pointer to the joint.
// *dFloat* *pin - pointer to an array of a least three floats containing the up vector direction in global space.
//
// Return: nothing.
//
// Remarks: the application ca call this function to change the joint up vector, this is useful to animate the up vector.
// if the application is going to animated the up vector, it must do so by applying only small rotation,
// too large rotation can cause vibration of the joint.
//
// See also: NewtonUpVectorSetUserCallback, NewtonUpVectorGetPin
void NewtonUpVectorSetPin(NewtonJoint *upVector, const dFloat *pin) {
	dgUpVectorConstraint *contraint;

	TRACE_FUNTION(__FUNCTION__);
	contraint = (dgUpVectorConstraint *)upVector;

	dgVector pinVector(pin[0], pin[1], pin[2], dgFloat32(0.0f));
	contraint->SetPinDir(pinVector);
}

// ***************************************************************************************************************
//
// Name: User defined joint interface
//
// ***************************************************************************************************************

// Name: NewtonConstraintCreateUserJoint
// Create a user define bilateral joint.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *in*t maxDOF - is the maximum number of degree of freedom controlled by this joint.
// *NewtonUserBilateralCallBack* submitConstraints - pointer to the joint constraint definition function call back.
// *NewtonUserBilateralGetInfoCallBack* getInfo - pointer to callback for collecting joint information.
// *const NewtonBody* *childBody - is the pointer to the attached rigid body, this body can not be NULL or it can not have an infinity (zero) mass.
// *const NewtonBody* *parentBody - is the pointer to the parent rigid body, this body can be NULL or any kind of rigid body.
//
// Remark: Bilateral joint are constraints that can have up to 6 degree of freedoms, 3 linear and 3 angular.
// By restricting the motion along any number of these degree of freedom a very large number of useful joint between
// two rigid bodies can be accomplished. Some of the degree of freedoms restriction makes no sense, and also some
// combinations are so rare that only make sense to a very specific application, the Newton engine implements the more
// commons combinations like, hinges, ball and socket, etc. However if and application is in the situation that any of
// the provided joints can achieve the desired effect, then the application can design it own joint.
//
// Remark: User defined joint is a very advance feature that should be look at, only for very especial situations.
// The designer must be a person with a very good understanding of constrained dynamics, and it may be the case
// that many trial have to be made before a good result can be accomplished.
//
// Remark: function *submitConstraints* is called before the solver state to get the jacobian derivatives and the righ hand acceleration
// for the definition of the constraint.
//
// Remark: maxDOF is and upper bound as to how many degrees of freedoms the joint can control, usually this value
// can be 6 for bilateral joints, but it can be higher for special joints like vehicles where by the used of friction clamping
// the number of rows can be higher.
// In general the application should determine maxDof correctly, passing an unnecessary excessive value will lead to performance decreased.
//
// See also: NewtonUserJointSetFeedbackCollectorCallback
NewtonJoint *NewtonConstraintCreateUserJoint(
    NewtonWorld *newtonWorld, int maxDOF,
    NewtonUserBilateralCallBack submitConstraints,
    NewtonUserBilateralGetInfoCallBack getInfo,
    NewtonBody *childBody, NewtonBody *parentBody) {
	dgBody *body0;
	dgBody *body1;
	Newton *world;

	TRACE_FUNTION(__FUNCTION__);
	world = (Newton *)newtonWorld;
	body0 = (dgBody *)childBody;
	body1 = (dgBody *)parentBody;

	return (NewtonJoint *)new (world->dgWorld::GetAllocator()) NewtonUserJoint(
	           world, maxDOF, submitConstraints, getInfo, body0, body1);
}

// Name: NewtonUserJointAddLinearRow
// Add a linear restricted degree of freedom.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *const dFloat* *pivot0 - pointer of a vector in global space fixed on body zero.
// *const dFloat* *pivot1 - pointer of a vector in global space fixed on body one.
// *const dFloat* *pin - pointer of a unit vector in global space along which the relative position, velocity and acceleration between the bodies will be driven to zero.
//
// Remark: A linear constraint row calculates the Jacobian derivatives and relative acceleration required to enforce the constraint condition at
// the attachment point and the pin direction considered fixed to both bodies.
//
// Remark: The acceleration is calculated such that the relative linear motion between the two points is zero, the application can
// afterward override this value to create motors.
//
// Remark: after this function is call and internal DOF index will point to the current row entry in the constraint matrix.
//
// Remark: This function call only be called from inside a *NewtonUserBilateralCallBack* callback.
//
// See also: NewtonUserJointAddAngularRow,
void NewtonUserJointAddLinearRow(NewtonJoint *joint,
                                 const dFloat *const pivot0, const dFloat *const pivot1,
                                 const dFloat *const dir) {
	NewtonUserJoint *userJoint;
	userJoint = (NewtonUserJoint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	dgVector direction(dir[0], dir[1], dir[2], dgFloat32(0.0f));
	direction = direction.Scale(dgRsqrt(direction % direction));
	NEWTON_ASSERT(
	    dgAbsf(direction % direction - dgFloat32(1.0f)) < dgFloat32(1.0e-2f));
	dgVector pivotPoint0(pivot0[0], pivot0[1], pivot0[2], dgFloat32(0.0f));
	dgVector pivotPoint1(pivot1[0], pivot1[1], pivot1[2], dgFloat32(0.0f));

	userJoint->AddLinearRowJacobian(pivotPoint0, pivotPoint1, direction);
}

// Name: NewtonUserJointAddAngularRow
// Add an angular restricted degree of freedom.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *dFloat* relativeAngleError - relative angle error between both bodies around pin axis.
// *const dFloat* *pin - pointer of a unit vector in global space along which the relative position, velocity and acceleration between the bodies will be driven to zero.
//
// Remark: An angular constraint row calculates the Jacobian derivatives and relative acceleration required to enforce the constraint condition at
// pin direction considered fixed to both bodies.
//
// Remark: The acceleration is calculated such that the relative angular motion between the two points is zero, The application can
// afterward override this value to create motors.
//
// Remark: After this function is called and internal DOF index will point to the current row entry in the constraint matrix.
//
// Remark: This function call only be called from inside a *NewtonUserBilateralCallBack* callback.
//
// Remark: This function is of not practical to enforce hard constraints, but it is very useful for making angular motors.
//
// See also: NewtonUserJointAddLinearRow, NewtonUserJointAddIndependentAngularRow
void NewtonUserJointAddAngularRow(NewtonJoint *joint,
                                  dFloat relativeAngleError, const dFloat *const pin) {
	NewtonUserJoint *userJoint;

	TRACE_FUNTION(__FUNCTION__);
	userJoint = (NewtonUserJoint *)joint;
	dgVector direction(pin[0], pin[1], pin[2], dgFloat32(0.0f));
	direction = direction.Scale(dgRsqrt(direction % direction));
	NEWTON_ASSERT(
	    dgAbsf(direction % direction - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));

	userJoint->AddAngularRowJacobian(direction, relativeAngleError);
}

// Name: NewtonUserJointAddGeneralRow
// set the general linear and angular Jacobian for the desired degree of freedom
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *const dFloat* *jacobian0 - pointer of a set of six values defining the linear and angular Jacobian for body0.
// *const dFloat* *jacobian1 - pointer of a set of six values defining the linear and angular Jacobian for body1.
//
// Remark: In general this function must be used for very special effects and in combination with other joints.
// it is expected that the user have a knowledge of Constrained dynamics to make a good used of this function.
// Must typical application of this function are the creation of synchronization or control joints like gears, pulleys,
// worm gear and some other mechanical control.
//
// Remark: this function set the relative acceleration for this degree of freedom to zero. It is the
// application responsibility to set the relative acceleration after a call to this function
//
// See also: NewtonUserJointAddLinearRow, NewtonUserJointAddAngularRow
void NewtonUserJointAddGeneralRow(NewtonJoint *joint,
                                  const dFloat *const jacobian0, const dFloat *const jacobian1) {
	NewtonUserJoint *userJoint;

	TRACE_FUNTION(__FUNCTION__);
	userJoint = (NewtonUserJoint *)joint;
	userJoint->AddGeneralRowJacobian(jacobian0, jacobian1);
}

// Name: NewtonUserJointSetRowMaximumFriction
// Set the maximum friction value the solver is allow to apply to the joint row.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *dFloat* friction - maximum friction value for this row. It must be a positive value between 0.0 and INFINITY.
//
// Remark: This function will override the default friction values set after a call to NewtonUserJointAddLinearRow or NewtonUserJointAddAngularRow.
// friction value is context sensitive, if for linear constraint friction is a Max friction force, for angular constraint friction is a
// max friction is a Max friction torque.
//
// See also: NewtonUserJointSetRowMinimumFriction, NewtonUserJointAddLinearRow, NewtonUserJointAddAngularRow
void NewtonUserJointSetRowMaximumFriction(NewtonJoint *joint,
        dFloat friction) {
	NewtonUserJoint *userJoint;
	userJoint = (NewtonUserJoint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	userJoint->SetHighFriction(friction);
}

// Name: NewtonUserJointSetRowMinimumFriction
// Set the minimum friction value the solver is allow to apply to the joint row.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *dFloat* friction -  friction value for this row. It must be a negative value between 0.0 and -INFINITY.
//
// Remark: This function will override the default friction values set after a call to NewtonUserJointAddLinearRow or NewtonUserJointAddAngularRow.
// friction value is context sensitive, if for linear constraint friction is a Min friction force, for angular constraint friction is a
// friction is a Min friction torque.
//
// See also: NewtonUserJointSetRowMaximumFriction, NewtonUserJointAddLinearRow, NewtonUserJointAddAngularRow
void NewtonUserJointSetRowMinimumFriction(NewtonJoint *joint,
        dFloat friction) {
	NewtonUserJoint *userJoint;
	userJoint = (NewtonUserJoint *)joint;

	userJoint->SetLowerFriction(friction);
}

// Name: NewtonUserJointSetRowAcceleration
// Set the value for the desired acceleration for the current constraint row.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *dFloat* acceleration -  desired acceleration value for this row.
//
// Remark: This function will override the default acceleration values set after a call to NewtonUserJointAddLinearRow or NewtonUserJointAddAngularRow.
// friction value is context sensitive, if for linear constraint acceleration is a linear acceleration, for angular constraint acceleration is an
// angular acceleration.
//
// See also: NewtonUserJointAddLinearRow, NewtonUserJointAddAngularRow
void NewtonUserJointSetRowAcceleration(NewtonJoint *joint,
                                       dFloat acceleration) {
	NewtonUserJoint *userJoint;
	userJoint = (NewtonUserJoint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	userJoint->SetAcceleration(acceleration);
}

// Name: NewtonUserJointSetRowSpringDamperAcceleration
// Calculates the row acceleration to satisfy the specified the spring damper system.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *dFloat* springK - desired spring stiffness, it must be a positive value.
// *dFloat* springD - desired spring damper, it must be a positive value.
//
// Remark: This function will override the default acceleration values set after a call to NewtonUserJointAddLinearRow or NewtonUserJointAddAngularRow.
// friction value is context sensitive, if for linear constraint acceleration is a linear acceleration, for angular constraint acceleration is an
// angular acceleration.
//
// Remark: the acceleration calculated by this function represent the mass, spring system of the form
// a = -ks * x - kd * v.
//
// Remark: for this function to take place the joint stiffness must be set to a values lower than 1.0
//
// See also: NewtonUserJointSetRowAcceleration, NewtonUserJointSetRowStiffness
void NewtonUserJointSetRowSpringDamperAcceleration(
    NewtonJoint *joint, dFloat springK, dFloat springD) {
	NewtonUserJoint *userJoint;
	userJoint = (NewtonUserJoint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	userJoint->SetSpringDamperAcceleration(springK, springD);
}

// Name: NewtonUserJointSetRowStiffness
// Set the maximum percentage of the constraint force that will be applied to the constraint row.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *dFloat* stiffness - row stiffness, it must be a values between 0.0 and 1.0, the default is 0.9.
//
// Remark: This function will override the default stiffness value set after a call to NewtonUserJointAddLinearRow or NewtonUserJointAddAngularRow.
// the row stiffness is the percentage of the constraint force that will be applied to the rigid bodies. Ideally the value should be
// 1.0 (100% stiff) but dues to numerical integration error this could be the joint a little unstable, and lower values are preferred.
//
// See also: NewtonUserJointAddLinearRow, NewtonUserJointAddAngularRow, NewtonUserJointSetRowSpringDamperAcceleration
void NewtonUserJointSetRowStiffness(NewtonJoint *joint,
                                    dFloat stiffness) {
	NewtonUserJoint *userJoint;
	userJoint = (NewtonUserJoint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	userJoint->SetRowStiffness(stiffness);
}

// Name: NewtonUserJointGetRowForce
// Return the magnitude previews force or torque value calculated by the solver for this constraint row.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *int*  row - index to the constraint row.
//
// Remark: This function can be call for any of the previews row for this particular joint, The application must keep track of the meaning of the row.
//
// Remark: This function can be used to produce special effects like breakable or malleable joints, fro example a hinge can turn into ball and socket
// after the force in some of the row exceed  certain high value.
dFloat NewtonUserJointGetRowForce(const NewtonJoint *const joint, int row) {
	const NewtonUserJoint *userJoint;
	userJoint = (const NewtonUserJoint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	return userJoint->GetRowForce(row);
}

// Name: NewtonUserJointSetFeedbackCollectorCallback
// Set a constrain callback to collect the force calculated by the solver to enforce this constraint
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *NewtonUserBilateralCallBack* getFeedback - pointer to the joint constraint definition function call back.
//
// See also: NewtonUserJointGetRowForce
void NewtonUserJointSetFeedbackCollectorCallback(NewtonJoint *joint,
        NewtonUserBilateralCallBack getFeedback) {
	NewtonUserJoint *userJoint;
	userJoint = (NewtonUserJoint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	return userJoint->SetUpdateFeedbackFunction(getFeedback);
}

// ***************************************************************************************************************
//
// Name: Joint common function s
//
// ***************************************************************************************************************

// Name: NewtonJointSetUserData
// Store a user defined data value with the joint.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *void* *userDataPtr - pointer to the user defined user data value.
//
// Return: Nothing.
//
// Remarks: The application can store a user defined value with the Joint. This value can be the pointer to a structure containing some application data for special effect.
// if the application allocate some resource to store the user data, the application can register a joint destructor to get rid of the allocated resource when the Joint is destroyed
//
// See also: NewtonConstraintCreateJoint, NewtonJointSetDestructor
void NewtonJointSetUserData(NewtonJoint *joint,
                            void *const userData) {
	dgConstraint *contraint;

	contraint = (dgConstraint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	contraint->SetUserData(userData);
}

// Name: NewtonJointGetUserData
// Retrieve a user defined data value stored with the joint.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
//
// Return: The user defined data.
//
// Remarks: The application can store a user defined value with a joint. This value can be the pointer
// to a structure to store some game play data for special effect.
//
// See also: NewtonJointSetUserData
void *NewtonJointGetUserData(const NewtonJoint *const joint) {
	const dgConstraint *contraint;

	contraint = (const dgConstraint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	return contraint->GetUserData();
}

/*
 // Name: NewtonJointGetBody0
 // Retrieve the first rigid body controlled by this joint.
 //
 // Parameters:
 // *const NewtonJoint* *joint - pointer to the joint.
 //
 // Return: pointer the first body
 //
 // See also: NewtonJointGetBody1
 NewtonBody* NewtonJointGetBody0(const NewtonJoint* const joint)
 {
 dgBody* body;
 dgWorld* world;
 dgConstraint* contraint;
 contraint = (dgConstraint*) joint;

 body = contraint->GetBody0();
 world = body->GetWorld();
 if (body == world->GetSentinelBody()) {
 body = NULL;
 }
 return (NewtonBody*) body;
 }


 // Name: NewtonJointGetBody1
 // Retrieve the second rigid body controlled by this joint.
 //
 // Parameters:
 // *const NewtonJoint* *joint - pointer to the joint.
 //
 // Return: pointer the second body.
 //
 // See also: NewtonJointGetBody0
 NewtonBody* NewtonJointGetBody1(const NewtonJoint* const joint)
 {
 dgBody* body;
 dgWorld* world;
 dgConstraint* contraint;
 contraint = (dgConstraint*) joint;

 body = contraint->GetBody1();
 world = body->GetWorld();
 if (body == world->GetSentinelBody()) {
 body = NULL;
 }
 return (NewtonBody*) body;
 }
 */

// Name: NewtonJointGetInfo
// Get creation parameters for this joint.
//
// Parameters:
// *const NewtonJoint* joint - is the pointer to a convex collision primitive.
// *NewtonJointRecord* *jointInfo - pointer to a collision information record.
//
// Remarks: This function can be used by the application for writing file format and for serialization.
//
// See also:
void NewtonJointGetInfo(const NewtonJoint *const joint,
                        NewtonJointRecord *const jointInfo) {
	const dgConstraint *contraint;
	contraint = (const dgConstraint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	contraint->GetInfo((dgConstraintInfo *)jointInfo);
}

// Name: NewtonJointGetBody0
// Get the first body connected by this joint.
//
// Parameters:
// *const NewtonJoint* joint - is the pointer to a convex collision primitive.
//
//
// See also:
NewtonBody *NewtonJointGetBody0(const NewtonJoint *const joint) {

	const dgConstraint *contraint;
	contraint = (const dgConstraint *)joint;

	TRACE_FUNTION(__FUNCTION__);

	return (NewtonBody *)contraint->GetBody0();
}

// Name: NewtonJointGetBody1
// Get the secund body connected by this joint.
//
// Parameters:
// *const NewtonJoint* joint - is the pointer to a convex collision primitive.
//
// See also:
NewtonBody *NewtonJointGetBody1(const NewtonJoint *const joint) {
	dgBody *body;
	dgWorld *world;

	const dgConstraint *contraint;
	contraint = (const dgConstraint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	body = contraint->GetBody1();
	world = body->GetWorld();

	return (world->GetSentinelBody() != body) ? (NewtonBody *)body : NULL;
}

// Name: NewtonJointSetCollisionState
// Enable or disable collision between the two bodies linked by this joint. The default state is collision disable when the joint is created.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *int* state - collision state, zero mean disable collision, non zero enable collision between linked bodies.
//
// Return: nothing.
//
// Remarks: usually when two bodies are linked by a joint, the application wants collision between this two bodies to be disabled.
// This is the default behavior of joints when they are created, however when this behavior is not desired the application can change
// it by setting collision on. If the application decides to enable collision between jointed bodies, the application should make sure the
// collision geometry do not collide in the work space of the joint.
//
// Remarks: if the joint is destroyed the collision state of the two bodies linked by this joint is determined by the material pair assigned to each body.
//
// See also: NewtonJointGetCollisionState, NewtonBodySetJointRecursiveCollision
void NewtonJointSetCollisionState(NewtonJoint *joint, int state) {
	dgConstraint *contraint;

	contraint = (dgConstraint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	return contraint->SetCollidable(state ? true : false);
}

// Name: NewtonJointGetCollisionState
// Get the collision state of the two bodies linked by the joint.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
//
// Return: the collision state.
//
// Remarks: usually when two bodies are linked by a joint, the application wants collision between this two bodies to be disabled.
// This is the default behavior of joints when they are created, however when this behavior is not desired the application can change
// it by setting collision on. If the application decides to enable collision between jointed bodies, the application should make sure the
// collision geometry do not collide in the work space of the joint.
//
// See also: NewtonJointSetCollisionState
int NewtonJointGetCollisionState(const NewtonJoint *const joint) {
	const dgConstraint *contraint;
	contraint = (const dgConstraint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	return contraint->IsCollidable() ? 1 : 0;
}

// Name: NewtonJointSetStiffness
// Set the strength coefficient to be applied to the joint reaction forces.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *dFloat* stiffness - stiffness coefficient, a value between 0, and 1.0, the default value for most joint is 0.9
//
// Return: nothing.
//
// Remarks: Constraint keep bodies together by calculating the exact force necessary to cancel the relative acceleration between one or
// more common points fixed in the two bodies. The problem is that when the bodies drift apart due to numerical integration inaccuracies,
// the reaction force work to pull eliminated the error but at the expense of adding extra energy to the system, does violating the rule
// that constraint forces must be work less. This is a inevitable situation and the only think we can do is to minimize the effect of the
// extra energy by dampening the force by some amount. In essence the stiffness coefficient tell Newton calculate the precise reaction force
// by only apply a fraction of it to the joint point. And value of 1.0 will apply the exact force, and a value of zero will apply only
// 10 percent.
//
// Remark: The stiffness is set to a all around value that work well for most situation, however the application can play with these
// parameter to make finals adjustment. A high value will make the joint stronger but more prompt to vibration of instability; a low
// value will make the joint more stable but weaker.
//
// See also: NewtonJointGetStiffness
void NewtonJointSetStiffness(NewtonJoint *joint, dFloat stiffness) {
	dgConstraint *contraint;

	contraint = (dgConstraint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	contraint->SetStiffness(stiffness);
}

// Name: NewtonJointGetStiffness
// Get the strength coefficient bing applied to the joint reaction forces.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *dFloat* stiffness - stiffness coefficient, a value between 0, and 1.0, the default value for most joint is 0.9
//
// Return: stiffness coefficient.
//
// Remarks: Constraint keep bodies together by calculating the exact force necessary to cancel the relative acceleration between one or
// more common points fixed in the two bodies. The problem is that when the bodies drift apart due to numerical integration inaccuracies,
// the reaction force work to pull eliminated the error but at the expense of adding extra energy to the system, does violating the rule
// that constraint forces must be work less. This is a inevitable situation and the only think we can do is to minimize the effect of the
// extra energy by dampening the force by some amount. In essence the stiffness coefficient tell Newton calculate the precise reaction force
// by only apply a fraction of it to the joint point. And value of 1.0 will apply the exact force, and a value of zero will apply only
// 10 percent.
//
// Remark: The stiffness is set to a all around value that work well for most situation, however the application can play with these
// parameter to make finals adjustment. A high value will make the joint stronger but more prompt to vibration of instability; a low
// value will make the joint more stable but weaker.
//
// See also: NewtonJointSetStiffness
dFloat NewtonJointGetStiffness(const NewtonJoint *const joint) {
	const dgConstraint *contraint;

	contraint = (const dgConstraint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	return contraint->GetStiffness();
}

// Name: NewtonJointSetDestructor
// Register a destructor callback to be called when the joint is about to be destroyed.
//
// Parameters:
// *const NewtonJoint* *joint - pointer to the joint.
// *NewtonJointCallBack* destructor - pointer to the joint destructor callback.
//
// Return: nothing.
//
// Remarks: If application stores any resource with the joint, or the application wants to be notified when the
// joint is about to be destroyed. The application can register a destructor call back with the joint.
//
// See also: NewtonJointSetUserData
void NewtonJointSetDestructor(NewtonJoint *joint,
                              NewtonConstraintDestructor destructor) {
	dgConstraint *contraint;

	contraint = (dgConstraint *)joint;

	TRACE_FUNTION(__FUNCTION__);
	contraint->SetDestructorCallback((OnConstraintDestroy)destructor);
}

// Name: NewtonDestroyJoint
// destroy a joint.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the body.
// *const NewtonJoint* *joint - pointer to joint to be destroyed
//
// Return: nothing
//
// Remarks: The application can call this function when it wants to destroy a joint. This function can be used by the application to simulate
// breakable joints
//
// See also: NewtonConstraintCreateJoint,  NewtonConstraintCreateHinge, NewtonConstraintCreateSlider
void NewtonDestroyJoint(NewtonWorld *newtonWorld,
                        NewtonJoint *joint) {
	Newton *world;

	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	world->DestroyJoint((dgConstraint *)joint);
}

// ***************************************************************************************************************
//
// Name: Special effect mesh interface
//
// ***************************************************************************************************************

NewtonMesh *NewtonMeshCreate(NewtonWorld *newtonWorld) {
	TRACE_FUNTION(__FUNCTION__);

	Newton *world = (Newton *)newtonWorld;
	dgMeshEffect *mesh = new (world->dgWorld::GetAllocator()) dgMeshEffect(
	    world->dgWorld::GetAllocator(), true);
	return (NewtonMesh *)mesh;
}

NewtonMesh *NewtonMeshCreateFromMesh(NewtonMesh *mesh) {
	TRACE_FUNTION(__FUNCTION__);
	dgMeshEffect *srcMesh = (dgMeshEffect *)mesh;

	dgMeshEffect *clone = new (srcMesh->GetAllocator()) dgMeshEffect(*srcMesh);
	return (NewtonMesh *)clone;
}

NewtonMesh *NewtonMeshCreateFromCollision(NewtonCollision *collision) {
	TRACE_FUNTION(__FUNCTION__);

	dgCollision *const shape = (dgCollision *)collision;
	dgMeshEffect *const mesh = new (shape->GetAllocator()) dgMeshEffect(shape);
	return (NewtonMesh *)mesh;
}

NewtonMesh *NewtonMeshConvexHull(NewtonWorld *newtonWorld,
                                 int count, const dFloat *const vertexCloud, int strideInBytes,
                                 dFloat tolerance) {
	TRACE_FUNTION(__FUNCTION__);
	Newton *const world = (Newton *)newtonWorld;
	dgStack<dgBigVector> pool(count);

	dgInt32 stride = strideInBytes / sizeof(dgFloat32);
	for (dgInt32 i = 0; i < count; i++) {
		pool[i].m_x = vertexCloud[i * stride + 0];
		pool[i].m_y = vertexCloud[i * stride + 1];
		pool[i].m_z = vertexCloud[i * stride + 2];
		pool[i].m_w = dgFloat64(0.0);
	}
	dgMeshEffect *const mesh = new (world->dgWorld::GetAllocator()) dgMeshEffect(
	    world->dgWorld::GetAllocator(), &pool[0].m_x, count, sizeof(dgBigVector),
	    tolerance);
	return (NewtonMesh *)mesh;
}

/*
 NewtonMesh* NewtonMeshCreatePlane (const NewtonWorld* const newtonWorld, const dFloat* const locationMatrix, dFloat witdth, dFloat breadth, int material, const dFloat* const textureMatrix0, const dFloat* const textureMatrix1)
 {
 TRACE_FUNTION(__FUNCTION__);

 Newton* world = (Newton *) newtonWorld;
 dgMeshEffect* mesh = new (world->dgWorld::GetAllocator()) dgMeshEffect (world->dgWorld::GetAllocator(), *(dgMatrix*)locationMatrix, witdth, breadth, material, *(dgMatrix*)textureMatrix0, *(dgMatrix*)textureMatrix1);
 return (NewtonMesh*) mesh;
 }
 */

void NewtonMeshDestroy(const NewtonMesh *const mesh) {

	const dgMeshEffect *meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	delete meshEffect;
}

void NewtonMesApplyTransform(NewtonMesh *const mesh, dFloat *matrix) {
	TRACE_FUNTION(__FUNCTION__);
	dgMeshEffect *meshEffect = (dgMeshEffect *)mesh;

	dgMatrix transform(*((dgMatrix *)matrix));
	meshEffect->ApplyTransform(transform);
}

void NewtonMeshCalculateOOBB(const NewtonMesh *const mesh, dFloat *const matrix,
                             dFloat *const x, dFloat *const y, dFloat *const z) {
	const dgMeshEffect *meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	dgBigVector size;
	dgMatrix alignMatrix(meshEffect->CalculateOOBB(size));

	*((dgMatrix *)matrix) = alignMatrix;
	*x = dgFloat32(size.m_x);
	*y = dgFloat32(size.m_y);
	*z = dgFloat32(size.m_z);
}

void NewtonMeshCalculateVertexNormals(NewtonMesh *const mesh,
                                      dFloat angleInRadians) {

	dgMeshEffect *meshEffect = (dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	meshEffect->CalculateNormals(angleInRadians);
}

void NewtonMeshApplySphericalMapping(NewtonMesh *const mesh, int material) {

	dgMeshEffect *meshEffect = (dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	meshEffect->SphericalMapping(material);
}

void NewtonMeshApplyBoxMapping(NewtonMesh *const mesh, int front,
                               int side, int top) {

	dgMeshEffect *meshEffect = (dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	meshEffect->BoxMapping(front, side, top);
}

void NewtonMeshApplyCylindricalMapping(NewtonMesh *const mesh,
                                       int cylinderMaterial, int capMaterial) {

	dgMeshEffect *meshEffect = (dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	meshEffect->CylindricalMapping(cylinderMaterial, capMaterial);
}

void NewtonMeshTriangulate(NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);
	((dgMeshEffect *)mesh)->Triangulate();
}

void NewtonMeshPolygonize(NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);

	((dgMeshEffect *)mesh)->ConvertToPolygons();
}

int NewtonMeshIsOpenMesh(const NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);

	return ((const dgMeshEffect *)mesh)->HasOpenEdges() ? 1 : 0;
}

void NewtonMeshFixTJoints(NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);

	return ((dgMeshEffect *)mesh)->RepairTJoints(false);
}

void NewtonMeshClip(const NewtonMesh *const mesh,
                    const NewtonMesh *const clipper, dFloat *clipperMatrix,
                    NewtonMesh **const topMesh, NewtonMesh **const bottomMesh) {
	TRACE_FUNTION(__FUNCTION__);

	*topMesh = NULL;
	*bottomMesh = NULL;
	dgMatrix &matrix = *((dgMatrix *)clipperMatrix);

	((const dgMeshEffect *)mesh)->ClipMesh(matrix, (const dgMeshEffect *)clipper, (dgMeshEffect **)topMesh, (dgMeshEffect **)bottomMesh);
}

void NewtonMeshPlaneClip(const NewtonMesh *const mesh,
                         dFloat *const planeMatrix, const dFloat *const planeTextureMatrix,
                         int planeMaterial, NewtonMesh **const topMesh,
                         NewtonMesh **const bottomMesh) {
	TRACE_FUNTION(__FUNCTION__);
	*topMesh = NULL;
	*bottomMesh = NULL;
	dgMatrix &matrix = *((dgMatrix *)planeMatrix);
	const dgMatrix &texMatrix = *((const dgMatrix *)planeTextureMatrix);
	((const dgMeshEffect *)mesh)->PlaneClipMesh(matrix, texMatrix, planeMaterial, (dgMeshEffect **)topMesh, (dgMeshEffect **)bottomMesh);
}

NewtonMesh *NewtonMeshApproximateConvexDecomposition(NewtonMesh *mesh, dFloat maxConcavity, int maxCount) {
	TRACE_FUNTION(__FUNCTION__);

	return (NewtonMesh *)((dgMeshEffect *)mesh)->CreateConvexApproximation(maxConcavity, maxCount);
}

NewtonMesh *NewtonMeshTetrahedralization(NewtonMesh *mesh,
        int internalMaterial, dFloat *textureMatrix) {
	TRACE_FUNTION(__FUNCTION__);
	dgMatrix &tetMatrix = *((dgMatrix *)textureMatrix);
	return (NewtonMesh *)((dgMeshEffect *)mesh)->CreateDelanayTretrahedralization(internalMaterial, tetMatrix);
}

NewtonMesh *NewtonMeshVoronoiDecomposition(NewtonMesh *mesh,
        int pointCount, int pointStrideInBytes, const dFloat *const pointCloud,
        int internalMaterial, dFloat *const textureMatrix) {
	TRACE_FUNTION(__FUNCTION__);
	dgMatrix &tetMatrix = *((dgMatrix *)textureMatrix);
	return (NewtonMesh *)((dgMeshEffect *)mesh)->CreateVoronoiPartition(pointCount, pointStrideInBytes, pointCloud, internalMaterial, tetMatrix);
}

NewtonMesh *NewtonMeshUnion(NewtonMesh *mesh,
                            const NewtonMesh *const clipper, const dFloat *const clipperMatrix) {
	TRACE_FUNTION(__FUNCTION__);
	const dgMatrix &matrix = *((const dgMatrix *)clipperMatrix);

	return (NewtonMesh *)((dgMeshEffect *)mesh)->Union(matrix, (const dgMeshEffect *)clipper);
}

NewtonMesh *NewtonMeshDifference(NewtonMesh *const mesh,
                                 const NewtonMesh *const clipper, const dFloat *const clipperMatrix) {
	TRACE_FUNTION(__FUNCTION__);
	const dgMatrix &matrix = *((const dgMatrix *)clipperMatrix);

	return (NewtonMesh *)((dgMeshEffect *)mesh)->Difference(matrix, (const dgMeshEffect *)clipper);
}

NewtonMesh *NewtonMeshIntersection(NewtonMesh *const mesh,
                                   const NewtonMesh *const clipper, const dFloat *const clipperMatrix) {
	TRACE_FUNTION(__FUNCTION__);
	const dgMatrix &matrix = *((const dgMatrix *)clipperMatrix);

	return (NewtonMesh *)((dgMeshEffect *)mesh)->Intersection(matrix, (const dgMeshEffect *)clipper);
}

void NewtonRemoveUnusedVertices(NewtonMesh *const mesh,
                                int32 *const vertexRemapTable) {
	TRACE_FUNTION(__FUNCTION__);

	((dgMeshEffect *)mesh)->RemoveUnusedVertices(vertexRemapTable);
}

void NewtonMeshBeginFace(NewtonMesh *const mesh) {

	dgMeshEffect *const meshEffect = (dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	meshEffect->BeginPolygon();
}

void NewtonMeshAddFace(NewtonMesh *const mesh, int vertexCount,
                       const dFloat *const vertex, int strideInBytes, int materialIndex) {
	dgMeshEffect *const meshEffect = (dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	meshEffect->AddPolygon(vertexCount, vertex, strideInBytes, materialIndex);
}

void NewtonMeshEndFace(NewtonMesh *const mesh) {
	dgMeshEffect *const meshEffect = (dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	meshEffect->EndPolygon(dgFloat64(1.0e-8f));
}

void NewtonMeshBuildFromVertexListIndexList(NewtonMesh *const mesh,
        int faceCount, const int32 *const faceIndexCount,
        const int32 *const faceMaterialIndex, const dFloat *const vertex,
        int vertexStrideInBytes, const int32 *const vertexIndex,
        const dFloat *const normal, int normalStrideInBytes,
        const int32 *const normalIndex, const dFloat *const uv0, int uv0StrideInBytes,
        const int32 *const uv0Index, const dFloat *const uv1, int uv1StrideInBytes,
        const int32 *const uv1Index) {
	dgMeshEffect *const meshEffect = (dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	meshEffect->BuildFromVertexListIndexList(faceCount, faceIndexCount,
	        faceMaterialIndex, vertex, vertexStrideInBytes, vertexIndex, normal,
	        normalStrideInBytes, normalIndex, uv0, uv0StrideInBytes, uv0Index, uv1,
	        uv1StrideInBytes, uv1Index);
}

int NewtonMeshGetVertexCount(const NewtonMesh *const mesh) {

	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	//  return meshEffect->GetPropertiesCount();
	return meshEffect->GetVertexCount();
}

int NewtonMeshGetVertexStrideInByte(const NewtonMesh *const mesh) {
	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetVertexStrideInByte();
}

dFloat64 *NewtonMeshGetVertexArray(const NewtonMesh *const mesh) {
	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetVertexPool();
}

int NewtonMeshGetPointCount(const NewtonMesh *const mesh) {
	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetPropertiesCount();
}

int NewtonMeshGetPointStrideInByte(const NewtonMesh *const mesh) {

	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetPropertiesStrideInByte();
}

dFloat64 *NewtonMeshGetPointArray(const NewtonMesh *const mesh) {
	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetAttributePool();
}

dFloat64 *NewtonMeshGetNormalArray(const NewtonMesh *const mesh) {
	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetNormalPool();
}

dFloat64 *NewtonMeshGetUV0Array(const NewtonMesh *const mesh) {
	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetUV0Pool();
}

dFloat64 *NewtonMeshGetUV1Array(const NewtonMesh *const mesh) {
	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetUV1Pool();
}

void NewtonMeshGetVertexStreams(const NewtonMesh *const mesh,
                                dgInt32 vetexStrideInByte, dFloat *const vertex, dgInt32 normalStrideInByte,
                                dFloat *const normal, dgInt32 uvStrideInByte0, dFloat *const uv0,
                                dgInt32 uvStrideInByte1, dFloat *const uv1) {

	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	meshEffect->GetVertexStreams(vetexStrideInByte, (dgFloat32 *)vertex,
	                             normalStrideInByte, (dgFloat32 *)normal, uvStrideInByte0,
	                             (dgFloat32 *)uv0, uvStrideInByte1, (dgFloat32 *)uv1);
}

void NewtonMeshGetIndirectVertexStreams(const NewtonMesh *const mesh,
                                        int vetexStrideInByte, dFloat *const vertex, int *const vertexIndices,
                                        int *const vertexCount, int normalStrideInByte, dFloat *const normal,
                                        int *const normalIndices, int *const normalCount, int uvStrideInByte0,
                                        dFloat *const uv0, int *const uvIndices0, int *const uvCount0,
                                        int uvStrideInByte1, dFloat *const uv1, int *const uvIndices1,
                                        int *const uvCount1) {
	NEWTON_ASSERT(0);
	/*

	 const dgMeshEffect* const meshEffect = (const dgMeshEffect*) mesh;

	 TRACE_FUNTION(__FUNCTION__);
	 meshEffect->GetIndirectVertexStreams (vetexStrideInByte, (dgFloat32*) vertex, (dgInt32*) vertexIndices, (dgInt32*) vertexCount,
	 normalStrideInByte, (dgFloat32*) normal, (dgInt32*) normalIndices, (dgInt32*) normalCount,
	 uvStrideInByte0, (dgFloat32*) uv0, (dgInt32*) uvIndices0, (dgInt32*) uvCount0,
	 uvStrideInByte1, (dgFloat32*) uv1, (dgInt32*) uvIndices1, (dgInt32*) uvCount1);
	 */
}

void *NewtonMeshBeginHandle(const NewtonMesh *const mesh) {

	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->MaterialGeometryBegin();
}

void NewtonMeshEndHandle(const NewtonMesh *const mesh, void *const handle) {

	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);

	meshEffect->MaterialGeomteryEnd((dgMeshEffect::dgIndexArray *)handle);
}

int NewtonMeshFirstMaterial(const NewtonMesh *const mesh, void *const handle) {

	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetFirstMaterial((dgMeshEffect::dgIndexArray *)handle);
}

int NewtonMeshNextMaterial(const NewtonMesh *const mesh, void *const handle,
                           int materialId) {

	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetNextMaterial((dgMeshEffect::dgIndexArray *)handle,
	                                   materialId);
}

int NewtonMeshMaterialGetMaterial(const NewtonMesh *const mesh,
                                  void *const handle, int materialId) {

	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetMaterialID((dgMeshEffect::dgIndexArray *)handle,
	                                 materialId);
}

int NewtonMeshMaterialGetIndexCount(const NewtonMesh *const mesh,
                                    void *const handle, int materialId) {

	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	return meshEffect->GetMaterialIndexCount((dgMeshEffect::dgIndexArray *)handle,
	        materialId);
}

void NewtonMeshMaterialGetIndexStream(const NewtonMesh *const mesh,
                                      void *const handle, int materialId, int32 *const index) {

	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;

	TRACE_FUNTION(__FUNCTION__);
	meshEffect->GetMaterialGetIndexStream((dgMeshEffect::dgIndexArray *)handle,
	                                      materialId, index);
}

void NewtonMeshMaterialGetIndexStreamShort(const NewtonMesh *const mesh,
        void *const handle, int materialId, short int *const index) {
	const dgMeshEffect *const meshEffect = (const dgMeshEffect *)mesh;
	TRACE_FUNTION(__FUNCTION__);
	meshEffect->GetMaterialGetIndexStreamShort(
	    (dgMeshEffect::dgIndexArray *)handle, materialId, index);
}

NewtonMesh *NewtonMeshCreateFirstSingleSegment(const NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);

	const dgMeshEffect *const effectMesh = (const dgMeshEffect *)mesh;

	dgPolyhedra segment(effectMesh->GetAllocator());

	effectMesh->BeginConectedSurface();
	if (effectMesh->GetConectedSurface(segment)) {
		dgMeshEffect *const solid = new (effectMesh->GetAllocator()) dgMeshEffect(
		    segment, *((const dgMeshEffect *)mesh));
		return (NewtonMesh *)solid;
	} else {
		return NULL;
	}
}

NewtonMesh *NewtonMeshCreateNextSingleSegment(const NewtonMesh *const mesh,
        const NewtonMesh *const segment) {
	TRACE_FUNTION(__FUNCTION__);

	const dgMeshEffect *const effectMesh = (const dgMeshEffect *)mesh;
	dgPolyhedra nextSegment(effectMesh->GetAllocator());

	NEWTON_ASSERT(segment);
	dgInt32 moreSegments = effectMesh->GetConectedSurface(nextSegment);

	dgMeshEffect *solid;
	if (moreSegments) {
		solid = new (effectMesh->GetAllocator()) dgMeshEffect(nextSegment,
		        *effectMesh);
	} else {
		solid = NULL;
		effectMesh->EndConectedSurface();
	}

	return (NewtonMesh *)solid;
}

NewtonMesh *NewtonMeshCreateFirstLayer(const NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);

	const dgMeshEffect *const effectMesh = (const dgMeshEffect *)mesh;
	return (NewtonMesh *)effectMesh->GetFirstLayer();
}

NewtonMesh *NewtonMeshCreateNextLayer(const NewtonMesh *const mesh,
                                      const NewtonMesh *const segment) {
	TRACE_FUNTION(__FUNCTION__);

	const dgMeshEffect *const effectMesh = (const dgMeshEffect *)mesh;
	return (NewtonMesh *)effectMesh->GetNextLayer((const dgMeshEffect *)segment);
}

int NewtonMeshGetTotalFaceCount(const NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetTotalFaceCount();
}

int NewtonMeshGetTotalIndexCount(const NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetTotalIndexCount();
}

void NewtonMeshGetFaces(const NewtonMesh *const mesh, int32 *const faceIndexCount,
                        int32 *const faceMaterial, void **const faceIndices) {
	((const dgMeshEffect *)mesh)->GetFaces(faceIndexCount, faceMaterial, faceIndices);
}

void *NewtonMeshGetFirstVertex(const NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetFirstVertex();
}

void *NewtonMeshGetNextVertex(const NewtonMesh *const mesh,
                              void *const vertex) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetNextVertex(vertex);
}

int NewtonMeshGetVertexIndex(const NewtonMesh *const mesh,
                             void *const vertex) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetVertexIndex(vertex);
}

void *NewtonMeshGetFirstPoint(const NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetFirstPoint();
}
void *NewtonMeshGetNextPoint(const NewtonMesh *const mesh,
                             void *const point) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetNextPoint(point);
}

int NewtonMeshGetPointIndex(const NewtonMesh *const mesh,
                            const void *const point) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetPointIndex(point);
}

int NewtonMeshGetVertexIndexFromPoint(const NewtonMesh *const mesh,
                                      void *const point) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetVertexIndexFromPoint(point);
}

void *NewtonMeshGetFirstEdge(const NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetFirstEdge();
}

void *NewtonMeshGetNextEdge(const NewtonMesh *const mesh,
                            void *const edge) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetNextEdge(edge);
}

void NewtonMeshGetEdgeIndices(const NewtonMesh *const mesh,
                              const void *const edge, int32 *const v0, int32 *const v1) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetEdgeIndex(edge, *v0, *v1);
}

// void NewtonMeshGetEdgePointIndices (const NewtonMesh* const mesh, const void* const edge, int* const v0, int* const v1)
//{
//	return ((dgMeshEffect*)mesh)->GetEdgeAttributeIndex (edge, *v0, *v1);
// }

void *NewtonMeshGetFirstFace(const NewtonMesh *const mesh) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetFirstFace();
}

void *NewtonMeshGetNextFace(const NewtonMesh *const mesh,
                            void *const face) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetNextFace(face);
}

int NewtonMeshIsFaceOpen(const NewtonMesh *const mesh, const void *const face) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->IsFaceOpen(face);
}

int NewtonMeshGetFaceIndexCount(const NewtonMesh *const mesh,
                                const void *const face) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetFaceIndexCount(face);
}

int NewtonMeshGetFaceMaterial(const NewtonMesh *const mesh,
                              const void *const face) {
	TRACE_FUNTION(__FUNCTION__);
	return ((const dgMeshEffect *)mesh)->GetFaceMaterial(face);
}

void NewtonMeshGetFaceIndices(const NewtonMesh *const mesh,
                              const void *const face, int *const indices) {
	TRACE_FUNTION(__FUNCTION__);
	((const dgMeshEffect *)mesh)->GetFaceIndex(face, indices);
}

void NewtonMeshGetFacePointIndices(const NewtonMesh *const mesh,
                                   const void *const face, int *const indices) {
	TRACE_FUNTION(__FUNCTION__);
	((const dgMeshEffect *)mesh)->GetFaceAttributeIndex(face, indices);
}

#if 0

// ***************************************************************************************************************
//
// Name: Rag doll joint container Interface
//
// ***************************************************************************************************************

// Name: NewtonCreateRagDoll
// Create an empty rag doll container.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
//
// Return: handle to an empty rag doll container.
//
// Remarks: A rag doll container is the encapsulation of a group of ball and socket joints, under a common object. It provides common functional
// shared by all joints in the tree like structure. The rag doll object job is to simplify some of the common task that the programmer encounters
// when it tries to make a rag doll model by connecting joints.
// It also has some limitations, for example the hierarchy of joints is made of Ball and socket joints, it only support tree like structures of
// joints, the joints can not by detached from the array once they are added to the rag doll.
// Rag doll joints are good to make articulated creatures, like humans, monsters, horses, etc.
// They are good to simulate effects like death of a character in a game.
//
// See also: NewtonDestroyRagDoll
NewtonRagDoll *NewtonCreateRagDoll(const NewtonWorld *newtonWorld) {
	Newton *world;

	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	return (NewtonRagDoll *) world->RagDollList::Create(world);
}

// Name: NewtonDestroyRagDoll
// Destroy a rag doll containers and all it components.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const NewtonRagDoll* *ragDoll - handle to a rag doll container.
//
// Return: nothing
//
// See also: NewtonCreateRagDoll
void NewtonDestroyRagDoll(const NewtonWorld *newtonWorld, const NewtonRagDoll *ragDoll) {
	Newton *world;

	world = (Newton *)newtonWorld;

	TRACE_FUNTION(__FUNCTION__);
	world->RagDollList::Destroy((RagdollHeader *) ragDoll);
}

// Name: NewtonRagDollBegin
// Prepare a rag doll object to accept bones and making the skeleton.
//
// Parameters:
// *const NewtonRagDoll* *ragDoll - handle to a rag doll container.
//
// Return: nothing
//
// Remarks: This function will clean up the rag doll container of any previously
// added bone. After the constructions process of the rag doll is completed the application should call
// *NewtonRagDollEnd*, not doing so will result in unpredictable results.
//
// See also: NewtonRagDollEnd, NewtonRagDollAddBone, SetExtForceAndTorqueCallback, SetTransformCallback
void NewtonRagDollBegin(const NewtonRagDoll *ragDoll) {
	RagdollHeader *ragDollHeader;

	ragDollHeader = (RagdollHeader *) ragDoll;

	TRACE_FUNTION(__FUNCTION__);
	ragDollHeader->Begin();
}

// Name: NewtonRagDollEnd
// Finalized the rag doll construction process.
//
// Parameters:
// *const NewtonRagDoll* *ragDoll - handle to a rag doll container.
//
// Return: nothing
//
// Remarks: This function should be called after the constructions process of the rag doll in completed.
// calling this function without first calling *NewtonRagDollBegin* will produce unpredictable results.
//
// See also: NewtonRagDollBegin
void NewtonRagDollEnd(const NewtonRagDoll *ragDoll) {
	RagdollHeader *ragDollHeader;

	ragDollHeader = (RagdollHeader *) ragDoll;

	TRACE_FUNTION(__FUNCTION__);
	ragDollHeader->End();
}

// Name: NewtonRagDollAddBone
// Add a bone to a rag doll objects.
//
// Parameters:
// *const NewtonRagDoll* *ragDoll - handle to a rag doll container.
// *const NewtonRagDollBone* *parent - handle to the bone parent for this bone. If this is the root bone the *parent* should be NULL.
// *void* *userData - user data value. The application can use this parameter to store the pointer on index into the graphical part representing this bone.
// *dFloat* mass - mass of this body part.
// *const dFloat* *matrix - pointer to a 4x4 transformation matrix in the local space of the bone. The matrix should be expressed relative to the parent bone.
// *const NewtonCollision* *boneCollision - bone collision geometry.
// *const dFloat* size - pointer to an array of three dFloat specifying the size of this body part. The first component of the array is interpreted as the length of the bone.
//
// Return: the handle to a Rag doll bone
//
// Remarks: The user data value of a rag doll rigid body is set to the *NewtonRagDollBone*, Unpredictable result will happens if the application set the
// user data of a rag doll node rigid body.
//
// Remarks: this function can only be called from inside of a NewtonRagDollBegin/NewtonRagDollEnd pair.
//
// See also: NewtonRagDollBegin
NewtonRagDollBone *NewtonRagDollAddBone(const NewtonRagDoll *ragDoll, const NewtonRagDollBone *parent, void *userData,
                                        dFloat mass, const dFloat *matrix, const NewtonCollision *boneCollision, const dFloat *size) {
	dgCollision *collision;
	RagdollHeader *ragDollHeader;

	TRACE_FUNTION(__FUNCTION__);
	ragDollHeader = (RagdollHeader *) ragDoll;

	dgMatrix boneMatrix(*(dgMatrix *) matrix);
	dgVector boneSize(size[0], size[1], size[2], dgFloat32(0.0f));
	collision = (dgCollision *)boneCollision;

	return (NewtonRagDollBone *) ragDollHeader->AddBone((RagdollBone *) parent, userData, mass, boneMatrix, collision, boneSize);
}

// Name: NewtonRagDollBoneGetUserData
// Retrieve a user defined data value stored with the rag doll bone.
//
// Parameters:
// *const NewtonRagDollBone* *bone - pointer to the bone.
//
// Return: The user defined data.
//
// Remarks: The application can store a user defined value with a rigid body. This value can be the pointer
// to a structure which is the graphical representation of the bone.
//
// See also: NewtonRagDollAddBone
void *NewtonRagDollBoneGetUserData(const NewtonRagDollBone *bone) {
	RagdollBone *dollBone;

	TRACE_FUNTION(__FUNCTION__);
	dollBone = (RagdollBone *) bone;
	return dollBone->GetUserData();
}

// Name: NewtonRagDollSetTransformCallback
// Assign a transformation event function to a rag doll object.
//
// Parameters:
// *const NewtonRagDoll* *ragDoll - handle to a rag doll container.
// *NewtonSetRagDollTransform callback - pointer to a function callback is used to update the transformation matrix of the visual object that represents the rag doll.
//
// Return: Nothing.
//
// Remarks: The function *NewtonSetRagDollTransform callback* is called by the Newton engine every time a visual object that represent a bone of a rag doll has changed.
// The application can obtain the pointer user data value that points to the visual object.
// The Newton engine does not call the *NewtonSetRagDollTransform callback* function for bones that are inactive or have reached a state of stable equilibrium.
//
// Remarks: The user data value of a rag doll rigid body is set to the *NewtonRagDollBone*, Unpredictable result will happens if the application set the
// user data of a rag doll node rigid body.
// The application can get the pointer to the application graphical data by retrieving the user data stored with the *NewtonRagDollBone*.
//
// Remarks: The matrix should be organized in row-major order (this is the way directX stores matrices).
// If you are using OpenGL matrices (column-major) you will need to transpose the matrices into a local array, before
// you pass them to Newton.
//
// Remarks: this function can only be called from inside of a NewtonRagDollBegin/NewtonRagDollEnd pair.
//
// See also: NewtonRagDollBegin, NewtonRagDollAddBone
void NewtonRagDollSetTransformCallback(const NewtonRagDoll *ragDoll, NewtonSetRagDollTransform callback) {
	RagdollHeader *ragDollHeader;

	TRACE_FUNTION(__FUNCTION__);
	ragDollHeader = (RagdollHeader *) ragDoll;
	ragDollHeader->SetTransformCallback(callback);
}

// Name: NewtonRagDollSetForceAndTorqueCallback
// Assign an event function for applying external force and torque to a Rag doll.
//
// Parameters:
// *const NewtonRagDoll* *ragDoll - handle to a rag doll container.
// *NewtonApplyForceAndTorque callback - pointer to a function callback used to apply force and torque to a rigid body bone.
//
// Return: Nothing.
//
// Remarks: this function can be seen as a utility function that will call *NewtonBodySetForceAndTorqueCallback* for every bone of a rag doll object.
//
// Remarks: The user data value of a rag doll rigid body is set to the *NewtonRagDollBone*, unpredictable result will happens if the application set the
// user data of a rag doll node rigid body.
// The application can get the pointer to the application graphical data by retrieving the user data stored with the *NewtonRagDollBone*.
//
// Remarks: Before the *NewtonApplyForceAndTorque callback* is called for a body, Newton first clears the net force and net torque for the body.
//
// Remarks: The function *NewtonApplyForceAndTorque callback* is called by the Newton Engine every time an active body is going to be simulated.
// The Newton Engine does not call the *NewtonApplyForceAndTorque callback* function for bodies that are inactive or have reached a state of stable equilibrium.
//
// See also: NewtonRagDollBegin, NewtonRagDollAddBone
void NewtonRagDollSetForceAndTorqueCallback(const NewtonRagDoll *ragDoll, NewtonApplyForceAndTorque callback) {
	RagdollHeader *ragDollHeader;

	TRACE_FUNTION(__FUNCTION__);
	ragDollHeader = (RagdollHeader *) ragDoll;
	ragDollHeader->SetExtForceAndTorqueCallback((OnApplyExtForceAndTorque) callback);
}

// Name: NewtonRagDollBoneSetID
// Set an id for this particular bone.
//
// Parameters:
// *const NewtonRagDollBone* *bone - handle to the bone.
// *int* id - identifier for this bone.
//
// Return: Nothing.
//
// Remarks: during the construction, the application has the option to set an identifier for each bone. It is good idea to make this identifier unique for the rag doll.
//
// See also: NewtonRagDollAddBone, NewtonRagDollFindBone
void NewtonRagDollBoneSetID(const NewtonRagDollBone *bone, int id) {
	RagdollBone *dollBone;

	TRACE_FUNTION(__FUNCTION__);
	dollBone = (RagdollBone *) bone;
	dollBone->SetNameID(id);
}

// Name: NewtonRagDollFindBone
// Find the first bone with this id in this rag doll.
//
// Parameters:
// *const NewtonRagDoll* *ragDoll - handle to a rag doll container.
// *int* id - identifier for this bone.
//
// Return: the handle to the bone with this identifier. NULL if no bone in the rag doll has this id
//
// Remarks: during the construction, the application has the option to set an identifier for each bone. It is good idea to make this identifier unique for the rag doll.
// the application can use this id to find particular bones in the rag doll body. This is useful for authoring tolls.
//
// See also: NewtonRagDollBoneSetID
NewtonRagDollBone *NewtonRagDollFindBone(const NewtonRagDoll *ragDoll, int id) {
	RagdollHeader *ragDollHeader;

	TRACE_FUNTION(__FUNCTION__);
	ragDollHeader = (RagdollHeader *) ragDoll;
	return (NewtonRagDollBone *) ragDollHeader->FindBone(id);
}

// Name: NewtonRagDollBoneGetBody
// Retrieve the rigid body assigned to this bone.
//
// Parameters:
// *const NewtonRagDollBone* *bone - handle to the bone.
//
// Return: The rigid body assigned to this bone
//
// Remarks: this function can be used to customized some of the properties of the rigid body assigned to the bone.
// the application should not override the pointer to *TransformCallback* or *ApplyForceAndTorque* of a rigid body assigned
// to a Rag doll bone. It should call the functions *NewtonRagDollSetTransformCallback* and *NewtonRagDollSetForceAndTorqueCallback* instead.
//
// See also: NewtonRagDollAddBone, NewtonRagDollSetTransformCallback, NewtonRagDollSetForceAndTorqueCallback
NewtonBody *NewtonRagDollBoneGetBody(const NewtonRagDollBone *bone) {
	RagdollBone *dollBone;

	TRACE_FUNTION(__FUNCTION__);
	dollBone = (RagdollBone *) bone;
	return (NewtonBody *) dollBone->m_body;
}

// Name: NewtonRagDollBoneSetLimits
// Set the stop limits for this bone.
//
// Parameters:
// *const NewtonRagDollBone* *bone - handle to this particular bone. If this parameter is the root bone, then the limits do not have any effect.
// *const dFloat* *coneDir - pointer to an array of tree dFloat specifying the direction in global space of the cone limits for this bone.
// *dFloat* minConeAngle - minimum value of the cone of the cone limit.
// *dFloat* maxConeAngle - maximum value of the cone of the cone limit.
// *dFloat* maxTwistAngle - maximum and minimum that this bone is allow to spin around the coneDir.
// *const dFloat* *lateralConeDir - this parameter is ignored in this release.
// *dFloat* negativeBilateralConeAngle - this parameter is ignored in this release.
// *dFloat* negativeBilateralConeAngle - this parameter is ignored in this release.
//
//
// Remarks: This function set a cone fixed on the frame of the parent of this bone and defining the work space of the bone.
//
// See also: NewtonRagDollBegin
void NewtonRagDollBoneSetLimits(const NewtonRagDollBone *bone, const dFloat *coneDir, dFloat minConeAngle, dFloat maxConeAngle, dFloat maxTwistAngle, const dFloat *lateralConeDir, dFloat negativeBilateralConeAngle, dFloat positiveBilateralConeAngle) {
	RagdollBone *dollBone;

	TRACE_FUNTION(__FUNCTION__);
	dollBone = (RagdollBone *) bone;

	dgVector coneAxis(coneDir[0], coneDir[1], coneDir[2], dgFloat32(0.0f));
	//    dgVector lateralAxis (lateralConeDir[0], lateralConeDir[1], lateralConeDir[2]);
	dgVector lateralAxis(dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	dollBone->SetJointLimits(coneDir, minConeAngle, maxConeAngle, maxTwistAngle, lateralAxis, negativeBilateralConeAngle, positiveBilateralConeAngle);
}

// Name: NewtonRagDollBoneGetLocalMatrix
// Get the transformation matrix for this bone in the local space of the bone.
//
// Parameters:
// *const NewtonRagDollBone* *bone - handle to this particular bone. If this parameter is the root bone, then the limits do not have any effect.
// *dFloat* *matrix - pointer to a 4x4 transformation matrix to receive the transformation matrix for this bone.
//
// Remarks: the application can call this function from a NewtonSetRagDollTransform to get the transformation matrix for the graphical representation of the bone in local space.
//
// See also: NewtonRagDollSetTransformCallback
void NewtonRagDollBoneGetLocalMatrix(const NewtonRagDollBone *bone, dFloat *matrixPtr) {
	RagdollBone *dollBone;

	TRACE_FUNTION(__FUNCTION__);
	dgMatrix &matrix(*((dgMatrix *) matrixPtr));
	dollBone = (RagdollBone *) bone;
	matrix = dollBone->GetBoneLocalMatrix();
}

// Name: NewtonRagDollBoneGetGlobalMatrix
// Get the transformation matrix for this bone in the global space of the bone.
//
// Parameters:
// *const NewtonRagDollBone* *bone - handle to this particular bone. If this parameter is the root bone, then the limits do not have any effect.
// *dFloat* *matrix - pointer to a 4x4 transformation matrix to receive the transformation matrix for this bone.
//
// Remarks: the application can call this function from a NewtonSetRagDollTransform to get the transformation matrix for the graphical representation of the bone in global space.
//
// See also: NewtonRagDollSetTransformCallback
void NewtonRagDollBoneGetGlobalMatrix(const NewtonRagDollBone *bone, dFloat *matrixPtr) {
	RagdollBone *dollBone;

	TRACE_FUNTION(__FUNCTION__);
	dgMatrix &matrix(*((dgMatrix *) matrixPtr));
	dollBone = (RagdollBone *) bone;
	matrix = dollBone->GetBoneMatrix();
}

//void NewtonRagDollSetFriction (const NewtonRagDoll* ragDoll, dFloat friction)
//{
//RagdollHeader* ragDollHeader;
//ragDollHeader = (RagdollHeader*) ragDoll;
//ragDollHeader->SetFriction(friction);
//}

// ***************************************************************************************************************
//
// Name: Vehicle constraint interface
//
// ***************************************************************************************************************

// Name: NewtonConstraintCreateVehicle
// Create and empty vehicle joint.
//
// Parameters:
// *const NewtonWorld* *newtonWorld - is the pointer to the Newton world.
// *const dFloat* *upDir - is the unit vector defined by the tires pivot points, usually the opposite direction to the gravity vector.
// *const NewtonBody* *body - is the pointer to the attached rigid body, this body can not be NULL or it can not have an infinity (zero) mass.
//
// Return: Pointer to the vehicle joint.
//
// Remarks: This joint provides basics functionality for simulating real-time simplistic vehicle dynamics. The joint is not meant to be and
// accurate and realistic representation of a real vehicle, that is out of the scope of a real-time physics engine. The vehicle is made out
// of a main rigid body visible to the application and attached to it a set of tires not visible to the application directly as rigid bodies.
// The tires are connected to the body via rigid wheel joints providing the ability for the tire to spin, have suspension, and turn. The
// internal vehicle mechanics like transmission, power transfer shaft, suspension mechanism, doors etc. It only models the vehicle body
// mounted on a set of wheels, with suspension and the ability to roll.
NewtonJoint *NewtonConstraintCreateVehicle(const NewtonWorld *newtonWorld, const dFloat *upDir, const NewtonBody *body) {
	Newton *world;
	dgBody *carBody;

	TRACE_FUNTION(__FUNCTION__);
	carBody = (dgBody *)body;
	world = (Newton *)newtonWorld;
	dgVector pin(upDir[0], upDir[1], upDir[2], dgFloat32(0.0f));
	return (NewtonJoint *) world->CreateVehicleContraint(pin, carBody);
}

// Name: NewtonVehicleSetTireCallback
// Store a function callback pointer for vehicle update.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *NewtonVehicleTireUpdate* update - pointer to the callback function.
//
// Return: nothing
//
// Remarks: The only way to control a vehicle is by implementing the vehicle update callback.
// The application should iterate through each tire applying tire dynamics to each one.
//
// See also: NewtonVehicleGetFirstTireID, NewtonVehicleGetNextTireID
void NewtonVehicleSetTireCallback(const NewtonJoint *vehicle, NewtonVehicleTireUpdate update) {
	dgVehicleConstraint *joint;

	TRACE_FUNTION(__FUNCTION__);
	joint = (dgVehicleConstraint *)vehicle;
	joint->SetTireCallback((OnVehicleUpdate) update);
}

// Name: NewtonVehicleAddTire
// Add a new tire to the vehicle container.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *const dFloat* *localMatrix - pointer to an array of 16 floats containing the offset of the tire relative to the vehicle body.
// *const dFloat* pin - pointer to an array of 3 floats containing the rotation axis of the tire, in the space of the tire.
// *dFloat* mass - tire mass, must be much smaller than the vehicle body. ratio of 50:1 to 100:1 are the recommended values.
// *dFloat* width - width of the tire, must be smaller than the tire radius.
// *dFloat* radius - tire radius.
// *dFloat* suspesionShock - parametrized damping constant for a spring, mass, damper system. A value of one corresponds to a critically damped system.
// *dFloat* suspesionSpring - parametrized spring constant for a spring, mass, damper system. A value of one corresponds to a critically damped system.
// *dFloat* suspesionLength - distance from the tire set position to the upper stop on the vehicle body frame. The total suspension length is twice that.
// *void* *userData - pointer to a user define data value. Usually used to store the pointer to the graphical representation of the tire.
// *int* collisionID - the collision ID use by the application to identify the tire contacts in a contact callback function.
//
// Return: the tire ID.
//
// Remarks: After the application creates the vehicle joint, it must add the tires.
// Tires are added one at a time at the graphics set position and with the appropriate.
// the application should calculate the correct tire parameters, like tire mass, position, width height,
// spring and damper constants.
//
// See also: NewtonVehicleRemoveTire
void *NewtonVehicleAddTire(const NewtonJoint *vehicle, const dFloat *localMatrix, const dFloat *pin, dFloat mass, dFloat width, dFloat radius,
                           dFloat suspesionShock, dFloat suspesionSpring, dFloat suspesionLength, void *userData, int collisionID) {
	dgVehicleConstraint *joint;

	TRACE_FUNTION(__FUNCTION__);
	joint = (dgVehicleConstraint *)vehicle;

	dgMatrix &matrix(*((dgMatrix *) localMatrix));
	dgVector tirePin(pin[0], pin[1], pin[2], dgFloat32(0.0f));
	//return joint->AddTire (matrix, pin, mass, width, radius, suspesionShock, suspesionSpring, suspesionLength, (void*)(dgUnsigned64(collisionID)), userData);
	return joint->AddTire(matrix, pin, mass, width, radius, suspesionShock, suspesionSpring, suspesionLength, IntToPointer(collisionID), userData);
}

// Name: NewtonVehicleReset
// Reset all tires velocities to zero.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
//
// Return: nothing.
//
// This function is useful for reposition the vehicle.
void NewtonVehicleReset(const NewtonJoint *vehicle) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->Reset();
}

// Name: NewtonVehicleRemoveTire
// Detach and destroy a tire from the vehicle.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - tire index to be destroyed.
//
// Return: nothing.
void NewtonVehicleRemoveTire(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;

	TRACE_FUNTION(__FUNCTION__);
	joint = (dgVehicleConstraint *)vehicle;
	joint->RemoveTire(tireId);
}

// Name: NewtonVehicleGetFirstTireID
// Get the index of the first tire of the vehicle tire set.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
//
// Return: tire index.
//
// Remarks: This function is usually used from inside the vehicle update callback. It is used to iterate through the tire set applying the tire dynamics.
//
// See also: NewtonVehicleGetNextTireID
void *NewtonVehicleGetFirstTireID(const NewtonJoint *vehicle) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	return joint->GetFirstTireIndex();
}

// Name: NewtonVehicleGetNextTireID
// Get the index of the next tire on the tire set.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: Next tire index, or zero if index *tireId* was pointing to the last tire in the set.
//
// Remarks: This function is usually used from inside a tire update callback. It is used to iterate through the tire set applying the tire dynamics.
//
// See also: NewtonVehicleGetFirstTireID
void *NewtonVehicleGetNextTireID(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;

	TRACE_FUNTION(__FUNCTION__);
	joint = (dgVehicleConstraint *)vehicle;
	return joint->GetNextTireIndex(tireId);
}

// Name: NewtonVehicleGetTireUserData
// Retrieve the pointer to the tire user data.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: tire user data.
//
// Remarks: This function is usually used from the vehicle update callback or from transformation callback of the vehicle body,
// It can used do set the transformation matrix of the tire graphical representation.
void *NewtonVehicleGetTireUserData(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;

	TRACE_FUNTION(__FUNCTION__);
	joint = (dgVehicleConstraint *)vehicle;
	return joint->GetTireUserData(tireId);
}

// Name: NewtonVehicleGetTireMatrix
// Retrieve the transformation matrix of the tire in global space.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
// *dFloat* matrix - pointer to an array of 16 floats containing the global matrix of the tire.
//
// Return: nothing
//
// Remarks: This function is usually used from the tire update callback or from transformation callback of the vehicle body,
// It can be used to set the transformation of the tire graphical representation.
void NewtonVehicleGetTireMatrix(const NewtonJoint *vehicle, void *tireId, dFloat *matrix) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	dgMatrix &retMatrix = *((dgMatrix *) matrix);
	joint->GetTireMatrix(tireId, retMatrix);
}

// Name: NewtonVehicleGetTireSteerAngle
// Get the tire steering angle.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: steering angle.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application wants to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
dFloat NewtonVehicleGetTireSteerAngle(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->GetSteerAngle(tireId);
}

// Name: NewtonVehicleSetTireSteerAngle
// Set the tire steering angle.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
// *dFloat* angle - new steering angle.
//
// Return: nothing.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application wants to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
void NewtonVehicleSetTireSteerAngle(const NewtonJoint *vehicle, void *tireId, dFloat angle) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->SetSteerAngle(tireId, angle);
}

// Name: NewtonVehicleSetTireTorque
// Set the tire torque.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
// *dFloat* torque - new torque value.
//
// Return: nothing.
//
// Remarks: This function is useful to simulate normal vehicles with wheels that propel by applying torque to a the tire axis
// in order to move.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application wants to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
void NewtonVehicleSetTireTorque(const NewtonJoint *vehicle, void *tireId, dFloat torque) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->SetTireTorque(tireId, torque);
}

// Name: NewtonVehicleGetTireOmega
// Retrieve the tire angular velocity.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: angular velocity.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application wants to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
dFloat NewtonVehicleGetTireOmega(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->GetTireOmega(tireId);
}

// Name: NewtonVehicleGetTireNormalLoad
// Return the part of the vehicle weight supported by this tire.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: magnitude of the vehicle weight supported by this tire.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application wants to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
dFloat NewtonVehicleGetTireNormalLoad(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->GetTireNormalLoad(tireId);
}

// Name: NewtonVehicleTireSetBrakeAcceleration
// Apply the acceleration and max friction torque to a tire axis.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
// *dFloat* acceleration - desire tire acceleration.
// *dFloat* maxFrictionTorque - maximum friction torque the tire brake, or tire motor can withstand.
//
// Return: noting.
//
// Remarks: This is a multipurpose function. The more common use is to apply hand or soft brakes to a vehicle.
// To apply brakes the application may use the function *NewtonVehicleTireSetBrakeAcceleration* to determine the exact acceleration
// needed to stop the tire from continue to spin in one frame. To simulated the variable brakes strength the application can use
// a nominal maximum friction torque (just like in real life any device will withstand a max value) and modulate this value with an analog
// control. For hand brakes the application set the control to the maximum and for soft brakes it can just modulate the variable friction.
// Another use for this function is to simulate rolling friction, For this effect the application apply the acceleration to stop
// but with a friction value set to a minimum non zero fixed value. Note that brakes and tire torque are not mutually exclusive,
// the application can apply then simultaneously. As a matter of fact doing so is quite a satisfying test showing how the vehicles rocks
// forth and back due to the engine torque, while the tire prevent it from moving. Another use for this function is the simulation of
// track based vehicles. For this the application apply an arbitrary fix acceleration to
// the tires on each side of the vehicle. A function as simple as *A = Ad minus Ks x Omega* can do the trick, where Ad is the desire acceleration
// controlled by the application joystick, Ks is some viscous velocity damping, and omega is the current tire angular velocity reported by the
// function *NewtonVehicleGetTireOmega*.
// To make the vehicle take turns the application can elaborate the equation like *A = Ad + At minus Ks x Omega* where At is the differential
// acceleration supplied by the steering control, for the tires on the right side At is positive while for tires of the left side At is negative.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application wants to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
//
// See also: NewtonVehicleTireCalculateMaxBrakeAcceleration, NewtonVehicleGetTireOmega
void NewtonVehicleTireSetBrakeAcceleration(const NewtonJoint *vehicle, void *tireId, dFloat acceleration, dFloat maxFrictionTorque) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->TireSetBrakeAcceleration(tireId, acceleration, maxFrictionTorque);
}

// Name: NewtonVehicleTireCalculateMaxBrakeAcceleration
// Calculate the exact acceleration needed to be applied to a tire axis in order to bring it to full stop in one time step.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: exact acceleration for full stop of the tire.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application want to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
//
// See also: NewtonVehicleTireSetBrakeAcceleration
dFloat NewtonVehicleTireCalculateMaxBrakeAcceleration(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->TireCalculateMaxBrakeAcceleration(tireId);
}

// Name: NewtonVehicleGetTireLateralSpeed
// Return the tire speed along the tire pin axis.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: tire lateral speed.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application want to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
dFloat NewtonVehicleGetTireLateralSpeed(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->GetTireLateralSpeed(tireId);
}

// Name: NewtonVehicleGetTireLongitudinalSpeed
// Return the tire speed along tire center line.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: tire longitudinal speed.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application want to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
dFloat NewtonVehicleGetTireLongitudinalSpeed(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->GetTireLongitudinalSpeed(tireId);
}

// Name: NewtonVehicleSetTireMaxSideSleepSpeed
// Set the maximum side slip velocity for the tire to be considered to lose grip.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
// *dFloat* speed - maximum side speed before the vehicle is considered to loose side grip.
//
// Return: nothing.
//
// Remarks: Tire operation involve a mix of elastic distortion and sliding friction. To have and idea how to code a convincing
// approximation of a real tire we must run some experiment and make some reflection upon the result. First we will run a static
// test: keeping a tire at equilibrium applying a constant load and without rolling, we will apply a lateral force perpendicular
// to the tire plane and applied at the tire center. If we run this experiment we will find that the tire will deflect in response
// to the lateral force. If we increase the magnitude of the lateral force, the magnitude of the deflection is proportional to the
// magnitude of the lateral force, until the tire begins to slide. This show that when a tire is not moving it behaves like a spring
// (elastic distortion). If we repeat this experiment but this time increasing the tire load, but still not moving the tire, we will
// see that the max deflection is proportional to the magnitude of the tire load. This indicates the tire behavior is proportional
// to two variables, the lateral force and the tire load. (Fortunately the side force in practice is a linear function of the tire
// load so this keeps the model simple) Now we will run the first experiment but this time we will rotate the tire with a constant
// angular velocity (think of those tune up machines at check up stations.) With the tire rolling at constant angular velocity if we
// apply a lateral force we will see that as the tire deflect, the part of the tire in contact with the floor keeps rolling and another
// part take its place, but this part also start to deflect, allowing the tire to move sideways with a velocity proportional to the
// tire rolling angular velocity. Notice that the tire does this without sliding as the part of it in contact with the floor never
// loses grip. Now if we increase the lateral force we will find that the lateral speed of the tire will also increase. This suggests
// that the side speed of the tire is proportional to the lateral force and also proportional to the rolling angular velocity. This
// is the tire elastic properties give then some kind of damping property when they are rolling. There is not known macroscopic
// mathematical model that can explain this behavior. The best we can do is to write the values of the experiment and use then to
// interpolate and extrapolate intermediate values. One thing we know is that the tires operates within some limits, and we can use
// those parameters to treat then as a constraint optimization problem, which is the Newton approach. When the tire is rolling and
// side slipping is not that the tire lost grip, nor that the tire is generating some force. It is rather that the tire have the
// capacity to absorb some of the lateral force by sliding and convert it to side velocity, this means that for the tire to
// loose grip a stronger force is necessary. In another word at rest a tire will lose grip under a much lower lateral force than
// if the tire was rolling. In Newton this behavior is treaded as a constrain optimization problem by asking the application how
// much side slip velocity is the tire allow to have before it is considered to lose grip, and how much of the lateral forces
// generated by the rigid body dynamics will be adsorbed by the tire at a given speed. It is the application responsibility to
// set these parameters as close to the real tire as it chooses. This approach allows for a very wide range of behaviors form arcade,
// to toy cars to very realistic.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application want to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
//
// See also: NewtonVehicleSetTireSideSleepCoeficient
void NewtonVehicleSetTireMaxSideSleepSpeed(const NewtonJoint *vehicle, void *tireId, dFloat speed) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->SetTireMaxSideSleepSpeed(tireId, speed);
}

// Name: NewtonVehicleSetTireSideSleepCoeficient
// Set the coefficient that tell the engine how much of the lateral force can be absorbed by the tire.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
// *dFloat* coefficient - side slip coefficient.
//
// Return: nothing.
//
// Remarks: See description of side slip on function *NewtonVehicleSetTireMaxSideSleepSpeed*
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application want to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
//
// See also: NewtonVehicleSetTireMaxSideSleepSpeed
void NewtonVehicleSetTireSideSleepCoeficient(const NewtonJoint *vehicle, void *tireId, dFloat coeficient) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->SetTireSideSleepCoeficient(tireId, coeficient);
}

// Name: NewtonVehicleSetTireMaxLongitudinalSlideSpeed
// Set the maximum side slide velocity for the tire to be considered to lose traction.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
// *dFloat* speed - maximum side speed before the vehicle is considered to loose side traction.
//
// Return: nothing.
//
// Remarks: The explanation of longitudinal slide is similar to the side slip, however it is not so critical to achieve realistic behavior.
// See description of side slip on function *NewtonVehicleSetTireMaxSideSleepSpeed*
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application want to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
//
// See also: NewtonVehicleSetTireLongitudinalSlideCoeficient
void NewtonVehicleSetTireMaxLongitudinalSlideSpeed(const NewtonJoint *vehicle, void *tireId, dFloat speed) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->SetTireMaxLongitudinalSlideSpeed(tireId, speed);
}

// Name: NewtonVehicleSetTireLongitudinalSlideCoeficient
// Set the coefficient that tell the engine how much of the longitudinal force can be absorbed by the tire.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
// *dFloat* coefficient - longitudinal slide coefficient.
//
// Return: nothing.
//
// Remarks: The explanation of longitudinal slide is similar to the side slip, however it is not so critical to achieve realistic behavior.
// See description of side slip on function *NewtonVehicleSetTireMaxSideSleepSpeed*
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application want to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
//
// See also: NewtonVehicleSetTireMaxLongitudinalSlideSpeed
void NewtonVehicleSetTireLongitudinalSlideCoeficient(const NewtonJoint *vehicle, void *tireId, dFloat coeficient) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->SetTireLongitudinalSlideCoeficient(tireId, coeficient);
}

// Name: NewtonVehicleTireIsAirBorne
// Return a boolean value that tells the application if this tire is touching the ground.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: airborne state 1 on the air, 0 on the ground.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application want to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
int NewtonVehicleTireIsAirBorne(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->IsTireAirBorned(tireId) ? 1 : 0;
}

// Name: NewtonVehicleTireLostSideGrip
// Return a boolean value that tell the application if this tire lost side grip..
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: Grip state.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application want to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
int NewtonVehicleTireLostSideGrip(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->TireLostSideGrip(tireId) ? 1 : 0;
}

// Name: NewtonVehicleTireLostTraction
// Return a boolean value that tell the application if this tire lost longitudinal traction.
//
// Parameters:
// *const NewtonJoint* *vehicle - pointer to the vehicle joint.
// *void* *tireId - index to current tire.
//
// Return: traction state.
//
// Remarks: The vehicle joint provides a rich set of interface functions to the application. Which function to use
// is only determined by the level of fidelity the application want to achieve. In not case the use of one method is better than
// other, and it may be that some tweaking and trial is necessary before the desired vehicle behavior is achieved.
//
// Remarks: The parameters applied to a tire are reset to default values each time the update function is called.
// So the application should set the desired value in each simulation frame.
//
// Remarks: This function can only be called from the vehicle update call back. It can be used by the application to generate the custom vehicle dynamics.
int NewtonVehicleTireLostTraction(const NewtonJoint *vehicle, void *tireId) {
	dgVehicleConstraint *joint;
	joint = (dgVehicleConstraint *)vehicle;

	TRACE_FUNTION(__FUNCTION__);
	return joint->TireLostTraction(tireId) ? 1 : 0;
}
#endif
