#!/usr/bin/perl

use strict;
use warnings;

use IO::Socket::INET;

while (<STDIN>) {
    chomp;

    my $line = $_;

    if ($line =~ m/^\d+$/) {

	my $sock = IO::Socket::INET->new(PeerAddr => '127.0.0.1',
					 PeerPort => '1521',
					 Proto    => 'tcp');

	print $sock $line, "\n";

	while (<$sock>) {
	    chomp;

	    my $netline = $_;

	    if ($netline =~ m/^msg>/) {
		if ($netline =~ m/First byte of padding/) {
		    print '1', "\n";
		}
		else {
		    print '0', "\n";
		}
		close($sock);
		last;
	    }
	}
    }
}

