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
Copyright (c) 2000-2009 Lee Thomason (www.grinninglizard.com)

Grinning Lizard Utilities.

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

#define MPASSERT assert

//#define DEBUG_PATH
//#define DEBUG_PATH_DEEP
//#define TRACK_COLLISION
//#define DEBUG_CACHING

//#ifdef DEBUG_CACHING
//#include "../grinliz/gldebug.h"
//#endif

#include "micropather.h"

using namespace Tetraedge::micropather;

class OpenQueue
{
  public:
	OpenQueue( Graph* _graph )
	{
		graph = _graph;
		sentinel = (PathNode*) sentinelMem;
		sentinel->InitSentinel();
		#ifdef DEBUG
			sentinel->CheckList();
		#endif
	}
	~OpenQueue()	{}

	void Push( PathNode* pNode );
	PathNode* Pop();
	void Update( PathNode* pNode );

	bool Empty()	{ return sentinel->next == sentinel; }

  private:
	OpenQueue( const OpenQueue& );	// undefined and unsupported
	void operator=( const OpenQueue& );

	PathNode* sentinel;
	int sentinelMem[ ( sizeof( PathNode ) + sizeof( int ) ) / sizeof( int ) ];
	Graph* graph;	// for debugging
};


void OpenQueue::Push( PathNode* pNode )
{

	MPASSERT( pNode->inOpen == 0 );
	MPASSERT( pNode->inClosed == 0 );

#ifdef DEBUG_PATH_DEEP
	debug( "Open Push: " );
	graph->PrintStateInfo( pNode->state );
	debug( " total=%.1f\n", pNode->totalCost );
#endif

	// Add sorted. Lowest to highest cost path. Note that the sentinel has
	// a value of FLT_MAX, so it should always be sorted in.
	MPASSERT( pNode->totalCost < FLT_MAX );
	PathNode* iter = sentinel->next;
	while ( true )
	{
		if ( pNode->totalCost < iter->totalCost ) {
			iter->AddBefore( pNode );
			pNode->inOpen = 1;
			break;
		}
		iter = iter->next;
	}
	MPASSERT( pNode->inOpen );	// make sure this was actually added.
#ifdef DEBUG
	sentinel->CheckList();
#endif
}

PathNode* OpenQueue::Pop()
{
	MPASSERT( sentinel->next != sentinel );
	PathNode* pNode = sentinel->next;
	pNode->Unlink();
#ifdef DEBUG
	sentinel->CheckList();
#endif

	MPASSERT( pNode->inClosed == 0 );
	MPASSERT( pNode->inOpen == 1 );
	pNode->inOpen = 0;

#ifdef DEBUG_PATH_DEEP
	debug( "Open Pop: " );
	graph->PrintStateInfo( pNode->state );
	debug( " total=%.1f\n", pNode->totalCost );
#endif

	return pNode;
}

void OpenQueue::Update( PathNode* pNode )
{
#ifdef DEBUG_PATH_DEEP
	debug( "Open Update: " );
	graph->PrintStateInfo( pNode->state );
	debug( " total=%.1f\n", pNode->totalCost );
#endif

	MPASSERT( pNode->inOpen );

	// If the node now cost less than the one before it,
	// move it to the front of the list.
	if ( pNode->prev != sentinel && pNode->totalCost < pNode->prev->totalCost ) {
		pNode->Unlink();
		sentinel->next->AddBefore( pNode );
	}

	// If the node is too high, move to the right.
	if ( pNode->totalCost > pNode->next->totalCost ) {
		PathNode* it = pNode->next;
		pNode->Unlink();

		while ( pNode->totalCost > it->totalCost )
			it = it->next;

		it->AddBefore( pNode );
#ifdef DEBUG
		sentinel->CheckList();
#endif
	}
}


class ClosedSet
{
  public:
	ClosedSet( Graph* _graph )		{ this->graph = _graph; }
	~ClosedSet()	{}

	void Add( PathNode* pNode )
	{
		#ifdef DEBUG_PATH_DEEP
			debug( "Closed add: " );
			graph->PrintStateInfo( pNode->state );
			debug( " total=%.1f\n", pNode->totalCost );
		#endif
		#ifdef DEBUG
		MPASSERT( pNode->inClosed == 0 );
		MPASSERT( pNode->inOpen == 0 );
		#endif
		pNode->inClosed = 1;
	}

	void Remove( PathNode* pNode )
	{
		#ifdef DEBUG_PATH_DEEP
			debug( "Closed remove: " );
			graph->PrintStateInfo( pNode->state );
			debug( " total=%.1f\n", pNode->totalCost );
		#endif
		MPASSERT( pNode->inClosed == 1 );
		MPASSERT( pNode->inOpen == 0 );

		pNode->inClosed = 0;
	}

  private:
	ClosedSet( const ClosedSet& );
	void operator=( const ClosedSet& );
	Graph* graph;
};


PathNodePool::PathNodePool( unsigned _allocate, unsigned _typicalAdjacent )
	: firstBlock( 0 ),
	  blocks( 0 ),
#if defined( MICROPATHER_STRESS )
	  allocate( 32 ),
#else
	  allocate( _allocate ),
#endif
	  nAllocated( 0 ),
	  nAvailable( 0 )
{
	freeMemSentinel.InitSentinel();

	cacheCap = allocate * _typicalAdjacent;
	cacheSize = 0;
	cache = (NodeCost*)malloc(cacheCap * sizeof(NodeCost));

	// Want the behavior that if the actual number of states is specified, the cache
	// will be at least that big.
	hashShift = 3;	// 8 (only useful for stress testing)
#if !defined( MICROPATHER_STRESS )
	while( HashSize() < allocate )
		++hashShift;
#endif
	hashTable = (PathNode**)calloc( HashSize(), sizeof(PathNode*) );

	blocks = firstBlock = NewBlock();
//	debug( "HashSize=%d allocate=%d\n", HashSize(), allocate );
	totalCollide = 0;
}


PathNodePool::~PathNodePool()
{
	Clear();
	free( firstBlock );
	free( cache );
	free( hashTable );
#ifdef TRACK_COLLISION
	debug( "Total collide=%d HashSize=%d HashShift=%d\n", totalCollide, HashSize(), hashShift );
#endif
}


bool PathNodePool::PushCache( const NodeCost* nodes, int nNodes, int* start ) {
	*start = -1;
	if ( nNodes+cacheSize <= cacheCap ) {
		for( int i=0; i<nNodes; ++i ) {
			cache[i+cacheSize] = nodes[i];
		}
		*start = cacheSize;
		cacheSize += nNodes;
		return true;
	}
	return false;
}


void PathNodePool::GetCache( int start, int nNodes, NodeCost* nodes ) {
	MPASSERT( start >= 0 && start < cacheCap );
	MPASSERT( nNodes > 0 );
	MPASSERT( start + nNodes <= cacheCap );
	memcpy( nodes, &cache[start], sizeof(NodeCost)*nNodes );
}


void PathNodePool::Clear()
{
#ifdef TRACK_COLLISION
	// Collision tracking code.
	int collide=0;
	for( unsigned i=0; i<HashSize(); ++i ) {
		if ( hashTable[i] && (hashTable[i]->child[0] || hashTable[i]->child[1]) )
			++collide;
	}
	//debug( "PathNodePool %d/%d collision=%d %.1f%%\n", nAllocated, HashSize(), collide, 100.0f*(float)collide/(float)HashSize() );
	totalCollide += collide;
#endif

	Block* b = blocks;
	while( b ) {
		Block* temp = b->nextBlock;
		if ( b != firstBlock ) {
			free( b );
		}
		b = temp;
	}
	blocks = firstBlock;	// Don't delete the first block (we always need at least that much memory.)

	// Set up for new allocations (but don't do work we don't need to. Reset/Clear can be called frequently.)
	if ( nAllocated > 0 ) {
		freeMemSentinel.next = &freeMemSentinel;
		freeMemSentinel.prev = &freeMemSentinel;

		memset( hashTable, 0, sizeof(PathNode*)*HashSize() );
		for( unsigned i=0; i<allocate; ++i ) {
			freeMemSentinel.AddBefore( &firstBlock->pathNode[i] );
		}
	}
	nAvailable = allocate;
	nAllocated = 0;
	cacheSize = 0;
}


PathNodePool::Block* PathNodePool::NewBlock()
{
	Block* block = (Block*) calloc( 1, sizeof(Block) + sizeof(PathNode)*(allocate-1) );
	block->nextBlock = 0;

	nAvailable += allocate;
	for( unsigned i=0; i<allocate; ++i ) {
		freeMemSentinel.AddBefore( &block->pathNode[i] );
	}
	return block;
}


unsigned PathNodePool::Hash( void* voidval )
{
	/*
		Spent quite some time on this, and the result isn't quite satifactory. The
		input set is the size of a void*, and is generally (x,y) pairs or memory pointers.

		FNV resulting in about 45k collisions in a (large) test and some other approaches
		about the same.

		Simple folding reduces collisions to about 38k - big improvement. However, that may
		be an artifact of the (x,y) pairs being well distributed. And for either the x,y case
		or the pointer case, there are probably very poor hash table sizes that cause "overlaps"
		and grouping. (An x,y encoding with a hashShift of 8 is begging for trouble.)

		The best tested results are simple folding, but that seems to beg for a pathelogical case.
		FNV-1a was the next best choice, without obvious pathelogical holes.

		Finally settled on h%HashMask(). Simple, but doesn't have the obvious collision cases of folding.
	*/
	/*
	// Time: 567
	// FNV-1a
	// http://isthe.com/chongo/tech/comp/fnv/
	// public domain.
	MP_UPTR val = (MP_UPTR)(voidval);
	const unsigned char *p = (unsigned char *)(&val);
	uint h = 2166136261;

	for( size_t i=0; i<sizeof(MP_UPTR); ++i, ++p ) {
		h ^= *p;
		h *= 16777619;
	}
	// Fold the high bits to the low bits. Doesn't (generally) use all
	// the bits since the shift is usually < 16, but better than not
	// using the high bits at all.
	return ( h ^ (h>>hashShift) ^ (h>>(hashShift*2)) ^ (h>>(hashShift*3)) ) & HashMask();
	*/
	/*
	// Time: 526
	MP_UPTR h = (MP_UPTR)(voidval);
	return ( h ^ (h>>hashShift) ^ (h>>(hashShift*2)) ^ (h>>(hashShift*3)) ) & HashMask();
	*/

	// Time: 512
	// The HashMask() is used as the divisor. h%1024 has lots of common
	// repetitions, but h%1023 will move things out more.
	MP_UPTR h = (MP_UPTR)(voidval);
	return h % HashMask();
}



PathNode* PathNodePool::Alloc()
{
	if ( freeMemSentinel.next == &freeMemSentinel ) {
		MPASSERT( nAvailable == 0 );

		Block* b = NewBlock();
		b->nextBlock = blocks;
		blocks = b;
		MPASSERT( freeMemSentinel.next != &freeMemSentinel );
	}
	PathNode* pathNode = freeMemSentinel.next;
	pathNode->Unlink();

	++nAllocated;
	MPASSERT( nAvailable > 0 );
	--nAvailable;
	return pathNode;
}


void PathNodePool::AddPathNode( unsigned key, PathNode* root )
{
	if ( hashTable[key] ) {
		PathNode* p = hashTable[key];
		while( true ) {
			int dir = (root->state < p->state) ? 0 : 1;
			if ( p->child[dir] ) {
				p = p->child[dir];
			}
			else {
				p->child[dir] = root;
				break;
			}
		}
	}
	else {
		hashTable[key] = root;
	}
}


PathNode* PathNodePool::FetchPathNode( void* state )
{
	unsigned key = Hash( state );

	PathNode* root = hashTable[key];
	while( root ) {
		if ( root->state == state ) {
			break;
		}
		root = ( state < root->state ) ? root->child[0] : root->child[1];
	}
	MPASSERT( root );
	return root;
}


PathNode* PathNodePool::GetPathNode( unsigned frame, void* _state, float _costFromStart, float _estToGoal, PathNode* _parent )
{
	unsigned key = Hash( _state );

	PathNode* root = hashTable[key];
	while( root ) {
		if ( root->state == _state ) {
			if ( root->frame == frame )		// This is the correct state and correct frame.
				break;
			// Correct state, wrong frame.
			root->Init( frame, _state, _costFromStart, _estToGoal, _parent );
			break;
		}
		root = ( _state < root->state ) ? root->child[0] : root->child[1];
	}
	if ( !root ) {
		// allocate new one
		root = Alloc();
		root->Clear();
		root->Init( frame, _state, _costFromStart, _estToGoal, _parent );
		AddPathNode( key, root );
	}
	return root;
}


void PathNode::Init(	unsigned _frame,
						void* _state,
						float _costFromStart,
						float _estToGoal,
						PathNode* _parent )
{
	state = _state;
	costFromStart = _costFromStart;
	estToGoal = _estToGoal;
	CalcTotalCost();
	parent = _parent;
	frame = _frame;
	inOpen = 0;
	inClosed = 0;
}


void PathNode::Clear()
{
	memset( this, 0, sizeof( PathNode ) );
	numAdjacent = -1;
	cacheIndex  = -1;
}

MicroPather::MicroPather( Graph* _graph, unsigned allocate, unsigned typicalAdjacent, bool cache )
	:	pathNodePool( allocate, typicalAdjacent ),
		graph( _graph ),
		frame( 0 )
{
	MPASSERT( allocate );
	MPASSERT( typicalAdjacent );
	pathCache = 0;
	if ( cache ) {
		pathCache = new PathCache( allocate*4 );	// untuned arbitrary constant
	}
}


MicroPather::~MicroPather()
{
	delete pathCache;
}


void MicroPather::Reset()
{
	pathNodePool.Clear();
	if ( pathCache ) {
		pathCache->Reset();
	}
	frame = 0;
}


void MicroPather::GoalReached( PathNode* node, void* start, void* end, Common::Array< void* > *_path )
{
	Common::Array< void* >& path = *_path;
	path.clear();

	// We have reached the goal.
	// How long is the path? Used to allocate the vector which is returned.
	int count = 1;
	PathNode* it = node;
	while( it->parent )
	{
		++count;
		it = it->parent;
	}

	// Now that the path has a known length, allocate
	// and fill the vector that will be returned.
	if ( count < 3 )
	{
		// Handle the short, special case.
		path.resize(2);
		path[0] = start;
		path[1] = end;
	}
	else
	{
		path.resize(count);

		path[0] = start;
		path[count-1] = end;
		count-=2;
		it = node->parent;

		while ( it->parent )
		{
			path[count] = it->state;
			it = it->parent;
			--count;
		}
	}

	if ( pathCache ) {
		costVec.clear();

		PathNode* pn0 = pathNodePool.FetchPathNode( path[0] );
		PathNode* pn1 = 0;
		for( unsigned i=0; i<path.size()-1; ++i ) {
			pn1 = pathNodePool.FetchPathNode( path[i+1] );
			nodeCostVec.clear();
			GetNodeNeighbors( pn0, &nodeCostVec );
			for( unsigned j=0; j<nodeCostVec.size(); ++j ) {
				if ( nodeCostVec[j].node == pn1 ) {
					costVec.push_back( nodeCostVec[j].cost );
					break;
				}
			}
			MPASSERT( costVec.size() == i+1 );
			pn0 = pn1;
		}
		pathCache->Add( path, costVec );
	}

	#ifdef DEBUG_PATH
	debug( "Path: " );
	int counter=0;
	#endif
	for ( unsigned k=0; k<path.size(); ++k )
	{
		#ifdef DEBUG_PATH
		graph->PrintStateInfo( path[k] );
		debug( " " );
		++counter;
		if ( counter == 8 )
		{
			debug( "\n" );
			counter = 0;
		}
		#endif
	}
	#ifdef DEBUG_PATH
	debug( "Cost=%.1f Checksum %d\n", node->costFromStart, checksum );
	#endif
}


void MicroPather::GetNodeNeighbors( PathNode* node, Common::Array< NodeCost >* pNodeCost )
{
	if ( node->numAdjacent == 0 ) {
		// it has no neighbors.
		pNodeCost->resize( 0 );
	}
	else if ( node->cacheIndex < 0 )
	{
		// Not in the cache. Either the first time or just didn't fit. We don't know
		// the number of neighbors and need to call back to the client.
		stateCostVec.resize( 0 );
		graph->AdjacentCost( node->state, &stateCostVec );

		#ifdef DEBUG
		{
			// If this assert fires, you have passed a state
			// as its own neighbor state. This is impossible --
			// bad things will happen.
			for ( unsigned i=0; i<stateCostVec.size(); ++i )
				MPASSERT( stateCostVec[i].state != node->state );
		}
		#endif

		pNodeCost->resize( stateCostVec.size() );
		node->numAdjacent = stateCostVec.size();

		if ( node->numAdjacent > 0 ) {
			// Now convert to pathNodes.
			// Note that the microsoft std library is actually pretty slow.
			// Move things to temp vars to help.
			const unsigned stateCostVecSize = stateCostVec.size();
			const StateCost* stateCostVecPtr = &stateCostVec[0];
			NodeCost* pNodeCostPtr = &(*pNodeCost)[0];

			for( unsigned i=0; i<stateCostVecSize; ++i ) {
				void* state = stateCostVecPtr[i].state;
				pNodeCostPtr[i].cost = stateCostVecPtr[i].cost;
				pNodeCostPtr[i].node = pathNodePool.GetPathNode( frame, state, FLT_MAX, FLT_MAX, 0 );
			}

			// Can this be cached?
			int start = 0;
			if ( pNodeCost->size() > 0 && pathNodePool.PushCache( pNodeCostPtr, pNodeCost->size(), &start ) ) {
				node->cacheIndex = start;
			}
		}
	}
	else {
		// In the cache!
		pNodeCost->resize( node->numAdjacent );
		NodeCost* pNodeCostPtr = &(*pNodeCost)[0];
		pathNodePool.GetCache( node->cacheIndex, node->numAdjacent, pNodeCostPtr );

		// A node is uninitialized (even if memory is allocated) if it is from a previous frame.
		// Check for that, and Init() as necessary.
		for( int i=0; i<node->numAdjacent; ++i ) {
			PathNode* pNode = pNodeCostPtr[i].node;
			if ( pNode->frame != frame ) {
				pNode->Init( frame, pNode->state, FLT_MAX, FLT_MAX, 0 );
			}
		}
	}
}


#ifdef DEBUG
/*
void MicroPather::DumpStats()
{
	int hashTableEntries = 0;
	for( int i=0; i<HASH_SIZE; ++i )
		if ( hashTable[i] )
			++hashTableEntries;

	int pathNodeBlocks = 0;
	for( PathNode* node = pathNodeMem; node; node = node[ALLOCATE-1].left )
		++pathNodeBlocks;
	debug( "HashTableEntries=%d/%d PathNodeBlocks=%d [%dk] PathNodes=%d SolverCalled=%d\n",
			  hashTableEntries, HASH_SIZE, pathNodeBlocks,
			  pathNodeBlocks*ALLOCATE*sizeof(PathNode)/1024,
			  pathNodeCount,
			  frame );
}
*/
#endif


void MicroPather::StatesInPool( Common::Array< void* >* stateVec )
{
	stateVec->clear();
	pathNodePool.AllStates( frame, stateVec );
}


void PathNodePool::AllStates( unsigned frame, Common::Array< void* >* stateVec )
{
	for ( Block* b=blocks; b; b=b->nextBlock )
	{
		for( unsigned i=0; i<allocate; ++i )
		{
			if ( b->pathNode[i].frame == frame )
				stateVec->push_back( b->pathNode[i].state );
		}
	}
}


PathCache::PathCache( int _allocated )
{
	mem = new Item[_allocated];
	memset( mem, 0, sizeof(*mem)*_allocated );
	allocated = _allocated;
	nItems = 0;
	hit = 0;
	miss = 0;
}


PathCache::~PathCache()
{
	delete [] mem;
}


void PathCache::Reset()
{
	if ( nItems ) {
		memset( mem, 0, sizeof(*mem)*allocated );
		nItems = 0;
		hit = 0;
		miss = 0;
	}
}


void PathCache::Add( const Common::Array< void* >& path, const Common::Array< float >& cost )
{
	if ( nItems + (int)path.size() > allocated*3/4 ) {
		return;
	}

	for( unsigned i=0; i<path.size()-1; ++i ) {
		// example: a->b->c->d
		// Huge memory saving to only store 3 paths to 'd'
		// Can put more in cache with also adding path to b, c, & d
		// But uses much more memory. Experiment with this commented
		// in and out and how to set.

		void* end   = path[path.size()-1];
		Item item = { path[i], end, path[i+1], cost[i] };
		AddItem( item );
	}
}


void PathCache::AddNoSolution( void* end, void* states[], int count )
{
	if ( count + nItems > allocated*3/4 ) {
		return;
	}

	for( int i=0; i<count; ++i ) {
		Item item = { states[i], end, 0, FLT_MAX };
		AddItem( item );
	}
}


int PathCache::Solve( void* start, void* end, Common::Array< void* >* path, float* totalCost )
{
	const Item* item = Find( start, end );
	if ( item ) {
		if ( item->cost == FLT_MAX ) {
			++hit;
			return MicroPather::NO_SOLUTION;
		}

		path->clear();
		path->push_back( start );
		*totalCost = 0;

		for ( ;start != end; start=item->next, item=Find(start, end) ) {
			MPASSERT( item );
			*totalCost += item->cost;
			path->push_back( item->next );
		}
		++hit;
		return MicroPather::SOLVED;
	}
	++miss;
	return MicroPather::NOT_CACHED;
}


void PathCache::AddItem( const Item& item )
{
	MPASSERT( allocated );
	int index = item.Hash() % allocated;
	while( true ) {
		if ( mem[index].Empty() ) {
			mem[index] = item;
			++nItems;
#ifdef DEBUG_CACHING
			GLOUTPUT(( "Add: start=%x next=%x end=%x\n", item.start, item.next, item.end ));
#endif
			break;
		}
		else if ( mem[index].KeyEqual( item ) ) {
			MPASSERT( (mem[index].next && item.next) || (mem[index].next==0 && item.next == 0) );
			// do nothing; in cache
			break;
		}
		++index;
		if ( index == allocated )
			index = 0;
	}
}


const PathCache::Item* PathCache::Find( void* start, void* end )
{
	MPASSERT( allocated );
	Item fake = { start, end, 0, 0 };
	int index = fake.Hash() % allocated;
	while( true ) {
		if ( mem[index].Empty() ) {
			return 0;
		}
		if ( mem[index].KeyEqual( fake )) {
			return mem + index;
		}
		++index;
		if ( index == allocated )
			index = 0;
	}
}


void MicroPather::GetCacheData( CacheData* data )
{
	memset( data, 0, sizeof(*data) );

	if ( pathCache ) {
		data->nBytesAllocated = pathCache->AllocatedBytes();
		data->nBytesUsed = pathCache->UsedBytes();
		data->memoryFraction = (float)( (double)data->nBytesUsed / (double)data->nBytesAllocated );

		data->hit = pathCache->hit;
		data->miss = pathCache->miss;
		if ( data->hit + data->miss ) {
		data->hitFraction = (float)( (double)(data->hit) / (double)(data->hit + data->miss) );
	}
		else {
			data->hitFraction = 0;
		}
	}
}



int MicroPather::Solve( void* startNode, void* endNode, Common::Array< void* >* path, float* cost )
{
	// Important to clear() in case the caller doesn't check the return code. There
	// can easily be a left over path  from a previous call.
	path->clear();

	#ifdef DEBUG_PATH
	debug( "Path: " );
	graph->PrintStateInfo( startNode );
	debug( " --> " );
	graph->PrintStateInfo( endNode );
	debug( " min cost=%f\n", graph->LeastCostEstimate( startNode, endNode ) );
	#endif

	*cost = 0.0f;

	if ( startNode == endNode )
		return START_END_SAME;

	if ( pathCache ) {
		int cacheResult = pathCache->Solve( startNode, endNode, path, cost );
		if ( cacheResult == SOLVED || cacheResult == NO_SOLUTION ) {
		#ifdef DEBUG_CACHING
			GLOUTPUT(( "PathCache hit. result=%s\n", cacheResult == SOLVED ? "solved" : "no_solution" ));
		#endif
			return cacheResult;
		}
		#ifdef DEBUG_CACHING
		GLOUTPUT(( "PathCache miss\n" ));
		#endif
	}

	++frame;

	OpenQueue open( graph );
	ClosedSet closed( graph );

	PathNode* newPathNode = pathNodePool.GetPathNode(	frame,
														startNode,
														0,
														graph->LeastCostEstimate( startNode, endNode ),
														0 );

	open.Push( newPathNode );
	stateCostVec.resize(0);
	nodeCostVec.resize(0);

	while ( !open.Empty() )
	{
		PathNode* node = open.Pop();

		if ( node->state == endNode )
		{
			GoalReached( node, startNode, endNode, path );
			*cost = node->costFromStart;
			#ifdef DEBUG_PATH
			DumpStats();
			#endif
			return SOLVED;
		}
		else
		{
			closed.Add( node );

			// We have not reached the goal - add the neighbors.
			GetNodeNeighbors( node, &nodeCostVec );

			for( int i=0; i<node->numAdjacent; ++i )
			{
				// Not actually a neighbor, but useful. Filter out infinite cost.
				if ( nodeCostVec[i].cost == FLT_MAX ) {
					continue;
				}
				PathNode* child = nodeCostVec[i].node;
				float newCost = node->costFromStart + nodeCostVec[i].cost;

				PathNode* inOpen   = child->inOpen ? child : 0;
				PathNode* inClosed = child->inClosed ? child : 0;
				PathNode* inEither = (PathNode*)( ((MP_UPTR)inOpen) | ((MP_UPTR)inClosed) );

				MPASSERT( inEither != node );
				MPASSERT( !( inOpen && inClosed ) );

				if ( inEither ) {
					if ( newCost < child->costFromStart ) {
						child->parent = node;
						child->costFromStart = newCost;
						child->estToGoal = graph->LeastCostEstimate( child->state, endNode );
						child->CalcTotalCost();
						if ( inOpen ) {
							open.Update( child );
						}
					}
				}
				else {
					child->parent = node;
					child->costFromStart = newCost;
					child->estToGoal = graph->LeastCostEstimate( child->state, endNode ),
					child->CalcTotalCost();

					MPASSERT( !child->inOpen && !child->inClosed );
					open.Push( child );
				}
			}
		}
	}
	#ifdef DEBUG_PATH
	DumpStats();
	#endif
	if ( pathCache ) {
		// Could add a bunch more with a little tracking.
		pathCache->AddNoSolution( endNode, &startNode, 1 );
	}
	return NO_SOLUTION;
}


int MicroPather::SolveForNearStates( void* startState, Common::Array< StateCost >* near, float maxCost )
{
	/*	 http://en.wikipedia.org/wiki/Dijkstra%27s_algorithm

		 1  function Dijkstra(Graph, source):
		 2      for each vertex v in Graph:           // Initializations
		 3          dist[v] := infinity               // Unknown distance function from source to v
		 4          previous[v] := undefined          // Previous node in optimal path from source
		 5      dist[source] := 0                     // Distance from source to source
		 6      Q := the set of all nodes in Graph
				// All nodes in the graph are unoptimized - thus are in Q
		 7      while Q is not empty:                 // The main loop
		 8          u := vertex in Q with smallest dist[]
		 9          if dist[u] = infinity:
		10              break                         // all remaining vertices are inaccessible from source
		11          remove u from Q
		12          for each neighbor v of u:         // where v has not yet been removed from Q.
		13              alt := dist[u] + dist_between(u, v)
		14              if alt < dist[v]:             // Relax (u,v,a)
		15                  dist[v] := alt
		16                  previous[v] := u
		17      return dist[]
	*/

	++frame;

	OpenQueue open( graph );			// nodes to look at
	ClosedSet closed( graph );

	nodeCostVec.resize(0);
	stateCostVec.resize(0);

	PathNode closedSentinel;
	closedSentinel.Clear();
	closedSentinel.Init( frame, 0, FLT_MAX, FLT_MAX, 0 );
	closedSentinel.next = closedSentinel.prev = &closedSentinel;

	PathNode* newPathNode = pathNodePool.GetPathNode( frame, startState, 0, 0, 0 );
	open.Push( newPathNode );

	while ( !open.Empty() )
	{
		PathNode* node = open.Pop();	// smallest dist
		closed.Add( node );				// add to the things we've looked at
		closedSentinel.AddBefore( node );

		if ( node->totalCost > maxCost )
			continue;		// Too far away to ever get here.

		GetNodeNeighbors( node, &nodeCostVec );

		for( int i=0; i<node->numAdjacent; ++i )
		{
			MPASSERT( node->costFromStart < FLT_MAX );
			float newCost = node->costFromStart + nodeCostVec[i].cost;

			PathNode* inOpen   = nodeCostVec[i].node->inOpen ? nodeCostVec[i].node : 0;
			PathNode* inClosed = nodeCostVec[i].node->inClosed ? nodeCostVec[i].node : 0;
			MPASSERT( !( inOpen && inClosed ) );
			PathNode* inEither = inOpen ? inOpen : inClosed;
			MPASSERT( inEither != node );

			if ( inEither && inEither->costFromStart <= newCost ) {
				continue;	// Do nothing. This path is not better than existing.
			}
			// Groovy. We have new information or improved information.
			PathNode* child = nodeCostVec[i].node;
			MPASSERT( child->state != newPathNode->state );	// should never re-process the parent.

			child->parent = node;
			child->costFromStart = newCost;
			child->estToGoal = 0;
			child->totalCost = child->costFromStart;

			if ( inOpen ) {
				open.Update( inOpen );
			}
			else if ( !inClosed ) {
				open.Push( child );
			}
		}
	}
	near->clear();

	for( PathNode* pNode=closedSentinel.next; pNode != &closedSentinel; pNode=pNode->next ) {
		if ( pNode->totalCost <= maxCost ) {
			StateCost sc;
			sc.cost = pNode->totalCost;
			sc.state = pNode->state;

			near->push_back( sc );
		}
	}
#ifdef DEBUG
	for( unsigned i=0; i<near->size(); ++i ) {
		for( unsigned k=i+1; k<near->size(); ++k ) {
			MPASSERT( (*near)[i].state != (*near)[k].state );
		}
	}
#endif

	return SOLVED;
}




