
##################################################################################################################

	@WorkingEngines = qw(
		scumm agos sky queen gob saga drascula 
		kyra lure agi touche parallaction 
	);
	@TestingEngines = qw(
		cruise igor made m4 cine 		
	);
	@BrokenEngines = qw(
		sword1
		sword2
	);

	@EnablableEngines = (@WorkingEngines, @TestingEngines);

	@EnablableSubEngines = qw(
		scumm_7_8
		he
	);

	%UseableFeatures = (
		'zlib'		=> 'zlib.lib', 
		'mad'		=> 'libmad.lib', 
		'tremor'	=> 'libtremor.lib',
		'mpeg2'		=> 'libmpeg2.lib'
	);
	
	# these are normally enabled for each variation
	$DefaultFeatures = qw(zlib mad tremor);
	#$DefaultFeatures = qw(zlib mad tremor);

	
	# you can use these below for speed & clarity or override with custom settings
	$DefaultTopMacros = "
		MACRO			USE_ZLIB			// LIB:zlib.lib
		MACRO			USE_MAD				// LIB:libmad.lib
		MACRO			USE_TREMOR			// LIB:libtremor.lib
	";
													
	$DefaultBottomMacros = "
		MACRO			DISABLE_SWORD1		// LIB:scummvm_sword1.lib
		MACRO			DISABLE_SWORD2		// LIB:scummvm_sword2.lib
	";
													
##################################################################################################################
	##
	## General system information, based on $COMPUTERNAME, so this way
	## you can use the same LocalSettings.pl file on multiple machines!
	##
##################################################################################################################

	if ($ENV{'COMPUTERNAME'} eq "PC-21") #########################################################################
	{
		# might use this string for file/dir naming in the future :)
		$Producer = "SumthinWicked";
		$RedirectSTDERR = 0;
		$HaltOnError = 0;
		$SkipExistingPackages = 0;
		$ReallyQuiet = 0;
		$DevBase = "C:\\S";
		
		# specify an optional FTP server to upload to after each Build+Package (can leave empty)
		#$FTP_Host = "host.com";
		$FTP_User = "something";
		$FTP_Pass = "password";
		$FTP_Dir  = "cvsbuilds";
	
		# What Platform SDKs are installed on this machine?
		# possible SDKs: ("UIQ2", UIQ3", "S60v1", "S60v2", "S60v3", "S80", "S90")
		# Note1: the \epoc32 directory needs to be in these rootdirs
		# Note2: these paths do NOT end in a backslash!
	#	$SDK_RootDirs{'UIQ2'}	= "$DevBase\\UIQ_21";
		$SDK_RootDirs{'UIQ3'}	= "$DevBase\\UIQ3";
	#	$SDK_RootDirs{'S60v1'}	= "$DevBase\\S60v1";
	#	$SDK_RootDirs{'S60v2'}	= "$DevBase\\S60v2";
		$SDK_RootDirs{'S60v3'}	= "$DevBase\\S60v3";
	#	$SDK_RootDirs{'S80'}	= "$DevBase\\S80";
	#	$SDK_RootDirs{'S90'}	= "$DevBase\\S90";

		$SDK_ToolchainDirs{'S60v3'} = "$DevBase\\arm-symbianelf\\bin";
		$SDK_ToolchainDirs{'UIQ2'}	= "$DevBase\\ECompXL\\bin"; # only needed for UIQ2/UIQ3
		$SDK_ToolchainDirs{'UIQ3'}	= "$DevBase\\ECompXL\\bin"; # only needed for UIQ2/UIQ3

		# these supporting libraries get built first, then all the Variations
		# Note: the string {'xxx.lib'} is used in checking in build success: so needs to be accurate!
		if (0) # so we can turn them on/off easily
		{
			## Standard libraries
			$SDK_LibraryDirs{'ALL'}{'zlib.lib'}		= "$DevBase\\zlib-1.2.2\\epoc";
			#$SDK_LibraryDirs{'ALL'}{'libmad.lib'}	= "$DevBase\\libmad-0.15.1b\\group";
			$SDK_LibraryDirs{'ALL'}{'libtremor.lib'}= "$DevBase\\tremor\\epoc";
			
			## SDL 1.2.12 / AnotherGuest / Symbian version
			my $SdlBase = "$DevBase\\SDL-1.2.12-ag\\Symbian";
			#$SDK_LibraryDirs{'S60v1'}{'esdl.lib'}	= "$SdlBase\\S60"; // unsupported? 
			#$SDK_LibraryDirs{'S60v2'}{'esdl.lib'}	= "$SdlBase\\S60v2";
			$SDK_LibraryDirs{'S60v3'}{'esdl.lib'}	= "$SdlBase\\S60v3";
			#$SDK_LibraryDirs{'S80'}{'esdl.lib'}	= "$SdlBase\\S80";
			#$SDK_LibraryDirs{'S90'}{'esdl.lib'}	= "$SdlBase\\S90";
			#$SDK_LibraryDirs{'UIQ2'}{'esdl.lib'}	= "$SdlBase\\UIQ2"
			#$SDK_LibraryDirs{'UIQ3'}{'esdl.lib'}	= "$SdlBase\\UIQ3";

			## HardlySupported(TM) :P
			#$SDK_LibraryDirs{'ALL'}{'libmpeg2.lib'} = "$DevBase\\mpeg2dec-0.4.0\\epoc";
		}
		
		# now you can add $VariationSets only built on this PC below this line :)

		#$VariationSets{'ALL'}{'scumm'} = "$DefaultFeatures scumm scumm_7_8 he";
		#$VariationSets{'ALL'}{'all'} = "$DefaultFeatures @WorkingEngines @EnablableSubEngines";

	}
	elsif ($ENV{'COMPUTERNAME'} eq "TSSLND0106") #################################################################
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

		# now you can add $VariationSets only built on this PC below this line :)

	}
	elsif ($ENV{'COMPUTERNAME'} eq "BIGMACHINE") #################################################################
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

		#$SDK_RootDirs{'UIQ2'}= "D:\\UIQ2";
		$SDK_RootDirs{'UIQ3'}= "D:\\UIQ3";
		#$SDK_RootDirs{'S60v1'}= "D:\\S60v1";
		#$SDK_RootDirs{'S60v2'}= "D:\\S60v2";
		$SDK_RootDirs{'S60v3'}= "D:\\S60v3";
		#$SDK_RootDirs{'S80'}= "D:\\S80";
		#$SDK_RootDirs{'S90'}= "D:\\S90";
		$ECompXL_BinDir= "D:\\ECompXL\\";
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

		# now you can add $VariationSets only built on this PC below this line :)

	}
	else #########################################################################################################
	{
		print "ERROR: Computer name ".$ENV{'COMPUTERNAME'}." not recognized! Plz edit _LocalSettings.pl!";
		exit 1;
	}

##################################################################################################################
	##
	## Variation defines:
	##
##################################################################################################################

	# second hash index = literal string used in .sis file created.
	# empty string also removes the trailing '_'. Some 051101 examples:

	# $VariationSets{'UIQ2'}{''} would produce:
	#   scummvm-051101-SymbianUIQ2.sis

	# $VariationSets{'S60v2'}{'agos'} would produce:
	#   scummvm-051101-SymbianS60v2_agos.sis

	# $VariationSets{'ALL'}{'queen'} with all $SDK_RootDirs defined would produce:
	#   scummvm-051101-SymbianUIQ2_queen.sis
	#   scummvm-051101-SymbianUIQ3_queen.sis
	#   scummvm-051101-SymbianS60v1_queen.sis
	#   scummvm-051101-SymbianS60v2_queen.sis
	#   scummvm-051101-SymbianS60v3_queen.sis
	#   scummvm-051101-SymbianS80_queen.sis
	#   scummvm-051101-SymbianS90_queen.sis

	# NOTE: empty $VariationSets{''} string instead of 'ALL' = easy way to disable pkg!
	
	if (1) # all regular combo's
	{
		# the first one includes all SDKs & release-ready engines
	
			$VariationSets{'ALL'}{'all'} = "$DefaultFeatures @WorkingEngines @EnablableSubEngines";
	
		# now one for each ready-for-release engine
		if (0)
		{
			foreach (@WorkingEngines)
			{
				$VariationSets{'ALL'}{$_} = "$DefaultFeatures $_";
			}
			# for scumm, we need to add 2 features:
			#$VariationSets{'ALL'}{'scumm'} .= " scumm_7_8 he";
		}

		# now one for each not-ready-for-release-or-testing engine
		if (0)
		{
			foreach (@TestingEngines)
			{
				$VariationSets{'ALL'}{"test_$_"} = "$DefaultFeatures $_";
			}
		}
		# below here you could specify weird & experimental combinations, non-ready engines
	
			# a small version of the saga engine, because it is so big (no tremor,mad,zlib)
			#$VariationSets{'ALL'}{'saga_mini'} = "saga";
		
			# a smaller version of scumm without support for v7, v8 and HE games
			#$VariationSets{'ALL'}{'scumm_no78he'} = "$DefaultFeatures scumm";
	
			# maybe you feel lucky and want to test the sword engines? :P
			#$VariationSets{'S60v2'}{'test_sword'} = "$DefaultFeatures mpeg2 sword1 sword2";
			#$VariationSets{'UIQ2'}{'test_sword'}  = "$DefaultFeatures mpeg2 sword1 sword2";
	
			# for mega-fast-testing only plz! Warning: contains to engines!
			#$VariationSets{'ALL'}{'fast_empty'} = "";
	
	} # end quick-n-fast if (1|0)


##################################################################################################################

1;
