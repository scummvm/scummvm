#ifndef __SECURITY__
#define __SECURITY__

#ifndef EC
  #ifdef  __cplusplus
    #define	EC	extern "C"
  #else
    #define	EC
  #endif
#endif

EC int DskChk(int DriveID);

#endif
