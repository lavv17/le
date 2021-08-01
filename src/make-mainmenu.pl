#!/usr/bin/env perl

use strict;

while(<>) {
   next if /^#/;
   chomp;
   my ($type,$text,$action,$options)=(m'^\s*(\w+)(?:\s+("[^"]+")(?:\s+(\S+)(.*))?)?');
   ($action,my $arg)=($action=~/^([^(]*)(?:\((.*)\))?$/);
   my $A='A_'.uc($action);
   $A=~s/-/_/g;
   if(defined $arg && $arg ne '') {
      $arg=~s{([^\\]|^)_}{$1 }g;
      $arg=~s{\\_}{_}g;
      $A.=qq{,{(char*)"$arg"}};
   }

   $options=~s/^\s+|\s+$//g;
   my @options=split ' ',$options;
   my $hide='';
   for(@options) {
      $hide='|HIDE',undef $_,next if $_ eq 'hide';
      s/-/_/g;
      $_='MENU_COND_'.uc($_);
   }

   $options=join('|',grep {$_} @options);
   $options='|'.$options if $options;
   $text='(char*)'.$text;
   if($type eq 'submenu') {
      print "{$text, SUBM$options},\n";
   } elsif($type eq 'function') {
      die if !$action;
      $type='FUNC'.$hide;
      print "{$text, FUNC$hide$options, { $A } },\n";
   } elsif($type eq 'end') {
      print "{NULL},\n";
   } elsif($type eq 'hline') {
      print qq{{(char*)"---"},\n};
   }
}
