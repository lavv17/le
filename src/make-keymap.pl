#!/usr/bin/perl

use strict;

while(<>) {
   chomp;
   my ($name,$code)=split;
   my $A=uc($name);
   $A=~s/-/_/g;
   $code=~s{\\([\$|])}{\\\\$1};
   print qq{\t{A_$A,(char*)"$code"},\n};
}
