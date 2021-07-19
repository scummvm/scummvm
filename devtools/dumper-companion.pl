#!/usr/bin/perl

use strict;
use Getopt::Std;
use Encode;
use File::Find;

sub VERSION_MESSAGE() {
	print "$0 version 1.0\n"
}

sub HELP_MESSAGE();
sub processIso($);
sub processMacbinary();

getopts('hmf:e:');

if ($::opt_h) {
	HELP_MESSAGE();
	exit 0;
}

if ($::opt_m) {
	processMacbinary;
	exit 0;
}

if ($::opt_f) {
	processIso($::opt_f);
}

exit 0;

sub processIso($) {
	my $isofile = shift;

	print "Mounting ISO...";
	flush STDOUT;

	system("hmount \"$isofile\" >/dev/null 2>&1") == 0 or die "Can't execute hmount";
	print "done\n";

	open(my $ls, "-|", "hls -1alRU");

	my $dir = "";
	my $mdir = "";

	my $numfiles = 0;
	my $numdirs = 0;
	my $prevlen = 0;

	while (<$ls>) {
		chomp;
		flush STDOUT;

		if (/^:/) {
			$mdir = $_;
			s/^://;
			s/:/\//g;
			$dir = $_;
			if ($::opt_e) {
				$dir = encode_utf8(decode($::opt_e, $dir));
			}
			mkdir "$dir";
			$numdirs++;
		} elsif (/^[fF]/) {
			if (/[fF]i?\s+[^\s]+\s+([0-9]+)\s+([0-9]+)\s+\w+\s+\d+\s+\d+\s+(.*)/) {
				my $res = $1;
				my $data = $2;
				my $fname = $3;

				$fname =~ s'/':'g; # Replace / with :

				my $decfname = $fname;

				if ($::opt_e) {
					$decfname = encode_utf8(decode($::opt_e, $fname));
				}

				print " " x $prevlen;
				print "\r$dir$decfname\r";
				$prevlen = length "$dir$decfname";
				flush STDOUT;

				if ($res != 0) {
					system("hcopy -m -- \"$mdir$fname\" \"$dir$decfname\"");
				} else {
					system("hcopy -r -- \"$mdir$fname\" \"$dir$decfname\"");
				}
				$numfiles++;
			} else {
				die "Bad format:\n$_\n";
			}
		}
	}
	print " " x $prevlen;
	print "\rExtracted $numdirs dirs and $numfiles files\n";

	print "Unounting ISO...";
	flush STDOUT;

	system("humount >/dev/null 2>&1") == 0 or die "Can't execute humount";
	print "done\n";
}

sub processMacbinary() {
	find( sub {
		my $fname = $_;

		if (open F, "$fname/..namedfork/rsrc") {
			print "Resource in $fname\n";
			close F;

			system("macbinary encode \"$fname\"");
			system("touch -r \"$fname\" \"$fname.bin\"");
			system("mv \"$fname.bin\" \"$fname\"");
		}
	}, ".");

}

sub HELP_MESSAGE() {
		print <<EOF;
Usage: $0 [OPTIONS]...

Dumping Mac files into MacBinary format

There are 2 operation modes. Direct MacBinary encoding (Mac-only) and dumping ISO
contents with hfsutils.

Mode 1:
  $0 -m
	  Operate in MacBinary encoding mode. Recursively encode all resource forks in the current directory

Mode 2:
  $0 [-e <encoding>] -f <file.iso>
	  Operate in disk dumping mode
	  Optionally specify encoding (MacRoman, MacJapanese)

Miscellaneous:
  -h, --help   display this help and exit
EOF
}
