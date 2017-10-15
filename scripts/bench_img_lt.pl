#!/bin/perl

use warnings;
use strict;

my $count = 10;
my $cmd = "./bin/contr";
my $sum = 0;

for (my $i = 0; $i < $count; $i++) {
    my $out = `$cmd $ARGV[0]`;

    foreach my $line (split /\n/, $out) {
        chomp $line;
        if ($line =~ /Loaded media in (\d+)usec/) {
            $sum += $1;
        }
    }
}

my $avg = $sum / $count;
print "$avg\n", 



