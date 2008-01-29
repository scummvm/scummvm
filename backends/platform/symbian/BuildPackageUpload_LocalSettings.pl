
##################################################################################################################

	# you can use these below for speed & clarity or override with custom settings	
	$DefaultTopMacros = "
		MACRO			USE_ZLIB			// LIB:zlib.lib
		MACRO			USE_MAD				// LIB:libmad.lib
		MACRO			USE_TREMOR			// LIB:libtremor.lib
	";
		
	$DefaultBottomMacros = "
		MACRO			DISABLE_SWORD1		// LIB:scummvm_sword1.lib
		MACRO			DISABLE_SWORD2		// LIB:scummvm_sword2.lib
		MACRO			DISABLE_PARALLACTION	// LIB:scummvm_parallaction.lib		
		MACRO			DISABLE_TOUCHE		// LIB:scummvm_touche.lib
		MACRO			DISABLE_DRASCULA	// LIB:scummvm_drascula.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	";

	##
	## General system information:
	##

	# this way you can use the same LocalSettings.pl file on multiple machines!
	if ($ENV{'COMPUTERNAME'} eq "BRAAMBOOK")
	{
		# might use this string for file/dir naming in the future :)
		$Producer = "SumthinWicked";
		$RedirectSTDERR = 0;
		$HaltOnError = 1;
		$SkipExistingPackages = 0;
		$ReallyQuiet = 0;
		
		# specify an optional FTP server to upload to after each Build+Package (can leave empty)
		#$FTP_Host = "host.domain";
		#$FTP_User = "test";
		#$FTP_Pass = "test";
		#$FTP_Dir  = "test";

		#$FTP_Host = "host.com";
		$FTP_User = "something";
		$FTP_Pass = "password";
		$FTP_Dir  = "cvsbuilds";
	
		# possible SDKs: ("UIQ2", UIQ3", "S60v1", "S60v2", "S60v3", "S80", "S90")
		# Note1: the \epoc32 directory needs to be in these rootdirs
		# Note2: these paths do NOT end in a backslash!
		$SDK_RootDirs{'UIQ2'}	= "C:\\S\\UIQ_21";
		$SDK_RootDirs{'S60v1'}	= "C:\\S\\S60v1";
		$SDK_RootDirs{'S60v2'}	= "C:\\S\\S60v2";
		$SDK_RootDirs{'S80'}	= "C:\\S\\S80";
		$SDK_RootDirs{'S90'}	= "C:\\S\\S90";
		$ECompXL_BinDir			= "C:\\S\\ECompXL\\bin"; # only needed for UIQ
		# you need to specify each of the SDKs used in the blocks below!

		# these supporting libraries get built first, then all the Variations
		# Note: the string {'xxx.lib'} is used in checking in build success: so needs to be accurate!
		if (0) # so we can turn them on/off easily
		{
			#$SDK_LibraryDirs{'ALL'}{'zlib.lib'}		= "C:\\S\\zlib-1.2.2\\epoc";
			#$SDK_LibraryDirs{'ALL'}{'libmad.lib'}	= "C:\\S\\libmad-0.15.1b\\group";
			#$SDK_LibraryDirs{'ALL'}{'libtremor.lib'}= "C:\\S\\tremor\\epoc";
			$SDK_LibraryDirs{'UIQ2'}{'esdl.lib'}	= $SDK_LibraryDirs{'UIQ3'}{'esdl.lib'} = "C:\\S\\ESDL\\epoc\\UIQ";
			#$SDK_LibraryDirs{'S60v1'}{'esdl.lib'}	= $SDK_LibraryDirs{'S60v2'}{'esdl.lib'} = $SDK_LibraryDirs{'S60v3'}{'esdl.lib'} = "C:\\S\\ESDL\\epoc\\S60";
			#$SDK_LibraryDirs{'S80'}{'esdl.lib'}		= "C:\\S\\ESDL\\epoc\\S80";
			#$SDK_LibraryDirs{'S90'}{'esdl.lib'}		= "C:\\S\\ESDL\\epoc\\S90";
			#$SDK_LibraryDirs{'ALL'}{'libmpeg2.lib'} = "C:\\S\\mpeg2dec-0.4.0\\epoc";
		}
			
		# backup :P
		#Path=C:\Progra~1\Active\Python24\.;C:\Program Files\Active\Tcl\bin;C:\Progra~1\Active\Perl\bin\;C:\WINDOWS\system32;C:\W
		#INDOWS;C:\WINDOWS\System32\Wbem;C:\Program Files\ATI Technologies\ATI Control Panel;C:\Program Files\GNU\cvsnt;C:\Progra
		#m Files\WinSCP3\;"C:\Program Files\Common Files\Microsoft Shared\VSA\8.0\VsaEnv\";"c:\Program Files\Microsoft Visual Stu
		#dio 8\VC\bin";"C:\Program Files\UltraEdit-32"		
	}
	elsif ($ENV{'COMPUTERNAME'} eq "TSSLND0106")
	{
		$Producer = "AnotherGuest";
		$RedirectSTDERR = 1;
		$HaltOnError = 0;
		$SkipExistingPackages = 0;
		$ReallyQuiet = 0;
		
		#$FTP_Host = "host.com";
		#$FTP_User = "ag@host.com";
		#$FTP_Pass = "password";
		#$FTP_Dir  = "cvsbuilds";
		
		#$SDK_RootDirs{'UIQ2'}= "C:\\UIQ2";
		$SDK_RootDirs{'UIQ3'}= "C:\\UIQ3";
		#$SDK_RootDirs{'S60v1'}= "C:\\S60v1";
		$SDK_RootDirs{'S60v2'}= "C:\\S60v2";
		$SDK_RootDirs{'S60v3'}= "C:\\S60v3";
		#$SDK_RootDirs{'S80'}= "C:\\S80";
		#$SDK_RootDirs{'S90'}= "C:\\S90";
		$ECompXL_BinDir= "C:\\ECompXL\\";
if (0) # so we can turn them on/off easily
		{
#			$SDK_LibraryDirs{'ALL'}{'zlib.lib'}		= "C:\\S\\zlib-1.2.2\\epoc";
#			$SDK_LibraryDirs{'ALL'}{'libmad.lib'}	= "C:\\S\\libmad-0.15.1b\\group";
			$SDK_LibraryDirs{'ALL'}{'libtremor.lib'}= "C:\\tremor\\epoc";
#			$SDK_LibraryDirs{'UIQ2'}{'esdl.lib'}	= $SDK_LibraryDirs{'UIQ3'}{'esdl.lib'} = "C:\\S\\ESDL\\epoc\\UIQ";
#			$SDK_LibraryDirs{'S60v1'}{'esdl.lib'}	= $SDK_LibraryDirs{'S60v2'}{'esdl.lib'} = $SDK_LibraryDirs{'S60v3'}{'esdl.lib'} = "C:\\S\\ESDL\\epoc\\S60";
#			$SDK_LibraryDirs{'S80'}{'esdl.lib'}		= "C:\\S\\ESDL\\epoc\\S80";
#			$SDK_LibraryDirs{'S90'}{'esdl.lib'}		= "C:\\S\\ESDL\\epoc\\S90";
			#$SDK_LibraryDirs{'ALL'}{'libmpeg2.lib'} = "C:\\S\\mpeg2dec-0.4.0\\epoc";
		}
		# now you can add $SDK_Variations only built on this PC here :)
	}
elsif ($ENV{'COMPUTERNAME'} eq "BIGMACHINE")
	{
		$Producer = "AnotherGuest";
		$RedirectSTDERR = 1;
		$HaltOnError = 0;
		$SkipExistingPackages = 1;
		$ReallyQuiet = 1;
		
		#$FTP_Host = "host.com";
		#$FTP_User = "ag@host.com";
		#$FTP_Pass = "password";
		#$FTP_Dir  = "cvsbuilds";
		
		#$SDK_RootDirs{'UIQ2'}= "E:\\UIQ2";
		$SDK_RootDirs{'UIQ3'}= "E:\\UIQ3";
		#$SDK_RootDirs{'S60v1'}= "E:\\S60v1";
		#$SDK_RootDirs{'S60v2'}= "E:\\S60v2";
		$SDK_RootDirs{'S60v3'}= "E:\\S60v3";
		#$SDK_RootDirs{'S80'}= "E:\\S80";
		#$SDK_RootDirs{'S90'}= "E:\\S90";
		$ECompXL_BinDir= "E:\\ECompXL\\";
if (0) # so we can turn them on/off easily
		{
#			$SDK_LibraryDirs{'ALL'}{'zlib.lib'}		= "C:\\S\\zlib-1.2.2\\epoc";
#			$SDK_LibraryDirs{'ALL'}{'libmad.lib'}	= "C:\\S\\libmad-0.15.1b\\group";
#			$SDK_LibraryDirs{'ALL'}{'libtremor.lib'}= "C:\\tremor\\epoc";
			$SDK_LibraryDirs{'UIQ2'}{'esdl.lib'} = "E:\\WICKED\\ESDL\\epoc\\UIQ";
			$SDK_LibraryDirs{'S60v1'}{'esdl.lib'}	= $SDK_LibraryDirs{'S60v2'}{'esdl.lib'} = "E:\\WICKED\\ESDL\\epoc\\S60";
			$SDK_LibraryDirs{'S80'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S80";
			$SDK_LibraryDirs{'S90'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S90";
			$SDK_LibraryDirs{'S60v3'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\S60\\S60V3";
			$SDK_LibraryDirs{'UIQ3'}{'esdl.lib'}		= "E:\\WICKED\\ESDL\\epoc\\UIQ\\UIQ3";
			#$SDK_LibraryDirs{'ALL'}{'libmpeg2.lib'} = "C:\\S\\mpeg2dec-0.4.0\\epoc";
		}
		# now you can add $SDK_Variations only built on this PC here :)
	}
	else
	{
		print "ERROR: Computer name ".$ENV{'COMPUTERNAME'}." not recognized! Plz edit _LocalSettings.pl!"; 
		exit 1;
	}

	##
	## Variation defines:
	##

	# second hash index = literal string used in .sis file created.
	# empty string also removes the trailing '_'. Some 051101 examples:
	
	# $SDK_Variations{'UIQ2'}{''} would produce:
	#   scummvm-051101-SymbianUIQ2.sis

	# $SDK_Variations{'S60v2'}{'agos'} would produce:
	#   scummvm-051101-SymbianS60v2_agos.sis

	# $SDK_Variations{'ALL'}{'queen'} with all $SDK_RootDirs defined would produce:
	#   scummvm-051101-SymbianUIQ2_queen.sis
	#   scummvm-051101-SymbianUIQ3_queen.sis
	#   scummvm-051101-SymbianS60v1_queen.sis
	#   scummvm-051101-SymbianS60v2_queen.sis
	#   scummvm-051101-SymbianS60v3_queen.sis
	#   scummvm-051101-SymbianS80_queen.sis
	#   scummvm-051101-SymbianS90_queen.sis

	#$SDK_Variations{'ALL'}{'test'} = "$DefaultTopMacro
	#	//MACRO			USE_TREMOR			// LIB:libtremor.lib
	#	//MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
	#	//MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
	#	//MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
	#	//MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
	#	//MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
	#	//MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
	#	//MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
	#$DefaultBottomMacros";
	#$SDK_Variations{'S60v1'}{'test'} = $SDK_Variations{'UIQ2'}{'test'};

if (1) # all regular combo's
{
	# the first one includes all SDKs & release-ready engines
	
	$SDK_Variations{'ALL'}{'all'} = "$DefaultTopMacros
		//MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		//MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		//MACRO			DISABLE_SKY		// LIB:scummvm_sky.lib
		//MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		//MACRO			DISABLE_GOB		// LIB:scummvm_gob.lib
		//MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		//MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
		//MACRO			DISABLE_AGI		// LIB:scummvm_agi.lib
		//MACRO			DISABLE_TOUCHE		// LIB:scummvm_touche.lib
		//MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
		//MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		//MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		//MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib

	$DefaultBottomMacros";

	# now one for each ready-for-release engine
	
	$SDK_Variations{'ALL'}{'scumm'} = "$DefaultTopMacros
		//MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
		MACRO			DISABLE_AGI		// LIB:scummvm_agi.lib	
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";

	$SDK_Variations{'ALL'}{'agos'} = "$DefaultTopMacros
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		//MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY		// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB		// LIB:scummvm_gob.lib
		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
		MACRO			DISABLE_AGI		// LIB:scummvm_agi.lib
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";

	$SDK_Variations{'ALL'}{'sky'} = "$DefaultTopMacros
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		//MACRO			DISABLE_SKY		// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB		// LIB:scummvm_gob.lib
		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
		MACRO			DISABLE_AGI		// LIB:scummvm_agi.lib	
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";

	$SDK_Variations{'ALL'}{'queen'} = "$DefaultTopMacros
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY		// LIB:scummvm_sky.lib
		//MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB		// LIB:scummvm_gob.lib
		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
		MACRO			DISABLE_AGI		// LIB:scummvm_agi.lib
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";

	$SDK_Variations{'ALL'}{'gob'} = "$DefaultTopMacros
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		//MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
		MACRO			DISABLE_AGI			// LIB:scummvm_agi.lib
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";

	$SDK_Variations{'ALL'}{'saga'} = "$DefaultTopMacros
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY		// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB		// LIB:scummvm_gob.lib
		//MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
		MACRO			DISABLE_AGI		// LIB:scummvm_agi.lib
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";

	$SDK_Variations{'ALL'}{'kyra'} = "$DefaultTopMacros
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		//MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
		MACRO			DISABLE_AGI			// LIB:scummvm_agi.lib		
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib	
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";

	# below here you could specify weird & experimental combinations, non-ready engines

	$SDK_Variations{'ALL'}{'saga_mini'} = "
		//MACRO			USE_ZLIB			// LIB:zlib.lib
		//MACRO			USE_MAD				// LIB:libmad.lib
		//MACRO			USE_TREMOR			// LIB:libtremor.lib		
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
		//MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
		MACRO			DISABLE_AGI			// LIB:scummvm_agi.lib		
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib		
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";

#	$SDK_Variations{'ALL'}{'scumm_no78he'} = "
#		MACRO			USE_ZLIB			// LIB:zlib.lib
#		MACRO			USE_MAD				// LIB:libmad.lib
#		//MACRO			USE_TREMOR			// LIB:libtremor.lib
#		MACRO			DISABLE_SCUMM_7_8
#		MACRO			DISABLE_SCUMM_HE		
#		//MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
#		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
#		MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
#		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
#		MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
#		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
#		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
#		MACRO			DISABLE_AGI			// LIB:scummvm_agi.lib		
#		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
#		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib		
#		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
#	$DefaultBottomMacros";
	
#	$SDK_Variations{'ALL'}{'all_vorbis'} = "
#		MACRO			USE_ZLIB			// LIB:zlib.lib
#		MACRO			USE_MAD				// LIB:libmad.lib
#		MACRO			USE_TREMOR			// LIB:libtremor.lib
#		
#		//MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
#		//MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
#		//MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
#		//MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
#		//MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
#		//MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
#		//MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
#	$DefaultBottomMacros";

	$SDK_Variations{'ALL'}{'lure'} = "$DefaultTopMacros
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib				
		//MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_agi.lib
		MACRO			DISABLE_AGI			// LIB:scummvm_agi.lib			
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";

	# empty $SDK_Variations{''} string instead of 'ALL' = package disabled
	$SDK_Variations{'ALL'}{'test_cine'} = "$DefaultTopMacros
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib				
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		//MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
		MACRO			DISABLE_AGI			// LIB:scummvm_agi.lib		
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";

	$SDK_Variations{'ALL'}{'agi'} = "$DefaultTopMacros
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY		// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB		// LIB:scummvm_gob.lib
		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib				
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
		//MACRO			DISABLE_AGI		// LIB:scummvm_agi.lib
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	$DefaultBottomMacros";


	$SDK_Variations{'ALL'}{'test_touche'} = "$DefaultTopMacros
		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
		MACRO			DISABLE_SKY		// LIB:scummvm_sky.lib
		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
		MACRO			DISABLE_GOB		// LIB:scummvm_gob.lib
		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib		
		MACRO			DISABLE_LURE		// LIB:scummvm_lure.lib
		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
		MACRO			DISABLE_AGI		// LIB:scummvm_agi.lib	
		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
		//MACRO			DISABLE_TOUCHE		// LIB:scummvm_touche.lib
		MACRO			DISABLE_SWORD1		// LIB:scummvm_sword1.lib
		MACRO			DISABLE_SWORD2		// LIB:scummvm_sword2.lib
		MACRO			DISABLE_PARALLACTION	// LIB:scummvm_parallaction.lib	
		MACRO			DISABLE_DRASCULA	// LIB:scummvm_drascula.lib		
		MACRO			DISABLE_IGOR		// LIB:scummvm_igor.lib
	";
}

#
#	$SDK_Variations{'S60v2'}{'test_sword'} = "$DefaultTopMacros
#		MACRO			USE_MPEG2			// LIB:libmpeg2.lib
#		MACRO			USE_TREMOR			// LIB:libtremor.lib
#		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
#		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
#		MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
#		MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
#		MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
#		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
#		MACRO			DISABLE_KYRA		// LIB:scummvm_kyra.lib
#		MACRO			DISABLE_PARALLACTION	// LIB:scummvm_parallaction.lib
#		MACRO			DISABLE_CINE		// LIB:scummvm_cine.lib
#		MACRO			DISABLE_CRUISE		// LIB:scummvm_cruise.lib
#		//MACRO			DISABLE_SWORD1		// LIB:scummvm_sword1.lib
#		//MACRO			DISABLE_SWORD2		// LIB:scummvm_sword2.lib
#	";
#	$SDK_Variations{'UIQ2'}{'test_sword'} = $SDK_Variations{'S60v2'}{'test_sword'}
#

# for mega-fast-testing only plz!
#	$SDK_Variations{'ALL'}{'(fast_empty)'} = "
#		//MACRO			USE_ZLIB			// LIB:zlib.lib
#		//MACRO			USE_MAD				// LIB:libmad.lib
#		//MACRO			USE_TREMOR			// LIB:libtremor.lib
#		MACRO			USE_UIQ_SE_VIBRA	// LIB:vibration.lib
#		MACRO			DISABLE_SCUMM		// LIB:scummvm_scumm.lib
#		MACRO			DISABLE_AGOS		// LIB:scummvm_agos.lib
#		MACRO			DISABLE_SKY			// LIB:scummvm_sky.lib
#		//MACRO			DISABLE_QUEEN		// LIB:scummvm_queen.lib
#		MACRO			DISABLE_GOB			// LIB:scummvm_gob.lib
#		MACRO			DISABLE_SAGA		// LIB:scummvm_saga.lib
#	$DefaultBottomMacros";

##################################################################################################################

1;
