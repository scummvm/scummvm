#!/usr/bin/perl

#
# This script is a hack to update the ScummVM version in all (?) files that
# contain it. Obviously, it should be used before a release.

use strict;

if ($#ARGV+1 < 3 or $#ARGV+1 > 4) {
	# TODO: Allow the user to specify the version as "1.2.3svn"
	# and automatically split that into 1, 2, 3, svn
	print STDERR "Usage: $0 MAJOR MINOR PATCH [EXTRA]\n";
	print STDERR "  TODO\n";
	exit 1;
}

# TODO: Verify that major/minor/patch are actually numbers
my $VER_MAJOR = $ARGV[0];
my $VER_MINOR = $ARGV[1];
my $VER_PATCH = $ARGV[2];
my $VER_EXTRA = $ARGV[3];
my $VERSION = "$VER_MAJOR.$VER_MINOR.$VER_PATCH$VER_EXTRA";

die "MAJOR must be a natural number\n" unless ($VER_MAJOR =~ /^\d+$/);
die "MINOR must be a natural number\n" unless ($VER_MINOR =~ /^\d+$/);
die "PATCH must be a natural number\n" unless ($VER_PATCH =~ /^\d+$/);


print "Setting version to '$VERSION'\n";


# List of the files in which we need to perform substitution.
my @subs_files = qw(
	base/internal_version.h
	dists/macosx/Info.plist
	dists/redhat/scummvm.spec
	dists/scummvm.rc
	);

# The following files also contain version data and should possible be modified
# by this script, too. However, the relevant parts of these documentation files
# talk about "Known issues", and I am not sure if we really should automatically
# update these... in fact, it might be better to factor out that information and
# put it into a single "ISSUES" or "KNOWN_ISSUES" file like other projects do...
# * README
# * doc/03_10.tex / doc/03_11.tex  (depends on which branch you look at <shudder>)

# TODO: The following substituion table could easily be extended. In particular
# one could add a function similar to autoconf's AC_SUBST:
#    addSubst(FOO)
# which would automagically add FOO => $FOO to the table
my %subs = (
	VER_MAJOR	=>	$VER_MAJOR,
	VER_MINOR	=>	$VER_MINOR,
	VER_PATCH	=>	$VER_PATCH,
	VER_EXTRA	=>	$VER_EXTRA,
	VERSION		=>	$VERSION
	);

foreach my $file (@subs_files) {
	print "Processing $file...\n";
	open(INPUT, "< $file.in") or die "Can't open '$file.in' for reading: $!\n";
	open(OUTPUT, "> $file") or die "Can't open '$file' for writing: $!\n";
	
	while (<INPUT>) {
		while (my ($key, $value) = each(%subs)) {
			s/\@$key\@/$value/;
		}
		print OUTPUT;
	}
	
	close(INPUT);
	close(OUTPUT);
}
