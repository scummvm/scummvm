/*******************************************************************************
*                                                                              *
* Author    :  Angus Johnson                                                   *
* Version   :  6.4.2                                                           *
* Date      :  27 February 2017                                                *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2017                                         *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
* Attributions:                                                                *
* The code in this library is an extension of Bala Vatti's clipping algorithm: *
* "A generic solution to polygon clipping"                                     *
* Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
* http://portal.acm.org/citation.cfm?id=129906                                 *
*                                                                              *
* Computer graphics and geometric modeling: implementation and algorithms      *
* By Max K. Agoston                                                            *
* Springer; 1 edition (January 4, 2005)                                        *
* http://books.google.com/books?q=vatti+clipping+agoston                       *
*                                                                              *
* See also:                                                                    *
* "Polygon Offsetting by Computing Winding Numbers"                            *
* Paper no. DETC2005-85513 pp. 565-575                                         *
* ASME 2005 International Design Engineering Technical Conferences             *
* and Computers and Information in Engineering Conference (IDETC/CIE2005)      *
* September 24-28, 2005 , Long Beach, California, USA                          *
* http://www.me.berkeley.edu/~mcmains/pubs/DAC05OffsetPolygon.pdf              *
*                                                                              *
*******************************************************************************/

#ifndef clipper_hpp
#define clipper_hpp

#define CLIPPER_VERSION "6.4.2"

//use_int32: When enabled 32bit ints are used instead of 64bit ints. This
//improve performance but coordinate values are limited to the range +/- 46340
#define use_int32

//use_xyz: adds a Z member to IntPoint. Adds a minor cost to performance.
//#define use_xyz

//use_lines: Enables line clipping. Adds a very minor cost to performance.
#define use_lines

//use_deprecated: Enables temporary support for the obsolete functions
//#define use_deprecated

#include "common/array.h"

namespace ClipperLib {

enum ClipType { ctIntersection, ctUnion, ctDifference, ctXor };
enum PolyType { ptSubject, ptClip };
//By far the most widely used winding rules for polygon filling are
//EvenOdd & NonZero (GDI, GDI+, XLib, OpenGL, Cairo, AGG, Quartz, SVG, Gr32)
//Others rules include Positive, Negative and ABS_GTR_EQ_TWO (only in OpenGL)
//see http://glprogramming.com/red/chapter11.html
enum PolyFillType { pftEvenOdd, pftNonZero, pftPositive, pftNegative };

#ifdef use_int32
typedef int32 cInt;
static cInt const loRange = 0x7FFF;
static cInt const hiRange = 0x7FFF;
#else
typedef int64 cInt;
static cInt const loRange = 0x3FFFFFFF;
static cInt const hiRange = 0x3FFFFFFFFFFFFFFFLL;
typedef int64 long64;     //used by Int128 class
typedef uint64 ulong64;

#endif

/**
 * Queue ordered by a provided priority function
 * NOTE: Unlike in the C std library, we have to provde a comparitor that sorts
 * the array so that the smallest priority comes last
 */
template <class _Ty, class _Container = Common::Array<_Ty>, class _Pr = Common::Less<_Ty>>
class priority_queue {
public:
	typedef const _Ty& const_reference;

public:
	priority_queue() : c(), comp() {}

	explicit priority_queue(const _Pr &_Pred) : c(), comp(_Pred) {}

	priority_queue(const _Pr &_Pred, const _Container &_Cont) : c(_Cont), comp(_Pred) {
		make_heap(c.begin(), c.end(), comp);
	}

	template <class _InIt>
	priority_queue(_InIt _First, _InIt _Last, const _Pr &_Pred, const _Container &_Cont) : c(_Cont), comp(_Pred) {
		c.insert(c.end(), _First, _Last);
		make_heap(c.begin(), c.end(), comp);
	}

	template <class _InIt>
	priority_queue(_InIt _First, _InIt _Last) : c(_First, _Last), comp() {
		make_heap(c.begin(), c.end(), comp);
	}

	template <class _InIt>
	priority_queue(_InIt _First, _InIt _Last, const _Pr &_Pred) : c(_First, _Last), comp(_Pred) {
		make_heap(c.begin(), c.end(), comp);
	}

	bool empty() const {
		return c.empty();
	}

	size_t size() const {
		return c.size();
	}

	const_reference top() const {
		return c.back();
	}

	void push(const typename _Container::value_type &_Val) {
		c.push_back(_Val);
		Common::sort(c.begin(), c.end(), comp);
	}

	void pop() {
		c.pop_back();
	}

	void swap(priority_queue &_Right) {
		SWAP(c, _Right.c);
		SWAP(comp, _Right.comp);
	}

protected:
	_Container c;
	_Pr comp;
};

struct IntPoint {
  cInt X;
  cInt Y;
#ifdef use_xyz
  cInt Z;
  IntPoint(cInt x = 0, cInt y = 0, cInt z = 0): X(x), Y(y), Z(z) {};
#else
  IntPoint(cInt x = 0, cInt y = 0) : X(x), Y(y) {};
#endif

  friend inline bool operator==(const IntPoint &a, const IntPoint &b) {
    return a.X == b.X && a.Y == b.Y;
  }
  friend inline bool operator!=(const IntPoint &a, const IntPoint &b) {
    return a.X != b.X || a.Y != b.Y;
  }
};
//------------------------------------------------------------------------------

typedef Common::Array<IntPoint> Path;
typedef Common::Array<Path> Paths;

inline Path &operator<<(Path &poly, const IntPoint &p) {
  poly.push_back(p);
  return poly;
}
inline Paths &operator<<(Paths &polys, const Path &p) {
  polys.push_back(p);
  return polys;
}

struct DoublePoint {
  double X;
  double Y;
  DoublePoint(double x = 0, double y = 0) : X(x), Y(y) {}
  DoublePoint(IntPoint ip) : X((double) ip.X), Y((double) ip.Y) {}
};
//------------------------------------------------------------------------------

enum InitOptions { ioReverseSolution = 1, ioStrictlySimple = 2, ioPreserveCollinear = 4 };
enum JoinType { jtSquare, jtRound, jtMiter };
enum EndType { etClosedPolygon, etClosedLine, etOpenButt, etOpenSquare, etOpenRound };

class PolyNode;
typedef Common::Array<PolyNode *> PolyNodes;

class PolyNode {
public:
  PolyNode();
  virtual ~PolyNode() {};
  Path Contour;
  PolyNodes Childs;
  PolyNode *Parent;
  PolyNode *GetNext() const;
  bool IsHole() const;
  bool IsOpen() const;
  int ChildCount() const;
private:
  //PolyNode& operator =(PolyNode& other);
  unsigned Index; //node index in Parent.Childs
  bool m_IsOpen;
  JoinType m_jointype;
  EndType m_endtype;
  PolyNode *GetNextSiblingUp() const;
  void AddChild(PolyNode &child);
  friend class Clipper; //to access Index
  friend class ClipperOffset;
};

bool Orientation(const Path &poly);
double Area(const Path &poly);
int PointInPolygon(const IntPoint &pt, const Path &path);

void ReversePath(Path &p);
void ReversePaths(Paths &p);

struct IntRect { cInt left; cInt top; cInt right; cInt bottom; };

//enums that are used internally ...
enum EdgeSide { esLeft = 1, esRight = 2 };

//forward declarations (for stuff used internally) ...
struct TEdge;
struct IntersectNode;
struct LocalMinimum;
struct OutPt;
struct OutRec;
struct Join;

typedef Common::Array<OutRec *> PolyOutList;
typedef Common::Array<TEdge *> EdgeList;
typedef Common::Array<Join *> JoinList;
typedef Common::Array<IntersectNode *> IntersectList;

//------------------------------------------------------------------------------

//ClipperBase is the ancestor to the Clipper class. It should not be
//instantiated directly. This class simply abstracts the conversion of sets of
//polygon coordinates into edge objects that are stored in a LocalMinima list.
class ClipperBase {
public:
  ClipperBase();
  virtual ~ClipperBase();
  virtual bool AddPath(const Path &pg, PolyType PolyTyp, bool Closed);
  bool AddPaths(const Paths &ppg, PolyType PolyTyp, bool Closed);
  void Clear();
  IntRect GetBounds();
  bool PreserveCollinear() { return m_PreserveCollinear; };
  void PreserveCollinear(bool value) { m_PreserveCollinear = value; };
protected:
  void DisposeLocalMinimaList();
  virtual void Reset();
  TEdge *ProcessBound(TEdge *E, bool IsClockwise);
  void InsertScanbeam(const cInt Y);
  bool PopScanbeam(cInt &Y);
  bool LocalMinimaPending();
  bool PopLocalMinima(cInt Y, const LocalMinimum *&locMin);
  OutRec *CreateOutRec();
  void DisposeAllOutRecs();
  void DisposeOutRec(PolyOutList::size_type index);
  void SwapPositionsInAEL(TEdge *edge1, TEdge *edge2);
  void DeleteFromAEL(TEdge *e);
  void UpdateEdgeIntoAEL(TEdge *&e);

  typedef Common::Array<LocalMinimum> MinimaList;
  MinimaList::iterator m_CurrentLM;
  MinimaList m_MinimaList;

  bool m_UseFullRange;
  EdgeList m_edges;
  bool m_PreserveCollinear;
  bool m_HasOpenPaths;
  PolyOutList m_PolyOuts;
  TEdge *m_ActiveEdges;

  typedef priority_queue<cInt> ScanbeamList;
  ScanbeamList m_Scanbeam;
};
//------------------------------------------------------------------------------

class Clipper : public virtual ClipperBase {
public:
  Clipper(int initOptions = 0);
  bool Execute(ClipType clipType,
               Paths &solution,
               PolyFillType fillType = pftEvenOdd);
  bool Execute(ClipType clipType,
               Paths &solution,
               PolyFillType subjFillType,
               PolyFillType clipFillType);

  bool ReverseSolution() { return m_ReverseOutput; };
  void ReverseSolution(bool value) { m_ReverseOutput = value; };
  bool StrictlySimple() { return m_StrictSimple; };
  void StrictlySimple(bool value) { m_StrictSimple = value; };
protected:
  virtual bool ExecuteInternal();
private:
  JoinList m_Joins;
  JoinList m_GhostJoins;
  IntersectList m_IntersectList;
  ClipType m_ClipType;
  typedef Common::Array<cInt> MaximaList;
  MaximaList m_Maxima;
  TEdge *m_SortedEdges;
  bool m_ExecuteLocked;
  PolyFillType m_ClipFillType;
  PolyFillType m_SubjFillType;
  bool m_ReverseOutput;
  bool m_UsingPolyTree;
  bool m_StrictSimple;
  void SetWindingCount(TEdge &edge);
  bool IsEvenOddFillType(const TEdge &edge) const;
  bool IsEvenOddAltFillType(const TEdge &edge) const;
  void InsertLocalMinimaIntoAEL(const cInt botY);
  void InsertEdgeIntoAEL(TEdge *edge, TEdge *startEdge);
  void AddEdgeToSEL(TEdge *edge);
  bool PopEdgeFromSEL(TEdge *&edge);
  void CopyAELToSEL();
  void DeleteFromSEL(TEdge *e);
  void SwapPositionsInSEL(TEdge *edge1, TEdge *edge2);
  bool IsContributing(const TEdge &edge) const;
  void DoMaxima(TEdge *e);
  void ProcessHorizontals();
  void ProcessHorizontal(TEdge *horzEdge);
  void AddLocalMaxPoly(TEdge *e1, TEdge *e2, const IntPoint &pt);
  OutPt *AddLocalMinPoly(TEdge *e1, TEdge *e2, const IntPoint &pt);
  OutRec *GetOutRec(int idx);
  void AppendPolygon(TEdge *e1, TEdge *e2);
  void IntersectEdges(TEdge *e1, TEdge *e2, IntPoint &pt);
  OutPt *AddOutPt(TEdge *e, const IntPoint &pt);
  OutPt *GetLastOutPt(TEdge *e);
  bool ProcessIntersections(const cInt topY);
  void BuildIntersectList(const cInt topY);
  void ProcessIntersectList();
  void ProcessEdgesAtTopOfScanbeam(const cInt topY);
  void BuildResult(Paths &polys);
  void SetHoleState(TEdge *e, OutRec *outrec);
  void DisposeIntersectNodes();
  bool FixupIntersectionOrder();
  void FixupOutPolygon(OutRec &outrec);
  void FixupOutPolyline(OutRec &outrec);
  void FixHoleLinkage(OutRec &outrec);
  void AddJoin(OutPt *op1, OutPt *op2, const IntPoint offPt);
  void ClearJoins();
  void ClearGhostJoins();
  void AddGhostJoin(OutPt *op, const IntPoint offPt);
  bool JoinPoints(Join *j, OutRec *outRec1, OutRec *outRec2);
  void JoinCommonEdges();
  void DoSimplePolygons();
  void FixupFirstLefts1(OutRec *OldOutRec, OutRec *NewOutRec);
  void FixupFirstLefts2(OutRec *InnerOutRec, OutRec *OuterOutRec);
  void FixupFirstLefts3(OutRec *OldOutRec, OutRec *NewOutRec);
#ifdef use_xyz
  void SetZ(IntPoint& pt, TEdge& e1, TEdge& e2);
#endif
};
//------------------------------------------------------------------------------

} //ClipperLib namespace

#endif //clipper_hpp


