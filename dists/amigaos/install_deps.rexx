/*
$VER: install_deps.rexx 0.6 (28.04.2024) Install dependant shared libraries

Extracts and installs .so libraries, the main binary depends on, to it's own sobjs/ dir
*/

PARSE ARG argBinary argPath

/*
Determine REXX interpreter
*/
PARSE UPPER VERSION language .

/*
Check if arguments are available, otherwise quit
*/
IF ~ARG() THEN DO
	SAY 'No Arguments given!'
	SAY 'Usage: Install_deps.rexx argBinary argPath'
	EXIT
END

/*
If the given filename/path has spaces in it, AmigaDOS/CLI will add extra
quotation marks to secure a sane working path
Get rid of those to make AREXX find the file and remove leading and trailing spaces
*/
IF ~EXISTS(argBinary) THEN DO
	SAY argBinary' not available!'
	EXIT
END
ELSE DO
	argBinary=STRIP(argBinary)
	argBinary=COMPRESS(argBinary,'"')
END
IF installpath = '' THEN DO
	SAY 'No installation destination/path given!'
	EXIT
END
ELSE DO
	argPath=STRIP(argPath)
	argPath=STRIP(argPath,'T','/')
	argPath=COMPRESS(argPath,'"')
	/*
	Check for destination path and create it, if needed
	*/
	IF ~EXISTS(argPath'/sobjs/') THEN
		ADDRESS COMMAND 'makedir all 'argPath'/sobjs'
END

/*
Save used gcc version, which is needed later on to install the correct libgcc.so version
*/
ADDRESS COMMAND 'gcc -dumpversion >so_dump'

/*
Create shared objects dump
*/
ADDRESS COMMAND 'readelf -d 'argBinary' >>so_dump'

/*
Error check, if I/O went wrong
*/
IF ~OPEN(fileLiblist,'so_dump','R') THEN DO
	SAY 'File so_dump opening failed!'
	EXIT
END

/*
Get used gcc version
*/
gcc_version=READLN(fileLiblist)

/*
We know that the dumped shared library entries always start at line 4 (line 5 now with
added gcc version)
Skip unneeded lines to speed up processing
*/
lineRead=CALL READLN(fileLiblist)
lineRead=CALL READLN(fileLiblist)
lineRead=CALL READLN(fileLiblist)

i=1
libPaths.i = 'SDK:local/newlib/lib/'
i=i+1
libPaths.i = 'SDK:newlib/lib/'
i=i+1
libPaths.i = 'SDK:gcc/lib/'
i=i+1
libPaths.i = 'SDK:gcc/lib/gcc/ppc-amigaos/'gcc_version'/'
i=i+1
libPaths.i = 'SOBJS:'
i=i+1

/*
VALUE(arg,, 'ENVIRONMENT') is a Regina REXX extension
*/
IF POS('REGINA', language) ~= 0 THEN DO
	prefix = VALUE('PREFIX',, 'ENVIRONMENT')
	IF prefix <> '' THEN DO
		libPaths.i = prefix'/lib/'
		i=i+1
	END
	prefix = VALUE('CROSS_PREFIX',, 'ENVIRONMENT')
	IF prefix ~= '' THEN DO
		libPaths.i = prefix'/lib/gcc/ppc-amigaos/'gcc_version'/'
		i=i+1
		libPaths.i = prefix'/ppc-amigaos/lib/'
		i=i+1
	END
END
libPaths.0 = i - 1

i=1
DO WHILE i>0
	lineRead=READLN(fileLiblist)
	IF POS('Shared library:', lineRead) > 0 THEN DO
		i=1
		/*
		We know that the shared library names always start at position 59
		*/
		lineLib=SUBSTR(lineRead,59,LASTPOS(']', lineRead)-59)

		/*
		- Find and install the dependant shared libraries from their varying home dirs
		- libgcc.so is deeply hidden inside the gcc install directory tree by default
		  Since people can use different gcc versions we have to determine which to use
		  the correct path
		*/
		DO j = 1 TO libPaths.0
			IF EXISTS(libPaths.j''lineLib) THEN DO
				ADDRESS COMMAND 'copy clone quiet' libPaths.j''lineLib argPath'/sobjs/'
				LEAVE
			END
		END
		IF j > libPaths.0 THEN DO
			/*
			If no shared library is found, abort
			*/
			SAY lineLib' not found! Aborting!'
			EXIT
		END
	END
	ELSE
		i=0
END

/*
AREXX is doing its own cleaning of open files
Closing the file anyway
*/
IF ~CLOSE(fileLiblist) THEN DO
	SAY 'File so_dump closing failed!'
	EXIT
END

ADDRESS COMMAND 'delete force quiet so_dump'

EXIT
