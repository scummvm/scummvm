#ifndef	__ENGINE__
#define	__ENGINE__


#define		TMR_RATE		600
#define		TMR_DIV			((0x8000/TMR_RATE)*2)


class ENGINE
{
  static void interrupt (far * OldTimer) (...);
  static void interrupt NewTimer (...);
public:
  ENGINE (void);
  ~ENGINE (void);
};




#endif
