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

This is a lightly modified version of MicroPather, from
github.com/leethomason/MicroPather.  Modifications were made to fit with
ScummVM coding style and APIs.

The original copyright message is:

-------

Copyright (c) 2000-2013 Lee Thomason (www.grinninglizard.com)
Micropather

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/


#ifndef TETRAEDGE_TE_MICROPATHER
#define TETRAEDGE_TE_MICROPATHER

#include "common/array.h"
#include "common/util.h"
#include "common/math.h"
#include "common/types.h"


/** @mainpage MicroPather

	MicroPather is a path finder and A* solver (astar or a-star) written in platform independent
	C++ that can be easily integrated into existing code. MicroPather focuses on being a path
	finding engine for video games but is a generic A* solver. MicroPather is open source, with
	a license suitable for open source or commercial use.
*/

namespace Tetraedge
{

namespace micropather
{

	typedef uintptr	MP_UPTR;

	/**
		Used to pass the cost of states from the cliet application to MicroPather. This
		structure is copied in a vector.

		@sa AdjacentCost
	*/
	struct StateCost
	{
		void* state;			///< The state as a void*
		float cost;				///< The cost to the state. Use FLT_MAX for infinite cost.
	};


	/**
		A pure abstract class used to define a set of callbacks.
		The client application inherits from
		this class, and the methods will be called when MicroPather::Solve() is invoked.

		The notion of a "state" is very important. It must have the following properties:
		- Unique
		- Unchanging (unless MicroPather::Reset() is called)

		If the client application represents states as objects, then the state is usually
		just the object cast to a void*. If the client application sees states as numerical
		values, (x,y) for example, then state is an encoding of these values. MicroPather
		never interprets or modifies the value of state.
	*/
	class Graph
	{
	public:
		virtual ~Graph() {}

		/**
			Return the least possible cost between 2 states. For example, if your pathfinding
			is based on distance, this is simply the straight distance between 2 points on the
			map. If you pathfinding is based on minimum time, it is the minimal travel time
			between 2 points given the best possible terrain.
		*/
		virtual float LeastCostEstimate( void* stateStart, void* stateEnd ) = 0;

		/**
			Return the exact cost from the given state to all its neighboring states. This
			may be called multiple times, or cached by the solver. It *must* return the same
			exact values for every call to MicroPather::Solve(). It should generally be a simple,
			fast function with no callbacks into the pather.
		*/
		virtual void AdjacentCost( void* state, Common::Array< micropather::StateCost > *adjacent ) = 0;

		/**
			This function is only used in DEBUG mode - it dumps output to stdout. Since void*
			aren't really human readable, normally you print out some concise info (like "(1,2)")
			without an ending newline.
		*/
		virtual void  PrintStateInfo( void* state ) = 0;
	};


	class PathNode;

	struct NodeCost
	{
		PathNode* node;
		float cost;
	};


	/*
		Every state (void*) is represented by a PathNode in MicroPather. There
		can only be one PathNode for a given state.
	*/
	class PathNode
	{
	public:
		void Init(	unsigned _frame,
					void* _state,
					float _costFromStart,
					float _estToGoal,
					PathNode* _parent );

		void Clear();
		void InitSentinel() {
			Clear();
			Init( 0, 0, FLT_MAX, FLT_MAX, 0 );
			prev = next = this;
		}

		void *state;			// the client state
		float costFromStart;	// exact
		float estToGoal;		// estimated
		float totalCost;		// could be a function, but save some math.
		PathNode* parent;		// the parent is used to reconstruct the path
		unsigned frame;			// unique id for this path, so the solver can distinguish
								// correct from stale values

		int numAdjacent;		// -1  is unknown & needs to be queried
		int cacheIndex;			// position in cache

		PathNode *child[2];		// Binary search in the hash table. [left, right]
		PathNode *next, *prev;	// used by open queue

		bool inOpen;
		bool inClosed;

		void Unlink() {
			next->prev = prev;
			prev->next = next;
			next = prev = 0;
		}
		void AddBefore( PathNode* addThis ) {
			addThis->next = this;
			addThis->prev = prev;
			prev->next = addThis;
			prev = addThis;
		}
#ifdef TETRAEDGE_MICROPATHER_DEBUG
		void CheckList()
		{
			MPASSERT( totalCost == FLT_MAX );
			for( PathNode* it = next; it != this; it=it->next ) {
				MPASSERT( it->prev == this || it->totalCost >= it->prev->totalCost );
				MPASSERT( it->totalCost <= it->next->totalCost );
			}
		}
#endif

		void CalcTotalCost() {
			if ( costFromStart < FLT_MAX && estToGoal < FLT_MAX )
				totalCost = costFromStart + estToGoal;
			else
				totalCost = FLT_MAX;
		}

	private:

		void operator=( const PathNode& );
	};


	/* Memory manager for the PathNodes. */
	class PathNodePool
	{
	public:
		PathNodePool( unsigned allocate, unsigned typicalAdjacent );
		~PathNodePool();

		// Free all the memory except the first block. Resets all memory.
		void Clear();

		// Essentially:
		// pNode = Find();
		// if ( !pNode )
		//		pNode = New();
		//
		// Get the PathNode associated with this state. If the PathNode already
		// exists (allocated and is on the current frame), it will be returned.
		// Else a new PathNode is allocated and returned. The returned object
		// is always fully initialized.
		//
		// NOTE: if the pathNode exists (and is current) all the initialization
		//       parameters are ignored.
		PathNode* GetPathNode(		unsigned frame,
									void* _state,
									float _costFromStart,
									float _estToGoal,
									PathNode* _parent );

		// Get a pathnode that is already in the pool.
		PathNode* FetchPathNode( void* state );

		// Store stuff in cache
		bool PushCache( const NodeCost* nodes, int nNodes, int* start );

		// Get neighbors from the cache
		// Note - always access this with an offset. Can get re-allocated.
		void GetCache( int start, int nNodes, NodeCost* nodes );

		// Return all the allocated states. Useful for visuallizing what
		// the pather is doing.
		void AllStates( unsigned frame, Common::Array< void* >* stateVec );

	private:
		struct Block
		{
			Block* nextBlock;
			PathNode pathNode[1];
		};

		unsigned Hash( void* voidval );
		unsigned HashSize() const	{ return 1<<hashShift; }
		unsigned HashMask()	const	{ return ((1<<hashShift)-1); }
		void AddPathNode( unsigned key, PathNode* p );
		Block* NewBlock();
		PathNode* Alloc();

		PathNode**	hashTable;
		Block*		firstBlock;
		Block*		blocks;

		NodeCost*	cache;
		int			cacheCap;
		int			cacheSize;

		PathNode	freeMemSentinel;
		unsigned	allocate;				// how big a block of pathnodes to allocate at once
		unsigned	nAllocated;				// number of pathnodes allocated (from Alloc())
		unsigned	nAvailable;				// number available for allocation

		unsigned	hashShift;
		unsigned	totalCollide;
	};


	/* Used to cache results of paths. Much, much faster
	   to return an existing solution than to calculate
	   a new one. A post on this is here: http://grinninglizard.com/altera/programming/a-path-caching-2/
	*/
	class PathCache
	{
	public:
		struct Item {
			// The key:
			void* start;
			void* end;

			bool KeyEqual( const Item& item ) const	{ return start == item.start && end == item.end; }
			bool Empty() const						{ return start == 0 && end == 0; }

			// Data:
			void*	next;
			float	cost;	// from 'start' to 'next'. FLT_MAX if unsolveable.

			unsigned Hash() const {
				const unsigned char *p = (const unsigned char *)(&start);
				uint h = 2166136261U;

				for( unsigned i=0; i<sizeof(void*)*2; ++i, ++p ) {
					h ^= *p;
					h *= 16777619;
				}
				return h;
			}
		};

		PathCache( int itemsToAllocate );
		~PathCache();

		void Reset();
		void Add( const Common::Array< void* >& path, const Common::Array< float >& cost );
		void AddNoSolution( void* end, void* states[], int count );
		int Solve( void* startState, void* endState, Common::Array< void* >* path, float* totalCost );

		int AllocatedBytes() const { return allocated * sizeof(Item); }
		int UsedBytes() const { return nItems * sizeof(Item); }

		int hit;
		int miss;

	private:
		void AddItem( const Item& item );
		const Item* Find( void* start, void* end );

		Item*	mem;
		int		allocated;
		int		nItems;
	};

	struct CacheData {
		CacheData() { reset(); }

		int nBytesAllocated;
		int nBytesUsed;
		float memoryFraction;

		int hit;
		int miss;
		float hitFraction;

		void reset() {
			nBytesAllocated = 0;
			nBytesUsed = 0;
			memoryFraction = 0;

			hit = 0;
			miss = 0;
			hitFraction = 0;
		}
	};

	/**
		Create a MicroPather object to solve for a best path. Detailed usage notes are
		on the main page.
	*/
	class MicroPather
	{
		friend class micropather::PathNode;

	public:
		enum
		{
			SOLVED,
			NO_SOLUTION,
			START_END_SAME,

			// internal
			NOT_CACHED
		};

		/**
			Construct the pather, passing a pointer to the object that implements
			the Graph callbacks.

			@param graph		The "map" that implements the Graph callbacks.
			@param allocate		How many states should be internally allocated at a time. This
								can be hard to get correct. The higher the value, the more memory
								MicroPather will use.
								- If you have a small map (a few thousand states?) it may make sense
								  to pass in the maximum value. This will cache everything, and MicroPather
								  will only need one main memory allocation. For a chess board, allocate
								  would be set to 8x8 (64)
								- If your map is large, something like 1/4 the number of possible
								  states is good.
								- If your state space is huge, use a multiple (5-10x) of the normal
								  path. "Occasionally" call Reset() to free unused memory.
			@param typicalAdjacent	Used to determine cache size. The typical number of adjacent states
									to a given state. (On a chessboard, 8.) Higher values use a little
									more memory.
			@param cache		Turn on path caching. Uses more memory (yet again) but at a huge speed
								advantage if you may call the pather with the same path or sub-path, which
								is common for pathing over maps in games.
		*/
		MicroPather( Graph* graph, unsigned allocate = 250, unsigned typicalAdjacent=6, bool cache=true );
		~MicroPather();

		/**
			Solve for the path from start to end.

			@param startState	Input, the starting state for the path.
			@param endState		Input, the ending state for the path.
			@param path			Output, a vector of states that define the path. Empty if not found.
			@param totalCost	Output, the cost of the path, if found.
			@return				Success or failure, expressed as SOLVED, NO_SOLUTION, or START_END_SAME.
		*/
		int Solve( void* startState, void* endState, Common::Array< void* >* path, float* totalCost );

		/**
			Find all the states within a given cost from startState.

			@param startState	Input, the starting state for the path.
			@param near			All the states within 'maxCost' of 'startState', and cost to that state.
			@param maxCost		Input, the maximum cost that will be returned. (Higher values return
								larger 'near' sets and take more time to compute.)
			@return				Success or failure, expressed as SOLVED or NO_SOLUTION.
		*/
		int SolveForNearStates( void* startState, Common::Array< StateCost >* near, float maxCost );

		/** Should be called whenever the cost between states or the connection between states changes.
			Also frees overhead memory used by MicroPather, and calling will free excess memory.
		*/
		void Reset();

		// Debugging function to return all states that were used by the last "solve"
		void StatesInPool( Common::Array< void* >* stateVec );
		void GetCacheData( CacheData* data );

	private:
		MicroPather( const MicroPather& );	// undefined and unsupported
		void operator=( const MicroPather ); // undefined and unsupported

		void GoalReached( PathNode* node, void* start, void* end, Common::Array< void* > *path );

		void GetNodeNeighbors(	PathNode* node, Common::Array< NodeCost >* neighborNode );

#ifdef TETRAEDGE_MICROPATHER_DEBUG
		void DumpStats();
#endif

		PathNodePool			pathNodePool;
		Common::Array< StateCost >	stateCostVec;	// local to Solve, but put here to reduce memory allocation
		Common::Array< NodeCost >	nodeCostVec;	// local to Solve, but put here to reduce memory allocation
		Common::Array< float >		costVec;

		Graph* graph;
		unsigned frame;						// incremented with every solve, used to determine if cached data needs to be refreshed
		PathCache* pathCache;
	};

}	// namespace micropather

}	// namespace Tetraedge

#endif // TETRAEDGE_TE_MICROPATHER

