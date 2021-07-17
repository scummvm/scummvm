#!/usr/bin/perl

use strict;
use Getopt::Std;

sub VERSION_MESSAGE() {
    print "$0 version 1.0\n"
}

sub HELP_MESSAGE();
sub processIso($);

getopts('hmf:e:');

if ($::opt_h) {
    HELP_MESSAGE();
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

    open(my $ls, "-|", "hls -1ablRU");

    my $dir = "";
    my $mdir = "";

    while (<$ls>) {
        chomp;
        print "$_...                          \r";
        flush STDOUT;

        if (/^:/) {
            $mdir = $_;
            s/^://;
            s/:/\//g;
            $dir = $_;
            mkdir "$_";
        } elsif (/^[fF]/) {
            if (/[fF]i?\s+[^\s]+\s+([0-9]+)\s+([0-9]+)\s+\w+\s+\d+\s+\d+\s+(.*)/) {
                my $res = $1;
                my $data = $2;
                my $fname = $3;

                if ($res != 0) {
                    system("hcopy -m -- $mdir$fname $dir$fname");
                } else {
                    system("hcopy -r -- $mdir$fname $dir$fname");
                }
            } else {
                die "Bad format:\n$_\n";
            }
        }
    }
    print "\n";

    print "Unounting ISO...";
    flush STDOUT;

    system("humount >/dev/null 2>&1") == 0 or die "Can't execute humount";
    print "done\n";
}

sub HELP_MESSAGE() {
		print <<EOF;
Usage: $0 [OPTIONS]...

Dumping Mac files into MacBinary format

There are 2 operation modes. Direct MacBinary encoding (Mac-only) and dumping ISO
contents with hfsutils.

Mode 1:
  $0 -m
      Operate in MacBinary encoding mode

Mode 2:
  $0 [-e <encoding>] -f <file.iso>
      Operate in disk dumping mode
	  Optionally specify 'jap' for using 'recode' for converting Japanese file names

Miscellaneous:
  -h, --help   display this help and exit
EOF
}
