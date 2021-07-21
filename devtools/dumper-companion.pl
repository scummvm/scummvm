#!/usr/bin/perl
#
# Dumping Mac files into MacBinary format
# Extractins HFS+ disk volumes
# Encoding/decoding into punycode

use strict;
use utf8;
use Carp;

use Getopt::Std;
use Encode;
use File::Find;

use integer;

use constant BASE => 36;
use constant TMIN => 1;
use constant TMAX => 26;
use constant SKEW => 38;
use constant DAMP => 700;
use constant INITIAL_BIAS => 72;
use constant INITIAL_N => 128;

use constant UNICODE_MIN => 0;
use constant UNICODE_MAX => 0x10FFFF;

my $Delimiter = chr 0x2D;
my $BasicRE   = "\x00-\x7f";
my $PunyRE    = "A-Za-z0-9";

sub VERSION_MESSAGE() {
	print "$0 version 1.0\n"
}

sub HELP_MESSAGE();
sub processIso($);
sub processMacbinary();
sub decode_punycode;
sub encode_punycode;

getopts('hmf:c:ed');

if ($::opt_c and $::opt_e) {
	die "$0: -c and -e are mutually exclusive";
}

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
			$dir = $_;

			if ($::opt_c) {
				$dir = encode_utf8(decode($::opt_c, $dir));
			}

			if ($::opt_e) {
				# make 0x81 an escape symbol
				$dir =~ s/\x81/\x81\x81/g;
				# escape non-printables, '/', "'" and '"'
				$dir =~ s/([\x00-\x1f\/'"])/\x81@{[chr(ord($1) + 0x80)]}/g;

				if ($dir =~ /[\x80-\xff]/) {
					$dir = encode_punycode $dir;
				}
			}
			# Replace Mac separators with *nix
			$dir =~ s/:/\//g;

			mkdir "$dir";
			$numdirs++;
		} elsif (/^[fF]/) {
			if (/[fF]i?\s+[^\s]+\s+([0-9]+)\s+([0-9]+)\s+\w+\s+\d+\s+\d+\s+(.*)/) {
				my $res = $1;
				my $data = $2;
				my $fname = $3;

				my $decfname = $fname;

				if ($::opt_c) {
					$decfname = encode_utf8(decode($::opt_c, $fname));
				}

				if ($::opt_e) {
					# make 0x81 an escape symbol
					$decfname =~ s/\x81/\x81\x81/g;
					# escape non-printables, '/', "'" and '"'
					$decfname =~ s/([\x00-\x1f\/'"])/\x81@{[chr(ord($1) + 0x80)]}/g;

					if ($decfname =~ /[\x80-\xff]/) {
						$decfname = encode_punycode $decfname;
					}
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

There are 3 operation modes. Direct MacBinary encoding (Mac-only) and dumping ISO
contents with hfsutils.

Mode 1:
  $0 -m [-e] [-d]
	  Operate in MacBinary encoding mode. Recursively encode all resource forks in the current directory
	  -e encode filenames into punycode
	  -d decode filenames from punycode

Mode 2:
  $0 [-c <encoding>] [-e] -f <file.iso>
	  Operate in disk dumping mode
	  Optionally specify encoding (MacRoman, MacJapanese)
	  If -e is specified, then encode filenames into punycode

Miscellaneous:
  -h, --help   display this help and exit
EOF
}

######### Punycode implementation.
## Borrowed from Net::IDN::Punycode::PP CPAN module version 2.500
##
## Copyright 2002-2004 Tatsuhiko Miyagawa miyagawa@bulknews.net
##
## Copyright 2007-2018 Claus FE<auml>rber CFAERBER@cpan.org

sub _adapt {
    my($delta, $numpoints, $firsttime) = @_;
    $delta = int($firsttime ? $delta / DAMP : $delta / 2);
    $delta += int($delta / $numpoints);
    my $k = 0;
    while ($delta > int(((BASE - TMIN) * TMAX) / 2)) {
        $delta /= BASE - TMIN;
        $k += BASE;
    }
    return $k + (((BASE - TMIN + 1) * $delta) / ($delta + SKEW));
}

sub decode_punycode {
    no warnings 'utf8';

    my $input = shift;

    my $n      = INITIAL_N;
    my $i      = 0;
    my $bias   = INITIAL_BIAS;
    my @output;

    return undef unless defined $input;
    return '' unless length $input;
    return $input unless $input =~ m/^xn--/;

	$input =~ s/^xn--//;

    if ($input =~ s/(.*)$Delimiter//os) {
      my $base_chars = $1;
      croak("non-base character in input for decode_punycode")
        if $base_chars =~ m/[^$BasicRE]/os;
      push @output, split //, $base_chars;
    }
    my $code = $input;

    croak('invalid digit in input for decode_punycode') if $code =~ m/[^$PunyRE]/os;

    utf8::downgrade($input);    ## handling failure of downgrade is more expensive than
                                ## doing the above regexp w/ utf8 semantics

    while (length $code) {
        my $oldi = $i;
        my $w    = 1;
    LOOP:
        for (my $k = BASE; 1; $k += BASE) {
            my $cp = substr($code, 0, 1, '');
            croak("incomplete encoded code point in decode_punycode") if !defined $cp;
            my $digit = ord $cp;

            ## NB: this depends on the PunyRE catching invalid digit characters
            ## before they turn up here
            ##
            $digit = $digit < 0x40 ? $digit + (26-0x30) : ($digit & 0x1f) -1;

            $i += $digit * $w;
            my $t =  $k - $bias;
            $t = $t < TMIN ? TMIN : $t > TMAX ? TMAX : $t;

            last LOOP if $digit < $t;
            $w *= (BASE - $t);
        }
        $bias = _adapt($i - $oldi, @output + 1, $oldi == 0);
        $n += $i / (@output + 1);
        $i = $i % (@output + 1);
        croak('invalid code point') if $n < UNICODE_MIN or $n > UNICODE_MAX;
        splice(@output, $i, 0, chr($n));
        $i++;
    }
    return join '', @output;
}

sub encode_punycode {
    no warnings 'utf8';

    my $input = shift;
    my $input_length = length $input;

    my $output = $input; $output =~ s/[^$BasicRE]+//ogs;

    my $h = my $bb = length $output;
    $output .= $Delimiter if $bb > 0;
    utf8::downgrade($output);   ## no unnecessary use of utf8 semantics

    my @input = map ord, split //, $input;
    my @chars = sort { $a<=> $b } grep { $_ >= INITIAL_N } @input;

    my $n = INITIAL_N;
    my $delta = 0;
    my $bias = INITIAL_BIAS;

    foreach my $m (@chars) {
        next if $m < $n;
        $delta += ($m - $n) * ($h + 1);
        $n = $m;
        for (my $i = 0; $i < $input_length; $i++) {
            my $c = $input[$i];
            $delta++ if $c < $n;
            if ($c == $n) {
                my $q = $delta;
            LOOP:
                for (my $k = BASE; 1; $k += BASE) {
                    my $t = $k - $bias;
                    $t = $t < TMIN ? TMIN : $t > TMAX ? TMAX : $t;

                    last LOOP if $q < $t;

                    my $o = $t + (($q - $t) % (BASE - $t));
                    $output .= chr $o + ($o < 26 ? 0x61 : 0x30-26);

                    $q = int(($q - $t) / (BASE - $t));
                }
                croak("input exceeds punycode limit") if $q > BASE;
                $output .= chr $q + ($q < 26 ? 0x61 : 0x30-26);

                $bias = _adapt($delta, $h + 1, $h == $bb);
                $delta = 0;
                $h++;
            }
        }
        $delta++;
        $n++;
    }
    return 'xn--' . $output;
}
