#! /usr/bin/perl

# Uncomment the following line to debug
# $DEBUG=1;
@codes;

$leaf_value = 0;
while (<>) {
    chop;
    @tokens = split //;

    if ($_ eq "-") {
	calc_values();
	print stderr "$leaf_value tokens processed; result is a huffman tree.\n";
	exit(0);
    }

    $codes_len[$leaf_value] = scalar @tokens;

    for ($i = 0; $i < scalar @tokens; $i++) {
	$codes[$leaf_value][$i] = $tokens[$i];
    }

    $leaf_value++;
}

$nodes_counter = 0;
@unlinked;

sub branch_node {
    $left = shift;
    $right = shift;
    print ("\tBRANCH_NODE(", $nodes_counter || "0" , ", $left, $right)\n");
    $nodes_counter++;
}

sub leaf_node {
    $value = shift;
    print ("\tLEAF_NODE  (", $nodes_counter || "0" ,", ", $value || "0" , ")\n");
    $nodes_counter++;
}

sub intval {
    my $nr = shift;
    my $retval = sub_intval(0, $codes_len[$nr], $nr);
    return $retval >> 1;
}

sub sub_intval {
    my $lv = shift;
    my $maxlv = shift;
    my $nr = shift;

    if ($maxlv >= 0) {
	my $v = $codes[$nr][$maxlv];
	my $retval = sub_intval($lv + 1, $maxlv-1, $nr) << 1;

	if ($v == "1") {
	    $retval |= 1;
	}
	return $retval || 0;
    } else {
	return 0;
    }
}

sub calc_values() {

    $depth = 1;
    my $startdepth = 100000;

    for ($i; $i < scalar @codes; $i++) {
	if ($codes_len[$i] > $depth) {
	    $depth = $codes_len[$i];
	}

	if ($codes_len[$i] < $startdepth) {
	    $startdepth = $codes_len[$i];
	}
    }

    branch_node(1, 2);

    $level = 1;
    $unlinked[0] = 1;
    $unlinked[1] = 2;
    $linkctr = 3;

    for (; $level <= $depth; $level++) {
	my $entries = 1 << $level;

	for ($j = 0; $j < ($entries << 1); $j++) {
	    $new_unlinked[$j] = -1;
	}

	for ($i = 0; $i < $entries; $i++) {
	    if ($unlinked[$i] > -1) {
		$match = -1;

		if ($DEBUG) {
		    print " Finding len=$level val=$i: ";
		}
		for ($j = 0; ($match == -1) && $j < $leaf_value; $j++) {
		    if (($codes_len[$j] == $level)
			&& (intval($j) == $i)) {
			$match = $j;
		    } else {
			if ($DEBUG) {
			    print "($j:$codes_len[$j],",intval($j),") ";
			}
		    }
		}
		if ($DEBUG) {
		    print "\n";
		}

		if ($match == -1) {
		    die "Expected $unlinked[$i], but counted $nodes_counter in $i at level $level" unless ($unlinked[$i] == $nodes_counter);
		    my $lnr = $linkctr++;
		    my $rnr = $linkctr++;
		    $new_unlinked[$i << 1] = $lnr;
		    $new_unlinked[1+($i << 1)] = $rnr;
		    branch_node($lnr, $rnr);
		} else {
		    leaf_node($match);
		    $new_unlinked[$i << 1] = -1;
		    $new_unlinked[1+($i << 1)] = -1;
		}
		
	    }
	}

	if ($DEBUG) {
	    print "level $level: Copying ", ($entries << 1), "\n";
	}
	for ($j = 0; $j < ($entries << 1); $j++) {
	    $unlinked[$j] = $new_unlinked[$j];
	    if ($DEBUG) {
		print $unlinked[$j], " ";
	    }
	}
	if ($DEBUG) {
	    print "\n";
	}
    }

    my $ok = 1;
    for ($j = 0; $j < ($entries << 1); $j++) {
	if ($unlinked[$j] != -1) {
	    $ok = 0;
	}
    }

    print "#warning \"Tree is not a huffman tree!\"\n" unless $ok;
}


