#ifndef PALMDEFS_H
#define PALMDEFS_H

#define appFileCreator						'ScVM'
#define	sysAppLaunchCustomEngineDelete		(sysAppLaunchCmdCustomBase + 0)
#define	sysAppLaunchCustomEngineGetInfo		(sysAppLaunchCmdCustomBase + 1)

enum {
	GET_NONE		= 0,
	GET_DATACOMMON	= 1 << 1,
	GET_DATAENGINE	= 1 << 2,

	GET_MODE68K		= 1 << 14,
	GET_MODEARM		= 1 << 15,
};

SysAppInfoPtr SysGetAppInfo(SysAppInfoPtr *uiAppPP, SysAppInfoPtr *actionCodeAppPP)
	SYS_TRAP(sysTrapSysGetAppInfo);


#endif
