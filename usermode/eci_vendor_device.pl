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

$active = 0;

@urb_list = ();
$idx = -1;

$file = "eci_vendor.bin";

open BIN, ">$file" or die "Can't open $file: $!\n";

while (<>) {

	if (/\[([0-9]+) ms\]/) {
		$t = $1;
	}

	if (/going down/ || /coming back/) {
		undef ($request_type);
		undef ($request);
		undef ($value);
		undef ($index);
		undef ($size);

		$buf = '';
		$vendor = 0;
	}

	if (/URB ([0-9]+) going down/) {
		$active = 0;
		$urb = $1;

		$idx ++;
		$urb_list[$idx] = "$t URB $1 going down ";
	}

	if (/URB ([0-9]+) coming back/) {
		$active = 0;
		$urb = $1;

		$idx ++;
		$urb_list[$idx] = "$t URB $1 coming back";
	}

	if (/URB_FUNCTION_CONTROL_TRANSFER/) {
		$urb_list[$idx] .= ":ignored(transfer)";
		$active = 1;
	}

	if (/URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE/) {
		$urb_list[$idx] .= ":ignored(descriptor)";
	}

	if (/URB_FUNCTION_SELECT_CONFIGURATION/) {
		$urb_list[$idx] .= ":ignored(configuration)";
	}

	if (/URB_FUNCTION_RESET_PIPE/) {
		$urb_list[$idx] .= ":ignored(reset)";
	}

	if (/URB_FUNCTION_VENDOR_DEVICE/) {
		$urb_list[$idx] .= ":vendor";
		$active = 1;
		$vendor = 1;
	}

	if (/non printable/) {
		$urb_list[$idx] .= ":ignored(non printable)";
	}

#	if (/SetupPacket[ \t]+:([ 0-9a-f]+)/) {
#		$urb_list[$idx] .= "[Setup: $1]";
#	}

	if (/USBD_TRANSFER_DIRECTION_OUT/) {
		$direction = "out";
		if ($vendor) {
			$urb_list[$idx] .= " request_type=0x40";
			$request_type = 0x40;
		}
	}

	if (/USBD_TRANSFER_DIRECTION_IN/) {
		$direction = "in";
		if ($vendor) {
			$urb_list[$idx] .= " request_type=0xc0";
			$request_type = 0xc0;
		}
	}

	if (/SetupPacket/) {
		$active = 0;
	}

	if (/endpoint[ \t]+0x([0-9a-f]+)/) {
#		print "endpoint $1\n";
		
#		print "x : $urb_list[$idx]\n";
		$urb_list[$idx] .= ":endpoint $1";
		
		$active = 1;
	}

	if ($active && /Request[ \t]+=[ \t]+([0-9a-f]+)/) {
		if ($vendor) {
			$urb_list[$idx] .= " request=$1";
			$request = hex ($1);
		}
	}
	
	if ($active && /Value[ \t]+=[ \t]+([0-9a-f]+)/) {
		if ($vendor) {
			$urb_list[$idx] .= " value=$1";
			$value = hex($1);
		}
	}
		
	if ($active && /TransferBufferLength[ \t]+=[ \t]+([0-9a-f]+)/) {
		if ($vendor) {
			$size = hex ($1);
		}
	}

# since we save the buffer in the .bin file when the 'Index' keyword
# is present, data received are not saved. (buffer from IN URBs).

	if ($active && /Index[ \t]+=[ \t]+([0-9a-f]+)/) {
		if ($vendor) {
			$urb_list[$idx] .= " index=$1 size=$size";
			$index = hex($1);

			# everything should be OK!

			if (defined ($request_type)
				&& defined($request)
				&& defined($value)
				&& defined($index)
				&& defined($size)) {

#			if ((length($buf) eq $size)) {

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

			} else {
				print "Error ($urb): length=" . length($buf) . " size=$size\n";
			}
		}
	}

	if ($active && /[0-9a-f]+: ([0-9a-f ]+)/i) {
		@list = split / /, $1;

		foreach $byte (@list) {
			$buf .= chr(hex($byte));
		}
	}
}

close (BIN);

