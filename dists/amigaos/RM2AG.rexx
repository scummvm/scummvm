/*
$VER: RM2AG.rexx 0.25 (09.01.2021) README (.md) to .guide converter.
Converts the ScummVM markdown README file to a basic hypertext AmigaGuide
file and installs it to the given path.
*/

PARSE ARG readme_md install_path

/*
Check if arguments are available, otherwise quit.
*/
IF ~ARG() THEN DO
	SAY 'No Arguments given!'
	SAY 'Usage: RM2AG.rexx README_MD INSTALL_PATH'
	EXIT
END

/*
If the given filename/path has spaces in it, AmigaDOS/CLI will add extra
quotation marks to secure a sane working path.
Get rid of them to make AREXX find the file and remove leading and trailing
spaces.
*/
IF ~EXISTS(readme_md) THEN DO
	SAY readme_md' not available!'
	EXIT
END
ELSE DO
	readme_md=STRIP(readme_md)
	readme_md=COMPRESS(readme_md,'"')
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
	IF ~EXISTS(install_path'/') THEN
		ADDRESS COMMAND 'makedir ALL FORCE 'install_path
END

IF ~OPEN(check_readme,readme_md,'R') THEN DO
	SAY readme_md' opening failed!'
	EXIT
END

IF READCH(check_readme,18) = '# [ScummVM README]' THEN DO
	IF ~CLOSE(check_readme) THEN DO
		SAY readme_md' closing failed!'
		EXIT
	END
END
ELSE DO
	IF ~CLOSE(check_readme) THEN DO
		SAY readme_md' closing failed!'
		EXIT
	END
	SAY "Not the ScummVM README.md file. Aborting!"
	EXIT
END

IF ~OPEN(readme_read,readme_md,'R')  THEN DO
	SAY 'File 'readme_md' opening failed!'
	EXIT
END
IF ~OPEN(guide_write,'README.guide','W') THEN DO
	SAY README.guide' opening failed!'
	EXIT
END

/*
Prepare AmigaGuide file, add intro and fixed text.
*/
CALL WRITELN guide_write,'@DATABASE ScummVM README.guide'
CALL WRITELN guide_write,'@$VER: ScummVM Readme 2.3.0git'
CALL WRITELN guide_write,'@(C) by The ScummVM team'
CALL WRITELN guide_write,'@AUTHOR The ScummVM team'
CALL WRITELN guide_write,'@WORDWRAP'
CALL WRITELN guide_write,'@NODE "main" "ScummVM README Guide"'
CALL WRITELN guide_write,'@{b}'
CALL WRITELN guide_write,SUBSTR(READLN(readme_read),4,14)
CALL WRITELN guide_write,'@{ub}'

/*
Creating main (TOC) link nodes.
*/

DO WHILE EOF(readme_read) = 0
	working_line=READLN(readme_read)

	/*
	Check for start of actual content and, if available, leave TOC link loop.
	*/
	IF POS('## <>1.0<>)',working_line) = 1 THEN
		LEAVE

	/*
	Check for any "rolled over" lines and, if available, read in the rest
	(on the following line) and rejoin them before processing any further.
	e.g.      - [3.11) Flight of the Amazon Queen
        notes](#311-flight-of-the-amazon-queen-notes)
	*/
	IF POS('- [<>',working_line) > 0 THEN DO
		IF POS('(#',working_line) = 0 THEN DO
			rejoin_line=COMPRESS(READLN(readme_read),' ')
			working_line=working_line rejoin_line
		END
	END

	/*
	If no chapter has been found, simply write the line and skip the rest.
	*/
	IF POS('- [',working_line) = 0 THEN
		CALL WRITELN guide_write,working_line
	ELSE DO
		/*
		Fix empty chapters:
		Two chapters (1.0 and 7.8) are "empty", consisting of only it's
		chapter names. Link them to their respective sub chapters (1.1 and
		7.8.1) to not display a blank page.
		If chapter 1.0 is found, add a link node to chapter 1.1.
		*/
		IF POS('  - [<>1.0<>)',working_line) = 1 THEN DO
			/*
			Get rid of the markers, so the following loops won't process them
			again.
			*/
			working_line=COMPRESS(working_line,'[<>')
			CALL WRITELN guide_write,'  @{" 1.0 " Link "1.1"} 'SUBSTR(working_line,1,LASTPOS(']',working_line)-1)
		END

		/*
		If chapter 7.8 is found, add a link node to 7.8.1.
		*/
		IF POS('      - [<>7.8<>)',working_line) = 1 THEN DO
			/*
			Get rid of the markers, so the following loops won't process them
			again.
			*/
			working_line=COMPRESS(working_line,'[<>')
			CALL WRITELN guide_write,'    @{" 7.8 " Link "7.8.1"} 'SUBSTR(working_line,1,LASTPOS(']',working_line)-1)
		END

		/*
		If a single number main chapter is found (1.0 upto 9.0), prepare and
		write the link node.
		Note:
		A "\" (backslash) is treated as escape character in AmigaGuides.
		Remove it from the node links.
		*/
		IF POS('- [<>',working_line) = 3 THEN DO
			CALL WRITELN guide_write,' '
			CALL WRITELN guide_write,'  @{" 'SUBSTR(working_line,POS('<>',working_line)+2,LASTPOS('<>',working_line)-POS('<>',working_line)-2) '" Link "'SUBSTR(working_line,POS('<>',working_line)+2,LASTPOS('<>',working_line)-POS('<>',working_line)-2)'"} 'COMPRESS(SUBSTR(working_line,1,LASTPOS(']',working_line)-1),'*<>[]\')
			/*
			Get rid of the markers, so the following loops won't process them
			again.
			*/
			working_line=COMPRESS(working_line,'-[<>')
		END

		/*
		If a level one sub chapter is found (i.e. 1.1, 1.2 etc.), prepare and
		write the link node.
		*/
		IF POS('- [<>',working_line) = 7 THEN DO
			CALL WRITELN guide_write,'    @{" 'SUBSTR(working_line,POS('<>',working_line)+2,LASTPOS('<>',working_line)-POS('<>',working_line)-2) '" Link "'SUBSTR(working_line,POS('<>',working_line)+2,LASTPOS('<>',working_line)-POS('<>',working_line)-2)'"} 'COMPRESS(SUBSTR(working_line,1,LASTPOS(']',working_line)-1),'*<>[]\')
			/*
			Get rid of the markers, so the following loops won't process them
			again.
			*/
			working_line=COMPRESS(working_line,'.[<>')
		END

		/*
		If a level two sub chapter is found (i.e. 3.6.1, 3.6.2 etc.), prepare
		and write the link node.
		*/
		IF POS('- [<>',working_line) = 11 THEN DO
			CALL WRITELN guide_write,'      @{" 'SUBSTR(working_line,POS('<>',working_line)+2,LASTPOS('<>',working_line)-POS('<>',working_line)-2) '" Link "'SUBSTR(working_line,POS('<>',working_line)+2,LASTPOS('<>',working_line)-POS('<>',working_line)-2)'"} 'COMPRESS(SUBSTR(working_line,1,LASTPOS(']',working_line)-1),'*<>[]\')
			/*
			Get rid of the markers, so the following loops won't process them
			again.
			*/
			working_line=COMPRESS(working_line,'.[<>')
		END
	END
END

/*
Finish TOC (hardcoded as the outro text would be read in last, but needs to
be written after TOC creation finished).
*/
CALL WRITELN guide_write,'-----'
CALL WRITELN guide_write,' '
CALL WRITELN guide_write,'Good Luck and Happy Adventuring!'
CALL WRITELN guide_write,'The ScummVM team.'
CALL WRITELN guide_write,'@{"https://www.scummvm.org/" System "URLOpen https://www.scummvm.org/"}'

/*
Creating sub link nodes.
*/
DO WHILE EOF(readme_read) = 0
	/*
	Change html/markdown links to AmigaGuide ones.
	*/
	IF POS('[here](',working_line) > 0 THEN DO
		working_line=INSERT('@{"',working_line,POS('[',working_line)-1)
		working_line=INSERT('" link ',working_line,POS(']',working_line))
		working_line=INSERT('/main}',working_line,POS(')',working_line))
		working_line=COMPRESS(working_line,'()')
	END

	/*
	If no chapter has been found, simply write the line and skip the rest.
	*/
	IF POS('<>',working_line) = 0 THEN
		CALL WRITELN guide_write,working_line
	ELSE DO
		/*
		Fix empty chapters:
		Two chapters (1.0 and 7.8) are "empty", consisting of only it's
		chapter names. Link them to their respective sub chapters (1.1 and
		7.8.1) to not display a blank page.
		If chapter 1.1 is found, don't close the NODE, just write the line.
		*/
		IF POS('<>1.1<>',working_line) = 1 THEN DO
			/*
			Get rid of the markers, so the following loops won't process them
			again.
			*/
			CALL WRITELN guide_write,COMPRESS(working_line,'<>')
		END

		/*
		If chapter 7.8.1 is found don't close the NODE, just write the line.
		*/
		IF POS('<>7.8.1<>',working_line) = 1 THEN DO
			/*
			Get rid of the markers, so the following loops won't process them
			again.
			*/
			CALL WRITELN guide_write,COMPRESS(working_line,'<>')
		END

		IF POS('<>',working_line) > 0 THEN DO
			/*
			Check for link references inside the text and create link nodes
			for them.
			*/
			IF POS('section <>',working_line) > 0 THEN DO
				working_line=SUBSTR(working_line,1,POS('<>',working_line)-1)'@{"'SUBSTR(working_line,POS('<>',working_line)+2,LASTPOS('<>',working_line)-POS('<>',working_line)-2)'" Link "'SUBSTR(working_line,POS('<>',working_line)+2,LASTPOS('<>',working_line)-POS('<>',working_line)-2)'"}'SUBSTR(working_line,LASTPOS('<>',working_line)+2)
				/*
				Get rid of the markers, so the following loops won't process
				them again.
				*/
				CALL WRITELN guide_write,COMPRESS(working_line,'<>')
			END
			ELSE DO
				/*
				If a chapter has been found, prepare and write the link node.
				*/
				CALL WRITELN guide_write,'@ENDNODE'
				CALL WRITELN guide_write,'@NODE "'SUBSTR(working_line,POS('<>',working_line)+2,LASTPOS('<>',working_line)-POS('<>',working_line)-2)'" "'COMPRESS(working_line,'<>#')'"'
				CALL WRITELN guide_write,' '
				/*
				Get rid of the markers, so the following loops won't process
				them again.
				*/
				CALL WRITELN guide_write,COMPRESS(working_line,'<>')
			END
		END
	END
	/*
	Read in the line at the end of the second loop, as the first line to work
	with was already read in on the end of the first loop.
	*/
	working_line=READLN(readme_read)

	/*
	If the outtro text has been found, leave loop and prepare for closing.
	*/
	IF WORD(working_line,1) = '-----' THEN
		LEAVE
END

CALL WRITELN guide_write,'@ENDNODE'

/*
Close guide and clean up.
*/
CALL WRITELN guide_write,'@ENDNODE'

IF ~CLOSE(readme_read) THEN DO
	SAY readme_md' closing failed!'
	EXIT
END
IF ~CLOSE(guide_write) THEN DO
	SAY 'README.guide closing failed!'
	EXIT
END

/*
Install finished README.guide to installation path and delete it on closure.
*/
ADDRESS COMMAND 'copy QUIET README.guide 'install_path
ADDRESS COMMAND 'delete FORCE QUIET README.guide'

EXIT
