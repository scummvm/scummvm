#include	"bitmaps.h"
/*

#define	W	255,
#define	x	252,
#define	_	TRANS,
#define	o	0,
#define	L	LGRAY,
#define	G	GRAY,
#define	D	DGRAY,

static	byte	MCDesign0[]= { W W W W W W _
			       W W W W W o _
			       W W W W o _ _
			       W W W W W _ _
			       W W o W W W _
			       W o _ o W W W
			       o _ _ _ o W W
			       _ _ _ _ _ o o };


static	byte	MCDesign1[]= { _ };



static	byte	SLDesign[] = { G G G G G G G G G _ _ _ _ _ _
			       L G G G G G G G G D _ _ _ _ _
			       _ L G G G G G G G D _ _ _ _ _
			       _ _ L G G G G G G G D _ _ _ _
			       _ _ _ L G G G G G G D _ _ _ _
			       _ _ _ _ L G G G G G D _ _ _ _
			       _ _ _ _ _ L G G G G G D _ _ _
			       _ _ _ _ _ _ L G G G G D _ _ _
			       _ _ _ _ _ _ _ L G G G D _ _ _
			       _ _ _ _ _ _ _ _ L G G G D _ _
			       _ _ _ _ _ _ _ _ _ L G G D _ _
			       _ _ _ _ _ _ _ _ _ _ L G D _ _
			       _ _ _ _ _ _ _ _ _ _ _ L G D _
			       _ _ _ _ _ _ _ _ _ _ _ _ L D _
			       _ _ _ _ _ _ _ _ _ _ _ _ _ L D
			       _ _ _ _ _ _ _ _ _ _ _ _ _ _ D
			     };

static	byte	SRDesign[] = { _ _ _ _ _ _ G G G G G G G G G
			       _ _ _ _ _ L G G G G G G G G D
			       _ _ _ _ _ L G G G G G G G D _
			       _ _ _ _ L G G G G G G G D _ _
			       _ _ _ _ L G G G G G G D _ _ _
			       _ _ _ _ L G G G G G D _ _ _ _
			       _ _ _ L G G G G G D _ _ _ _ _
			       _ _ _ L G G G G D _ _ _ _ _ _
			       _ _ _ L G G G D _ _ _ _ _ _ _
			       _ _ L G G G D _ _ _ _ _ _ _ _
			       _ _ L G G D _ _ _ _ _ _ _ _ _
			       _ _ L G D _ _ _ _ _ _ _ _ _ _
			       _ L G D _ _ _ _ _ _ _ _ _ _ _
			       _ L D _ _ _ _ _ _ _ _ _ _ _ _
			       L D _ _ _ _ _ _ _ _ _ _ _ _ _
			       D _ _ _ _ _ _ _ _ _ _ _ _ _ _
			     };

static	byte	MapBrick[] = { L L L L L L L G
			       L G G G G G G D
			       L G G G G G G D
			       G D D D D D D D
			     };

#undef	W
#undef	_
#undef	x
#undef	o
#undef	L
#undef	G
#undef	D


#if 0

#define	_	TRANS,
#define	A	213,
#define	B	207,
#define	C	225,
#define	D	219,
#define	E	231,

static	byte	PRDesign[] = { A E E E C C D A B
			       C _ _ _ _ _ _ D A
			       C _ _ _ _ _ _ D A
			       C _ _ _ _ _ _ D A
			       C _ _ _ _ _ _ D A
			       C _ _ _ _ _ _ D A
			       C _ _ _ _ _ _ D A
			       B A A A A A A A B
			       B B B B B B B B B
			     };

#else

#define	_	TRANS,
#define	A	213,
#define	B	207,
#define	C	225, // DGRAY
#define	D	219,
#define	E	231,
#define	F	237,

static	byte	PRDesign[] = { D D D D D D D D _
			       D D D D D D D D _
			       D _ _ _ _ _ _ _ _
			       D _ _ _ _ _ _ _ _
			       D _ _ _ _ _ _ _ _
			       D _ _ _ _ _ _ _ _
			       D _ _ _ _ _ _ _ _
			       D _ _ _ _ _ _ C _
			       D C C C C C C C _
			       _ _ _ _ _ _ _ _ _
			     };
#endif


#undef	_
#undef	A
#undef	B
#undef	C
#undef	D
#undef	E



#define	_ 0x00,
#define	x 0xFF,
#define	A _ x _ x _ x _ x
#define	B A A A A A A A A

static	byte	HLDesign[] = { B B B B B };

#undef	_
#undef	x
#undef	A
#undef	B


// 228 yellow
// 211 red
// 226 light green
// 221 blue

#define	A  208,
#define	B  214,
#define	C  220,
#define	D  226,
#define	E  255,

static	byte	LIDesign[][9] = { { A A A
				    A B A
				    A A A },

				  { A B A
				    B C B
				    A B A },

				  { B C B
				    C D C
				    B C B },

				  { C D C
				    D E D
				    C D C },
					     };

#undef	A
#undef	B
#undef	C
#undef	D
#undef	E


#define	R	211,
#define	G	  0,
//226,

static	byte	MEDesign[][9] = { { R R R R R R R R R }, // 0
				  { R R R R R R R R G }, // 1
				  { R R R R R R R G G }, // 2
				  { R R R R R R G G G }, // 3
				  { R R R R R G G G G }, // 4
				  { R R R R G G G G G }, // 5
				  { R R R G G G G G G }, // 6
				  { R R G G G G G G G }, // 7
				  { R G G G G G G G G }, // 8
				  { G G G G G G G G G }, // 9
					     };

#undef	R
#undef	G
*/

#ifdef DEBUG
	BMP_PTR		MB[] = { new BITMAP("BRICK"), NULL };
	BMP_PTR		HL[] = { new BITMAP("HLINE"), NULL };
#endif

	BMP_PTR		MC[] = { new BITMAP("MOUSE"),
				 new BITMAP("DUMMY"),
				 NULL };
	BMP_PTR		PR[] = { new BITMAP("PRESS"), NULL };
	BMP_PTR		SP[] = { new BITMAP("SPK_L"),
				 new BITMAP("SPK_R"),
				 NULL };
	BMP_PTR		LI[] = { new BITMAP("LITE0"),
				 new BITMAP("LITE1"),
				 new BITMAP("LITE2"),
				 new BITMAP("LITE3"),
				 NULL };
