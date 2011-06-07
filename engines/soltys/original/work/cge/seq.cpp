#include	"vga13h.h"

#define		DLY1	3
#define		DLY2	(DLY1+DLY1)

#define		N	48
#define		NW	49
#define		W	50
#define		SW	51
#define		S	52
#define		SE	53
#define		E	54
#define		NE	55

#define		N01	 0
#define		N02	 1
#define		N03	 2
#define		N04	 3
#define		N05	 4
#define		N06	 5
#define		N07	 6
#define		N08	 7

#define		W01	 8
#define		W02	 9
#define		W03	10
#define		W04	11
#define		W05	12
#define		W06	13
#define		W07	14
#define		W08	15
#define		W09	16
#define		W10	17
#define		W11	18
#define		W12	19
#define		W13	20
#define		W14	21
#define		W15	22
#define		W16	23

#define		S01	24
#define		S02	25
#define		S03	26
#define		S04	27
#define		S05	28
#define		S06	29
#define		S07	30
#define		S08	31

#define		E01	32
#define		E02	33
#define		E03	34
#define		E04	35
#define		E05	36
#define		E06	37
#define		E07	38
#define		E08	39
#define		E09	40
#define		E10	41
#define		E11	42
#define		E12	43
#define		E13	44
#define		E14	45
#define		E15	46
#define		E16	47


//  PHA  NXT   DX   DY   DEALY
SEQ	North[] = {
  //turn left - 0
  {  NW,  1,    0,   0,    DLY2},
  {   W,  2,    0,   0,    DLY2},
  {  SW,  3,    0,   0,    DLY2},
  {   S,  4,    0,   0,    DLY2},
  {  SE,  5,    0,   0,    DLY2},
  {   E,  6,    0,   0,    DLY2},
  {  NE,  7,    0,   0,    DLY2},
  {   N, 16,    0,   0,    DLY2},
  //turn right - 8
  {  NE,  9,    0,   0,    DLY2},
  {   E, 10,    0,   0,    DLY2},
  {  SE, 11,    0,   0,    DLY2},
  {   S, 12,    0,   0,    DLY2},
  {  SW, 13,    0,   0,    DLY2},
  {   W, 14,    0,   0,    DLY2},
  {  NW, 15,    0,   0,    DLY2},
  {   N, 16,    0,   0,    DLY2},
  //walk - 16
  { N01, 17,    0,  -2,    DLY2},
  { N02, 18,    0,  -2,    DLY2},
  { N03, 19,    0,  -2,    DLY2},
  { N04, 20,    0,  -2,    DLY2},
  { N05, 21,    0,  -2,    DLY2},
  { N06, 22,    0,  -2,    DLY2},
  { N07, 23,    0,  -2,    DLY2},
  { N08, 16,    0,  -2,    DLY2},
  };

SEQ	East[] = {
  //turn left - 0
  {  NE,  1,    0,   0,    DLY2},
  {   N,  2,    0,   0,    DLY2},
  {  NW,  3,    0,   0,    DLY2},
  {   W,  4,    0,   0,    DLY2},
  {  SW,  5,    0,   0,    DLY2},
  {   S,  6,    0,   0,    DLY2},
  {  SE,  7,    0,   0,    DLY2},
  {   E, 16,    0,   0,    DLY2},
  //turn right - 8
  {  SE,  9,    0,   0,    DLY2},
  {   S, 10,    0,   0,    DLY2},
  {  SW, 11,    0,   0,    DLY2},
  {   W, 12,    0,   0,    DLY2},
  {  NW, 13,    0,   0,    DLY2},
  {   N, 14,    0,   0,    DLY2},
  {  NE, 15,    0,   0,    DLY2},
  {   E, 16,    0,   0,    DLY2},
  //walk - 16
  { E01, 17,    2,   0,    DLY1},
  { E02, 18,    2,   0,    DLY1},
  { E03, 19,    2,   0,    DLY1},
  { E04, 20,    2,   0,    DLY1},
  { E05, 21,    2,   0,    DLY1},
  { E06, 22,    2,   0,    DLY1},
  { E07, 23,    2,   0,    DLY1},
  { E08, 24,    2,   0,    DLY1},
  { E09, 25,    2,   0,    DLY1},
  { E10, 26,    2,   0,    DLY1},
  { E11, 27,    2,   0,    DLY1},
  { E12, 28,    2,   0,    DLY1},
  { E13, 29,    2,   0,    DLY1},
  { E14, 30,    2,   0,    DLY1},
  { E15, 31,    2,   0,    DLY1},
  { E16, 16,    2,   0,    DLY1},
  };

SEQ	South[] = {
  //turn left - 0
  {  SE,  1,    0,   0,    DLY2},
  {   E,  2,    0,   0,    DLY2},
  {  NE,  3,    0,   0,    DLY2},
  {   N,  4,    0,   0,    DLY2},
  {  NW,  5,    0,   0,    DLY2},
  {   W,  6,    0,   0,    DLY2},
  {  SW,  7,    0,   0,    DLY2},
  {   S, 16,    0,   0,    DLY2},
  //turn right - 8
  {  SW,  9,    0,   0,    DLY2},
  {   W, 10,    0,   0,    DLY2},
  {  NW, 11,    0,   0,    DLY2},
  {   N, 12,    0,   0,    DLY2},
  {  NE, 13,    0,   0,    DLY2},
  {   E, 14,    0,   0,    DLY2},
  {  SE, 15,    0,   0,    DLY2},
  {   S, 16,    0,   0,    DLY2},
  //walk - 16
  { S01, 17,    0,   2,    DLY2},
  { S02, 18,    0,   2,    DLY2},
  { S03, 19,    0,   2,    DLY2},
  { S04, 20,    0,   2,    DLY2},
  { S05, 21,    0,   2,    DLY2},
  { S06, 22,    0,   2,    DLY2},
  { S07, 23,    0,   2,    DLY2},
  { S08, 16,    0,   2,    DLY2},
  };

SEQ	West[] = {
  //turn left - 0
  {  SW,  1,    0,   0,    DLY2},
  {   S,  2,    0,   0,    DLY2},
  {  SE,  3,    0,   0,    DLY2},
  {   E,  4,    0,   0,    DLY2},
  {  NE,  5,    0,   0,    DLY2},
  {   N,  6,    0,   0,    DLY2},
  {  NW,  7,    0,   0,    DLY2},
  {   W, 16,    0,   0,    DLY2},
  //turn right - 8
  {  NW,  9,    0,   0,    DLY2},
  {   N, 10,    0,   0,    DLY2},
  {  NE, 11,    0,   0,    DLY2},
  {   E, 12,    0,   0,    DLY2},
  {  SE, 13,    0,   0,    DLY2},
  {   S, 14,    0,   0,    DLY2},
  {  SW, 15,    0,   0,    DLY2},
  {   W, 16,    0,   0,    DLY2},
  //walk - 16
  { W01, 17,   -2,   0,    DLY1},
  { W02, 18,   -2,   0,    DLY1},
  { W03, 19,   -2,   0,    DLY1},
  { W04, 20,   -2,   0,    DLY1},
  { W05, 21,   -2,   0,    DLY1},
  { W06, 22,   -2,   0,    DLY1},
  { W07, 23,   -2,   0,    DLY1},
  { W08, 24,   -2,   0,    DLY1},
  { W09, 25,   -2,   0,    DLY1},
  { W10, 26,   -2,   0,    DLY1},
  { W11, 27,   -2,   0,    DLY1},
  { W12, 28,   -2,   0,    DLY1},
  { W13, 29,   -2,   0,    DLY1},
  { W14, 30,   -2,   0,    DLY1},
  { W15, 31,   -2,   0,    DLY1},
  { W16, 16,   -2,   0,    DLY1},
  };

extern	SEQ *	Compass[] = { North, West, South, East };
extern	SEQ	TurnToS[] = {
//left
  {  SE,  1,    0,   0,    DLY2},
  {   E,  2,    0,   0,    DLY2},
  {  NE,  3,    0,   0,    DLY2},
  {   N,  4,    0,   0,    DLY2},
  {  NW,  5,    0,   0,    DLY2},
  {   W,  6,    0,   0,    DLY2},
  {  SW,  7,    0,   0,    DLY2},
  {   S,  7,    0,   0,    DLY2},
//right
  {  SW,  9,    0,   0,    DLY2},
  {   W, 10,    0,   0,    DLY2},
  {  NW, 11,    0,   0,    DLY2},
  {   N, 12,    0,   0,    DLY2},
  {  NE, 13,    0,   0,    DLY2},
  {   E, 14,    0,   0,    DLY2},
  {  SE, 15,    0,   0,    DLY2},
  {   S, 15,    0,   0,    DLY2},
  };