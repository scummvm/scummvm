#ifndef	__BTFILE__
#define	__BTFILE__

#include	<general.h>

#define		BT_SIZE		K(1)
#define		BT_KEYLEN	13
#define		BT_LEVELS	2

#define		BT_NONE		0xFFFF
#define		BT_ROOT		0


struct BT_KEYPACK
{
  byte Key[BT_KEYLEN];
  dword Mark;
  word Size;
};



struct far BT_PAGE
{
  struct HEA
    {
      word Count;
      word Down;
    } Hea;
  union
    {
      // dummy filler to make proper size of union
      byte Data[BT_SIZE-sizeof(HEA)];
      // inner version of data: key + word-sized page link
      struct INNER
	{
	  byte Key[BT_KEYLEN];
	  word Down;
	} Inn[(BT_SIZE-sizeof(HEA))/sizeof(INNER)];
      // leaf version of data: key + all user data
      BT_KEYPACK Lea[(BT_SIZE-sizeof(HEA))/sizeof(BT_KEYPACK)];
    };
};





class BTFILE : public IOHAND
{
  struct
    {
      BT_PAGE far * Page;
      word PgNo;
      int Indx;
      Boolean Updt;
    } Buff[BT_LEVELS];
  void PutPage (int lev, Boolean hard = FALSE);
  BT_PAGE far * GetPage (int lev, word pgn);
public:
  BTFILE (const char * name, IOMODE mode = REA, CRYPT * crpt = NULL);
  virtual ~BTFILE (void);
  BT_KEYPACK far * Find(const byte * key);
  BT_KEYPACK far * Next(void);
  void Make(BT_KEYPACK far * keypack, word count);
};

#endif
