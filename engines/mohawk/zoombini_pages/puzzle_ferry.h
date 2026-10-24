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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_FERRY_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_FERRY_H

#include "common/array.h"

#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Captain Cajun's Ferry puzzle page (@ref ZoombiniPageType::kFerry).
 * Route 2, Puzzle 1
 *
 * Zoombinis must board rafts by matching traits.
 * Adjacent seated Zoombinis must share at least one trait.
 * The captain (frogman) enforces seating rules with reaction animations.
 *
 */
class ZoombiniPuzzleFerry : public ZoombiniPuzzle {
public:
	/** Create Captain Cajun's Ferry. */
	ZoombiniPuzzleFerry(MohawkEngine_Zoombini *vm);
	/** Release raft, captain, and Zoombini features. */
	~ZoombiniPuzzleFerry() override;
	/** Describe page-specific debugger commands. */
	Common::String debugGetPageCommandHelp() const override;
	/**
	 * Handle a page-specific debugger command.
	 * @param args Page subcommand followed by its arguments; excludes the registered page/puzzle command.
	 * @param output Command result or usage text to print in the debugger.
	 * @return True to keep the debugger open; false to resume the game.
	 */
	bool debugDoPageCommand(const Common::Array<Common::String> &args, Common::String &output) override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNameCaptainCajunsFerryboat;
	static constexpr int kRouteNumber = 2;
	static constexpr int kRoutePuzzleIdx = 1;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate the adjacency rule and initialize raft state. */
	void open() override;
	/** Select the ferry puzzle music. */
	void setBackgroundMusic() override;
	/** Select the ferry background bitmap. */
	void setBackgroundBitmap() override;
	/** Initialize puzzle states and departure transfer source. */
	void initStates() override;
	/** Load raft, seats, captain, and Zoombini features. */
	void loadFeatures() override;
	/** Select the Ferry help prompt after all page-load RNG draws that precede it. */
	void initHelpPrompt() override;
	/** Restart the global Snoid fidget inactivity period after Captain Cajun's Ferry loads. */
	bool requiresFidgetActivityResetOnLoad() const override { return true; }

	/** Return Captain Cajun's Ferry script-sound range priority. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Classify Captain Cajun's authored dialogue separately from Ferry effects. */
	Audio::Mixer::SoundType getFeatureSoundType(const ZmbFeature *feature, ZmbResource resource) const override;

	/** Advance raft movement, reactions, and pending walk-ins. */
	void onPreTickFrame() override;
	/** Process Captain reactions, reject callbacks, and idle scheduling after rendering. */
	void onPostRenderFrame() override;
	/** Start a seat drag or handle ferry controls. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Process raft and rejected-Snoid animation callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;

protected:
	/** Captain Cajun actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kAdjacency,
		kCaptain,
		kLevel
	};
	static constexpr const char *kBuiltinDebugActionAdjacency = "adjacency";
	static constexpr const char *kBuiltinDebugActionCaptain = "captain";
	static constexpr const char *kBuiltinDebugActionLevel = "level";
	/** Depart with accepted raft occupants after the ferry sequence completes. */
	void onGoButtonActivated() override;
	/** Keep the authored raft callbacks in control of the departure state machine. */
	void updateDepartureState() override;
	/** Seat every remaining pack Snoid before the debugger forces departure. */
	void debugPrepareForDeparture() override;
	/** Find and describe one full-party seating, using an optional budget. */
	Common::String debugGetAnswer(const Common::Array<Common::String> &args = Common::Array<Common::String>()) const override;
	/** Describe the Ferry answer command and its optional search budget. */
	Common::String debugGetAnswerCommandHelp() const override;
	/** Describe the Captain Cajun built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Run one Captain Cajun built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Handle Captain Cajun built-in debug keys. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Parse one console-facing Captain Cajun action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Captain Cajun built-in debug action. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	/** Ferry permits unlimited placement attempts. */
	ZmbChanceInfo debugGetChances() const override;

public:
	/** Page-local ferry backgrounds, raft animations, and reaction sounds. */
	enum PageResourceId : int16 {
		kResBackground1300 = 1300,

		// Shape bitmaps
		kResBitmapShape1400_Snoid = 1400,
		kResBitmapShape1450_Adjacency = 1450,
		kResBitmapShape1500_Seats = 1500,
		kResBitmapShape1600_Water = 1600,
		kResBitmapShape1700_Navigation = 1700,
		kResBitmapShape1800_Raft = 1800,

		// Terrain bitmaps
		kResBitmapTerrain100 = 100,

		kResScrb1450_OverlayBase = 1450,
		kResScrb1500_SeatBase = 1500,
		kResScrb1510_SeatingBase = 1510,
		kResScrb1600_WaterBase = 1600,
		kResScrb1601_Landscape = 1601,
		kResScrb1602_RaftApproachA = 1602,
		kResScrb1603_RaftApproachB = 1603,
		kResScrb1604_RejectWalkA = 1604,
		kResScrb1605_DockExit = 1605,
		kResScrb1606_RejectWalkB = 1606,
		kResScrb1607_RaftDeparture = 1607,
		kResScrb1608_DepartureA = 1608,
		kResScrb1609_DepartureB = 1609,
		kResScrb1700_NavigationBase = 1700,
		kResScrb1701_NavigationVariant1 = 1701,
		kResScrb1702_NavigationVariant2 = 1702,
		kResScrb1703_NavigationVariant3 = 1703,
		kResScrb1704_DepartureOverlay = 1704,
		kResScrb1705_RaftDepartureA = 1705,
		kResScrb1706_RaftDepartureB = 1706,
		kResScrb1800_RaftBase = 1800,
		kResScrb1801_RaftVariant1 = 1801,
		kResScrb1802_RaftVariant2 = 1802,
		kResScrb1803_RaftFirstVisit = 1803,
		kResScrb1804_BadReaction0 = 1804,
		kResScrb1805_BadReaction1 = 1805,
		kResScrb1806_BadReaction2 = 1806,
		kResScrb1807_BadReaction3 = 1807,
		kResScrb1808_BadReaction4 = 1808,
		kResScrb1809_BadReaction5 = 1809,
		kResScrb1810_BadReaction6 = 1810,
		kResScrb1811_BadReaction7 = 1811,
		kResScrb1812_BadReaction8 = 1812,
		kResScrb1813_BadReaction9 = 1813,
		kResScrb1814_BadReaction10 = 1814,
		kResScrb1815_HarshBadReaction = 1815,
		kResScrb1816_FirstGoodReaction = 1816,
		kResScrb1817_GoodReaction0 = 1817,
		kResScrb1818_GoodReaction1 = 1818,
		kResScrb1820_MoveReaction0 = 1820,
		kResScrb1821_MoveReaction1 = 1821,
		kResScrb1822_MoveReaction2 = 1822,
		kResScrb1823_Fidget0 = 1823,
		kResScrb1824_Fidget1 = 1824,
		kResScrb1825_Fidget2 = 1825,
		kResScrb1826_Fidget3 = 1826,
		kResScrb1827_Fidget4 = 1827,
		kResScrb1828_FlightFidget0 = 1828,
		kResScrb1829_FlightFidget1 = 1829,
		kResScrb1830_FlightFidget2 = 1830,
		kResScrb1831_FlightFidget3 = 1831,
		kResScrb1832_FlightFidget4 = 1832,

		kResScrs0998_RaftTakeoffBase = 998,
		kResScrs0999_RaftLandingBase = 999,
		kResScrs1000_NormalBase = 1000,
		kResScrs1900_RejectBase = 1900,
		kResScrs1900_RejectVariant0 = 1900,
		kResScrs1901_RejectVariant1 = 1901,
		kResScrs1902_RejectVariant2 = 1902,
		kResScrs1903_RejectVariant3 = 1903,
		kResScrs1904_RejectVariant4 = 1904,
		kResScrs1905_RejectVariant5 = 1905,
		kResScrs1906_RejectVariant6 = 1906,
		kResScrs1907_RejectVariant7 = 1907,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange0425_PageRangeBase = 425,
		kResSoundRange0499_PageRangeLast = 499,
		kResSoundRange1600_PageRangeBase = 1600,
		kResSoundRange1606_SystemRangeBase = 1606,
		kResSoundRange1607_SystemRangeLast = 1607,
		kResSoundRange1699_PageRangeLast = 1699,
		kResSoundRange1700_PageRangeBase = 1700,
		kResSoundRange1704_PagePriorityBase = 1704,
		kResSoundRange1705_PagePriorityLast = 1705,
		kResSoundRange1799_PageRangeLast = 1799,
		kResSoundRange1800_SystemRangeBase = 1800,
		kResSoundRange1899_SystemRangeLast = 1899,
		kResSound1800_CaptainSpeechBase = 1800,
		kResSound1833_CaptainSpeechLast = 1833,
		kResSoundRange1900_PageRangeBase = 1900,
		kResSoundRange1999_PageRangeLast = 1999,
	};

private:
	/** Maximum number of vertices in each seat or passenger graph. */
	static constexpr int kSolverMaxVertices = 16;

	/** Existence of an adjacency-preserving bijection, or an interrupted search. */
	enum class SolverResult {
		/** An adjacency-preserving bijection was found. */
		kSolvable,
		/** The search proved that no adjacency-preserving bijection exists. */
		kUnsolvable,
		/** The time limit was reached before solvability could be determined. */
		kUnknown,
	};

	/** Candidate bipartite graph for a subproblem proved to have no valid vertex mapping. */
	struct SolverFailedState {
		/** Adjacency masks from seat vertices to passenger vertices; an empty array marks an unused cache entry. */
		Common::Array<uint32> candidateAdjacency;
	};

	/** Copied simple graphs and mutable state for one bounded graph-monomorphism search. */
	struct SolverState {
		/**
		 * Copy two simple undirected graphs of equal order.
		 * A solution is a bijection preserving seat edges; extra passenger edges are allowed.
		 * Universal passenger vertices represent empty seats when the party is smaller.
		 * @param[in] seatGraph Open-neighborhood masks indexed by seat vertex; its size is the graph order, from 1 through 16.
		 * @param[in] passengerGraph Compatibility open-neighborhood masks indexed by passenger vertex; must match the seat graph's size.
		 * @param[in] searchTimeLimit Wall-clock search budget in milliseconds.
		 */
		SolverState(const Common::Array<uint32> &seatGraph, const Common::Array<uint32> &passengerGraph, uint32 searchTimeLimit);
		/** Release the owned graph, mapping, and failed-state arrays. */
		~SolverState() = default;
		/** Do not copy an active search. */
		SolverState(const SolverState &) = delete;
		/** Do not assign an active search. */
		SolverState &operator=(const SolverState &) = delete;

		/** Order derived from the input arrays, including universal passenger vertices representing empty seats. */
		const int vertexCount;
		/** Owned immutable open-neighborhood masks indexed by seat vertex; one entry per vertex. */
		const Common::Array<uint32> seatAdjacency;
		/** Owned immutable compatibility open-neighborhood masks indexed by passenger vertex; one entry per vertex. */
		const Common::Array<uint32> passengerAdjacency;
		/** Adjacency-preserving bijection indexed by seat vertex; sized to the graph order, with -1 marking an unavailable mapping. */
		Common::Array<int> vertexMapping;
		/** Search budget in milliseconds. */
		uint32 timeLimit;
		/** Clock value at the beginning of the current search. */
		uint32 started = 0;
		/** DP subset masks processed or backtracking nodes visited in the current query, excluding candidate preparation. */
		uint32 iterations = 0;
		/** Owned direct-mapped cache of 4096 rejected candidate graphs, allocated only for general graph search. */
		Common::Array<SolverFailedState> failedStates;
	};

	/**
	 * Return the cardinality of a vertex set represented by a bit mask.
	 * @param[in] vertexSet Vertex set whose cardinality is required.
	 */
	static int debugFerrySetCardinality(uint32 vertexSet);
	/**
	 * Return the lowest-indexed vertex in @p vertexSet, which must be nonempty.
	 * @param[in] vertexSet Nonempty vertex set represented by a bit mask.
	 */
	static int debugFerryFirstVertex(uint32 vertexSet);
	/**
	 * Find the connected component of @p startVertex after deleting @p excludedVertices.
	 * A connected component is a maximal connected subgraph: all its vertices are reachable from one another by paths.
	 * It includes every vertex reachable from @p startVertex; an isolated vertex forms a component by itself.
	 * The result represents that component's vertex set as a bit mask.
	 * Excluded vertices cannot be traversed, so their removal can split a component.
	 *
	 * @param[in] adjacency Adjacency masks indexed by vertex; bit j marks an edge to zero-based vertex j.
	 * @param[in] startVertex Zero-based starting vertex, which must not be excluded.
	 * @param[in] excludedVertices Mask of vertices omitted from traversal; zero excludes none.
	 * @return Mask of reachable vertices, including the starting vertex.
	 */
	static uint32 debugFerryConnectedComponent(const Common::Array<uint32> &adjacency, int startVertex, uint32 excludedVertices = 0);
	/**
	 * Compute connected-component statistics after deleting one vertex.
	 * Uses @ref ZoombiniPuzzleFerry::debugFerryConnectedComponent to partition the remaining vertices into connected components.
	 * Component order means vertex count; vertex degree means the number of incident edges.
	 * For vertices 0-5 with edges 0-1, 1-2, and 3-4, deleting vertex 1 leaves components {0}, {2}, {3, 4}, and {5}.
	 * This gives @p componentCount = 4 and @p largestComponentOrder = 2, counting isolated vertices as components of order one.
	 *
	 * @param[in] adjacency Adjacency masks indexed by vertex; bit j marks an edge to zero-based vertex j.
	 * @param[in] removedVertex Zero-based vertex index to exclude.
	 * @param[out] componentCount Number of connected components remaining after removal.
	 * @param[out] largestComponentOrder Order (vertex count) of the largest remaining component, or zero if none remain.
	 */
	static void debugFerryVertexDeletionComponents(const Common::Array<uint32> &adjacency, int removedVertex, int &componentCount, int &largestComponentOrder);
	/**
	 * Test the elapsed wall-clock budget, including unsigned clock wraparound.
	 * @param[in] state Search start timestamp and time budget in milliseconds.
	 * @return True when the elapsed time has reached or exceeded the budget.
	 */
	static bool debugFerrySolverExpired(const SolverState &state);
	/**
	 * Find an adjacency-preserving bijection from the seat graph to the passenger graph.
	 * @param[in,out] state Input graphs and budget; resets timing and the vertex mapping, and retains a bijection on success.
	 */
	static SolverResult debugFerrySolve(SolverState &state);
	/**
	 * Find a Hamiltonian path or cycle in the passenger graph using subset dynamic programming.
	 * @param[in,out] state Search state whose seat graph is a path or cycle; receives the corresponding vertex mapping on success.
	 * @param[in] isCycle True when the seat graph is a cycle; false when it is a path.
	 */
	static SolverResult debugFerrySolveHamiltonianPathOrCycle(SolverState &state, bool isCycle);
	/**
	 * Reconstruct a Hamiltonian path or cycle and its corresponding vertex mapping.
	 * @param[in,out] state Seat and passenger graphs; receives the reconstructed adjacency-preserving bijection.
	 * @param[in] pathEndpoints DP table with one entry per vertex subset, indexed by subset mask; each entry contains possible path endpoints.
	 * @param[in] isCycle Whether the Hamiltonian path must close into a cycle.
	 */
	static void debugFerryRestoreHamiltonianPathOrCycle(SolverState &state, const Common::Array<uint32> &pathEndpoints, bool isCycle);
	/**
	 * Enforce adjacency support and perfect-matching constraints to a fixed point.
	 * @param[in] state Seat and passenger adjacency and the order of each graph.
	 * @param[in,out] candidateAdjacency Candidate bipartite adjacency indexed by seat vertex; sized to the graph order and pruned in place.
	 * @return True if pruning finishes without proving that no vertex mapping exists.
	 */
	static bool debugFerryPruneCandidateGraph(const SolverState &state, Common::Array<uint32> &candidateAdjacency);
	/**
	 * Augment the candidate bipartite matching along an augmenting path.
	 * @param[in] seatVertex Seat vertex from which to search for an augmenting path.
	 * @param[in] candidateAdjacency Adjacency masks from seat vertices to candidate passenger vertices.
	 * @param[in,out] matchedSeatVertices Seat vertex matched to each passenger vertex; same size as the candidate graph, with -1 marking unmatched vertices.
	 * @param[in,out] visitedPassengerVertices Passenger vertices visited by the current augmenting-path search.
	 * @return True if an augmenting path was found and the matching was updated.
	 */
	static bool debugFerryAugmentMatching(int seatVertex, const Common::Array<uint32> &candidateAdjacency,
										  Common::Array<int> &matchedSeatVertices, uint32 &visitedPassengerVertices);
	/**
	 * Prune candidate bipartite edges not contained in any perfect matching.
	 * @param[in,out] candidateAdjacency Balanced bipartite adjacency indexed by seat vertex; edges absent from all perfect matchings are removed.
	 * @param[in,out] changed Set to true when edges are removed; an existing true value is preserved.
	 * @return True if a perfect matching exists; false if no such matching exists.
	 */
	static bool debugFerryFilterPerfectMatching(Common::Array<uint32> &candidateAdjacency, bool &changed);
	/**
	 * Branch on an undecided seat vertex of minimum candidate degree, caching only proved failures.
	 * Break equal candidate degrees by maximum degree in the seat graph.
	 * @param[in,out] state Search graphs, timing, and failed-state cache; receives an adjacency-preserving bijection on success.
	 * @param[in,out] candidateAdjacency Candidate bipartite adjacency indexed by seat vertex; sized to the graph order and pruned before branching.
	 */
	static SolverResult debugFerrySearch(SolverState &state, Common::Array<uint32> &candidateAdjacency);

	/**
	 * Describe the shared search budget and either the answer or spoiler-free command.
	 * @param[in] showSolution True for answer-command help; false for the spoiler-free solvable command.
	 */
	static Common::String getDebugSearchCommandHelp(bool showSolution);
	/**
	 * Parse the optional seconds argument after the command and subcommand.
	 * An empty argument array uses five seconds.
	 * Numeric syntax follows @ref ZmbResource::parseUnsignedInt.
	 * @param[in] args Optional seconds argument, with the command and subcommand already removed.
	 * @param[out] timeLimit The budget in milliseconds on success; unchanged on failure.
	 * @return True for an omitted budget or an integer from 1 to 60; false for invalid arguments.
	 */
	static bool parseDebugSearchTimeLimit(const Common::Array<Common::String> &args, uint32 &timeLimit);
	/**
	 * Search copied live state, optionally describing one solution without applying it.
	 * @param[in] timeLimit Search budget in milliseconds.
	 * @param[in] showSolution True to include a successful seating with positions and traits; false for a spoiler-free result.
	 */
	Common::String debugCheckSolvability(uint32 timeLimit, bool showSolution) const;

	/**
	 * Ferry interpretation of @ref ZmbSnoid::_runnerStatus.
	 * The same byte has different meanings on other pages; keep this enum page-local.
	 */
	enum SnoidRunnerStatus : byte {
		/** Available for pickup, whether unseated or seated. */
		kSnoidRunnerStatus00_Draggable = 0,
		/** The Captain or raft controller still owns the rejected Snoid. */
		kSnoidRunnerStatus01_RejectReturning = 1
	};

	/** Event codes emitted by the raft and rejected-Snoid callback chain. */
	enum RaftEventCode : int16 {
		kRaftEventCode001_StartRejectFlight = 1,
		kRaftEventCode002_StartLandingOrRaftPath = 2,
		kRaftEventCode003_StartHiddenLanding = 3,
		kRaftEventCode004_StartRaftTakeoff = 4,
		kRaftEventCode005_StartRaftLanding = 5,
		kRaftEventCode006_QueueCaptainFidget = 6
	};

	/** Event interpreted specially by the feet-specific raft landing SCRS callback. */
	enum RaftLandingEventCode : int16 {
		kRaftLandingEventCode006_StartLandingEffect = 6
	};

	/** Start one reject request in the Captain's single pending slot. */
	void startRejectRequest(uint16 snoidId, ZmbFeature *rejectedSeatFeature, int16 reactionScrbId);
	/** Release the reject controller when its return SCRS takes ownership. */
	void completeRejectRequest();
	/** Interpret the authored two-frame seating SCRB as layout metadata and create the seat and decoration runners. */
	void loadSeatLayout();
	/** Build the default ferry seat adjacency matrix. */
	void buildAdjacencyMatrix();
	/** Build zero-based seat-neighbor indices into a caller-provided buffer. */
	void buildAdjacencyMatrix(int8 seatNeighborIndices[20][8]) const;
	/** Resolve a ferry drop and assign or reject the dragged Snoid. */
	void endDrag(const Common::Point &mousePos) override;
	/** Return the one-based Ferry seat number under a point, or zero. */
	int16 getDropTargetSeatNumber(const Common::Point &pos) const;
	/** Return whether a dropped Snoid satisfies the neighboring seat rule. */
	bool testAdjacentMatch(int16 seatIdx, ZmbSnoid *droppedSnoid);
	/** Count Snoids accepted onto the ferry. */
	int16 countAcceptedSnoids() const;
	/** Return whether a walk-in animation is still pending. */
	bool hasPendingWalkIn() const;
	/** Find an unused reject-flight slot. */
	int16 findOpenRejectSlot(bool occupiedSlots[20]) const;
	/** Start the reject controller for destination code 0 through 9. */
	void startRejectWalk(int16 destinationCode);
	/** Prepare the next reject runner after its reaction SCRB. */
	void handleRejectWalkSetup();
	/** Shift active departure runners by a ferry-specific amount. */
	void shiftDepartureRunners(int16 shiftAmount);

	/**
	 * Dispatch the complete SCRB-to-SCRS hand-off sequence.
	 * Events arrive from raft controller SCRB 1604-1607,
	 * raft overlay SCRB 1704-1705, and the rejected Snoid's SCRS.
	 * Cases 1-3 run the seat, flight, and landing chain.
	 * Cases 4-5 run the feet-specific raft return, and case 6 queues the post-flight Captain fidget.
	 */
	void processRaftFlightEvent(int16 callbackCode);
	/** Process feet-specific raft landing callbacks. */
	void processRaftLandingEvent(ZmbSnoid *snoid, int16 callbackCode);

	/** Fixed dock positions assigned to occupied Zoombinis in pack order. */
	static constexpr Common::Point kDockSnoidPositions[20]{
		Common::Point(370, 160),
		Common::Point(395, 196),
		Common::Point(332, 156),
		Common::Point(348, 196),
		Common::Point(294, 168),
		Common::Point(316, 196),
		Common::Point(253, 166),
		Common::Point(276, 196),
		Common::Point(214, 157),
		Common::Point(237, 196),
		Common::Point(175, 160),
		Common::Point(196, 190),
		Common::Point(135, 152),
		Common::Point(150, 191),
		Common::Point(94, 145),
		Common::Point(110, 186),
		Common::Point(57, 146), // Zero-based slot 16: unused on entry; eligible as a reject or unseat return target.
		Common::Point(71, 182), // Zero-based slot 17: unused on entry; eligible as a reject or unseat return target.
		Common::Point(25, 145), // Zero-based slot 18: unused on entry; eligible as a reject or unseat return target.
		Common::Point(27, 183), // Zero-based slot 19: unused on entry; eligible as a reject or unseat return target.
	};
	// Captain Cajun SCRB pools for non-repeating random idle and reaction animations.
	/**
	 * Captain Cajun and raft introduction SCRB pool selected randomly after the first visit.
	 */
	static constexpr int16 kCaptainIntroScrbPool[4]{
		kResScrb1800_RaftBase,
		kResScrb1801_RaftVariant1,
		kResScrb1802_RaftVariant2,
		kResScrb1803_RaftFirstVisit,
	};
	/**
	 * Captain Cajun's idle fidget SCRB pool selected with a non-repeating random index.
	 */
	static constexpr int16 kCaptainIdleFidgetScrbPool[5]{
		kResScrb1823_Fidget0,
		kResScrb1824_Fidget1,
		kResScrb1825_Fidget2,
		kResScrb1826_Fidget3,
		kResScrb1827_Fidget4,
	};
	/**
	 * Captain Cajun's correct placement reaction SCRB pool selected with a non-repeating random index.
	 */
	static constexpr int16 kCaptainGoodReactionScrbPool[2]{
		kResScrb1817_GoodReaction0,
		kResScrb1818_GoodReaction1,
	};
	/**
	 * Captain Cajun's bad placement reaction SCRB pool selected with a non-repeating random index.
	 */
	static constexpr int16 kCaptainBadReactionScrbPool[11]{
		kResScrb1804_BadReaction0,
		kResScrb1805_BadReaction1,
		kResScrb1806_BadReaction2,
		kResScrb1807_BadReaction3,
		kResScrb1808_BadReaction4,
		kResScrb1809_BadReaction5,
		kResScrb1810_BadReaction6,
		kResScrb1811_BadReaction7,
		kResScrb1812_BadReaction8,
		kResScrb1813_BadReaction9,
		kResScrb1814_BadReaction10,
	};
	/**
	 * Captain Cajun's moved-from-dock reaction SCRB pool selected with a non-repeating random index.
	 */
	static constexpr int16 kCaptainMoveReactionScrbPool[3]{
		kResScrb1820_MoveReaction0,
		kResScrb1821_MoveReaction1,
		kResScrb1822_MoveReaction2,
	};
	/**
	 * Captain Cajun's reject-flight fidget SCRB pool selected with a non-repeating random index by callback case 6.
	 */
	static constexpr int16 kCaptainPostFlightFidgetScrbPool[5]{
		kResScrb1828_FlightFidget0,
		kResScrb1829_FlightFidget1,
		kResScrb1830_FlightFidget2,
		kResScrb1831_FlightFidget3,
		kResScrb1832_FlightFidget4,
	};
	/** Reject animation SCRB table indexed by the reject-flight destination code. */
	static constexpr int16 kRejectDestinationSeatScrbIds[10]{
		kResScrb1701_NavigationVariant1,
		kResScrb1700_NavigationBase,
		kResScrb1700_NavigationBase,
		kResScrb1702_NavigationVariant2,
		kResScrb1702_NavigationVariant2,
		kResScrb1703_NavigationVariant3,
		kResScrb1703_NavigationVariant3,
		kResScrb1700_NavigationBase,
		kResScrb1702_NavigationVariant2,
		kResScrb1703_NavigationVariant3,
	};

	/** SCRB 1510-1529 selected for the current difficulty and loaded Zoombini count. */
	int16 _seatLayoutScrbId = 0;
	/** Number of accepted Ferry occupants. Go is enabled while this count is positive. */
	int16 _acceptedSnoidCount = 0;
	/** Captain reaction or fidget SCRB waiting in the page's single replacement slot. */
	int16 _pendingCaptainScrbId = 0;
	/** Captain SCRB 1800-1832 retained by the built-in replay key. */
	int16 _debugCaptainScrbId = 0;
	/** Captain reaction SCRB that must start before the pending reject controller. */
	int16 _pendingRejectReactionScrbId = 0;
	/** Whether the player may pick up a Ferry Snoid; pending walk-ins remain a separate gate. */
	bool _snoidInputEnabled = true;

	/** Whether a rejected drop is waiting for its Captain reaction to finish before controller setup. */
	bool _rejectSetupPending = false;
	/** Whether reject callback case 2 selected the SCRS 1907 raft-overlay path for activation on the next frame. */
	bool _rejectRaftOverlayPending = false;
	/** One-based seat number under the current drag release, or zero when no seat was hit. */
	int16 _dropTargetSeatNumber = 0;
	/** Concrete seat feature whose SCRB must be replaced by reject callback case 1. */
	ZmbFeature *_rejectedSeatFeature = nullptr;
	/** Whether the dragged Zoombini occupied an accepted Ferry seat before pickup. */
	bool _pickupWasSeated = false;
	/** Reject-flight destination code: 0 dock exit, 1-6 ordinary flight, or 7-9 raft return. */
	int16 _rejectDestinationCode = 0;
	/** ID of the Zoombini retained by the pending or active reject request. */
	uint16 _rejectedSnoidId = 0;
	/**
	 * Zoombini owned by the active reject-flight callback chain.
	 * Reject setup resolves this pointer from @ref _rejectedSnoidId.
	 * @ref ZoombiniPuzzleFerry::_captainRaftFeature continues to own the raft runner.
	 */
	ZmbSnoid *_rejectFlightSnoid = nullptr;
	/**
	 * True while reject controller SCRB 1604, 1605, 1606, or 1607 is loaded on the Captain/raft runner.
	 * Gates the SCRB-to-SCRS hand-off in @ref ZoombiniPuzzleFerry::onFeatureAnimEvent()
	 * so controller events are distinguished from ordinary captain reaction/fidget
	 * SCRBs on the same runner. Overlay and Snoid events are dispatched separately.
	 * Cleared by case 5 or by the raft's end-of-animation event.
	 */
	bool _rejectControllerActive = false;
	/**
	 * True only while the rejected Zoombini owns the main cases 1-6 callback.
	 * The primary reject-flight and feet-specific raft-takeoff SCRS use this callback.
	 * The paired landing SCRS is dispatched separately by its active resource ID.
	 */
	bool _mainRejectSnoidCallbackActive = false;
	/** Up to eight zero-based neighboring seat indices per seat; -1 marks an unused entry. */
	int8 _seatNeighborIndices[20][8] = {};
	/** Whether the required initial render populated @ref ZoombiniPuzzleFerry::_seatNeighborIndices. */
	bool _seatAdjacencyReady = false;
	/** Whether the optional matched-trait highlight was enabled when the page opened. */
	bool _traitHighlightEnabled = false;
	/** Low four-bit mask of traits shared by the most recent accepted drop and its occupied neighbors. */
	uint16 _matchedTraitMask = 0;
	/** ID of the accepted Zoombini waiting to display @ref _matchedTraitMask when idle. */
	uint16 _traitHighlightSnoidId = 0;
	/** Number of consecutive newly accepted placements since the last rejection or reaction threshold. */
	int16 _consecutiveAcceptedPlacements = 0;
	/** Number of consecutive rejected seat placements, capped at five after the harsh reaction. */
	int16 _consecutiveRejectedPlacements = 0;
	/** Accepted-placement streak target for the next good Captain reaction. */
	int16 _goodReactionStreakTarget = 1;
	/** Whether the fixed first good reaction SCRB 1816 has already been queued. */
	bool _firstGoodReactionQueued = false;
	/** SCRB ID currently owning the Captain/raft runner, or zero after completion released it. */
	uint16 _activeCaptainScrbId = 0;
	/** Reject controller SCRB 1604-1607 selected by @ref _rejectDestinationCode. */
	int16 _rejectControllerScrbId = 0;
	/** Target position used by the secondary reject landing SCRS. */
	Common::Point _rejectLandingPosition;
	/** Original pickup position used by the feet-specific raft landing SCRS. */
	Common::Point _rejectReturnPosition;
	/** Activity-idle frame threshold for the next independent Captain idle fidget. */
	uint32 _captainIdleFidgetFrameThreshold = 0;
	/** Whether the initial Captain SCRB has restarted after walk-in and system SND 997 completed. */
	bool _initialCaptainScriptRestarted = false;
	/** Whether the intro movement sound has a valid mixer handle. */
	bool _introMoveSfxHandleValid = false;
	/** Mixer handle for the intro movement sound. */
	Audio::SoundHandle _introMoveSfxHandle;

	// Puzzle-specific feature runners
	/** Landscape overlay feature loaded from SCRB 1601. */
	ZmbFeature *_landscapeOverlayFeature = nullptr;
	/** Shared Captain/raft feature that receives introduction, reaction, controller, and Go SCRBs. */
	ZmbFeature *_captainRaftFeature = nullptr;
	/** Optional raft approach feature loaded from SCRB 1602. */
	ZmbFeature *_raftApproachFeatureA = nullptr;
	/** Optional raft approach feature loaded from SCRB 1603. */
	ZmbFeature *_raftApproachFeatureB = nullptr;
	/** Deferred reject-raft overlay feature loaded from SCRB 1704. */
	ZmbFeature *_rejectRaftOverlayFeature = nullptr;
	/** Reject-raft takeoff feature loaded from SCRB 1705 for destination codes 7 through 9. */
	ZmbFeature *_rejectRaftTakeoffFeature = nullptr;
	/** Reject-raft landing effect loaded from SCRB 1706 for destination codes 7 through 9. */
	ZmbFeature *_rejectRaftLandingFeature = nullptr;
	/** SCRB 1450-1452 priority-chain overlays used while the seat layout is materialized. */
	ZmbFeature *_seatPriorityOverlayFeatures[3] = {};
	/** Seat and decoration features shifted with accepted Zoombinis during the real Go departure. */
	Common::Array<ZmbFeature *> _departureShiftFeatures;
};

} // End of namespace Mohawk

#endif
