#! /usr/bin/env perl
# The C File Storage Meta Language "reference" implementation
# This implementation is supposed to conform to version
$version = "0.8.2";
# of the spec. Please contact the maintainer if it doesn't.
#
# cfsml.pl Copyright (C) 1999, 2000, 2001 Christoph Reichenbach
#
#
# This program may be modified and copied freely according to the terms of
# the GNU general public license (GPL), as long as the above copyright
# notice and the licensing information contained herein are preserved.
#
# Please refer to www.gnu.org for licensing details.
#
# This work is provided AS IS, without warranty of any kind, expressed or
# implied, including but not limited to the warranties of merchantibility,
# noninfringement, and fitness for a specific purpose. The author will not
# be held liable for any damage caused by this work or derivatives of it.
#
# By using this source code, you agree to the licensing terms as stated
# above.
#
#
# Please contact the maintainer for bug reports or inquiries.
#
# Current Maintainer:
#
#    Christoph Reichenbach (CJR) [jameson@linuxgames.com]
#
#
# Warning: This is still a "bit" messy. Sorry for that.
#

#$debug = 1;

$write_lines = "true";
$source_file = "CFSML source file";
$type_integer = "integer";
$type_string = "string";
$type_record = "RECORD";
$type_pointer = "POINTER";
$type_abspointer = "ABSPOINTER";

%types;      # Contains all type bindings
%records;    # Contains all record bindings

$mode = undef;
while ($op = shift @ARGV) {
    if ($mode eq undef) {
	if ($op eq "-f") {
	    $mode = "fname";
	} elsif ($op eq "-l") {
	    $write_lines = undef;
	} elsif ($op eq "-v") {
	    print "cfsml.pl, the CFSML code generator, version $version\n";
	    print "This program is provided WITHOUT WARRANTY of any kind. It may be\n";
	    print "copied and modified freely according to the terms of the GNU\n";
	    print "General Public License.\n";
	    exit(0);
	} elsif ($op eq "-h") {
	    print "CFSML help:\n";
	    print "Usage: cat source | cfsml.pl [-v] [-h] [-l] [-f <filename>] > dest\n";
	    print "  -h : help\n";
	    print "  -v : print version\n";
	    print "  -l : disable line number printing in dest file\n";
	    print "  -f : specify file name for line number printing\n";
	    exit(0);
	} else {
	    die "Unknown option '$op'\n";
	}
    } elsif ($mode eq "fname") {
	$source_file = $op;
	$mode = 0;
    } else {
	die "Invalid internal state '$mode'\n";
    }
}

sub write_line_pp
# write_line_pp(int line_nr, bool input_file?)
{
    my $line_nr = shift;
    my $_file = shift;
    my $filename = "cfsml.pl";

    if (_file) {
	$filename = $source_file;
    }

    if ($write_lines) {
	print "#line $line_nr \"$filename\"\n";
    }
}

sub create_string_functions
  {
    $firstline = __LINE__;
    $firstline += 4;
    write_line_pp($firstline, 0);
    print <<'EOF';

#include <stdarg.h> // We need va_lists

#ifdef CFSML_DEBUG_MALLOC
/*
#define free(p)        dbg_sci_free(p)
#define malloc(s)      dbg_sci_malloc(s)
#define calloc(n, s)   dbg_sci_calloc(n, s)
#define realloc(p, s)  dbg_sci_realloc(p, s)
*/
#define free        dbg_sci_free
#define malloc      dbg_sci_malloc
#define calloc      dbg_sci_calloc
#define realloc     dbg_sci_realloc
#endif

static void _cfsml_error(const char *fmt, ...) {
	va_list argp;

	fprintf(stderr, "Error: ");
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
}


static struct _cfsml_pointer_refstruct {
	struct _cfsml_pointer_refstruct *next;
	void *ptr;
} *_cfsml_pointer_references = NULL;

static struct _cfsml_pointer_refstruct **_cfsml_pointer_references_current = &_cfsml_pointer_references;

static char *_cfsml_last_value_retrieved = NULL;
static char *_cfsml_last_identifier_retrieved = NULL;

static void _cfsml_free_pointer_references_recursively(struct _cfsml_pointer_refstruct *refs, int free_pointers) {
	if (!refs)
		return;

	_cfsml_free_pointer_references_recursively(refs->next, free_pointers);

	if (free_pointers)
		free(refs->ptr);

	free(refs);
}

static void _cfsml_free_pointer_references(struct _cfsml_pointer_refstruct **meta_ref, int free_pointers) {
	_cfsml_free_pointer_references_recursively(*meta_ref, free_pointers);
	*meta_ref = NULL;
	_cfsml_pointer_references_current = meta_ref;
}

static struct _cfsml_pointer_refstruct **_cfsml_get_current_refpointer() {
	return _cfsml_pointer_references_current;
}

static void _cfsml_register_pointer(void *ptr) {
	struct _cfsml_pointer_refstruct *newref = (struct _cfsml_pointer_refstruct*)sci_malloc(sizeof (struct _cfsml_pointer_refstruct));
	newref->next = *_cfsml_pointer_references_current;
	newref->ptr = ptr;
	*_cfsml_pointer_references_current = newref;
}

static char *_cfsml_mangle_string(const char *s) {
	const char *source = s;
	char c;
	char *target = (char *)sci_malloc(1 + strlen(s) * 2); // We will probably need less than that
	char *writer = target;

	while ((c = *source++)) {
		if (c < 32) { // Special character?
			*writer++ = '\\'; // Escape...
			c += ('a' - 1);
		} else if (c == '\\' || c == '"')
			*writer++ = '\\'; // Escape, but do not change
		*writer++ = c;
	}
	*writer = 0; // Terminate string

	return (char *)sci_realloc(target, strlen(target) + 1);
}

static char *_cfsml_unmangle_string(const char *s, unsigned int length) {
	char *target = (char *)sci_malloc(1 + strlen(s));
	char *writer = target;
	const char *source = s;
	const char *end = s + length;
	char c;

	while ((source != end) && (c = *source++) && (c > 31)) {
		if (c == '\\') { // Escaped character?
			c = *source++;
			if ((c != '\\') && (c != '"')) // Un-escape 0-31 only
				c -= ('a' - 1);
		}
		*writer++ = c;
	}
	*writer = 0; // Terminate string

	return (char *)sci_realloc(target, strlen(target) + 1);
}

static char *_cfsml_get_identifier(Common::SeekableReadStream *fd, int *line, int *hiteof, int *assignment) {
	int c;
	int mem = 32;
	int pos = 0;
	int done = 0;
	char *retval = (char *)sci_malloc(mem);

	if (_cfsml_last_identifier_retrieved) {
		free(_cfsml_last_identifier_retrieved);
		_cfsml_last_identifier_retrieved = NULL;
	}

	while (isspace(c = SRSgetc(fd)) && (c != EOF));
	if (c == EOF) {
	    _cfsml_error("Unexpected end of file at line %d\n", *line);
	    free(retval);
	    *hiteof = 1;
	    return NULL;
	}

	int first = 1;
	while ((first || (c = SRSgetc(fd)) != EOF) && ((pos == 0) || (c != '\n')) && (c != '=')) {
		first = 0;
		if (pos == mem - 1) // Need more memory?
			retval = (char *)sci_realloc(retval, mem *= 2);
		if (!isspace(c)) {
			if (done) {
				_cfsml_error("Single word identifier expected at line %d\n", *line);
				free(retval);
				return NULL;
			}
			retval[pos++] = c;
		} else
			if (pos != 0)
				done = 1; // Finished the variable name
			else if (c == '\n')
				++(*line);
	}

	if (c == EOF) {
		_cfsml_error("Unexpected end of file at line %d\n", *line);
		free(retval);
		*hiteof = 1;
		return NULL;
	}

	if (c == '\n') {
		++(*line);
		if (assignment)
			*assignment = 0;
	} else
		if (assignment)
			*assignment = 1;

	if (pos == 0) {
		_cfsml_error("Missing identifier in assignment at line %d\n", *line);
		free(retval);
		return NULL;
	}

	if (pos == mem - 1) // Need more memory?
		retval = (char *)sci_realloc(retval, mem += 1);

	retval[pos] = 0; // Terminate string
EOF

if ($debug) {
    print "		printf(\"identifier is '%s'\\n\", retval);\n";
}

  $firstline = __LINE__;
  $firstline += 4;
  write_line_pp($firstline, 0);
  print <<'EOF2';

	return _cfsml_last_identifier_retrieved = retval;
}

static char *_cfsml_get_value(Common::SeekableReadStream *fd, int *line, int *hiteof) {
	int c;
	int mem = 64;
	int pos = 0;
	char *retval = (char *)sci_malloc(mem);

	if (_cfsml_last_value_retrieved) {
		free(_cfsml_last_value_retrieved);
		_cfsml_last_value_retrieved = NULL;
	}

	while (((c = SRSgetc(fd)) != EOF) && (c != '\n')) {
		if (pos == mem - 1) // Need more memory?
			retval = (char *)sci_realloc(retval, mem *= 2);

		if (pos || (!isspace(c)))
			retval[pos++] = c;
	}

	while ((pos > 0) && (isspace(retval[pos - 1])))
		--pos; // Strip trailing whitespace

	if (c == EOF)
		*hiteof = 1;

	if (pos == 0) {
	    _cfsml_error("Missing value in assignment at line %d\n", *line);
	    free(retval);
	    return NULL;
	}

	if (c == '\n')
		++(*line);

	if (pos == mem - 1) // Need more memory?
		retval = (char *)sci_realloc(retval, mem += 1);

	retval[pos] = 0; // Terminate string
EOF2

    if ($debug) {
	print "		printf(\"value is '%s'\\n\", retval);\n";
    }

    $firstline = __LINE__;
    $firstline += 4;
    write_line_pp($firstline, 0);
  print <<'EOF3';
	return (_cfsml_last_value_retrieved = (char *)sci_realloc(retval, strlen(retval) + 1));
	// Re-allocate; this value might be used for quite some while (if we are restoring a string)
}
EOF3
  }


# Call with $expression as a simple expression, like "tos + 1".
# Returns (in this case) ("tos", "-1").
sub lvaluize
  {
    my @retval;
#    print "//DEBUG: $expression [";
    my @tokens = split (/([+-\/\*])/, $expression);
#    print join(",", @tokens);
    $retval[0] = $tokens[0];

    my $rightvalue = "";
    for ($i = 1; $tokens[$i]; $i++) {

      if ($tokens[$i] eq "+") {
	$rightvalue .= "-";
      } elsif ($tokens[$i] eq "-") {
	$rightvalue .= "+";
      } elsif ($tokens[$i] eq "/") {
	$rightvalue .= "*";
      } elsif ($tokens[$i] eq "*") {
	$rightvalue .= "/";
      } else {
	$rightvalue .= $tokens[$i];
      }
    }

    $retval[1] = $rightvalue;

#   print "] => ($retval[0];$retval[1])\n";

    return @retval;
  }



sub create_declaration
  {
    $typename = $type;
    $ctype = $types{$type}->{'ctype'};
    $constpctype = $types{$type}->{'constpctype'};

    if (not $types{$type}->{'external'}) {
      $types{$type}{'writer'} = "_cfsml_write_" . $typename;
      $types{$type}{'reader'} = "_cfsml_read_" . $typename;
      write_line_pp(__LINE__, 0);
      print "static void $types{$type}{'writer'}(Common::WriteStream *fh, $constpctype save_struc);\n";
      print "static int $types{$type}{'reader'}(Common::SeekableReadStream *fh, $ctype* save_struc, const char *lastval, int *line, int *hiteof);\n\n";
    };

  }

sub create_writer
  {
    $typename = $type;
    $ctype = $types{$type}{'ctype'};
    $constpctype = $types{$type}->{'constpctype'};

    write_line_pp(__LINE__, 0);
    print "static void\n_cfsml_write_$typename(Common::WriteStream *fh, $constpctype save_struc)\n{\n";

    if ($types{$type}{'type'} eq $type_integer) {
      print "	WSprintf(fh, \"%li\", (long)*save_struc);\n";
    }
    elsif ($types{$type}{'type'} eq $type_string) {
    write_line_pp(__LINE__, 0);
    print "	if (!(*save_struc))\n";
    print "		WSprintf(fh, \"\\\\null\\\\\");\n";
    print "	else {\n";
    print "		char *token = _cfsml_mangle_string((const char *) *save_struc);\n";
    print "		WSprintf(fh, \"\\\"%s\\\"\", token);\n";
    print "		free(token);\n";
    print "	}\n";
    }
    elsif ($types{$type}{'type'} eq $type_record) {
    write_line_pp(__LINE__, 0);
    print "	WSprintf(fh, \"{\\n\");\n";

    for $n (@{$records{$type}}) {

        print "	WSprintf(fh, \"$n->{'name'} = \");\n";

        if ($n->{'array'}) { # Check for arrays

        print "	int min, max;\n";
        if ($n->{'array'} eq 'static' or $n->{'size'} * 2) { # fixed integer value?
            print "	min = max = $n->{'size'};\n";
        }
        else { # No, a variable
            print "	min = max = save_struc->$n->{'size'};\n";
        }

        if ($n->{'maxwrite'}) { # A write limit?
            print "	if (save_struc->$n->{'maxwrite'} < min)\n";
            print "		min = save_struc->$n->{'maxwrite'};\n";
        }

        if ($n->{'array'} eq 'dynamic') {
            print "	if (!save_struc->$n->{'name'})\n";
            print "		min = max = 0; /* Don't write if it points to NULL */\n";
        }

        write_line_pp(__LINE__, 0);
        print "	WSprintf(fh, \"[%d][\\n\", max);\n";
        print "	for (int i = 0; i < min; i++) {\n";
        print "		$types{$n->{'type'}}{'writer'}";
        my $subscribstr = "[i]"; # To avoid perl interpolation problems
        print "(fh, &(save_struc->$n->{'name'}$subscribstr));\n";
        print "		WSprintf(fh, \"\\n\");\n";
        print "	}\n";
        print "	WSprintf(fh, \"]\");\n";

    } elsif ($n->{'type'} eq $type_pointer) { # Relative pointer

      print "	WSprintf(fh, \"%d\", save_struc->$n->{'name'} - save_struc->$n->{'anchor'}); // Relative pointer\n";

      } elsif ($n->{'type'} eq $type_abspointer) { # Absolute pointer

      print "	if (!save_struc->$n->{'name'})\n";
      print "		WSprintf(fh, \"\\\\null\\\\\");\n";
      print "	else\n";
      print "		$types{$n->{'reftype'}}{'writer'}";
      print "(fh, save_struc->$n->{'name'});\n";

    } else { # Normal record entry

      print "	$types{$n->{'type'}}{'writer'}";
      print "(fh, ($types{$n->{'type'}}{'constpctype'}) &(save_struc->$n->{'name'}));\n";

    }

    print "	WSprintf(fh, \"\\n\");\n";
      }

      print "	WSprintf(fh, \"}\");\n";
    }
    else {
      print STDERR "Warning: Attempt to create_writer for invalid type '$types{$type}{'type'}'\n";
    }
    print "}\n\n";

  }


sub create_reader
  {
    $typename = $type;
    $ctype = $types{$type}{'ctype'};

    write_line_pp(__LINE__, 0);
    print "static int\n_cfsml_read_$typename(Common::SeekableReadStream *fh, $ctype* save_struc, const char *lastval, int *line, int *hiteof)\n{\n";
    my $reladdress_nr = 0; # Number of relative addresses needed
    my $reladdress = 0; # Current relative address number
    my $reladdress_resolver = ""; # Relative addresses are resolved after the main while block

    if ($types{$type}{'type'} eq $type_record) {

      foreach $n (@{$records{$type}}) { # Count relative addresses we need
	if ($n->{'type'} eq $type_pointer) {
	  ++$reladdress_nr;
	}
      }

      if ($reladdress_nr) { # Allocate stack space for all relative addresses needed
	print "	int reladdresses[$reladdress_nr] = {0};\n";
      }
    }

    if ($types{$type}{'type'} eq $type_integer) {
	write_line_pp(__LINE__, 0);
	print "	char *token;\n";
	print "\n	*save_struc = strtol(lastval, &token, 0);\n";
	print "	if ((*save_struc == 0) && (token == lastval)) {\n";
	print "		_cfsml_error(\"strtol failed at line %d\\n\", *line);\n";
	print "		return CFSML_FAILURE;\n";
	print "	}\n";
	print "	if (*token != 0) {\n";
	print "		_cfsml_error(\"Non-integer encountered while parsing int value at line %d\\n\", *line);\n";
	print "		return CFSML_FAILURE;\n";
	print "	}\n";
	print "	return CFSML_SUCCESS;\n";
    } elsif ($types{$type}{'type'} eq $type_string) {
	write_line_pp(__LINE__, 0);
	print "\n";
	print "	if (strcmp(lastval, \"\\\\null\\\\\")) { // null pointer?\n";
	print "		unsigned int length = strlen(lastval);\n";
	print "		if (*lastval == '\"') { // Quoted string?\n";
	print "			while (lastval[length] != '\"')\n";
	print "				--length;\n\n";
	print "			if (!length) { // No matching double-quotes?\n";
	print "				_cfsml_error(\"Unbalanced quotes at line %d\\n\", *line);\n";
	print "				return CFSML_FAILURE;\n";
	print "			}\n\n";
	print "			lastval++; // ...and skip the opening quotes locally\n";
	print "			length--;\n";
	print "		}\n";
	print "		*save_struc = _cfsml_unmangle_string(lastval, length);\n";
	print "		_cfsml_register_pointer(*save_struc);\n";
	print "		return CFSML_SUCCESS;\n";
	print "	} else {\n";
	print "		*save_struc = NULL;\n";
	print "		return CFSML_SUCCESS;\n";
	print "	}\n";
    } elsif ($types{$type}{'type'} eq $type_record) {
	write_line_pp(__LINE__, 0);
	print "	char *token;\n";
	print "	int assignment, closed;\n\n";
	print "	if (strcmp(lastval, \"{\")) {\n";
	print "		_cfsml_error(\"Reading record $type; expected opening braces in line %d, got \\\"%s\\\"\\n\", *line, lastval);\n";
	print "		return CFSML_FAILURE;\n";
	print "	};\n";
	print "	closed = 0;\n";
	print "	do {\n";
	print "		const char *value;\n";
	print "		token = _cfsml_get_identifier(fh, line, hiteof, &assignment);\n\n";
	print "		if (!token) {\n";
	print "			_cfsml_error(\"Expected token at line %d\\n\", *line);\n";
	print "			return CFSML_FAILURE;\n";
	print "		}\n";
	print "		if (!assignment) {\n";
	print "			if (!strcmp(token, \"}\"))\n";
	print "				closed = 1;\n";
	print "			else {\n";
	print "				_cfsml_error(\"Expected assignment or closing braces in line %d\\n\", *line);\n";
	print "				return CFSML_FAILURE;\n";
	print "			}\n";
	print "		} else {\n";
	print "			value = \"\";\n";
	print "			while (!value || !strcmp(value, \"\"))\n";
	print "				value = _cfsml_get_value(fh, line, hiteof);\n";
	print "			if (!value) {\n";
	print "				_cfsml_error(\"Expected token at line %d\\n\", *line);\n";
	print "				return CFSML_FAILURE;\n";
	print "			}\n";
#	print "		}\n";


      foreach $n (@{$records{$type}}) { # Now take care of all record elements

	my $type = $n->{'type'};
	my $reference = undef;
	if ($type eq $type_abspointer) {
	    $reference = 1;
	    $type = $n->{'reftype'};
	}
	my $name = $n->{'name'};
	my $reader = $types{$type}{'reader'};
	my $size = $n->{'size'};

	print "				if (!strcmp(token, \"$name\")) {\n";

	if ($type eq $type_pointer) { # A relative pointer

	  $reader = $types{'int'}{'reader'}; # Read relpointer as int

	  write_line_pp(__LINE__, 0);
	  print "				if ($reader(fh, &(reladdresses[$reladdress]), value, line, hiteof)) {\n";
	  print "					_cfsml_error(\"Expected token at line %d\\n\", *line);\n";
	  print "					return CFSML_FAILURE;\n";
	  print "				}\n";

	  # Make sure that the resulting variable is interpreted correctly
	  $reladdress_resolver .= "			save_struc->$n->{'name'} = save_struc->$n->{'anchor'} + reladdresses[$reladdress];\n";

	  ++$reladdress; # Prepare reladdress for next element

	} elsif ($n->{'array'}) { # Is it an array?
	    write_line_pp(__LINE__, 0);
	    print "			if ((value[0] != '[') || (value[strlen(value) - 1] != '[')) {\n";
	    # The value must end with [, since we're starting array data, and it must also
	    # begin with [, since this is either the only character in the line, or it starts
	    # the "amount of memory to allocate" block
	    print "				_cfsml_error(\"Opening brackets expected at line %d\\n\", *line);\n";
	    print "				return CFSML_FAILURE;\n";
	    print "			}\n";

	  print "			int max,done,i;\n";
	  if ($n->{'array'} eq 'dynamic') {
	      write_line_pp(__LINE__, 0);
	    # We need to allocate the array first
	    print "			// Prepare to restore dynamic array\n";
	    # Read amount of memory to allocate
	    print "			max = strtol(value + 1, NULL, 0);\n";
	    print "			if (max < 0) {\n";
	    print "				_cfsml_error(\"Invalid number of elements to allocate for dynamic array '%s' at line %d\\n\", token, *line);\n";
	    print "				return CFSML_FAILURE;\n";
	    print "			}\n\n";

	    print "			if (max) {\n";
	    print "				save_struc->$name = ($n->{'type'} *)sci_malloc(max * sizeof($type));\n";
	    print "#ifdef SATISFY_PURIFY\n";
            print "				memset(save_struc->$name, 0, max * sizeof($type));\n";
	    print "#endif\n";
	    print "				_cfsml_register_pointer(save_struc->$name);\n";
	    print "			} else\n";
	    print "				save_struc->$name = NULL;\n"

	    } else { # static array
		print "			// Prepare to restore static array\n";
		print "			max = $size;\n";
	    }

	    write_line_pp(__LINE__, 0);
	    print "			done = i = 0;\n";
	    print "			do {\n";
	    if ($type eq $type_record) {
		print "			if (!(value = _cfsml_get_value(fh, line, hiteof))) {\n";
	    } else {
		print "			if (!(value = _cfsml_get_identifier(fh, line, hiteof, NULL))) {\n";
	    }
	    write_line_pp(__LINE__, 0);
	    
	    print "				_cfsml_error(\"Token expected at line %d\\n\", *line);\n";
	    print "				return 1;\n";
	    print "			}\n";
	    print "			if (strcmp(value, \"]\")) {\n";
	    print "				if (i == max) {\n";
	    print "					_cfsml_error(\"More elements than space available (%d) in '%s' at line %d\\n\", max, token, *line);\n";
	    print "					return CFSML_FAILURE;\n";
	    print "				}\n";
	    my $helper = "[i++]";
	    print "				if ($reader(fh, &(save_struc->$name$helper), value, line, hiteof)) {\n";
	    print "					_cfsml_error(\"Token expected by $reader() for $name$helper at line %d\\n\", *line);\n";
	    print "					return CFSML_FAILURE;\n";
	    print "				}\n";
	    print "			} else\n";
	    print "				done = 1;\n";
	    print "			} while (!done);\n";

	    if ($n->{'array'} eq "dynamic") {
		my @xpr = lvaluize($expression = $n->{'size'});
		print "			save_struc->$xpr[0] = max $xpr[1]; // Set array size accordingly\n";
	    }

	    if ($n->{'maxwrite'}) {
		my @xpr = lvaluize($expression = $n->{'maxwrite'});
		print "			save_struc->$xpr[0] = i $xpr[1]; // Set number of elements\n";
	    }

	}
	elsif ($reference) {
	    write_line_pp(__LINE__, 0);
	    print "			if (strcmp(value, \"\\\\null\\\\\")) { // null pointer?\n";
	    print "				save_struc->$name = sci_malloc(sizeof ($type));\n";
	    print "				_cfsml_register_pointer(save_struc->$name);\n";
	    print "				if ($reader(fh, save_struc->$name, value, line, hiteof)) {\n";
	    print "					_cfsml_error(\"Token expected by $reader() for $name at line %d\\n\", *line);\n";
	    print "					return CFSML_FAILURE;\n";
	    print "				}\n";
	    print "			} else\n";
	    print "				save_struc->$name = NULL;\n";
	}
	else { # It's a simple variable or a struct
	    write_line_pp(__LINE__, 0);
	    print "				if ($reader(fh, ($types{$type}{'ctype'}*) &(save_struc->$name), value, line, hiteof)) {\n";
	    print "					_cfsml_error(\"Token expected by $reader() for $name at line %d\\n\", *line);\n";
	    print "					return CFSML_FAILURE;\n";
	    print "				}\n";
	}
	print "			} else\n";

      }
	write_line_pp(__LINE__, 0);
	print "			{\n";
	print "				_cfsml_error(\"$type: Assignment to invalid identifier '%s' in line %d\\n\", token, *line);\n";
	print "				return CFSML_FAILURE;\n";
	print "			}\n";
	print "		}\n";

      print "	} while (!closed); // Until closing braces are hit\n";

      print $reladdress_resolver; # Resolves any relative addresses

      print "	return CFSML_SUCCESS;\n";
    } else {
      print STDERR "Warning: Attempt to create_reader for invalid type '$types{$type}{'type'}'\n";
    }

    print "}\n\n";
  }

# Built-in types

%types = (
	  'int' => {
		    'type' => $type_integer,
		    'ctype' => "int",
		    'constpctype' => "int const *",
		   },

	  'string' => {
		       'type' => $type_string,
		       'ctype' => "char *",
		       'constpctype' => "const char * const *",
		      },
	 );



sub create_function_block {
  print "\n// Auto-generated CFSML declaration and function block\n\n";
  write_line_pp(__LINE__, 0);
  print "#define CFSML_SUCCESS 0\n";
  print "#define CFSML_FAILURE 1\n\n";
  create_string_functions;

  foreach $n ( keys %types ) {
    create_declaration($type = $n);
  }

  foreach $n ( keys %types ) {
    if (not $types{$n}->{'external'}) {
      create_writer($type = $n);
      create_reader($type = $n);
    }
  }
  print "\n// Auto-generated CFSML declaration and function block ends here\n";
  print "// Auto-generation performed by cfsml.pl $version\n";
}


# Gnerates code to read a data type
# Parameters: $type: Type to read
#             $datap: Pointer to the write destination
#             $fh: Existing filehandle of an open file to use
#             $eofvar: Variable to store _cfsml_eof into
sub insert_reader_code {
  print "// Auto-generated CFSML data reader code\n";
  write_line_pp(__LINE__, 0);
  print "	{\n";
  if (!$linecounter) {
      write_line_pp(__LINE__, 0);
      print "		int _cfsml_line_ctr = 0;\n";
      $linecounter = '_cfsml_line_ctr';
  }
  if ($atomic) {
      write_line_pp(__LINE__, 0);
      print "		struct _cfsml_pointer_refstruct **_cfsml_myptrrefptr = _cfsml_get_current_refpointer();\n";
  }
  write_line_pp(__LINE__, 0);
  print "		int _cfsml_eof = 0, _cfsml_error;\n";

  if ($firsttoken) {
      write_line_pp(__LINE__, 0);
      print "		const char *_cfsml_inp = $firsttoken;\n";
      print "		{\n";
  } else {
      write_line_pp(__LINE__, 0);
      print "		const char *_cfsml_inp = _cfsml_get_identifier($fh, &($linecounter), &_cfsml_eof, 0);\n";
      print "		if (!_cfsml_inp) {\n";
      print "			_cfsml_error = CFSML_FAILURE;\n";
      print "		} else {\n";
  }

  write_line_pp(__LINE__, 0);
  print "			_cfsml_error = $types{$type}{'reader'}($fh, $datap, _cfsml_inp, &($linecounter), &_cfsml_eof);\n";
  print "		}\n";

  if ($eofvar) {
      write_line_pp(__LINE__, 0);
      print "		$eofvar = _cfsml_error;\n";
  }
  if ($atomic) {
      write_line_pp(__LINE__, 0);
      print "		_cfsml_free_pointer_references(_cfsml_myptrrefptr, _cfsml_error);\n";
  }
  write_line_pp(__LINE__, 0);
  print "		if (_cfsml_last_value_retrieved) {\n";
  print "			free(_cfsml_last_value_retrieved);\n";
  print "			_cfsml_last_value_retrieved = NULL;\n";
  print "		}\n";
  print "		if (_cfsml_last_identifier_retrieved) {\n";
  print "			free(_cfsml_last_identifier_retrieved);\n";
  print "			_cfsml_last_identifier_retrieved = NULL;\n";
  print "		}\n";
  print "	}\n";
  print "// End of auto-generated CFSML data reader code\n";
}

# Generates code to write a data type
# Parameters: $type: Type to write
#             $datap: Pointer to the write destination
#             $fh: Existing filehandle of an open file to use
sub insert_writer_code {
    write_line_pp(__LINE__, 0);
    print "// Auto-generated CFSML data writer code\n";
    print "	$types{$type}{'writer'}($fh, $datap);\n";
    print "	WSprintf($fh, \"\\n\");\n";
    print "// End of auto-generated CFSML data writer code\n";
}


################
# Main program #
################

$parsing = 0;
$struct = undef; # Not working on a struct
$commentmode = undef;
$line = 0;

while (<STDIN>) {

  $line++;

  if ($parsing) {
    ($data) = split "#"; # Remove shell-style comments
    @_ = ($data);

    s/\/\*.*\*\///g; # Remove C-style one-line comments

    ($data) = split "\/\/"; # Remove C++-style comments
    @_ = ($data);

    if ($commentmode) {

      if (grep /\*\//, $_) {
	($empty, $_) = split /\*\//;
      } else {
	@_ = (); # Empty line
      }

    } else {
      if (grep /\/\*/, $_) {
	$commentmode = 1;
	($_) = split /\/\*/;
      }
    }


    # Now tokenize:
    s/;//;
    split /(\".*\"|[,\[\]\(\)\{\}])|\s+/;

    @items = @_;

    @tokens = ();

    $tokens_nr = 0;
    for ($n = 0; $n < scalar @items; $n++) { # Get rid of all undefs
      if ($_[$n]) {
	$_ = $items[$n];
	s/\"//g;
	$tokens[$tokens_nr++] = $_;
      }
    }

    # Now all tokens are in @tokens, and we have $tokens_nr of them.

#    print "//DEBUG: " . join ("|", @tokens) . "\n";

    if ($tokens_nr) {
      if ($tokens_nr == 2 && $tokens[0] eq "%END" && $tokens[1] eq "CFSML") {

	$struct && die "Record $struct needs closing braces in intput file (line $line).";

	$parsing = 0;
	create_function_block;
	my $linep = $line + 1;
	write_line_pp($linep, 1);
      } elsif ($struct) { # Parsing struct
	if ($tokens_nr == 1) {
	  if ($tokens[0] eq "}") {
	    $struct = undef;
	  } else { die "Invalid declaration of $token[0] in input file (line $line)\n";};
	} else { # Must be a member declaration

	  my @structrecs = (@{$records{$struct}});
	  my $newidx = (scalar @structrecs) or "0";
	  my %member = ();
	  $member{'name'} = $tokens[1];
	  $member{'type'} = $tokens[0];

	  if ($tokens_nr == 3 && $tokens[1] == "*") {
	      $tokens_nr = 2;
	      $member{'name'} = $tokens[2];
	      $member{'reftype'} = $tokens[0];
	      $member{'type'} = $type_abspointer;
	  }

	  if ($tokens_nr == 4 and $tokens[0] eq $type_pointer) { # Relative pointer

	    if (not $tokens[2] eq "RELATIVETO") {
	      die "Invalid relative pointer declaration in input file (line $line)\n";
	    }

	    $member{'anchor'} = $tokens[3]; # RelPointer anchor

	  } else { # Non-pointer

	    if (not $types{$tokens[0]}) {
	      die "Unknown type $tokens[0] used in input file (line $line)\n";
	    }

	    if ($tokens_nr > 2) { # Array

	      if ($tokens[2] ne "[") {
		die "Invalid token '$tokens[2]' in input file (line $line)\n";
	      }

	      $member{'array'} = "static";

	      if ($tokens[$tokens_nr - 1] ne "]") {
		die "Array declaration incorrectly terminated in input file (line $line)\n";
	      }

	      $parsepos = 3;

	      while ($parsepos < $tokens_nr) {

		if ($tokens[$parsepos] eq ",") {

		  $parsepos++;

		} elsif ($tokens[$parsepos] eq "STATIC") {

		  $member{'array'} = "static";
		  $parsepos++;

		} elsif ($tokens[$parsepos] eq "DYNAMIC") {

		  $member{'array'} = "dynamic";
		  $parsepos++;

		} elsif ($tokens[$parsepos] eq "MAXWRITE") {

		  $member{'maxwrite'} = $tokens[$parsepos + 1];
		  $parsepos += 2;

		} elsif ($tokens[$parsepos] eq "]") {

		  $parsepos++;
		  if ($parsepos != $tokens_nr) {
		    die "Error: Invalid tokens after array declaration in input file (line $line)\n";

		  }
		} else {

		  if ($member{'size'}) {
		    die "Attempt to use more than one array size in input file (line $line)\n" .
		      "(Original size was \"$member->{'size'}\", new size is \"$tokens[$parsepos]\"\n";
		  }

		  $member{'size'} = $tokens[$parsepos];
		  $parsepos++;
		}
	      }


	      unless ($member{'size'}) {
		die "Array declaration without size in input file (line $line)\n";
	      }
	    }
	  }

	  @{$records{$struct}}->[$newidx] = \%member;
	}
      } else { # not parsing struct; normal operation.

	if ($tokens[0] eq "TYPE") { # Simple type declaration

	  my $newtype = $tokens[1];

	  $types{$newtype}->{'ctype'} = $tokens[2];
	  if ($tokens[2] =~ /\*\$/) {
	    $types{$newtype}->{'ctype'} = "const " . $tokens[2] . " const *";
	  } else {
	    $types{$newtype}->{'constpctype'} = $tokens[2] . " const *";
	  }

	  if ($tokens_nr == 5) { # must be ...LIKE...

	    unless ($tokens[3] eq "LIKE") {
	      die "Invalid TYPE declaration in input file (line $line)\n";
	    }

	    $types{$newtype}->{'type'} = $types{$tokens[4]}->{'type'};
	    $types{$newtype}->{'reader'} = $types{$tokens[4]}->{'reader'};
	    $types{$newtype}->{'writer'} = $types{$tokens[4]}->{'writer'};

	  } elsif ($tokens_nr == 6) { # must be ...USING...

	    unless ($tokens[3] eq "USING") {
	      die "Invalid TYPE declaration in input file (line $line)\n";
	    }

	    $types{$newtype}->{'writer'} = $tokens[4];
	    $types{$newtype}->{'reader'} = $tokens[5];
	    $types{$newtype}->{'external'} = 'T';

	  } else {
	    die "Invalid TYPE declaration in input file (line $line)\n";
	  }

	} elsif ($tokens[0] eq "RECORD") {

	  $struct = $tokens[1];
	  if ($types{$struct}) {
	    die "Attempt to re-define existing type $struct as a struct in input file (line $line)";
	  }
	  $types{$struct}{'type'} = $type_record;
	  if ($tokens_nr < 3 or $tokens_nr > 6 or $tokens[$tokens_nr - 1] ne "{") {
	    die "Invalid record declaration in input file (line $line)";
	  }

	  my $extoffset = 2;

	  if ($tokens_nr > 3) {
	      if ($tokens[2] ne "EXTENDS") { # Record declaration with explicit c type
		  $types{$struct}{'ctype'} = $tokens[2];
		  $extoffset = 3;
	      } else { # Record name is the same as the c type name
		  $types{$struct}{'ctype'} = $struct;
	      }
	  } elsif ($tokens_nr == 3) {
		  $types{$struct}{'ctype'} = $struct;
	  }
	  $types{$struct}{'constpctype'} = $struct . " const *";

	  if (($tokens_nr > $extoffset + 1) && ($extoffset + 1 <= $tokens_nr)) {
	      if ($tokens[$extoffset] ne "EXTENDS") {
		  die "Invalid or improper keyword \"$tokens[$extoffset]\" in input file (line $line)";
	      }
	      if ($extoffset + 2 >= $tokens_nr) {
		  die "RECORD \"$struct\" extends on unspecified type in input file (line $line)";
	      }
	      my $ext_type = $tokens[$extoffset + 1];

	      if (!($types{$ext_type}{type} eq $type_record)) {
		  print "$types{$ext_type}{type}";
		  die "RECORD \"$struct\" attempts to extend non-existing or non-record type \"$ext_type\" in input file (line $line)";
	      }

	      (@{$records{$struct}}) = (@{$records{$ext_type}}); # Copy type information from super type
	  }

	} else {
	  die "Invalid declaration \"$tokens[0]\" in line $line";
	}
      }
    }


  } else {

    ($subtoken) = split ";"; # Get rid of trailing ;s
    $tokens_nr = @tokens = split " ", $subtoken;

    if ($tokens_nr == 1 && $tokens[0] eq "%CFSML") {

      $parsing = 1;

    } elsif ($tokens[0] eq "%CFSMLWRITE" and $tokens[3] eq "INTO" and $tokens_nr >= 5) {

      insert_writer_code($type = $tokens[1], $datap = $tokens[2], $fh = $tokens[4]);
      my $templine = $line + 1;
      write_line_pp($templine, 1); # Yes, this sucks.

    } elsif (($tokens[0] eq "%CFSMLREAD") or ($tokens[0] eq "%CFSMLREAD-ATOMIC") and $tokens[3] eq "FROM" and $tokens_nr >= 5) {

      my $myeofvar = 0;
      my $myfirsttoken = 0;
      my $mylinecounter = 0;

      my $idcounter = 5;

      while ($idcounter < $tokens_nr) {
	if ($tokens[$idcounter] eq "ERRVAR" and $tokens_nr >= $idcounter + 2) {
	  $myeofvar = $tokens[$idcounter + 1];
	  $idcounter += 2;
	} elsif ($tokens[$idcounter] eq "FIRSTTOKEN" and $tokens_nr >= $idcounter + 2) {
	  $myfirsttoken = $tokens[$idcounter + 1];
	  $idcounter += 2;
	} elsif ($tokens[$idcounter] eq "LINECOUNTER" and $tokens_nr >= $idcounter + 2) {
	  $mylinecounter = $tokens[$idcounter + 1];
	  $idcounter += 2;
	} else {
	  die "Unknown %CFSMLREAD operational token: $tokens[$idcounter]\n";
	}
      }
      insert_reader_code($type = $tokens[1], $datap = $tokens[2],
			 $fh = $tokens[4], $eofvar = $myeofvar, $firsttoken = $myfirsttoken,
			$linecounter = $mylinecounter, $atomic = ($tokens[0] eq "%CFSMLREAD-ATOMIC"));
      my $templine = $line + 1;
      write_line_pp($templine, 1); # Yes, this sucks, too.

    } else {
      print;
    }
  }

}

if ($parsing) {
  print <STDERR>, "Warning: Missing %END CFSML\n";
}
