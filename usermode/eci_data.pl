#!/usr/bin/perl -w

# Auteur : Benoit PAPILLAULT <benoit.papillault@free.fr>
# Creation : 01/07/2001

# usage : used to analyze snoopy log file passed on STDIN. This script displays
# complete information on ISO, BULK, INTERRUPT and VENDOR_DEVICE (subset of
# CONTROL messages).

# pusb_control_msg ( dev, request_type , request, value, index, buf, size, tmo)
# request_type = 0x40 (vendor device OUT)

# 23/11/2001. Afficher les URBs classer par numero ne respecte pas
#   l'echelle de temps. En effet, pour une URB qui envoie des donnees, c'est
#   le temps de l'URB "going down" qui compte. Et pour une URB utilisee pour
#   recevoir des donnees, c'est le temps de l'URB "going back". On creer donc
#   une echec de temps lineaire ($t) pour classer les urbs.

# A la fin, on dispose pour chaque URB de son 'temps'. On fabrique un hash
# tel que $urb_t{$t} = $urb. Et enfin, on trie la liste des temps.
# avec sort {$a <=> $b} (keys %urb_t) 

# 19/12/2001 Added a protection against duplicated URB. This can happen
# if the input log file contains several logs, one after each other. This
# is detected by several "URB n going down" where n is identical. This result
# in a fatal error.

# 13/01/2001 Change 'in' to 'in ' (be carefull to the ending space). We now
# have the same number of letters as in 'out'. Change all occurrences of
# in&out to reflect this new value and we prefer using 'in ' instead of "in".

# Also added some checks on TransferBufferLength.

$t = 0;

# print_buffer ($buf)
# print a buffer in hexadecimal and ASCII format

sub print_buffer {
	my ($buf) = @_;

	for ($i=0;$i < length($buf); $i+=16) {
		for ($j=$i;$j<length($buf) && $j<$i+16;$j++) {
			printf ("%02x ", ord(substr($buf, $j, 1)));
		}
		for (;$j<$i+16;$j++) {
			print "   ";
		}
		for ($j=$i;$j<length($buf) && $j<$i+16;$j++) {
			$b = substr($buf,$j,1);
			if (ord($b)>=0x20 && ord($b)<0x7f) {
				print $b;
			} else {
				print ".";
			}
		}
		print "\n";
	}
}

# For each URB, we have the following fields ($urb is the original URB number)
# $urb_list{$urb}{'exist'} = 'yes' / 'no'
# $urb_list{$urb}{'t'}   is initialized at "going down"
#                      AND when transfer_direction is 'in '
# $urb_list{$urb}{'type'} = 'VENDOR_DEVICE' / 'BULK_OR_INTERRUPT'
#                           / 'ISO_TRANSFER' / 'GET_DESCRIPTOR_FROM_DEVICE'
# $urb_list{$urb}{'endpoint"}
# $urb_list{$urb}{'transfer_direction'} = 'in ' / 'out'
# $urb_list{$urb}{'buf'}
# $urb_list{$urb}{'lenght'} should be length(buf)
# $urb_list{$urb}{'request'}
# $urb_list{$urb}{'value'}
# $urb_list{$urb}{'index'}

undef ($urb_direction);

# we read the snoopy log file on stdin.

while (<>) {

	if (/URB ([0-9]+) going down/) {
		$urb_direction = 'out';
		$urb = $1;
		$buffer_ok = 1;

		if (defined ($urb_list{$urb})) {
			print "Fatal error : URB $urb is duplicated\n";
			exit ;
		}

		$urb_list{$urb}{'exist'} = 'yes';
		$urb_list{$urb}{'buf'} = '';

		$t ++;
		$urb_list{$urb}{'t'} = $t;
		next ;
	}

	if (/URB ([0-9]+) coming back/) {
		$urb_direction = 'in ';
		$urb = $1;
		$buffer_ok = 1;

		$urb_list{$urb}{'exist'} = 'yes';
		next;
	}

	if (/URB_FUNCTION_VENDOR_DEVICE/) {
		$urb_list{$urb}{'type'} = "VENDOR_DEVICE";
		next ;
	}

	if (/URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER/) {
		$urb_list{$urb}{'type'} = "BULK_OR_INTERRUPT";
		next ;
	}

	if (/URB_FUNCTION_ISOCH_TRANSFER/) {
		$urb_list{$urb}{'type'} = "ISO_TRANSFER";
		next;
	}

	if (/URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE/) {
		$urb_list{$urb}{'type'} = "GET_DESCRIPTOR_FROM_DEVICE";
		next;
	}

	if (/endpoint[ \t]+0x([0-9a-f]+)/i) {
		$urb_list{$urb}{'endpoint'} = hex($1);
		next ;
	}

	if (/USBD_TRANSFER_DIRECTION_IN/) {
		$urb_list{$urb}{'transfer_direction'} = 'in ';

		$t ++;
		$urb_list{$urb}{'t'} = $t;
		next;
	}

	if (/USBD_TRANSFER_DIRECTION_OUT/) {
		$urb_list{$urb}{'transfer_direction'} = 'out';
		next;
	}

	if (/TransferBufferLength[ \t]+=[ \t]+([0-9a-f]+)/i) {
		if (defined ($urb_list{$urb}{'transfer_direction'})) {
			$transfer_direction = $urb_list{$urb}{'transfer_direction'};
			if (($transfer_direction eq 'out')
				&& ($urb_direction eq 'out')) {
				$urb_list{$urb}{'length'} = $1;
			}

			if (($transfer_direction eq 'in ')
				&& ($urb_direction eq 'in ')) {
				$urb_list{$urb}{'length'} = $1;
			}
		}
		next;
	}

	if (/SetupPacket/) {
		$buffer_ok = 0;
		next ;
	}

	if (/Request[ \t]+=[ \t]+([0-9a-f]+)/i) {
		$urb_list{$urb}{'request'} = $1;
		next;
	}

	if (/Value[ \t]+=[ \t]+([0-9a-f]+)/i) {
		$urb_list{$urb}{'value'} = $1;
		next;
	}

	if (/Index[ \t]+=[ \t]+([0-9a-f]+)/i) {
		$urb_list{$urb}{'index'} = $1;
		next;
	}

	if ($buffer_ok && /[0-9a-f]+:[ \t]+([0-9a-f ]+)/i) {
		@list = split / /, $1;

		foreach $b (@list) {
			$urb_list{$urb}{'buf'} .= chr(hex($b));
		}
		next ;
	}
}

print "Summing up ... \n";

# on construit %urb_t

foreach $urb (keys %urb_list) {
	$t = $urb_list{$urb}{'t'};
	$urb_t{$t} = $urb;
}

# configuration begin

$display_vendor = 1;
$display_bulk_or_interrupt = 1;
$display_iso = 1;
$display_buffer = 1;

# configuration end

print "Displaying ...\n";

foreach $t (sort {$a <=> $b} (keys %urb_t)) {

	$k = $urb_t{$t};

	if (!defined ($urb_list{$k}{'type'})) {
		next;
	}

	$type = $urb_list{$k}{'type'};

	if ($type eq "VENDOR_DEVICE" && $display_vendor) {
	
		$transfer_direction = $urb_list{$k}{'transfer_direction'};
		$request = $urb_list{$k}{'request'};
		$value   = $urb_list{$k}{'value'};
		$index   = $urb_list{$k}{'index'};

		if (!defined($request)) {
			print "Fatal error: URB $k has undefined Request\n";
			exit;
		}

		if (!defined($value)) {
			print "Fatal error: URB $k has undefined Value\n";
			exit ;
		}

		if (!defined($index)) {
			print "Fatal error: URB $k has undefined Index\n";
			exit;
		}

		print "URB $k: ";
		if ($transfer_direction eq 'in ') {
			print "$transfer_direction request_type=0xc0 ";
		} else {
			print "$transfer_direction request_type=0x40 ";
		}
		print " request=$urb_list{$k}{'request'} value=$urb_list{$k}{'value'} index=$urb_list{$k}{'index'}\n";

		if ($display_buffer) {
			print_buffer ($urb_list{$k}{'buf'});
		}
	} elsif ($type eq "BULK_OR_INTERRUPT" && $display_bulk_or_interrupt) {

		if (!defined($urb_list{$k}{'length'})) {
			print "Fatal error: URB $k has undefined TransferBufferLength\n";
			exit ;
		}

		$endpoint = $urb_list{$k}{'endpoint'};
		$transfer_direction = $urb_list{$k}{'transfer_direction'};
		$buf = $urb_list{$k}{'buf'};
		$size = hex($urb_list{$k}{'length'});

		print "URB $k: ";
		if ($size != length ($buf)) {
			print "XXX ";
		}
		print "BULK/INT $transfer_direction endpoint=$endpoint size=$size\n";
		if ($display_buffer) {
			print_buffer ($buf);
		}
	} elsif ($type eq "ISO_TRANSFER" && $display_iso) {

		if (!defined($urb_list{$k}{'length'})) {
			print "Fatal error: URB $k has undefined TransferBufferLength\n";
			exit ;
		}

		$endpoint = $urb_list{$k}{'endpoint'};
		$transfer_direction = $urb_list{$k}{'transfer_direction'};
		$buf = $urb_list{$k}{'buf'};
		$size = hex($urb_list{$k}{'length'});

		if ($size != 0) {
			print "URB $k: ";
			if ($size != length ($buf)) {
				print "XXX ";
			}
			print "ISO $transfer_direction endpoint=$endpoint size=$size\n";
			if ($display_buffer) {
				print_buffer ($buf);
			}
		}
	}
}