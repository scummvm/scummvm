/*
$VER: Ext_Inst_so.rexx 0.4 (23.10.2019) Extract and install compiled-in shared libraries from a given ELF binary.
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
If the given filename/path has spaces in it, AmigaDOS/CLI
will add extra quotation marks to secure a sane working path.
Get rid of them to make AREXX find the file and remove leading
and trailing spaces.
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
Create shared objects dump.
*/
ADDRESS COMMAND 'readelf -d 'executable' >so_dump'

/*
Error check, if I/O went wrong.
*/
IF ~OPEN(SO_read,'so_dump','R') THEN DO
	SAY 'File so_dump opening failed!'
	EXIT
END

/*
We know that the dumped shared library entries always start
at line 4. Skip unneeded lines to speed up processing.
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
		Check whether the installed shared libraries are placed in the SDK
		(most of them) or AmigaOS SOBJS: drawer (few of them) and copy them accordingly.
		*/
		IF EXISTS('SDK:local/newlib/lib/'lib.so) THEN
			ADDRESS COMMAND 'copy clone SDK:local/newlib/lib/'lib.so install_path'/sobjs/'
		ELSE
			IF EXISTS('SDK:gcc/lib/'lib.so) THEN
				ADDRESS COMMAND 'copy clone SYS:SOBJS/'lib.so install_path'/sobjs/'
			ELSE DO
				/*
				If a shared library is not found, abort.
				*/
				SAY lib.so' not found! Aborting!'
				EXIT
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
