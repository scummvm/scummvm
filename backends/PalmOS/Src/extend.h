#ifndef EXTEND_H
#define EXTEND_H

extern const Char *SCUMMVM_SAVEPATH; // extras.cpp

#define DISABLE_SKY
#define DISABLE_SIMON

#define OutputDebugString	PalmFatalError

void WinDrawWarpChars(const Char *chars, Int16 len, Coord x, Coord y, Coord maxWidth);
UInt16 StrReplace(Char *ioStr, UInt16 inMaxLen, const Char *inParamStr, const Char *fndParamStr);
void PalmFatalError(const Char *err);

#endif