
use Cwd;

print "
=======================================================================================
Preparing to update all the Symbian MMP project files with objects from module.mk files
=======================================================================================

";
	
$buildDir = getcwd();
chdir("../../../");

my @std = (""); # section standard, no #ifdef
my @exc = ("mt32","fluidsynth"); # exclusions for sound

#arseModule(mmpStr,		dirStr,		ifdefArray,		[exclusionsArray])
#ParseModule("_base",	"base",		\@std); # now in ./TRG/EScummVM_TRG.mmp, these never change anyways...
ParseModule("_base",	"common",	\@std);
ParseModule("_base",	"gui",		\@std);
ParseModule("_base",	"graphics",	\@std);
ParseModule("_base",	"sound",	\@std,		\@exc);

ParseModule("_scumm",	"scumm",	\@std);
ParseModule("_queen",	"queen",	\@std);
ParseModule("_simon",	"simon",	\@std);
ParseModule("_sky",		"sky",		\@std);
ParseModule("_gob",		"gob",		\@std);
ParseModule("_saga",	"saga",		\@std);
ParseModule("_kyra",	"kyra",		\@std);

print "
=======================================================================================
Updating slave MACRO settings in MMP files from master 'scummvm_base.mmp'
=======================================================================================

";

@mmp_files = (	"scummvm_scumm.mmp", "scummvm_queen.mmp", "scummvm_simon.mmp", "scummvm_sky.mmp", "scummvm_gob.mmp", "scummvm_saga.mmp", "scummvm_kyra.mmp", 
				"UIQ/EScummVM_UIQ.mmp", "S60/EScummVM_S60_EXE.mmp", "S80/EScummVM_S80.mmp", "S90/EScummVM_S90.mmp");

UpdateSlaveMacros();

print "
=======================================================================================
Done. Enjoy :P
=======================================================================================
";

##################################################################################################################
##################################################################################################################

# parses multiple sections per mmp/module 
sub ParseModule
{
	my ($mmp,$module,$sections,$exclusions) = @_;
	my @sections = @{$sections};
	my @exclusions = @{$exclusions};

	foreach $section (@sections)
	{
		CheckForModuleMK($module, $section, @exclusions);
		UpdateProjectFile($mmp, $module, $section);
	}
}

##################################################################################################################

# parses all module.mk files in a dir and its subdirs
sub CheckForModuleMK
{
	my ($item,$section,@exclusions) = @_;

	if (-d $item)
	{
		#print "$item\n";
		
		opendir DIR, $item;
		#my @Files = readdir DIR;
		my @Files = grep s/^([^\.].*)$/$1/, readdir DIR;
		closedir DIR;

		foreach $entry (@Files)
		{
			CheckForModuleMK("$item/$entry", $section, @exclusions);
		}
	}

	if (-f $item and $item =~ /.*\/module.mk$/)
	{
		my $sec = "";
		my $secnum = 0;
		
		print "Parsing  $item for section '$section' ... ";

		open FILE, $item;
		my @lines = <FILE>;
		close FILE;

		my $count = @lines;
		print "$count lines";
		
		A: foreach $line (@lines)
		{
			# found a section? reset 
			if ($line =~ /^ifndef (.*)/)
			{
				$sec = $1;
			}
			
			# found an object? Not uncommented!
			if (!($line =~ /^#/) && $line =~ s/\.o/.cpp/)
			{
				# handle this section?
				if ($sec eq $section)
				{
					$line =~ s/^\s*//g; # remove possible leading whitespace
					$line =~ s/ \\//; # remove possible trailing ' \'
					$line =~ s/\//\\/g; # replace / with \
					chop($line); # remove \n
					
					# do we need to skip this file?
					foreach $exclusion (@exclusions)
					{
						if ($line =~ /$exclusion/)
						{
							#print "\n         !$line (excluded)";
							next A;
						}
					}
					
					$secnum++;
					#print "\n         $line";
					$output .= "SOURCE $line\n";
				}
			}
		}
		print " -- $secnum objects selected\n";
	}
}

##################################################################################################################

# update an MMP project file with the new objects
sub UpdateProjectFile
{
	my ($mmp,$module,$section) = @_;
	my $n = "AUTO_OBJECTS_".uc($module)."_$section";
	my $a = "\/\/START_$n\/\/";
	my $b = "\/\/STOP_$n\/\/";
	my $name = "scummvm$mmp.mmp";	
	my $file = "$buildDir/$name";
	my $updated = " Updated @ ".localtime();

	print "     ===>Updating backends/epoc/build/$name @ $n ... ";

	open FILE, "$file";
	my @lines = <FILE>;
	close FILE;
	
	my $onestr = join("",@lines);
	$onestr =~ s/$a.*$b/$a$updated\n$output$b/s;
	
	open FILE, ">$file";
	print FILE $onestr;
	close FILE;
	
	print "done.\n";
	
	$output = "";
}

##################################################################################################################

sub UpdateSlaveMacros
{
	my $updated = " Updated @ ".localtime();

	my $name = "scummvm_base.mmp";	
	my $file = "$buildDir/$name";
	print "Reading master MACROS from backends/epoc/build/$name ... ";

	open FILE, "$file";
	my @lines = <FILE>;
	close FILE;
	my $onestr = join("",@lines);

	my $n = "AUTO_MACROS_MASTER";
	my $a = "\/\/START_$n\/\/";
	my $b = "\/\/STOP_$n\/\/";
	$onestr =~ /$a(.*)$b/s;
	my $macros = $1;
	
	my $libs = "\n// automagically enabled static libs from macros above\n";
	my $libZ = "STATICLIBRARY	scummvm_base.lib // must be above USE_* .libs\n";
	my $macro_counter = 0;
	my $macros2 = "\n"; # output

	foreach $line (split("\n", $macros))
	{
		# do we need to add a static .lib?
		if ($line =~ /^.*MACRO\s*([A-Z_]*)\s*\/\/\s*LIB\:(.*)$/)
		{
			my $macro = $1; my $lib = $2;
			
			# this macro enabled? then also add the .lib
			if ($line =~ /^\s*MACRO\s*$macro/m)
			{
				$libZ .= "STATICLIBRARY	$lib\n" if ($macro =~ /^USE_/);
			}
			else
			{
				$libs .= "STATICLIBRARY	$lib\n" if ($macro =~ /^DISABLE_/);
			}
			$macro_counter++;
		}
		# no comment? add the macro
		if ($line =~ /^\s*MACRO/)
		{
			$macros2 .= "$line\n";
		}
	}		

	print "$macro_counter macro lines.\n";

	$n = "AUTO_MACROS_SLAVE";
	$a = "\/\/START_$n\/\/";
	$b = "\/\/STOP_$n\/\/";
	
	foreach $name (@mmp_files)
	{
		$file = "$buildDir/$name";
		print "Updating macros in backends/epoc/build/$name ... ";
	
		open FILE, "$file";
		@lines = <FILE>;
		close FILE;
		
		$onestr = join("",@lines);
	
		# slash in name means it's a phone specific build file: add LIBs
		my $libs2 = ""; # output
		$libs2 .= "$libs$libZ" if ($name =~ /\//);

		$onestr =~ s/$a.*$b/$a$updated$macros2$libs2$b/s;
		
		open FILE, ">$file";
		print FILE $onestr;
		close FILE;

		my $count = @lines;
		print "wrote $count lines.\n";
	}
}		

##################################################################################################################
