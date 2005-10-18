#ifndef PALMDEFS_H
#define PALMDEFS_H

#define appFileCreator						'ScVM'
#define	sysAppLaunchCustomDeleteEngine		(sysAppLaunchCmdCustomBase + 0)

SysAppInfoPtr SysGetAppInfo(SysAppInfoPtr *uiAppPP, SysAppInfoPtr *actionCodeAppPP)
	SYS_TRAP(sysTrapSysGetAppInfo);


#endif
