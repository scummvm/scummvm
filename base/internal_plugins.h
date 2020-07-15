#if !defined(INCLUDED_FROM_BASE_PLUGINS_H) && !defined(RC_INVOKED)
#error This file may only be included by base/plugins.h or dists/scummvm.rc
#endif

// plugin macros are defined in this simple internal header so that scummvm.rc
//  can include them without causing problems for Windows resource compilers.

#define STATIC_PLUGIN 1
#define DYNAMIC_PLUGIN 2

#define PLUGIN_ENABLED_STATIC(ID) \
	(ENABLE_##ID && !PLUGIN_ENABLED_DYNAMIC(ID))

#ifdef DYNAMIC_MODULES
	#define PLUGIN_ENABLED_DYNAMIC(ID) \
		(ENABLE_##ID && (ENABLE_##ID == DYNAMIC_PLUGIN))
#else
	#define PLUGIN_ENABLED_DYNAMIC(ID) 0
#endif

#define PLUGIN_ENABLED(ID) (ENABLE_##ID)
