#ifndef	__EDIT__
#define	__EDIT__

#include	<wind.h>
#include	<alloc.h>

#ifdef	FAR_EDIT
  #define	EditMemChr	_fmemchr
  #define	EditMovMem	_fmovmem
  #define	EditMemSet	_fmemset
  #define	EditMalloc	farmalloc
  #define	EditFree	farfree
  typedef	char far *	EditPtrType;
#else
  #define	EditMemChr	memchr
  #define	EditMovMem	movmem
  #define	EditMemSet	memset
  #define	EditMalloc	malloc
  #define	EditFree	free
  typedef	char *		EditPtrType;
#endif

#define		WES(w)		((EditStat *)(w->Body.Near))

#define MaxSc_Wid 160
#define MaxSc_Hig  64

typedef	struct	{
//		  Boolean	EditMode;
		  Boolean	InsMode;
		  Boolean	Updated;
		  Boolean	Valid;
		  int		LineCount;
		  int		Line, Column;
		  int		S, Y;
		  word		BufSiz;
		  word		Wrap;
		  EditPtrType	Buff;
		  EditPtrType	Tail;
		  EditPtrType	Here;
		  EditPtrType	LineTab[MaxSc_Hig];
		  byte		LgthTab[MaxSc_Hig];
//		  char		*ToFind;
		  int		(*XlatProc)(int);
		  void		(*StatProc)(Wind *);
		  void		*User;
		} EditStat;

extern	int	EOL_Ch;
extern	int	MRK_Ch;

EC	Boolean	EditKey		(Wind *w);
EC	void	EditInit	(Wind *w);
EC	Boolean	EditFind	(Wind *w, const char *s);
EC	Boolean	EditFormat	(Wind *w);
EC	Boolean	EditDown	(Wind *w);
EC	void	EditReptProc	(Wind *w);
EC	word	ShowEditChar	(void);

#endif
