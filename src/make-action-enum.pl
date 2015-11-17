#!/usr/bin/perl

use strict;

my $base=1024;

print <<EOF;
enum Action {
	A__FIRST=$base,
EOF
my $n=$base;
while(<>) {
   next if /^#/;
   chomp;
   my ($action,$func)=split;
   $action=uc($action);
   $action=~s/-/_/g;
   print "\tA_$action=$n,\n";
   ++$n;
}
--$n;
print "\tA__LAST=$n,\n";

$n=2048; # special action codes
print "\tMOUSE_ACTION=$n,\n";$n++;
print "\tWINDOW_RESIZE=$n,\n";$n++;
print "\tNO_ACTION=$n,\n";

print "};\n";
