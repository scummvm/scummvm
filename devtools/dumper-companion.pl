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
my $outPath = "./";
my $verbose = 0;

sub VERSION_MESSAGE() {
	print "$0 version 1.0\n"
}

sub HELP_MESSAGE();
sub processIso($);
sub processMacbinary();
sub decode_punycode;
sub encode_punycode;
sub encode_punycodefilename;
sub system1($);

getopts('hmf:c:edsS:o:v');

if ($::opt_h) {
	HELP_MESSAGE();
	exit 0;
}

if ($::opt_v) {
	$verbose = 1;
}

if ($::opt_s) {
	local $/;
	my $input = <>;

	print encode_punycodefilename($input) . "\n";
	exit 0;
}

if ($::opt_S) {
	print encode_punycodefilename($::opt_S) . "\n";
	exit 0;
}

if ($::opt_o) {
	$outPath = $::opt_o;

	if ($outPath !~ m'/$') {
		$outPath .= "/";
	}

	if (not -d $outPath) {
		die "Directory $outPath does not exits";
	}
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
	print "\n" if $verbose;

	my $redirect = $verbose ? "" : ">/dev/null 2>&1";

	system1("hmount \"$isofile\" $redirect") == 0 or die "Can't execute hmount";

	print "done\n" unless $verbose;

	print "C: hls -1alRU\n" if $verbose;

	open(my $ls, "-|", "hls -1alRU");

	my @lines;

	while (<$ls>) {
		print "LS: $_" if $verbose;

		push @lines, $_;
	}

	close $ls;


	my $dir = "";
	my $mdir = "";

	my $numfiles = 0;
	my $numdirs = 0;
	my $numrens = 0;
	my $numres = 0;
	my $prevlen = 0;

	for $_ (@lines) {
		print "LINE: $_" if $verbose;

		chomp;

		if (/^:/) {
			$mdir = $_;
			s/^://;
			$dir = $_;

			if ($::opt_c) {
				$dir = encode_utf8(decode($::opt_c, $dir));
			}

			if ($::opt_e) {
				my $dir1 = $dir;
				my $changed = 0;

				$dir = join '/', map { my $a = encode_punycodefilename $_; $changed = 1 if $a ne $_; $a } split /:/, $dir;

				if ($changed) {
					$dir1 =~ s/([\x00-\x1f])/@{[sprintf "\\x%02x", ord($1)]}/g;
					print ((" " x $prevlen) . "\r") unless $verbose;
					print "Renamed dir \"$dir1\" -> \"$dir\"\n";
					$numrens++;
				}
			}

			$dir .= '/' if $dir !~ m'/$';

			mkdir "$outPath$dir";
			print "mkdir \"$outPath$dir\"\n" if $verbose;
			$numdirs++;
		} elsif (/^[fF]/) {
			if (/[fF]i?\s+.{4}\/.{4}\s+([0-9]+)\s+([0-9]+)\s+\w+\s+\d+\s+\d+:?\d*\s+(.*)/) {
				my $res = $1;
				my $data = $2;
				my $fname = $3;

				my $decfname = $fname;

				if ($::opt_c) {
					$decfname = encode_utf8(decode($::opt_c, $fname));
				}

				if ($::opt_e) {
					my $decfname1 = $decfname;
					$decfname = encode_punycodefilename	$decfname;
					if ($decfname1 ne $decfname) {
						$decfname1 =~ s/([\x00-\x1f])/@{[sprintf "\\x%02x", ord($1)]}/g;
						print ((" " x $prevlen) . "\r") unless $verbose;
						print "Renamed file \"$decfname1\" -> \"$decfname\"" . ($res != 0 ? ", macbinary\n" : "\n");
						$numrens++;
					} else {
						if ($res != 0) {
							print ((" " x $prevlen) . "\r") unless $verbose;
							print "Resource \"$decfname\"\n" if $verbose;
						}
					}
				}

				print ((" " x $prevlen) . "\r") unless $verbose;
				print "$dir$decfname\r" unless $verbose;
				$prevlen = length "$dir$decfname";
				flush STDOUT;

				$fname =~ s/([*+\[\]\|<>&`\^\{\}\\])/\\$1/g; # Files cound have special symbols in them, escape, so shell is happy

				if ($res != 0) {
					system1("hcopy -m -- \"$mdir$fname\" \"$outPath$dir$decfname\"") == 0 or die "Can't execute hcopy";
					$numres++;
				} else {
					system1("hcopy -r -- \"$mdir$fname\" \"$outPath$dir$decfname\"") == 0 or die "Can't execute hcopy";
				}
				$numfiles++;
			} else {
				die "Bad format:\n$_\n";
			}
		}
	}
	print ((" " x $prevlen) . "\r") unless $verbose;
	print "Extracted $numdirs dirs and $numfiles files, Macbinary $numres files, made $numrens renames\n";

	print "Unmounting ISO...";
	flush STDOUT;
	print "\n" if $verbose;

	system1("humount $redirect") == 0 or die "Can't execute humount";
	print "done\n" unless $verbose;
}

sub processMacbinary() {
	my $countres = 0;
	my $countren = 0;

	find( sub {
		my $fname = $_;
		my $fname1 = $fname;

		if (open F, "$fname/..namedfork/rsrc") {
			print "Resource in $fname\n";
			close F;

			$countres++;

			system1("macbinary encode \"$fname\"");
			system1("touch -r \"$fname\" \"$fname.bin\"");

			if ($::opt_e) {
				$fname1 = encode_punycodefilename $fname;
			}

			if ($fname1 ne $fname) {
				print "Renamed \"$fname\" -> \"$fname1\"\n" unless $verbose;
				$countren++;
			}

			system1("mv \"$fname.bin\" \"$fname1\"");
		} else {
			if ($::opt_e) {
				$fname1 = encode_punycodefilename $fname;

				if ($fname1 ne $fname) {
					system1("mv \"$fname\" \"$fname1\"");

					$fname =~ s/([\x00-\x1f])/@{[sprintf "\\x%02x", ord($1)]}/g;
					print "Renamed \"$fname\" -> \"$fname1\"\n" unless $verbose;
					$countren++;
				}
			}
		}

	}, ".");
	print "Macbinary $countres files, Renamed $countren files\n";
}

sub HELP_MESSAGE() {
		print <<EOF;
Usage: $0 [OPTIONS]...

Dumping Mac files into MacBinary format

There are 3 operation modes. Direct MacBinary encoding (Mac-only) and dumping ISO
contents with hfsutils.

Mode 1:
  $0 -m [-e]
	  Operate in MacBinary encoding mode. Recursively encode all resource forks in the current directory.
	  It works only in-place.
	  -e encode filenames into punycode

Mode 2:
  $0 [-c <encoding>] [-e] [-o directory] -f <file.iso>
	  Operate in disk dumping mode
	  Optionally specify encoding (MacRoman, MacJapanese)
	  If -e is specified, then encode filenames into punycode
	  If -o is specified, outputs the file to the specified directory

Mode 3:
  $0 -s
     Read whole standard input and encode it with punycode
  $0 -S <string>
     Encodes specified string with punycode

Miscellaneous:
  -h, --help   display this help and exit
EOF
}

sub system1($) {
	my $cmd = shift;

	print "C: $cmd\n" if $verbose;

	return system $cmd;
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
    utf8::decode($input);
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

sub encode_punycodefilename {
	my $decfname = shift;

	$decfname =~ s/\x81/\x81\x79/g;
	# escape non-printables, "/"*[]:+|"
	$decfname =~ s/([\x00-\x1f\/":\*\[\]\+\|\\?%<>,;=])/\x81@{[chr(ord($1) + 0x80)]}/g;

	if ($decfname =~ /[\x80-\xff]/) {
		$decfname = encode_punycode $decfname;
	}

	return $decfname;
}
