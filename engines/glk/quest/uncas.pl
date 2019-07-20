#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Copyright (C) 2006 by Mark J. Tilford                                      #
#                                                                             #
#  This file is part of Geas.                                                 #
#                                                                             #
#  Geas is free software; you can redistribute it and/or modify               #
#  it under the terms of the GNU General Public License as published by       #
#  the Free Software Foundation; either version 2 of the License, or          #
#  (at your option) any later version.                                        #
#                                                                             #
#  Geas is distributed in the hope that it will be useful,                    #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of             #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              #
#  GNU General Public License for more details.                               #
#                                                                             #
#  You should have received a copy of the GNU General Public License          #
#  along with Geas; if not, write to the Free Software                        #
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA #
#                                                                             #
###############################################################################

use strict;

sub mtext {
	my $str = shift;
	my $rv = chr(254);
	foreach (split //, $str) {
	$rv . = chr(255 - ord $_);
}
return $rv . chr(0);
}


sub obfus {
	my $str = shift;
	my $rv = chr(10);
	foreach (split //, $str) {
	$rv . = chr(255 - ord $_);
}
return $rv . chr(0);
}

my $is_raw = 0;

my @hash_data =
    ([1, 'game'], [2, 'procedure'], [3, 'room'], [4, 'object'],
     [5, 'character'], [6, 'text'], [7, 'selection'], [8, 'define'],
     [9, 'end'], [11, 'asl-version'], [12, 'game'], [13, 'version'],
     [14, 'author'], [15, 'copyright'], [16, 'info'], [17, 'start'],
     [18, 'possitems'], [19, 'startitems'], [20, 'prefix'], [21, 'look'],
     [22, 'out'], [23, 'gender'], [24, 'speak'], [25, 'take'], [26, 'alias'],
     [27, 'place'], [28, 'east'], [29, 'north'], [30, 'west'], [31, 'south'],
     [32, 'give'], [33, 'hideobject'], [34, 'hidechar'], [35, 'showobject'],
     [36, 'showchar'], [37, 'collectable'], [38, 'collecatbles'],
     [39, 'command'], [40, 'use'], [41, 'hidden'], [42, 'script'],
     [43, 'font'], [44, 'default'], [45, 'fontname'], [46, 'fontsize'],
     [47, 'startscript'], [48, 'nointro'], [49, 'indescription'],
     [50, 'description'], [51, 'function'], [52, 'setvar'], [53, 'for'],
     [54, 'error'], [55, 'synonyms'], [56, 'beforeturn'], [57, 'afterturn'],
     [58, 'invisible'], [59, 'nodebug'], [60, 'suffix'], [61, 'startin'],
     [62, 'northeast'], [63, 'northwest'], [64, 'southeast'],
     [65, 'southwest'], [66, 'items'], [67, 'examine'], [68, 'detail'],
     [69, 'drop'], [70, 'everywhere'], [71, 'nowhere'], [72, 'on'],
     [73, 'anything'], [74, 'article'], [75, 'gain'], [76, 'properties'],
     [77, 'type'], [78, 'action'], [79, 'displaytype'], [80, 'override'],
     [81, 'enabled'], [82, 'disabled'], [83, 'variable'], [84, 'value'],
     [85, 'display'], [86, 'nozero'], [87, 'onchange'], [88, 'timer'],
     [89, 'alt'], [90, 'lib'], [91, 'up'], [92, 'down'], [93, 'gametype'],
     [94, 'singleplayer'], [95, 'multiplayer'], [150, 'do'], [151, 'if'],
     [152, 'got'], [153, 'then'], [154, 'else'], [155, 'has'], [156, 'say'],
     [157, 'playwav'], [158, 'lose'], [159, 'msg'], [160, 'not'],
     [161, 'playerlose'], [162, 'playerwin'], [163, 'ask'], [164, 'goto'],
     [165, 'set'], [166, 'show'], [167, 'choice'], [168, 'choose'],
     [169, 'is'], [170, 'setstring'], [171, 'displaytext'], [172, 'exec'],
     [173, 'pause'], [174, 'clear'], [175, 'debug'], [176, 'enter'],
     [177, 'movechar'], [178, 'moveobject'], [179, 'revealchar'],
     [180, 'revealobject'], [181, 'concealchar'], [182, 'concealobject'],
     [183, 'mailto'], [184, 'and'], [185, 'or'], [186, 'outputoff'],
     [187, 'outputon'], [188, 'here'], [189, 'playmidi'], [190, 'drop'],
     [191, 'helpmsg'], [192, 'helpdisplaytext'], [193, 'helpclear'],
     [194, 'helpclose'], [195, 'hide'], [196, 'show'], [197, 'move'],
     [198, 'conceal'], [199, 'reveal'], [200, 'numeric'], [201, 'string'],
     [202, 'collectable'], [203, 'property'], [204, 'create'], [205, 'exit'],
     [206, 'doaction'], [207, 'close'], [208, 'each'], [209, 'in'],
     [210, 'repeat'], [211, 'while'], [212, 'until'], [213, 'timeron'],
     [214, 'timeroff'], [215, 'stop'], [216, 'panes'], [217, 'on'],
     [218, 'off'], [219, 'return'], [220, 'playmod'], [221, 'modvolume'],
     [222, 'clone'], [223, 'shellexe'], [224, 'background'],
     [225, 'foreground'], [226, 'wait'], [227, 'picture'], [228, 'nospeak'],
     [229, 'animate'], [230, 'persist'], [231, 'inc'], [232, 'dec'],
     [233, 'flag'], [234, 'dontprocess'], [235, 'destroy'],
     [236, 'beforesave'], [237, 'onload']);

my % tokens = ();
my % rtokens = ();
foreach (@hash_data) {
	if ($_->[0] >= 0 && $_->[0] < 256) {
		if ($_->[1] eq '') {
			$_->[1] = "[?" . $_->[0] . "?]";
		}
		$rtokens{chr($_->[0])} = $_->[1];
		$tokens{$_->[1]} = chr($_->[0]);
	}
}

#print "{";
#for (my $i = 0; $i < 256; $i ++) {
#    print "\"", $rtokens{chr($i)}, "\", ";
#}
#print "}\n";
#die;


my % text_block_starters = map { $_ => 1 } qw / text synonyms type /;

sub uncompile_fil {
	my $IFH;
	open($IFH, "<", $_[0]);
	binmode $IFH;
	$ / = undef;
	my $dat = < $IFH >;
#print "uncompile_fil : ";
#print "\$IFH == '$IFH',";
#print "\$dat == '$dat'\n";
	my @dat = split //, $dat;


	my $OFH;
	if (@_ == 1) {
		push @_, "&STDOUT";
	}
	open $OFH, ">$_[1]" or die "Can't open '$_[1]' for output: $!";

	my @output = ();
	my $curline = "";


	my $obfus = 0;
	my $expect_text == 0;
	my($ch, $chn, $tok);
	for (my $n = 8; $n < @dat; $n ++) {
		$ch = $dat[$n];
		$chn = ord $ch;
		$tok = $rtokens{$ch};
		if ($obfus == 1 && $chn == 0) {
#print $OFH "> ";
			$curline . = "> ";
			$obfus = 0;
		}
		elsif($obfus == 1) {
#print $OFH chr (255 - $chn);
			$curline . = chr(255 - $chn);
		}
		elsif($obfus == 2 && $chn == 254) {
			$obfus = 0;
#print $OFH " ";
			$curline . = " ";
		}
		elsif($obfus == 2) {
#print $OFH chr ($chn);
			$curline . = chr($chn);
		}
		elsif($expect_text == 2) {
			if ($chn == 253) {
				$expect_text = 0;
##print $OFH "\n";
				push @output, $curline;
				$curline = "";
			}
			elsif($chn == 0) {
#print $OFH "\n";
				push @output, $curline;
				$curline = "";
			}
			else {
#print $OFH chr (255 - $chn);
				$curline . = chr(255 - $chn);
			}
		}
		elsif($obfus == 0 && $chn == 10) {
#print $OFH "<";
			$curline . = "<";
			$obfus = 1;
		}
		elsif($obfus == 0 && $chn == 254) {
			$obfus = 2;
		}
		elsif($chn == 255) {
			if ($expect_text == 1) {
				$expect_text = 2;
			}
#print $OFH "\n";
			push @output, $curline;
			$curline = "";
		}
		else {
			if (($tok eq 'text' || $tok eq 'synonyms' || $tok eq 'type') &&
			        $dat[$n - 1] eq chr(8)) {
				$expect_text = 1;
			}
#print $OFH "$tok ";
			$curline . = "$tok ";
		}
	}
	push @output, $curline;
	$curline = "";

	if (!$is_raw) {
		@output = pretty_print(reinline(@output));
	}

	foreach (@output) {
		print $OFH $_, "\r\n";
	}
}

sub list_grab_file {
	my $IFH;
	open($IFH, "<:crlf", $_[0]);
	my @rv = < $IFH >;
	chomp @rv;
	return @rv;
}


sub compile_fil {
	my @dat = list_grab_file($ARGV[0]);
	my $OFH;
	open $OFH, ">$ARGV[1]";

	print $OFH "QCGF200".chr(0);

# Mode 0 == normal, mode 1 == block text
	my $mode = 0;
	for (my $n = 0; $n < @dat; $n ++) {
		my $l = $dat[$n];
		while (substr($l, length($l) - 1, 1) eq '_' && $n < @dat) {
			$n ++;
			$l = substr($l, 0, length($l) - 1) . $dat[$n];
		}
		if ($l = ~ / ^ !include *<([\S] *)> /) {
			@dat = (@dat[0..$n], list_grab_file($1), @dat[$n + 1..$#dat]);
		}
		elsif($l = ~ / ^ !addto.* /) {
# TODO
		}
		else {
			my $i = 0;
			my $max = length $l;
			my @l = split //, $l;

			if ($mode == 1) {
				if ($l = ~ / ^\s * end\s * define\s*$ /) {
					print $OFH chr(253);
					$mode = 0;
# FALL THROUGH
				} else {
#print $OFH chr(0);
					foreach (split //, $l) {
					         print $OFH chr(255 - ord $_);
				}
			next;
		}
	}
	if ($l = ~ / ^\s*$ /) {
			next;
		}
		if ($l = ~ / ^\s * define\s * (text | synonyms | type) /) {
#[\s$]
			$mode = 1;
		}
		while ($i < $max) {
			while ($i <= $max && $l[$i] = ~ / \s /) {
				++ $i;
			}
			if ($i == $max) {
				next;
			}

			my $j = $i + 1;
			if ($l[$i] eq '<') {
				while ($j < $max && $l[$j] ne '>') {
					++ $j;
				}
				if ($l[$j] eq '>') {
					print $OFH obfus(substr($l, $i + 1, $j - $i - 1));
					$i = $j + 1;
					next;
				}
				$j = $i + 1;
				while ($j < $max && $l[$j] ne ' ') {
					++ $j;
				}
				print $OFH chr(254). substr($l, $i + 1, $j - $i - 1). chr(0);
				$i = $j + 1;
				next;
			}
			while ($j < $max && $l[$j] ne ' ') {
				++ $j;
			}
			my $str = substr($l, $i, $j - $i);
			if (defined $tokens{$str}) {
				print $OFH $tokens{$str};
			}
			else {
				print $OFH chr(254). $str. chr(254);
			}
			$i = $j + 1;
		}
	}
	print $OFH chr(255);
}
}

sub is_define_t {
	my($line, $type) = (@_);
	return ($line = ~ / ^ *define[\s] + $type + /);
}
sub is_define {
	my($line) = (@_);
	return ($line = ~ / ^ *define[\s] + [^\s] /);
}
sub is_end_define { return (shift = ~ / ^ *end + define *$ /); }

sub trim {
	my $tmp = trim1($_[0]);
#print "trimming ($_[0]) -> ($tmp)\n";
	return $tmp;
}

sub trim1 {
	if ($_[0] = ~ / ^[\s] * (.* ?)[\s]*$ /) {
		return $1;
	}
	print "* * * Huh on trimming '$_[0]' * * *\n";
}

sub reinline {
	my % reinlined = ();
	my @head_prog = ();
	my @rest_prog = ();
	while (@_) {
		push @rest_prog, (pop @_);
	}

	while (@rest_prog) {
		my $line = pop @rest_prog;
#print "processing $line\n";
		if ($line = ~ / ^ (.* |)do ( < !intproc[0 - 9] * >) * (.*)$ /) {
#print "  reinlining...\n";
				my($prefix, $func_name, $suffix) = ($1, $2, $3);
				$prefix = trim($prefix);
				$suffix = trim($suffix);
				$reinlined{$func_name} = 1;
				for (my $line_num = 0; $line_num < @rest_prog; $line_num ++) {
					if ($rest_prog[$line_num] = ~ / ^ *define + procedure + $func_name *$ /) {
						my $end_line = $line_num;
						while (!is_end_define($rest_prog[$end_line])) {
#print "   checking $rest_prog[$end_line]\n";
							-- $end_line;
						}
						++ $end_line;
#print "    backpushing } ".$suffix."\n";
#push @rest_prog, trim ("} " . $suffix);
						if ($suffix ne '') {
							push @rest_prog, $suffix;
						}
						push @rest_prog, "}";
						while ($end_line < $line_num) {
							push @rest_prog, $rest_prog[$end_line];
#print "    backpushing $rest_prog[$end_line]\n";
							$end_line ++;
						}
#print "    backpushing $prefix {\n";
						push @rest_prog, trim($prefix." {");
						$line_num = scalar @rest_prog;
					}
				}
			}
		else {
			push @head_prog, $line;
		}
	}
	my @rv = ();
	for (my $n = 0; $n < @head_prog; $n ++) {
		if ($head_prog[$n] = ~ / ^define procedure(<.*>) *$ / &&
		                     $reinlined{$1}) {
			while (!is_end_define($head_prog[$n])) {
				++ $n;
			}
		}
		else {
			push @rv, $head_prog[$n];
		}
	}
#for (my $n = 0; $n < @rv; $n ++) {
#   print "$n: $rv[$n]\n";
#}
	return @rv;
}

sub pretty_print {
	my $indent = 0;
	my $not_in_text_mode = 1;

	my @rv = ();

	for (my $n = 0; $n < @_; $n ++) {
		my $line = $_[$n];
		if (is_end_define($line)) {
			-- $indent;
			$not_in_text_mode = 1;
		}
		/ { /; if ($line = ~ / ^} /) {
			-- $indent;
		}
###if (is_define ($line) && ($n == 0 || !is_define ($_[$n-1]))) { print "\n"; }
		if (is_define($line) && ($n == 0 || !is_define($_[$n - 1]))) {
			push @rv, "";
		}
###if ($in_text_mode == 0) { print "    "x$indent; }
		push @rv, ("    "x($indent*$not_in_text_mode)).trim($line);
###print $line, "  line $n, indent $indent, text $in_text_mode\n";
###print $line, "\n";
		if (is_end_define($line) && $n < @_ && !is_end_define($_[$n + 1])
		        && !is_define($_[$n + 1])) {
###print "\n";
			push @rv, "";
		}
		if (is_define($line)) {
			++ $indent;
		}
		if ($line = ~ / {$ /) {
		++ $indent;
	} /
	   } /;
	if ($line = ~ / ^ *define + text /) {
			$not_in_text_mode = 0;
		}
	}
	return @rv;
}


sub error_msg {
	die "Usage: 'perl uncas.pl file.asl file2.cas' to compile to file\n".
	"       'perl uncas.pl file.cas'           to decompile to console\n".
	"       'perl uncas.pl file.cas file2.asl' to decompile to file\n";
}

if ($ARGV[0] eq '-raw') {
	$is_raw = 1;
	shift @ARGV;
}

if ($ARGV[0] = ~ / \.asl$ /) {
	if (@ARGV != 2) {
		error_msg();
	}
	compile_fil(@ARGV);
}
elsif($ARGV[0] = ~ / \.cas$ /) {
#print "compile_fil (", join (", ", @ARGV), ")\n";
	if (@ARGV != 1 && @ARGV != 2) {
		error_msg();
	}
	uncompile_fil(@ARGV);
}
