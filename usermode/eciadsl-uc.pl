#!/usr/bin/perl -w

# Auteur : Benoit PAPILLAULT <benoit.papillault@free.fr>
# Creation : 01/07/2001
# Licence : GPL

# 16/07/2001 : stopped the firmware image generation after "DriverUnload"

# pusb_control_msg ( dev, request_type , request, value, index, buf, size, tmo)
# request_type = 0x40 (vendor device OUT)

# 7f92 : CPUCS $12.5 EZ-USB tech. ref.

# firmware.bin has the following file format :
# it's a list of the following structure.
# 2 bytes : in big endian : addr
# 2 bytes : in big endian : content's length
# length  : data
# Reset sequence (0x7f92, 1) are put in the generated file

# 23/11/2001 : updated to the new USB log format. (few modifications in fact)

# 07/02/2002 : added the CVS Id
# CVS $Id: eciadsl-uc.pl,v 1.10 2003/01/19 16:04:55 wwp Exp $
# Tag $Name:  $

# 2002/01/06 Benoit PAPILLAULT <benoit.papillault@free.fr>
#
#            Decoding of the USB log file is done case insensitive.

$active = 0;

open BIN, ">firmware.bin";

while (<>) {
	last if (/DriverUnload/);

	if (/URB ([0-9]+) going down/) {
		print "URB $1 :\n";
		@list = ();
	}

	if (/URB_FUNCTION_VENDOR_DEVICE/) {
		$active = 1;
	}

	if (/coming back/) {
		@list = ();
		$active = 0;
	}

	if ($active && /Request[ \t]+=[ \t]+([0-9a-f]+)/i) {
# always a0
		print "request=$1\n";
	}

	if ($active && /Index[ \t]+=[ \t]+([0-9a-f]+)/i) {
# always 0
		print "index=$1\n";
	}

	if ($active && /TransferBufferLength[ \t]+=[ \t]+([0-9a-f]+)/i) {
		print "length=$1\n";
		$length = $1;
	}

#	if ($active && /[0-9]+[ \t]+[0-9]+\.[0-9]+[ \t]+[0-9a-f]+: ([0-9a-f ]+)/) {
	if ($active && /[0-9a-f]+: ([0-9a-f ]+)/i) {
		foreach $b (split / /, $1) {
			print $b . " ";
			push @list, $b; 
		}
		print "\n";
	}
	
	if ($active && /Value[ \t]+=[ \t]+([0-9a-f]+)/i) {
		$value = hex "0x$1";
# memory address 
#		print "value=$1\n";

		$count = @list;

		print "$value, $length, $count: ";

		# addr in big endian.
		$val = chr ( $value >> 8 );
		print BIN $val;
		$val = chr ( $value & 0xff);
		print BIN $val;
		
		# length in big endian
		$val = chr ( $count >> 8);
		print BIN $val;
		$val = chr ( $count & 0xff);
		print BIN $val;
		
		foreach $byte (@list) {
#			print "0x$byte ";
			
			$val = chr (hex($byte));
			print BIN $val;
		}
#		print "\n";
		
		if (defined ($last_value) && defined ($last_count)) {
			if ($value > $last_value + $last_count) {
				print "firmware blocks are not contiguous : ";
				print $value - ($last_value + $last_count) ;
				print " byte(s) gap\n";
			}
		}
		
		$last_value = $value;
		$last_count = $count;
		
		if ($count > 16) {
			print "some bytes are skipped => wrong firmware generated\n";
		}
	}
}

close (BIN);
