
#ifndef LSTX_H
#define LSTX_H

namespace Grim {

typedef union {
	int vInt;
	float vReal;
	char *pChar;
	long vLong;
	TaggedString *pTStr;
	TProtoFunc *pFunc;
} YYSTYPE;

#define	WRONGTOKEN	258
#define	NIL	259
#define	IF	260
#define	THEN	261
#define	ELSE	262
#define	ELSEIF	263
#define	WHILE	264
#define	DO	265
#define	REPEAT	266
#define	UNTIL	267
#define	END	268
#define	RETURN	269
#define	LOCAL	270
#define	FUNCTION	271
#define	DOTS	272
#define	NUMBER	273
#define	NAME	274
#define	STRING	275
#define	AND	276
#define	OR	277
#define	EQ	278
#define	NE	279
#define	LE	280
#define	GE	281
#define	CONC	282
#define	UNARY	283
#define	NOT	284


extern YYSTYPE luaY_lval;

} // end of namespace Grim

#endif
