#!/usr/bin/perl

use strict;

while(<>) {
   next if /^#/;
   chomp;
   my ($type,$text,$action,$options)=(m'^\s*(\w+)(?:\s+("[^"]+")(?:\s+([a-z0-9-]+)(.*))?)?');
   my $A='A_'.uc($action);
   $A=~s/-/_/g;

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
      print "{$text, FUNC$hide$options, $A},\n";
   } elsif($type eq 'end') {
      print "{NULL},\n";
   } elsif($type eq 'hline') {
      print qq{{(char*)"---"},\n};
   }
}
