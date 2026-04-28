#!/usr/bin/perl

use strict;
use warnings;

use FindBin ();
use lib ("$FindBin::Bin/../testlib");
use lib ("/home/ilia/ai/statgrab/vendor/libstatgrab/tests/multi_threaded/../testlib");

use Getopt::Long qw(:config bundling);
use Data::Dumper ('Dumper');
use Pod::Usage   ('pod2usage');

use run_tests    ();

my %cmdOptions = ();
GetOptions( \%cmdOptions,
	    "help|h" => sub { pod2usage(0); },
	    "items|i=i@" =>,
	  ) or exit(1);

my $test_prog = "$FindBin::Bin/diff_stats";
chdir($FindBin::Bin);

my $tester = run_tests->new( $test_prog );
my $test_variants;
foreach my $item_count (@{$cmdOptions{items}}) {
	push( @$test_variants, $tester->get_test_combinations( $item_count ) );
}
$tester->run_tests( \@ARGV, $test_variants );

=head1 NAME

run_tests (script) - runs the diff-stats tests

=head1 DESCRIPTION

This I<run_tests> script runs the diff-stats tests for libstatgrab.

=head1 SYNOPSIS

  # run all functions alone and all 2-tuple, each in 3 threads
  $ tests/multi_threaded/run_tests --items 1 --items 2 -- -m 3

  # run all 4-tuple of libstatgrab functions, use at least 40 threads
  $ tests/multi_threaded/run_tests --items 4 -- -n 40

  # run all 7-tuple of libstatgrab functions, each in 5 threads but
  # sequencial function execution
  $ tests/multi_threaded/run_tests --items 4 -- -m 5 -s 1


=head1 OPTIONS

=over 8

=item C<--items|-i>

Specify how amount of I<libstatgrab>'s functions to test. All possible
combinations of k elements from n functions are generated and passed to
C<diff_stats>.

=item C<-->

Separator for options passed directly to C<diff_stats>.

=back

=over 4

=item *

The options of C<diff_stats> can be seen when running
C<tests/multi_threaded/diff_stats> with the C<-h> option.

=item *

The available libstatgrab functions of C<diff_stats> can be seen
when running C<tests/multi_threaded/diff_stats> with the
C<-l> option.

=back

=head1 AUTHOR

Jens Rehsack <sno@NetBSD.org>

=cut

