#!/usr/bin/perl -w

# Auteur : Benoit PAPILLAULT <benoit.papillault@free.fr>
# Creation :23/11/2001

# 23/11/2001 : copy from eci_data.pl, left old comments

# pusb_control_msg ( dev, request_type , request, value, index, buf, size, tmo)
# request_type = 0x40 (vendor device OUT)

# 16/11/2001 updated to accept the log format from snoopy 1.4

# Format of the generated file (all number are stored in big endian)
# It contains CTRL URBs one after each other, until the end of the file.

# 1 byte  : $request_type
# 1 byte  : $request
# 2 bytes : $value
# 2 bytes : $index
# 2 bytes : $size
# $size bytes (only for ($request_type & 0x80) == 0) : buffer data.
#            used only for CTRL transfer from host-to-device.

# 20/12/2001 : removed stupid hardcoded value!

# 13/01/2002 : added the $interrupt flag which is set when we search for
# an interrupt packet of 64 bytes is received. At this time, we stop the
# generation of the .bin file.

# Due to the source code organisation, incorrect URBs are just ignored and
# not always reported! Use eci_data.pl to check the validity of your USB log
# file.

# 07/02/2002 : added the CVS Id.
# CVS $Id$
# Tag $Name$

$active = 0;

$file = "eci_vendor.bin";

open BIN, ">$file" or die "Can't open $file: $!\n";

while (<>) {

# At the start of each URB , we reset all our variables and write down the
# URB number.
	if ((/URB ([0-9]+) going down/)
		|| (/URB ([0-9]+) coming back/)) {

		$active = 0;
		$urb = $1;

		undef ($request_type);
		undef ($request);
		undef ($value);
		undef ($index);
		undef ($size);

		$buf = '';
		$vendor = 0;
		$interrupt = 0;

		if (/coming back/) {
			$urb_direction = 'in';
		} else {
			$urb_direction = 'out';
		}
	}

	if (/URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER/) {
# To received an INT packet, we are only interested in "coming back" URB.
		if ($urb_direction eq 'in') {
			$interrupt = 1;
		}
		next;
	}

	if (/URB_FUNCTION_CONTROL_TRANSFER/) {
		$active = 1;
		$vendor = 0;
		next;
	}

	if (/URB_FUNCTION_VENDOR_DEVICE/) {
		$active = 1;
		$vendor = 1;
		next;
	}

	if ($interrupt && /endpoint[ \t]+0x([0-9a-f]+)/i) {
# we are only interested in endpoint 0x86 which is the INT endpoint 
		if (hex($1) != 0x86) {
			$interrupt = 0;
		}
		next ;
	}

	if (/USBD_TRANSFER_DIRECTION_OUT/) {
		if ($vendor) {
			$request_type = 0x40;
		}
		next;
	}

	if (/USBD_TRANSFER_DIRECTION_IN/) {
		if ($vendor) {
			$request_type = 0xc0;
		}
		next;
	}

	if (/TransferBufferLength[ \t]+=[ \t]+([0-9a-f]+)/i) {
		if ($interrupt) {
# we check if the interrupt packet is 64 bytes long. If so, end the loop.
			if (hex ($1) == 64) {
				print "interrupt packet detected. properly ending the generation of eci_vendor.bin\n" ;
				last;
			}
		}
				
		if ($active) {
			if ($vendor) {
				$size = hex ($1);
			}
		}
		next;
	}
	
	if ($active && /[0-9a-f]+: ([0-9a-f ]+)/i) {
		@list = split / /, $1;

		foreach $byte (@list) {
			$buf .= chr(hex($byte));
		}
		next;
	}

	if ($active && /Request[ \t]+=[ \t]+([0-9a-f]+)/i) {
		if ($vendor) {
			$request = hex ($1);
		}
		next;
	}
	
	if ($active && /Value[ \t]+=[ \t]+([0-9a-f]+)/i) {
		if ($vendor) {
			$value = hex($1);
		}
		next;
	}
		
# since we save the buffer in the .bin file when the 'Index' keyword
# is present, data received is not saved. (buffer from IN URBs).

	if ($active && /Index[ \t]+=[ \t]+([0-9a-f]+)/i) {
		if ($vendor) {
			$index = hex($1);

			# we check everything!

			if (!defined ($request_type)) {
				print "Fatal error: URB $urb has undefined RequestType\n";
				exit;
			}

			if (!defined($request)) {
				print "Fatal error: URB $urb has undefined Request\n";
				exit;
			}

			if (!defined($value)) {
				print "Fatal error: URB $urb has undefined Value\n";
				exit;
			}

			if (!defined($index)) {
				print "Fatal error: URB $urb has undefined Index\n";
				exit;
			}

			if (!defined ($size)) {
				print "Fatal error: URB $urb has undefined TransferBufferLength\n";
				exit;
			}

# remember that buf is empty for "IN" request. Thus, we check $buf's length
# only for "OUT" requests.

			if ($request_type == 0x40) { # OUT request
				if (length($buf) != $size) {
					print "Fatal error: URB $urb: length=" .
						length($buf) . " expecting $size bytes\n";
					exit;
				}
			}

			if ($request_type == 0xc0) { # IN request
				if (length($buf) != 0) {
					print "Fatal error: URB $urb: length=" .
						length($buf) . " expecting nothing\n";
				}
			}
			
			print "saving urb $urb\n";
			
			print BIN chr($request_type);
			print BIN chr($request);
			print BIN chr($value >> 8);
			print BIN chr($value&0xff);
			print BIN chr($index >> 8);
			print BIN chr($index&0xff);
			print BIN chr($size >> 8);
			print BIN chr($size&0xff);
			print BIN $buf;
		}
		next;
	}

# 'SetupPacket' is followed by a buffer, that we don't want to analyze. That's
# why we reset the $active flag to 0.
	if (/SetupPacket/) {
		$active = 0;
		next;
	}
}

close (BIN);

