#!/usr/bin/env perl

use strict;

while(<>) {
   chomp;
   my ($name,$code)=split;
   ($name,my $arg)=($name=~/^([^(]*)(?:\((.*)\))?$/);
   my $A=uc($name);
   $A=~s/-/_/g;
   $arg//='';
   if($arg ne '') {
      $arg=~s{([^\\]|^)_}{$1 }g;
      $arg=~s{\\_}{_}g;
      $arg=qq{,(char*)"$arg"};
   }
   $code=~s{\\([\$|])}{\\\\$1};
   print qq{\t{A_$A,(char*)"$code"$arg},\n};
}
