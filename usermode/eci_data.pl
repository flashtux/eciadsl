#!/usr/bin/perl -w

# Auteur : Benoit PAPILLAULT <benoit.papillault@free.fr>
# Creation : 01/07/2001
# Licence: GPL

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

# 07/02/2002 Added the CVS Id & Tag
# CVS $Id$
# Tag $Name$

# 11/03/2002 Benoit PAPILLAULT
#            added 2 functions to correctly and fully display USB descriptors
#            the output is similar to 'pusb-view'.

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

# print_one_descriptor ($sbuf)
# this function prints only one descriptors

sub print_one_descriptor {
	my ($buf) = @_;

	my $descriptor_type = ord (substr($buf, 1, 1));

# Here are the value of $descriptor_type defined by USB 1.1
# 1 => DEVICE
# 2 => CONFIGURATION
# 3 => STRING
# 4 => INTERFACE
# 5 => ENDPOINT
	
	if ($descriptor_type eq 1) { # DEVICE Descriptor
		
		$bDeviceClass    = ord(substr $buf,4,1);
		$bDeviceSubClass = ord(substr $buf,5,1);
		$bDeviceProtocol = ord(substr $buf,6,1);

		$idVendor = ord(substr $buf, 8,1) + ord(substr $buf, 9,1) * 256;
		$idProduct = ord(substr $buf,10,1) + ord(substr $buf,11,1) * 256;
		$bcdDevice = ord(substr $buf,12,1) . "." . ord(substr $buf,13,1);

		$bNumConfigurations = ord(substr $buf,17,1);
		
		print "Device: VendorID=0x"	. sprintf("%04x",$idVendor)
			. " ProductID=0x" . sprintf("%04x",$idProduct)
				. " Class=" . sprintf("%02x/%02x/%02x", $bDeviceClass,
									  $bDeviceSubClass, $bDeviceProtocol) 
					. ", " . $bNumConfigurations . " configuration(s)\n";
		
	} elsif ($descriptor_type eq 2) { # CONFIGURATION Descriptor

		$bNumInterfaces = ord(substr $buf,4,1);
		$bConfigurationValue = ord(substr $buf,5,1);
		$iConfiguration = ord(substr $buf,6,1);
		$MaxPower = 2 * ord(substr $buf,8,1);
		
		print "  configuration " . $iConfiguration
			. ", " . $bNumInterfaces . " interface(s) [$iConfiguration], "
				. $MaxPower . "mA\n";
		
	} elsif ($descriptor_type eq 3) { # STRING Descriptor
		
		if ($index != 0) {
			
			$l = ord(substr $buf, 0, 1);
			
			$s = "";
			for ($i=2; $i<length($buf) && $i<$l ;$i+=2) {
				$s .= substr $buf, $i, 1;
			}
			
			print "\tSTRING DESCRIPTOR $index: $s\n";
		}

	} elsif ($descriptor_type eq 4) { # INTERFACE Descriptor

# interface 0 alt 0 class ff/ff/ff, 3 endpoint(s) []

		$bInterfaceNumber = ord(substr $buf,2,1);
		$bAlternateSetting = ord(substr $buf,3,1);
		$bNumEndpoints = ord(substr $buf,4,1);
		$bInterfaceClass = ord(substr $buf,5,1);
		$bInterfaceSubClass = ord(substr $buf,6,1);
		$bInterfaceProtocol = ord(substr $buf,7,1);
		$iInterface = ord(substr $buf,8,1);

		print "    interface $bInterfaceNumber alt $bAlternateSetting "
			. "class " . sprintf("%02x/%02x/%02x", $bInterfaceClass,
								 $bInterfaceSubClass, $bInterfaceProtocol)
				. ", $bNumEndpoints endpoint(s) [$iInterface]\n";

	} elsif ($descriptor_type eq 5) { # ENDPOINT Descriptor

# endpoint 0x88 [Isoc] 1008 bytes 1 ms

		$bEndpointAddress = ord(substr $buf,2,1);
		$bmAttributes = ord(substr $buf,3,1);
		$wMaxPacketSize = ord(substr $buf,4,1) + ord(substr $buf,5,1) * 256;
		$bInterval = ord(substr $buf,6,1);

		$endpoint_type = $bmAttributes & 3;
		if ($endpoint_type eq 0) {
			$endpoint_str = "Ctrl";
		} elsif ($endpoint_type eq 1) {
			$endpoint_str = "Isoc";
		} elsif ($endpoint_type eq 2) {
			$endpoint_str = "Bulk";
		} elsif ($endpoint_type eq 3) {
			$endpoint_str = "Intr";
		}

		print "      endpoint 0x" . sprintf("%02x", $bEndpointAddress)
			. "[$endpoint_str] $wMaxPacketSize bytes $bInterval ms\n";

	} else {
		print "\tDESCRIPTOR type $descriptor_type, length "
			. length($buf) ."\n";
	}
}

# print_descriptor ($buf)
# this function prints a block containing several descriptors

sub print_descriptor {
	my ($buf) = @_;

	do {
# the first byte is the length of the first block
		my $l = ord(substr($buf, 0, 1));

# check the validity of $l
		if ($l <= 0 || $l > length($buf)) {
			print "incorrect l value\n";
			last;
		}

# we extract and print the first block
		my $sbuf = substr ($buf, 0, $l);
		print_one_descriptor($sbuf);

# we remove the first block from the 'big' block.
		$buf = substr ($buf, $l);
	} while (length($buf) > 0);
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
		print "BULK/INT $transfer_direction endpoint="
			. sprintf("%02x",$endpoint) . " size=$size\n";
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
			print "ISO $transfer_direction endpoint="
				. sprintf("%02x",$endpoint) . " size=$size\n";
			if ($display_buffer) {
				print_buffer ($buf);
			}
		}
	} elsif ($type eq "GET_DESCRIPTOR_FROM_DEVICE") {

		$buf = $urb_list{$k}{'buf'};
		$size = hex($urb_list{$k}{'length'});
		$index   = hex($urb_list{$k}{'index'});

		if ($size != length ($buf)) {
			print "Fatal error: URB $k has wrong buffer length\n";
			exit;
		}

		print "URB $k:\n";
		print_descriptor ($buf);
	}
}
