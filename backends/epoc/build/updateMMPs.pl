
use Cwd;

print "
=======================================================================================
Preparing to update all the Symbian MMP project files with objects from module.mk files
=======================================================================================

";
	
$buildDir = getcwd();
chdir("../../../");

my @std = (""); # section standard, no #ifdef
#my @sec = ("", "DISABLE_SCUMM_7_8", "DISABLE_HE"); # sections for scumm DISABLED_s
my @exc = ("mt32","fluidsynth"); # exclusions for sound

#arseModule(mmpStr,		dirStr,		ifdefArray,	[exclusionsArray])
#ParseModule("_base",	"base",		\@std); # now in EScummVM_TRG.mmp, these never change anyways...
ParseModule("_base",	"common",	\@std);
ParseModule("_base",	"gui",		\@std);
ParseModule("_base",	"graphics",	\@std);
ParseModule("_base",	"sound",	\@std,		\@exc);

ParseModule("_scumm",	"scumm",	\@std); #\@sec # no more: enabled all again
ParseModule("_queen",	"queen",	\@std);
ParseModule("_simon",	"simon",	\@std);
ParseModule("_sky",		"sky",		\@std);
ParseModule("_gob",		"gob",		\@std);

print "
=======================================================================================
Done. Enjoy :P
=======================================================================================
";
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
	
#	open FILE, ">$file~";
#	print FILE @lines;
#	close FILE;

	my $onestr = join("",@lines);
	$onestr =~ s/$a.*$b/$a$updated\n$output$b/s;
	
	open FILE, ">$file";
	print FILE $onestr;
	close FILE;
	
	print "done.\n";
	
	$output = "";
}
	
	