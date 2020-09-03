#!/usr/bin/perl
#
# This tools is kind of a hack to be able to maintain the credits list of
# ScummVM in a single central location. We then generate the various versions
# of the credits in other places from this source. In particular:
# - The AUTHORS file
# - The gui/credits.h header file
# - The Credits.rtf file used by the Mac OS X port
# - The credits.xml file, part of the DocBook manual
# - The credits.xml, for use on the website (different format than the DocBook one)
# - The credits.yaml, alternative version for use on the website
#
# Initial version written by Fingolfin in December 2004.
#


use strict;
use Text::Wrap;

if ($Text::Wrap::VERSION < 2001.0929) {
	die "Text::Wrap version >= 2001.0929 is required. You have $Text::Wrap::VERSION\n";
}

my $mode = "";
my $max_name_width;

# Count the level in the section hierarchy, i.e. how deep we are currently nested
# in terms of 'sections'.
my $section_level = 0;

# Variables used for yaml output
my $person_started = 0;
my $group_started = 0;
my $group_indent = "";
my $paragraph_started = 0;
my $indent = "";

# Count how many sections there have been on this level already
my @section_count = ( 0, 0, 0 );

if ($#ARGV >= 0) {
	$mode = "TEXT" if ($ARGV[0] eq "--text");	# AUTHORS file
	$mode = "XML-WEB" if ($ARGV[0] eq "--xml-website");	# credits.xml (for use on the website)
	$mode = "CPP" if ($ARGV[0] eq "--cpp");		# credits.h (for use by about.cpp)
	$mode = "XML-DOC" if ($ARGV[0] eq "--xml-docbook");		# credits.xml (DocBook)
	$mode = "RTF" if ($ARGV[0] eq "--rtf");		# Credits.rtf (Mac OS X About box)
	$mode = "STRONGHELP" if ($ARGV[0] eq "--stronghelp");	# AUTHORS (RISC OS StrongHelp manual)
	$mode = "YAML" if ($ARGV[0] eq "--yaml");	# YAML (Simple format)
}

if ($mode eq "") {
	print STDERR "Usage: $0 [--text | --xml-website | --cpp | --xml-docbook | --rtf | --stronghelp | --yaml]\n";
	print STDERR " Just pass --text / --xml-website / --cpp / --xml-docbook / --rtf / --stronghelp / --yaml as parameter, and credits.pl\n";
	print STDERR " will print out the corresponding version of the credits to stdout.\n";
	exit 1;
}

$Text::Wrap::unexpand = 0;
if ($mode eq "TEXT") {
	$Text::Wrap::columns = 78;
	$max_name_width = 30; # The maximal width of a name.
} elsif ($mode eq "CPP") {
	$Text::Wrap::columns = 48;	# Approx.
}

# Convert HTML entities to ASCII for the plain text mode
sub html_entities_to_ascii {
	my $text = shift;

	# For now we hardcode these mappings
	# &Aacute;  -> A
	# &aacute;  -> a
	# &eacute;  -> e
	# &iacute;  -> i
	# &igrave;  -> i
	# &oacute;  -> o
	# &oslash;  -> o
	# &uacute;  -> u
	# &ouml;    -> o / oe
	# &auml;    -> a
	# &euml;    -> e
	# &uuml;    -> ue
	# &aring;   -> aa
	# &amp;     -> &
	# &#261;    -> a
	# &#321;    -> L
	# &#322;    -> l
	# &#347;    -> s
	# &Scaron;  -> S
	# &Lcaron;  -> L
	# &ntilde;  -> n
	$text =~ s/&Aacute;/A/g;
	$text =~ s/&aacute;/a/g;
	$text =~ s/&eacute;/e/g;
	$text =~ s/&iacute;/i/g;
	$text =~ s/&igrave;/i/g;
	$text =~ s/&oacute;/o/g;
	$text =~ s/&oslash;/o/g;
	$text =~ s/&uacute;/u/g;
	$text =~ s/&#261;/a/g;
	$text =~ s/&#321;/L/g;
	$text =~ s/&#322;/l/g;
	$text =~ s/&#347;/s/g;
	$text =~ s/&Lcaron;/L/g;
	$text =~ s/&Scaron;/S/g;
	$text =~ s/&aring;/aa/g;
	$text =~ s/&ntilde;/n/g;

	$text =~ s/&auml;/a/g;
	$text =~ s/&euml;/e/g;
	$text =~ s/&uuml;/ue/g;
	# HACK: Torbj*o*rn but G*oe*ffringmann and R*oe*ver and J*oe*rg
	$text =~ s/Torbj&ouml;rn/Torbjorn/g;
	$text =~ s/&ouml;/oe/g;

	$text =~ s/&amp;/&/g;

	return $text;
}

# Convert HTML entities to ISO/IEC 8859-1 for the StrongHelp manual
sub html_entities_to_iso8859_1 {
	my $text = shift;

	$text =~ s/&Aacute;/\xC1/g;
	$text =~ s/&aacute;/\xE1/g;
	$text =~ s/&eacute;/\xE9/g;
	$text =~ s/&iacute;/\xED/g;
	$text =~ s/&igrave;/\xEC/g;
	$text =~ s/&oacute;/\xF3/g;
	$text =~ s/&oslash;/\xF8/g;
	$text =~ s/&uacute;/\xFA/g;
	$text =~ s/&#261;/a/g;
	$text =~ s/&#321;/L/g;
	$text =~ s/&#322;/l/g;
	$text =~ s/&#347;/s/g;
	$text =~ s/&Lcaron;/L/g;
	$text =~ s/&Scaron;/S/g;
	$text =~ s/&aring;/\xE5/g;
	$text =~ s/&ntilde;/\xF1/g;

	$text =~ s/&auml;/\xE4/g;
	$text =~ s/&euml;/\xEB/g;
	$text =~ s/&uuml;/\xFC/g;
	$text =~ s/&ouml;/\xF6/g;

	$text =~ s/&amp;/&/g;

	return $text;
}

# Convert HTML entities to C++ characters
sub html_entities_to_cpp {
	my $text = shift;

	# The numerical values are octal!
	$text =~ s/&Aacute;/\\301/g;
	$text =~ s/&aacute;/\\341/g;
	$text =~ s/&eacute;/\\351/g;
	$text =~ s/&iacute;/\\355/g;
	$text =~ s/&igrave;/\\354/g;
	$text =~ s/&oacute;/\\363/g;
	$text =~ s/&oslash;/\\370/g;
	$text =~ s/&uacute;/\\372/g;
	$text =~ s/&#261;/a/g;
	$text =~ s/&#321;/L/g;
	$text =~ s/&#322;/l/g;
	$text =~ s/&#347;/s/g;
	$text =~ s/&Lcaron;/L/g;
	$text =~ s/&Scaron;/S/g;
	$text =~ s/&aring;/\\345/g;
	$text =~ s/&ntilde;/\\361/g;

	$text =~ s/&auml;/\\344/g;
	$text =~ s/&euml;/\\353/g;
	$text =~ s/&ouml;/\\366/g;
	$text =~ s/&uuml;/\\374/g;

	$text =~ s/&amp;/&/g;

	return $text;
}

# Convert HTML entities to RTF codes
# This is using the Mac OS Roman encoding
sub html_entities_to_rtf {
	my $text = shift;

	$text =~ s/&Aacute;/\\'c1/g;
	$text =~ s/&aacute;/\\'87/g;
	$text =~ s/&eacute;/\\'8e/g;
	$text =~ s/&iacute;/\\'92/g;
	$text =~ s/&igrave;/\\'93/g;
	$text =~ s/&oacute;/\\'97/g;
	$text =~ s/&oslash;/\\'bf/g;
	$text =~ s/&uacute;/\\'9c/g;
	$text =~ s/&aring;/\\'8c/g;
	# The following numerical values are decimal!
	$text =~ s/&#261;/\\uc0\\u261 /g;
	$text =~ s/&#321;/\\uc0\\u321 /g;
	$text =~ s/&#322;/\\uc0\\u322 /g;
	$text =~ s/&#347;/\\uc0\\u347 /g;
	$text =~ s/&Lcaron;/\\uc0\\u317 /g;
	$text =~ s/&Scaron;/\\uc0\\u352 /g;

	# Back to hex numbers
	$text =~ s/&ntilde;/\\'96/g;

	$text =~ s/&auml;/\\'8a/g;
	$text =~ s/&euml;/\\'91/g;
	$text =~ s/&ouml;/\\'9a/g;
	$text =~ s/&uuml;/\\'9f/g;

	$text =~ s/&amp;/&/g;

	return $text;
}

#
# Small reference of the RTF commands used here:
#
#  \fs28   switches to 14 point font (28 = 2 * 14)
#  \pard   reset to default paragraph properties
#
#  \ql     left-aligned text
#  \qr     right-aligned text
#  \qc     centered text
#  \qj     justified text
#
#  \b      turn on bold
#  \b0     turn off bold
#
# For more information: <http://latex2rtf.sourceforge.net/rtfspec.html>
#

sub begin_credits {
	my $title = shift;

	if ($mode eq "TEXT") {
		#print html_entities_to_ascii($title)."\n";
	} elsif ($mode eq "RTF") {
		print '{\rtf1\mac\ansicpg10000' . "\n";
		print '{\fonttbl\f0\fswiss\fcharset77 Helvetica-Bold;\f1\fswiss\fcharset77 Helvetica;}' . "\n";
		print '{\colortbl;\red255\green255\blue255;\red0\green128\blue0;\red128\green128\blue128;}' . "\n";
		print '\vieww6920\viewh15480\viewkind0' . "\n";
		print "\n";
	} elsif ($mode eq "CPP") {
		print "// This file was generated by credits.pl. Do not edit by hand!\n";
		print "static const char *credits[] = {\n";
	} elsif ($mode eq "XML-DOC") {
		print "<?xml version='1.0'?>\n";
		print "<!-- This file was generated by credits.pl. Do not edit by hand! -->\n";
		print "<!DOCTYPE appendix PUBLIC '-//OASIS//DTD DocBook XML V4.2//EN'\n";
		print "       'http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd'>\n";
		print "<appendix id='credits'>\n";
		print "  <title>" . $title . "</title>\n";
		print "  <informaltable frame='none'>\n";
		print "  <tgroup cols='3' align='left' colsep='0' rowsep='0'>\n";
		print "  <colspec colname='start' colwidth='0.5cm'/>\n";
		print "  <colspec colname='name' colwidth='4cm'/>\n";
		print "  <colspec colname='job'/>\n";
		print "  <tbody>\n";
	} elsif ($mode eq "XML-WEB") {
		print "<?xml version='1.0'?>\n";
		print "<!-- This file was generated by credits.pl. Do not edit by hand! -->\n";
		print "<credits>\n";
	} elsif ($mode eq "YAML") {
		print "# This file was generated by credits.pl. Do not edit by hand!\n";
		print "credits:\n";
	} elsif ($mode eq "STRONGHELP") {
		print "ScummVM - AUTHORS\n";
		print "# This file was generated by credits.pl. Do not edit by hand!\n";
	}
}

sub end_credits {
	if ($mode eq "TEXT") {
	} elsif ($mode eq "RTF") {
		print "}\n";
	} elsif ($mode eq "CPP") {
		print "};\n";
	} elsif ($mode eq "XML-DOC") {
		print "  </tbody>\n";
		print "  </tgroup>\n";
		print "  </informaltable>\n";
		print "</appendix>\n";
	} elsif ($mode eq "XML-WEB") {
		print "</credits>\n";
	}
}

sub begin_section {
	my $title = shift;
	my $anchor = shift;

	if ($mode eq "TEXT") {
		$title = html_entities_to_ascii($title);

		if ($section_level >= 2) {
			$title .= ":"
		}

		print "  " x $section_level . $title."\n";
		if ($section_level eq 0) {
			print "  " x $section_level . "*" x (length $title)."\n";
		} elsif ($section_level eq 1) {
			print "  " x $section_level . "-" x (length $title)."\n";
		}
	} elsif ($mode eq "RTF") {
		$title = html_entities_to_rtf($title);

		# Center text
		print '\pard\qc' . "\n";
		print '\f0\b';
		if ($section_level eq 0) {
			print '\fs40 ';
		} elsif ($section_level eq 1) {
			print '\fs32 ';
		}

		# Insert an empty line before this section header, *unless*
		# this is the very first section header in the file.
		if ($section_level > 0 || @section_count[0] > 0) {
			print "\\\n";
		}
		print '\cf2 ' . $title . "\n";
		print '\f1\b0\fs24 \cf0 \\' . "\n";
	} elsif ($mode eq "CPP") {
		if ($section_level eq 0) {
			# TODO: Would be nice to have a 'fat' or 'large' mode for
			# headlines...
			$title = html_entities_to_cpp($title);
			print '"C1""'.$title.'",' . "\n";
			print '"",' . "\n";
		} else {
			$title = html_entities_to_cpp($title);
			print '"C1""'.$title.'",' . "\n";
		}
	} elsif ($mode eq "XML-DOC") {
		print "  <row><entry namest='start' nameend='job'>";
		print "<emphasis role='bold'>" . $title . ":</emphasis>";
		print "</entry></row>\n";
	} elsif ($mode eq "XML-WEB") {
		if ($section_level eq 0) {
			print "\t<section>\n";
			print "\t\t<title>" . $title . "</title>\n";
			if ($anchor) {
				print "\t\t<anchor>" . $anchor . "</anchor>\n";
			}
		} elsif ($section_level eq 1) {
			print "\t\t<subsection>\n";
			print "\t\t\t<title>" . $title . "</title>\n";
			if ($anchor) {
				print "\t\t\t<anchor>" . $anchor . "</anchor>\n";
			}
		} else {
			#print "\t\t\t<group>" . $title . "</group>\n";
			#print "\t\t\t\t<name>" . $title . "</name>\n";
		}
	} elsif ($mode eq "YAML") {
		my $key = "section:\n";
		$indent = "  " . ("  " x $section_level);
		if ($section_level eq 1) {
			$key = "subsection:\n";
		}

		if ($section_level < 2) {
			if (@section_count[$section_level] eq 0) {
				print $indent . $key;
			}

			print $indent . "-\n";
			print $indent . "  title: \"" . $title . "\"\n";
			if ($anchor) {
				print $indent . "  anchor: \"" . $anchor . "\"\n";
			}
		}
	} elsif ($mode eq "STRONGHELP") {
		$title = html_entities_to_iso8859_1($title);
		print "#fH" . ($section_level + 1) . ":" . $title."\n";
	}
	# Implicit start of person list on section level 2
	if ($section_level >= 2) {
		begin_persons($title, 1);
	}
	@section_count[$section_level]++;
	$section_level++;
	@section_count[$section_level] = 0;
}

sub end_section {
	$section_level--;
	$paragraph_started = 0;
	$group_started = 0;

	# Implicit end of person list on section level 2
	if ($section_level >= 2) {
		end_persons();
		$group_started = 1;
	}

	if ($mode eq "TEXT") {
		# nothing
	} elsif ($mode eq "RTF") {
		# nothing
	} elsif ($mode eq "CPP") {
		print '"",' . "\n";
	} elsif ($mode eq "XML-DOC") {
		print "  <row><entry namest='start' nameend='job'> </entry></row>\n\n";
	} elsif ($mode eq "XML-WEB") {
		if ($section_level eq 0) {
			print "\t</section>\n";
		} elsif ($section_level eq 1) {
			print "\t\t</subsection>\n";
		} else {
			#print "\t\t\t</group>\n";
		}
	}
}

sub begin_persons {
	my $title = shift;
	my $level = shift;

	if ($mode eq "XML-WEB") {
		print "\t\t\t<group>\n";
		print "\t\t\t\t<name>" . $title . "</name>\n";
		#print "\t\t\t\t<persons>\n";
	} elsif ($mode eq "YAML") {
		$group_indent = $level eq 1 ? "      " : "  " . ("  " x $section_level);
		if ($group_started == 0) {
			print $group_indent . "group:\n";
			$group_started = 1;
		}
		print $group_indent . "-\n";
		print $group_indent . "  name: \"" . $title . "\"\n";

	}
}

sub end_persons {
	if ($mode eq "TEXT") {
		print "\n";
	} elsif ($mode eq "RTF") {
		# nothing
	} elsif ($mode eq "XML-WEB") {
		#print "\t\t\t\t</persons>\n";
		print "\t\t\t</group>\n";
	} elsif ($mode eq "STRONGHELP") {
		print "\n";
	} elsif ($mode eq "YAML") {
		$person_started = 0;
	}
}

sub add_person {
	my $name = shift;
	my $nick = shift;
	my $desc = shift;
	my $tab;

	if ($mode eq "TEXT") {
		my $min_name_width = length $desc > 0 ? $max_name_width : 0;
		$name = $nick if $name eq "";
		$name = html_entities_to_ascii($name);
		if (length $name > $max_name_width) {
			print STDERR "Warning: max_name_width is too small (" . $max_name_width . " < " . (length $name) . " for \"" . $name. "\")\n";
		}
		$desc = html_entities_to_ascii($desc);

		$tab = " " x ($section_level * 2 + 1);
		printf $tab."%-".$min_name_width.".".$max_name_width."s", $name;

		# Print desc wrapped
		if (length $desc > 0) {
			my $inner_indent = ($section_level * 2 + 1) + $max_name_width + 3;
			my $multitab = " " x $inner_indent;
			print " - " . substr(wrap($multitab, $multitab, $desc), $inner_indent);
		}
		print "\n";
	} elsif ($mode eq "RTF") {
		$name = $nick if $name eq "";
		$name = html_entities_to_rtf($name);

		# Center text
		print '\pard\qc' . "\n";
		# Activate 1.5 line spacing mode
		print '\sl360\slmult1';
		# The name
		print $name . "\\\n";
		# Description using italics
		if (length $desc > 0) {
			$desc = html_entities_to_rtf($desc);
			print '\pard\qc' . "\n";
			print "\\cf3\\i " . $desc . "\\i0\\cf0\\\n";
		}
	} elsif ($mode eq "CPP") {
		$name = $nick if $name eq "";
		$name = html_entities_to_cpp($name);
		print '"C0""'.$name.'",' . "\n";

		# Print desc wrapped
		if (length $desc > 0) {
			$desc = html_entities_to_cpp($desc);
			print '"C2""'.$desc.'",' . "\n";
		}
	} elsif ($mode eq "XML-DOC") {
		$name = $nick if $name eq "";
		print "  <row><entry namest='name'>" . $name . "</entry>";
		print "<entry>" . $desc . "</entry></row>\n";
	} elsif ($mode eq "XML-WEB") {
		$name = "???" if $name eq "";
		print "\t\t\t\t<person>\n";
		print "\t\t\t\t\t<name>" . $name . "</name>\n";
		print "\t\t\t\t\t<alias>" . $nick . "</alias>\n";
		print "\t\t\t\t\t<description>" . $desc . "</description>\n";
		print "\t\t\t\t</person>\n";
	} elsif ($mode eq "YAML") {
		$indent = $group_indent . "  ";

		if ($person_started eq 0) {
			print $indent . "person:\n";
			$person_started = 1;
		}
		print $indent . "-\n";
		$name = "???" if $name eq "";
		print $indent . "  name: \"" . $name . "\"\n";
		print $indent . "  alias: \"" . $nick . "\"\n";
		print $indent . "  description: \"" . $desc . "\"\n";
	} elsif ($mode eq "STRONGHELP") {
		my $min_name_width = length $desc > 0 ? $max_name_width : 0;
		$name = $nick if $name eq "";
		$name = html_entities_to_iso8859_1($name);
		$desc = html_entities_to_iso8859_1($desc);

		$tab = " " x ($section_level * 2 + 1);
		print $tab . "{*}" . $name . "{*}";
		print "\t" . $desc . "\n";
	}
}

sub add_paragraph {
	my $text = shift;
	my $tab;

	if ($mode eq "TEXT") {
		$tab = " " x ($section_level * 2 + 1);
		print wrap($tab, $tab, html_entities_to_ascii($text))."\n";
		print "\n";
	} elsif ($mode eq "RTF") {
		$text = html_entities_to_rtf($text);
		# Center text
		print '\pard\qc' . "\n";
		print "\\\n";
		print $text . "\\\n";
	} elsif ($mode eq "CPP") {
		$text = html_entities_to_ascii($text);
		my $line_start = '"C0""';
		my $line_end = '",';
		print $line_start . $text . $line_end . "\n";
		print $line_start . $line_end . "\n";
	} elsif ($mode eq "XML-DOC") {
		print "  <row><entry namest='start' nameend='job'>" . $text . "</entry></row>\n";
		print "  <row><entry namest='start' nameend='job'> </entry></row>\n\n";
	} elsif ($mode eq "XML-WEB") {
		print "\t\t<paragraph>" . $text . "</paragraph>\n";
	} elsif ($mode eq "YAML") {
		$indent = "  " . ("  " x $section_level);
		if ($paragraph_started eq 0) {
			print $indent . "paragraph:\n";
			$paragraph_started = 1;
		}
		print $indent . "- \"" . $text . "\"\n";
	} elsif ($mode eq "STRONGHELP") {
		$text = html_entities_to_iso8859_1($text);
		print "#Wrap On\n";
		$tab = " " x ($section_level * 2 + 1);
		print $text . "\n";
		print "#Wrap\n\n";
	}
}

#
# Now follows the actual credits data! The format should be clear, I hope.
# Note that people are sorted by their last name in most cases; in the
# 'Team' section, they are first grouped by category (Engine; porter; misc).
#

begin_credits("Credits");
    begin_section("ResidualVM");
	begin_section("Project Leaders");
	    begin_persons();
		add_person("Pawe&#322; Ko&#322;odziejski", "aquadran", "");
		add_person("Einar Johan T. S&oslash;m&aring;en", "somaen", "");
	    end_persons();
	end_section();

	begin_section("Teams");
	    begin_section("Grim Team");
		add_person("James Brown", "ender", "Grim developer (retired)");
		add_person("Giulio Camuffo", "giucam", "Grim developer (retired)");
		add_person("Dries Harnie", "Botje", "EMI, Android, OpenGL ES developer");
		add_person("Pawe&#322; Ko&#322;odziejski", "aquadran", "Core developer (suspended)");
		add_person("Christian Krause", "chkr", "EMI developer (retired)");
		add_person("Einar Johan T. S&oslash;m&aring;en", "somaen", "Grim, EMI developer");
		add_person("Joel Teichroeb ", "klusark", "EMI, Android developer");
		add_person("Joni V&auml;h&auml;m&auml;ki", "Akz", "EMI developer (retired)");
	    end_section();

	    begin_section("ICB Team");
		add_person("Pawe&#322; Ko&#322;odziejski", "aquadran", "Porting from original codebase");
		add_person("Joost Peters", "joostp", "Porting from original codebase");
		add_person("Einar Johan T. S&oslash;m&aring;en", "somaen", "Porting from original codebase");
	    end_section();

	    begin_section("Myst 3 Team");
		add_person("Bastien Bouclet", "bgK", "Core developer");
	    end_section();

	    begin_section("Stark Team");
		add_person("Bastien Bouclet", "bgK", "Core developer");
		add_person("Einar Johan T. S&oslash;m&aring;en", "somaen", "Core developer (retired)");
		add_person("Liu Zhaosong", "Douglas", "Core developer");
	    end_section();

	    begin_section("Wintermute 3D Team");
		add_person("Gunnar Birke", "Fury", "Porting from original Wintermute 3D codebase");
	    end_section();
	end_section();

	begin_section("Contributors");
	    add_paragraph(
	    "If you have contributed to this engine then you deserve to be on this ".
	    "list. Contact us and we'll add you.");
	    begin_section("Grim");
		add_person("Thomas Allen", "olldray", "Various engine code fixes and improvements");
		add_person("Torbj&ouml;rn Andersson", "eriktorbjorn", "Various code fixes");
		add_person("Ori Avtalion", "salty-horse", "Lipsync, LAF support, various code fixes");
		add_person("Robert Biro", "DarthJDG", "Antialiasing support");
		add_person("Bastien Bouclet", "bgK", "Various fixes to engine");
		add_person("David Cardwell", "d356", "Few fixes to EMI");
		add_person("Marcus Comstedt", "marcus_c", "Initial Dreamcast port");
		add_person("Andrea Corna", "Yak Bizzarro", "Patcher module, various engine improvements");
		add_person("Jonathan Gray", "khalek", "Various code fixes");
		add_person("Tobias Gunkel", "tobigun", "Initial Android port, few engines fixes");
		add_person("Azamat H. Hackimov", "winterheart", "Configure fix");
		add_person("Vincent Hamm", "yazoo", "Various engine code fixes and improvements");
		add_person("Sven Hesse", "DrMcCoy", "Various compilation fixes");
		add_person("Matthew Hoops", "clone2727", "Smush codec48, Grim and EMI engine improvements");
		add_person("Erich Hoover", "Compholio", "x86-64 fixes, various code fixes and improvements");
		add_person("Max Horn", "fingolfin", "Few code fixes");
		add_person("Travis Howell", "Kirben", "Various code fixes, Windows port");
		add_person("Joseph Jezak", "JoseJX", "A lot of engine improvements and fixes");
		add_person("Guillem Jover", "guillemj", "Few code improvements");
		add_person("Filippos Karapetis", "bluegr", "Compilation fixes");
		add_person("Ingo van Lil", "inguin", "Various fixes and improvements for EMI");
		add_person("Vincent Pelletier", "vpelletier", "Various engine and TinyGL improvements");
		add_person("Joost Peters", "joostp", "Various code fixes");
		add_person("George Macon", "gmacon", "Few fixes");
		add_person("Josh Matthews", "jdm", "Few fixes to engine");
		add_person("Matthieu Milan", "usineur", "Various engine improvements");
		add_person("Gregory Montoir", "cyx", "Few fixes to engine");
		add_person("Stefano Musumeci", "subr3v", "TinyGL backend and engine driver improvements");
		add_person("Christian Neumair", "mannythegnome", "Various optimisation patches");
		add_person("Daniel Schepler", "", "Initial grim engine contributor, LUA support");
		add_person("Dmitry Smirnov", "onlyjob", "Minor spelling corrections");
		add_person("Yaron Tausky", "yaront", "Fixes to subtitles");
		add_person("Julien Templier", "Littleboy", "create_project tool");
		add_person("Pino Toscano", "pinotree", "Debian GNU/Linux package files");
		add_person("Lionel Ulmer", "bbrox", "OpenGL optimisations");
		add_person("", "cmayer0087", "Various engine code fixes");
		add_person("", "JenniBee", "Compilation fixes");
		add_person("", "karjonas", "Various engine code fixes");
		add_person("", "PoulpiFr", "Few fixes to Android port");
		add_person("", "sietschie", "Few fixes to engine");
	    end_section();

	    begin_section("Myst 3");
		add_person("David Fioramonti", "dafioram", "Autosave support and few fixes");
		add_person("Matthew Hoops", "clone2727", "Various engine improvements and code fixes");
		add_person("Stefano Musumeci", "subr3v", "TinyGL engine driver support");
	    end_section();

	    begin_section("Stark");
		add_person("Bartosz Dudziak", "Snejp", "Various engine improvements and code fixes");
		add_person("Matthew Hoops", "clone2727", "ADPCM decoder");
		add_person("Pawe&#322; Ko&#322;odziejski", "aquadran", "Various engine code fixes");
		add_person("Awad Mackie", "firesock", "Few fixes to engine");
		add_person("Marius Ioan Orban", "mj0331", "Code fix");
		add_person("Vincent Pelletier", "vpelletier", "Raw sound support");
		add_person("Jordi Vilalta Prat", "jvprat", "Initial engine contributor");
		add_person("Scott Thomas", "ST", "Initial engine author");
		add_person("Will Thomson", "wlthomson", "Few fixes to engine");
		add_person("", "Faalagorn", "Few code improvements");
		add_person("", "orangeforest11", "Few engine improvements");
	    end_section();
	end_section();

    end_section();

    begin_section("ScummVM code");
	  add_paragraph(
	  "ResidualVM uses a lot of ScummVM code. ".
	  "For a list of authors look into COPYRIGHT file.");
    end_section();

  begin_section("Website code");
	begin_persons();
		add_person("Fredrik Wendel", "", "");
	end_persons();
  end_section();

  # HACK!
  $max_name_width = 16;

  begin_section("Special thanks to");

	  add_paragraph(
	  "The LUA developers, for creating a nice compact script interpreter.");

	  add_paragraph(
	  "Tim Schafer, for obvious reasons, and everybody else who helped make ".
	  "Grim Fandango a brilliant game; and the EMI team for giving it their ".
	  "best try.");

	  add_paragraph(
	  "Bret Mogilefsky, for managing to create a SPUTM-style 3D LUA engine, ".
	  "and avoiding the horrible hack it could have been.");

	  add_paragraph(
	  "Benjamin Haisch, for emimeshviewer, which our EMI code borrows heavily from.");

  end_section();

end_credits();
