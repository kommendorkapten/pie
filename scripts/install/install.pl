#!/usr/bin/env perl

use strict;
use warnings;
use Term::ReadKey;
use File::Path qw(make_path);
use Sys::Hostname qw(hostname);
use Cwd;

`sqlite3 --version >/dev/null 2>&1`;
if ($? != 0) {
    die "sqlite3 must be installed.";
}

if (getcwd() !~ /scripts\/install$/) {
    die "Install script must be run from the install directory\n";
}

my $path = "";
my $key = "";
my $done = 0;
my $hostname = hostname;
my $short_hostname = $hostname;
my $hid = 1;

if ($hostname =~ /([^\.]+)\.*+/) {
    $short_hostname = $1;
}


printf "Welcome to the PIE installer.\n";
while (! $done) {
    printf "Path to store db and other data in?\n";
    $path = readline(STDIN);
    chomp $path;
    printf "Root directory '$path', is this correct [Y]/n?\n";

    ReadMode 'cbreak';
    $key = ReadKey(0);
    ReadMode 'normal';

    if ($key eq 'y' || $key eq 'Y' || $key eq "\n") {
        $done = 1;
    }
}

$path = $1 if ($path =~ /(.*)\/$/);

if (! -d $path) {
    printf "Directory doesn't exist, create it [Y]/n?\n";
    ReadMode 'cbreak';
    $key = ReadKey(0);
    ReadMode 'normal';

    my $quit = 1;
    if ($key eq 'y' || $key eq 'Y' || $key eq "\n") {
        $quit = 0;
        make_path($path, {chmod => 0755}) or die "Failed to create $path";
    }
    if ($quit) {
        printf "Can't continue, leaving\n";
    }
}

foreach my $d (qw(stash thumbs proxy export)) {
    if (! -d "$path/$d") {
        make_path("$path/$d", {chmod => 0755}) or die "Failed to create $path/$d";
    }
}

my $db = "$path/pie.db";

if (-e $db) {
    printf "Database exist.\n";
} else {
    printf "Creating database...";
    my @files = `find ../../dm/ -name 'pie*.sql'`;
    foreach my $f (@files) {
        chomp $f;
        `sqlite3 $db < $f`;
    }
    printf "done\n";
}

printf "Initialising database...";

my $init = <<"EOF";
insert into pie_host values($hid, '$short_hostname', '$hostname');

insert into pie_storage values(1, 'stash', 1, $hid);
insert into pie_storage values(2, 'thumbs', 3, $hid);
insert into pie_storage values(3, 'proxy', 4, $hid);
insert into pie_storage values(4, 'export', 5, $hid);

insert into pie_mountpoint values($hid, 1, '$path/stash');
insert into pie_mountpoint values($hid, 2, '$path/thumbs');
insert into pie_mountpoint values($hid, 3, '$path/proxy');
insert into pie_mountpoint values($hid, 4, '$path/export');

insert into pie_collection values(3, "/", 0, 0, 493);
EOF

my $pid = open(my $fh, "| sqlite3 $db") or die "Couldn't fork: $!\n";
print $fh $init;
close($fh);

printf "done\n";

printf "Generating config file...";
`sed 's|DB_PATH|$db|' pie.conf.tmpl > pie.conf`;
printf "Done\n";
printf "Installation is almost done, review config in pie.conf and copy to\n";
printf "/etc/pie.config.\n";
