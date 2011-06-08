Comparing files VOL.H and ..\LM\VOL.H
***** VOL.H

#define         CAT_NAME        "VOL.CAT"
***** ..\LM\VOL.H

#define         CRP             XCrypt

#define         CAT_NAME        "VOL.CAT"
*****

***** VOL.H

#ifndef CRP
  #define       CRP             XCrypt
#endif

#define         XMASK           0xA5
***** ..\LM\VOL.H

#define         XMASK           0xA5
*****

***** VOL.H
  static Boolean Append (byte far * buf, word len);
  static Boolean Write (CFILE& f);
  static Boolean Read (long org, word len, byte far * buf);
***** ..\LM\VOL.H
  static Boolean Append (byte far * buf, word len);
  static Boolean Append (CFILE& f);
  static Boolean Read (long org, word len, byte far * buf);
*****

***** VOL.H
  static Boolean Exist (const char * name);
  static const char * Next (void);
  long Mark (void) { return (BufMark+Ptr) - BegMark; }
***** ..\LM\VOL.H
  static Boolean Exist (const char * name);
  long Mark (void) { return (BufMark+Ptr) - BegMark; }
*****


