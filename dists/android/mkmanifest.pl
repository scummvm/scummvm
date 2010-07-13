#!/usr/bin/perl

use File::Basename qw(dirname);
use File::Path qw(mkpath);
use IO::File;
use XML::Writer;
use XML::Parser;
use Getopt::Long;

use warnings;
use strict;

use constant ANDROID => 'http://schemas.android.com/apk/res/android';

my $id;
my $package_versionName;
my $package_versionCode;
my $configure = 'configure';
my $stringres = 'res/string/values.xml';
my $manifest = 'AndroidManifest.xml';
my $master_manifest;
my @unpack_libs;
GetOptions('id=s' => \$id,
	   'version-name=s' => \$package_versionName,
	   'version-code=i' => \$package_versionCode,
	   'configure=s' => \$configure,
	   'stringres=s' => \$stringres,
	   'manifest=s' => \$manifest,
	   'master-manifest=s' => \$master_manifest,
	   'unpacklib=s' => \@unpack_libs,
    ) or die;
die "Missing required arg"
    unless $id and $package_versionName and $package_versionCode;


sub grope_engine_info {
    my $configure = shift;
    my @ret;
    while (<$configure>) {
	m/^add_engine \s+ (\w+) \s+ "(.*?)" \s+ \w+ (?:\s+ "([\w\s]*)")?/x
	    or next;
	my $subengines = $3 || '';
	my %info = (id => $1, name => $2,
		    subengines => [split / /, $subengines]);
	push @ret, \%info;
    }
    return @ret;
}

sub read_constraints {
    my $manifest = shift;
    my @constraints;
    my $parser = new XML::Parser Handlers => {
	Start => sub {
	    my $expat = shift;
	    my $elem = shift;
	    return if $elem !~
		/^(uses-configuration|supports-screens|uses-sdk)$/;
	    my @constraint = ($elem);
	    while (@_) {
		my $attr = shift;
		my $value = shift;
		$attr = [ANDROID, $attr] if $attr =~ s/^android://;
		push @constraint, $attr, $value;
	    }
	    push @constraints, \@constraint;
	},
    };
    $parser->parse($manifest);
    return @constraints;
}

sub print_stringres {
    my $output = shift;
    my $info = shift;

    my $writer = new XML::Writer(OUTPUT => $output, ENCODING => 'utf-8',
				 DATA_MODE => 1, DATA_INDENT => 2);

    $writer->xmlDecl();
    $writer->startTag('resources');

    while (my ($k,$v) = each %$info) {
	$writer->dataElement('string', $v, name => $k);
    }

    $writer->endTag('resources');
    $writer->end();
}

sub print_manifest {
    my $output = shift;
    my $info = shift;
    my $constraints = shift;

    my $writer = new XML::Writer(OUTPUT => $output, ENCODING => 'utf-8',
				 DATA_MODE => 1, DATA_INDENT => 2,
				 NAMESPACES => 1,
				 PREFIX_MAP => {ANDROID, 'android'});

    $writer->xmlDecl();

    $writer->startTag(
	'manifest',
	'package' => "org.inodes.gus.scummvm.plugin.$info->{name}",
	[ANDROID, 'versionCode'] => $package_versionCode,
	[ANDROID, 'versionName'] => $package_versionName,
	[ANDROID, 'installLocation'] => 'preferExternal',
	);

    $writer->startTag(
	'application',
	[ANDROID, 'label'] => '@string/app_name',
	[ANDROID, 'description'] => '@string/app_desc',
	[ANDROID, 'icon'] => '@drawable/scummvm',
	);

    $writer->startTag(
	'receiver',
	[ANDROID, 'name'] => 'org.inodes.gus.scummvm.PluginProvider',
	[ANDROID, 'process'] => 'org.inodes.gus.scummvm');

    $writer->startTag('intent-filter');
    $writer->emptyTag('action', [ANDROID, 'name'] =>
		      'org.inodes.gus.scummvm.action.PLUGIN_QUERY');
    $writer->emptyTag('category', [ANDROID, 'name'] =>
		      'android.intent.category.INFO');
    $writer->endTag('intent-filter');
    $writer->emptyTag(
	'meta-data',
	[ANDROID, 'name'] => 'org.inodes.gus.scummvm.meta.UNPACK_LIB',
	[ANDROID, 'value'] => $_)
	for @{$info->{unpack_libs}};

    $writer->endTag('receiver');
    $writer->endTag('application');

    $writer->emptyTag('uses-permission', [ANDROID, 'name'] =>
		      'org.inodes.gus.scummvm.permission.SCUMMVM_PLUGIN');

    $writer->emptyTag(@$_) foreach @$constraints;

    $writer->endTag('manifest');
    $writer->end();
}


my %engines;
for my $engine (grope_engine_info(new IO::File $configure, 'r')) {
    $engines{$engine->{id}} = $engine;
}

my @games = ($id, @{$engines{$id}{subengines}});
my $games_desc = join('; ', map $engines{$_}{name}, @games);

my @constraints = read_constraints(new IO::File $master_manifest, 'r');

print "Writing $stringres ...\n";
mkpath(dirname($stringres));
print_stringres(IO::File->new($stringres, 'w'),
		{app_name => qq{ScummVM plugin: "$id"},
		 app_desc => "Game engine for: $games_desc",
		});

print "Writing $manifest ...\n";
mkpath(dirname($manifest));
print_manifest(IO::File->new($manifest, 'w'),
	       {name => $id, unpack_libs => \@unpack_libs}, \@constraints);

exit 0;
