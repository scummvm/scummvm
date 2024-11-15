/*
$VER: md2ag.rexx 0.2 (16.02.2022) README(.md) to (amiga).guide converter.
Converts ScummVM's markdown README file to a basic hypertext Amiga guide
file and installs it to a given location.
*/

PARSE ARG p_readme p_instpath

/*
Check if arguments are available, otherwise quit.
*/
IF ~ARG() THEN DO
	SAY 'No Arguments given!'
	SAY 'Usage: md2ag.rexx README.md INSTALLPATH'
	EXIT
END

/*
If the given filename/path has spaces in it, AmigaDOS/CLI
will add extra quotation marks to secure a sane working path.
Get rid of them to make AREXX find the file and remove leading
and trailing spaces.
*/
IF ~EXISTS(p_readme) THEN DO
	SAY p_readme' not available!'
	EXIT
END
ELSE DO
	p_readme=STRIP(p_readme)
	p_readme=COMPRESS(p_readme,'"')
END
IF p_instpath='' THEN DO
	SAY 'No installation destination given!'
	EXIT
END
ELSE DO
	p_instpath=STRIP(p_instpath)
	p_instpath=STRIP(p_instpath,'T','/')
	p_instpath=COMPRESS(p_instpath,'"')
END

/*
Convert README.md to ASCII to get rid of non-displayable characters.
*/
ADDRESS COMMAND 'iconv -f UTF-8 -t ASCII//TRANSLIT 'p_readme' >README.conv'

CALL OPEN read_md,'README.conv','R'

IF READCH(read_md,18)~='# [ScummVM README]' THEN DO
	CALL CLOSE read_md
	SAY p_readme' is not the ScummVM README.md file. Aborting!'
	EXIT
END
ELSE
	/*
	Skip the first "Build Status" line.
	*/
	CALL READLN read_md

CALL OPEN write_guide,'README.guide','W'

/*
Prepare Amiga guide, add intro and fixed text.
*/
CALL WRITELN write_guide,'@DATABASE ScummVM README.guide'
CALL WRITELN write_guide,'@$VER: ScummVM Readme 2.10.0git'
CALL WRITELN write_guide,'@(C) by The ScummVM team'
CALL WRITELN write_guide,'@AUTHOR The ScummVM team'
CALL WRITELN write_guide,'@WORDWRAP'
CALL WRITELN write_guide,'@NODE "main" "ScummVM README Guide"'
CALL WRITELN write_guide,'@{b}'
CALL WRITELN write_guide,'ScummVM README'
CALL WRITELN write_guide,'@{ub}'

DO WHILE ~EOF(read_md)
	v_line=READLN(read_md)

	/*
	Rejoin two rolled over lines that cut a weblink.
	Lines 12, 13 and 40 in the original .md file both hold a weblink
	which is cut due to the lines rolling over.
	Lines are:
	12 - latest release, progress reports and more, please visit the ScummVM [home
	13 - page](https://www.scummvm.org/).
	and
	40 - Please make sure the bug is reproducible, and still occurs in the latest git/Daily build version. Also check the [compatibility list](https://www.scummvm.org/compatibility/) for that game, to ensure the issue is not already known. Please do not report bugs for games that are not listed as completable on the [Supported Games](https://wiki.scummvm.org/index.php?title=Category:Supported_Games) wiki page, or on the compatibility list. We already know those games have bugs!
	*/
	IF POS('[',v_line)>0 THEN DO
		IF POS(']',v_line)=0 THEN DO
			rejoin_line=READLN(read_md)
			v_line=v_line''rejoin_line
		END
	END

	/*
	Change local markdown links to AmigaGuide ones.
	*/
	IF POS('[here](',v_line)>0 THEN DO
		v_locallink=SUBSTR(v_line,POS('(',v_line)+1,POS(')',v_line)-POS('(',v_line)-1)
		v_line=DELSTR(v_line,POS('(',v_line)+1,POS(')',v_line)-POS('(',v_line)-1)
		v_line=INSERT('@{"'v_locallink'" link 'v_locallink'/main}',v_line,POS(']',v_line)+1)
	END

	/*
	Change html markdown links to AmigaGuide ones.
	*/
	IF POS('http',v_line)>0 THEN DO
		v_protocol=SUBSTR(v_line,POS('http',v_line),POS('://',v_line)-POS('http',v_line))
		IF POS('<http',v_line)>0 THEN DO
			v_weblink=SUBSTR(v_line,POS('://',v_line)+3,LASTPOS('>',v_line)-POS('://',v_line)-3)
			v_line=DELSTR(v_line,POS('<',v_line)+1,LASTPOS('>',v_line)-POS('<',v_line)-1)
			SAY v_line
			v_line=INSERT('@{"'v_protocol'://'v_weblink'" System "URLOpen 'v_protocol' 'v_weblink'"}',v_line,POS('<',v_line))
			SAY v_line
		END
		ELSE DO
			v_weblink=SUBSTR(v_line,POS('://',v_line)+3,POS(')',v_line)-POS('://',v_line)-3)
			v_line=DELSTR(v_line,POS('(',v_line)+1,POS(')',v_line)-POS('(',v_line)-1)
			v_line=INSERT('@{"'v_protocol'://'v_weblink'" System "URLOpen 'v_protocol' 'v_weblink'"}',v_line,POS('(',v_line))
		END
	END

	/*
	Use bold font for all links to make the [ ] encapsulated text stand out.
	*/
	IF POS('[',v_line)>0 THEN DO
		v_line=INSERT('@{b}',v_line,POS('[',v_line)-1)
		v_line=INSERT('@{ub} ',v_line,POS(']',v_line))
	END

	CALL WRITELN write_guide,v_line
END

/*
Close guide and clean up.
*/
CALL WRITELN write_guide,'@ENDNODE'

CALL CLOSE read_md
CALL CLOSE write_guide

/*
Install finished README.guide to installation path and delete README.guide.
*/
ADDRESS COMMAND 'copy README.guide 'p_instpath
ADDRESS COMMAND 'delete quiet README.guide'
ADDRESS COMMAND 'delete quiet README.conv'

EXIT
