#include "backends/fs/AbstractFilesystemFactory.h"

/*
 * All the following includes choose, at compile time, which specific backend will be used
 * during the execution of the ScummVM.
 * 
 * It has to be done this way because not all the necessary libraries will be available in
 * all build environments. Additionally, this results in smaller binaries.
 */
#if defined(__amigaos4__)
	#include "backends/fs/amigaos4/AmigaOSFilesystemFactory.cpp"
#endif
 
#if defined(__DC__)
	#include "backends/fs/dc/RoninCDFilesystemFactory.cpp"
#endif

#if defined(__DS__)
	#include "backends/fs/ds/DSFilesystemFactory.cpp"
#endif

#if defined(__GP32__)
	#include "backends/fs/gp32/GP32FilesystemFactory.cpp"
#endif

#if defined(__MORPHOS__)
	#include "backends/fs/morphos/ABoxFilesystemFactory.cpp"
#endif

#if defined(PALMOS_MODE)
	#include "backends/fs/palmos/PalmOSFilesystemFactory.cpp"
#endif

#if defined(__PLAYSTATION2__)
	#include "backends/fs/ps2/Ps2FilesystemFactory.cpp"
#endif

#if defined(__PSP__)
	#include "backends/fs/psp/PSPFilesystemFactory.cpp"
#endif

#if defined(__SYMBIAN32__)
	#include "backends/fs/symbian/SymbianFilesystemFactory.cpp"
#endif

#if defined(UNIX)
	#include "backends/fs/posix/POSIXFilesystemFactory.cpp"
#endif

#if defined(WIN32)
	#include "backends/fs/windows/WindowsFilesystemFactory.cpp"
#endif

/**
 * Creates concrete FilesystemFactory objects depending on the current architecture.
 */
class FilesystemFactoryMaker {
public:

	/**
	 * Returns the correct concrete factory depending on the current build architecture.
	 */
	static AbstractFilesystemFactory *makeFactory();
	
protected:
	FilesystemFactoryMaker() {}; // avoid instances of this class
};

AbstractFilesystemFactory *FilesystemFactoryMaker::makeFactory(){
	#if defined(__amigaos4__)
		return AmigaOSFilesystemFactory::instance();
	#endif
	
	#if defined(__DC__)
		return RoninCDFilesystemFactory::instance();
	#endif
	
	#if defined(__DS__)
		return DSFilesystemFactory::instance();
	#endif
	
	#if defined(__GP32__)
		return GP32FilesystemFactory::instance();
	#endif
	
	#if defined(__MORPHOS__)
		return ABoxFilesystemFactory::instance();
	#endif
	
	#if defined(PALMOS_MODE)
		return PalmOSFilesystemFactory::instance();
	#endif
	
	#if defined(__PLAYSTATION2__)
		return Ps2FilesystemFactory::instance();
	#endif
	
	#if defined(__PSP__)
		return PSPFilesystemFactory::instance();
	#endif
	
	#if defined(__SYMBIAN32__)
		return SymbianFilesystemFactory::instance();
	#endif
	
	#if defined(UNIX)
		return POSIXFilesystemFactory::instance();
	#endif
	
	#if defined(WIN32)
		return WindowsFilesystemFactory::instance();
	#endif
}
