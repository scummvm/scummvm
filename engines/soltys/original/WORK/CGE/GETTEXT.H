#ifndef	__GETTEXT__
#define	__GETTEXT__

#include	<general.h>
#include	"talk.h"


#define		GTMAX		24
#define		GTBLINK		 6
#define		GTTIME		 6







class GET_TEXT : public TALK
{
  char Buff[GTMAX+2], * Text;
  word Size, Len;
  word Cntr;
  SPRITE * OldKeybClient;
  void (*Click)(void);
public:
  static GET_TEXT * Ptr;
  GET_TEXT (const char * info, char * text, int size, void (*click)(void) = NULL);
  ~GET_TEXT (void);
  void Touch (word mask, int x, int y);
  void Tick (void);
};



#endif
