#!/usr/bin/env perl

use strict;

while(<>) {
   next if /^#/;
   chomp;
   my ($action,$func)=split;
   print qq{\t{"$action", $func},\n};
}
