#ifndef ARGS_H
#define ARGS_H

#define MAX_ARG	25

#define ftrArgsData		1300
#define ftrArgsCount	1301
#define ftrVars			1302

Char **ArgsInit();
void ArgsAdd(Char **argvP, const Char *argP, const Char *parmP, UInt8 *countArgP);
void ArgsFree(Char **argvP);
void ArgsSetOwner(Char **argvP, UInt16 owner);

void ArgsExportInit(Char **argvP, UInt32 countArg, Boolean arm);
void ArgsExportRelease(Boolean arm);

#endif
