/*
$VER: Ext_Inst_so.rexx 0.5 (05.12.2020) Extract and install compiled-in shared libraries from a given ELF binary.
*/

PARSE ARG executable install_path

/*
Check if arguments are available, otherwise quit.
*/
IF ~ARG() THEN DO
	SAY 'No Arguments given!'
	SAY 'Usage: Ext_Inst_so.rexx EXECUTABLE INSTALL_PATH'
	EXIT
END

/*
If the given filename/path has spaces in it, AmigaDOS/CLI will add extra
quotation marks to secure a sane working path.
Get rid of them to make AREXX find the file and remove leading and trailing
spaces.
*/
IF ~EXISTS(executable) THEN DO
	SAY executable' not available!'
	EXIT
END
ELSE DO
	executable=STRIP(executable)
	executable=COMPRESS(executable,'"')
END
IF installpath='' THEN DO
	SAY 'No installation destination given!'
	EXIT
END
ELSE DO
	install_path=STRIP(install_path)
	install_path=STRIP(install_path,'T','/')
	install_path=COMPRESS(install_path,'"')
	/*
	Check for destination path and create it, if needed.
	*/
	IF ~EXISTS(install_path'/sobjs/') THEN
		ADDRESS COMMAND 'makedir 'install_path'/sobjs'
END

/*
Save used gcc version. Needed later to install libgcc.so.
*/
ADDRESS COMMAND 'gcc -dumpversion >so_dump'

/*
Create shared objects dump.
*/
ADDRESS COMMAND 'readelf -d 'executable' >>so_dump'

/*
Error check, if I/O went wrong.
*/
IF ~OPEN(SO_read,'so_dump','R') THEN DO
	SAY 'File so_dump opening failed!'
	EXIT
END

/*
Get used gcc version.
*/
gcc_version=READLN(SO_read)

/*
We know that the dumped shared library entries always start at line 4 (line 5
now with the added gcc version). Skip unneeded lines to speed up processing.
*/
working_line=CALL READLN(SO_read)
working_line=CALL READLN(SO_read)
working_line=CALL READLN(SO_read)

i=1

DO WHILE i>0
	working_line=READLN(SO_read)
	IF POS('Shared library:', working_line)>0 THEN DO
		i=1
		/*
		We know that the shared library names always start at position 59.
		*/
		lib.so=SUBSTR(working_line,59,LASTPOS(']', working_line)-59)
		/*
		- Find and install the mandatory shared libraries from their varying
		  home directories.
		- libgcc.so is deeply hidden inside the gcc install directory tree
		  by default. Since people can use different gcc versions we have
		  determine which to use the correct path.
		*/
		IF EXISTS('SDK:local/newlib/lib/'lib.so) THEN
			ADDRESS COMMAND 'copy clone SDK:local/newlib/lib/'lib.so install_path'/sobjs/'
		ELSE DO
			IF EXISTS('SDK:gcc/lib/'lib.so) THEN
				ADDRESS COMMAND 'copy clone SDK:gcc/lib/'lib.so install_path'/sobjs/'
			ELSE DO
				IF EXISTS('SDK:gcc/lib/gcc/ppc-amigaos/'gcc_version'/'lib.so) THEN
					ADDRESS COMMAND 'copy clone SDK:gcc/lib/gcc/ppc-amigaos/'gcc_version'/'lib.so install_path'/sobjs/'
				ELSE DO
					/*
					If a shared library is not found, abort.
					*/
					SAY lib.so' not found! Aborting!'
					EXIT
				END
			END
		END
	END
	ELSE
		i=0
END

/*
AREXX is doing its own cleaning up of open files.
Close the file manually anyway.
*/
IF ~CLOSE(SO_Read) THEN DO
	SAY 'File so_dump closing failed!'
	EXIT
END

ADDRESS COMMAND 'delete so_dump'

EXIT
