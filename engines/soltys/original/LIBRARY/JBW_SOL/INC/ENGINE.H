#ifndef	__ENGINE__
#define	__ENGINE__


#include	<jbw.h>



class ENGINE
{
protected:
  static void interrupt (far * OldTimer) (...);
  static void interrupt NewTimer (...);
public:
  ENGINE (word tdiv);
  ~ENGINE (void);
};




#endif
