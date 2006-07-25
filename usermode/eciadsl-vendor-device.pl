#!/usr/bin/perl -w

# Auteur : Benoit PAPILLAULT <benoit.papillault@free.fr>
# Creation :23/11/2001
# Licence : GPL

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
# CVS $Id: eciadsl-vendor-device.pl,v 1.1 2004/04/23 01:35:37 flashcode Exp $
# Tag $Name:  $

# 14/01/2004 : kolja <gava@bergamoblog.it> - enachement for GS7470 chipset support

#initialize input param value - kolja
$epintval = 0x86;
$synchnum = 999;
$fext='';
$surb=-1;
$eurb=-1;
$forceonefile=0;
$foundint64len=0;
$dheader ='';
$ok=0;
$chipset='GS7070';
$file_prefix='';

#help funciton
sub printhelp(){
	print "\nusage : eciadsl-vendor-device.pl input_sniff_file.log [parameters]\n\n";
	print "  parameters : \n";
	print "               -h : this help\n";	
	print "               -chipset=  : Globspan Chipset (optional)\n";
	print "                            this parameter set the chipset has your\n";
	print "                            modem (could be GS7070 or GS7470\n";
	print "                                   default GS7070)\n";
	print "               -synchnum= : synch file output number (optional)\n";
	print "                            create a bin file called synch##.bin or gs7470_synch##.bin \n";
	print "                            (depending on chipset param specified)\n";
	print "                            where ## is the number passed\n";
	print "                            (default file name : synch999.bin or gs7470_synch999.bin).\n";
	exit;	
}

# get parameters - kolja
$fin = $ARGV[0];
foreach $param (@ARGV)
{
	@param = split( '=', $param );
	if($param[0] eq '-chipset'){
		$chipset  = $param[1];
	}
	if($param[0] eq '-synchnum'){
		$synchnum = $param[1];
	}
	if($param[0] eq '-h'){
		printhelp();
	}
	if($param[0] eq '-forceonefile'){
		$forceonefile= 1;
	}
	if($param[0] eq '-fext'){
		$fext= $param[1];
	}
	if($param[0] eq '-surb'){
		$surb= 0 + $param[1];
	}
	if($param[0] eq '-eurb'){
		$eurb= 0 + $param[1];
	}
	if($param[0] eq '-dheader'){
		$dheader= $param[1];
	}
}
if ($surb!=-1){
	$forceonefile= 1;
}

if ($chipset eq 'GS7470'){
	$epintval = 0x81;
	$file_prefix = "gs7470_";
}

$file = $file_prefix."synch".$synchnum.$fext.".bin";
$active = 0;

open BIN, ">$file" or die "Can't open $file: $!\n";
# set file out in bin mode - kolja
binmode(BIN);

if ($dheader ne ''){
	print BIN $dheader;
}
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
		$buf1='';
		$foundint64len=0;

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
		if (hex($1) != $epintval) {
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
				$foundint64len=1;
			}
		}
				
		if ($active) {
			if ($vendor) {
				$size = hex ($1);
			}
		}
		next;
	}
	
	if (($active || $interrupt) && /[0-9a-f]+: ([0-9a-f ]+)/i) {
		@list = split / /, $1;
		foreach $byte (@list) {
			$buf .= chr(hex($byte));
			$buf1 .="$byte";
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
		
	#store ep buffer to verify repeated ep int response 
	if ($interrupt && $buf ne '' && /UsbSnoop - My/i){
		#verify if epint with lenght 64 is reached and if it's not zero filled end process otherwise continue - kolja 23/11/2004
		if($foundint64len) {
			if($buf1 eq "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000") {
				next
			} else {
				$ok=1;
				last;
			}
		}
		$epcount=0;
		$epfound=0;
		foreach $epstoreddata (@epstoreddata){
			if ($epstoreddata eq $buf){
				$epstoredcount[$epcount]++;
				$epstoredurbs[$epcount][$epstoredcount[$epcount] -1]=$prevurb;
				$epfound=1;
				last;
			}
			$epcount++;
		}
		if (!$epfound){
			$epstoreddata[$epcount]=$buf;
			$epstoredcount[$epcount]=1;
			$epstoredurbs[$epcount][0]=$prevurb;
		}
	}

		
# since we save the buffer in the .bin file when the 'Index' keyword
# is present, data received is not saved. (buffer from IN URBs).

	if ($active && /Index[ \t]+=[ \t]+([0-9a-f]+)/i) {
		if ($vendor) {
			$index = hex($1);

			#skip urb if is not needed - kolja
			if ($surb!=-1 && ($surb > $urb || $eurb < $urb)){
				next;
			}		

			# we check everything!
			$prevurb = $urb;
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
			
			#print "saving urb $urb\n";
			
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

#examin ep Interrupt response stored - kolja
#get max
$epcount=0;
$epMaxPos=-1;
for($epcount=0;$epcount<@epstoreddata;$epcount++){
	if ($epstoredcount[$epcount]>2 && $epstoredcount[$epcount]>$epstoredcount[$epMaxPos]){
		$epMaxPos=$epcount;
	}
#	print "data:".$epstoreddata[$epcount]."|num:".$epstoredcount[$epcount]."|urbs:";
#	foreach $urb (@{$epstoredurbs[$epcount]}){
#		print $urb."-";
#	}
#	print "\n";
}
#get second max
$epcount=0;
$epMax2ndPos=-1;
for($epcount=0;$epcount<@epstoreddata;$epcount++){
	if ($epcount!=$epMaxPos && $epstoredcount[$epcount]>1 && $epstoredcount[$epcount]>$epstoredcount[$epMax2ndPos]){
		$epMax2ndPos=$epcount;
	}
}
# if epMaxPos and epMax2ndPos was found verify if 2nd have count 1 less then 1st
# if yes 3 synch files must be generate 
# 1st : synch##a.bin : initialisation sequence (used in eci-load2_init function)
#       it will contain from 1 to first epMaxPos urbs saved excluded
# 2nd : synch##b.bin : Step 2 sequence (used in eci-load2_init function) [cycling one]
#       it will contain from first epMaxPos urbs saved included to third epMaxPos urbs saved excluded
#		(this file will contain also in first to byte exit control codes retreived from next epstoreddata byte 4 and 5
# 3rd : synch##.bin : finalize sequence (used in eci-load2 common process)
#       it will contain from next epMax2ndPos epstoreddata saved first urbs saved excluded to shync pahse end
# All these 3 files must be used.

if ($epMaxPos!=-1 && $epMax2ndPos!=-1 && $epstoredcount[$epMax2ndPos]+1==$epstoredcount[$epMaxPos] && $forceonefile!=1){
 	close (BIN);
	print "ATTENTION : Your log seams to have a cycling sequence\n";
	print "            this script will generate 3 files \n";
	print "            (".$file_prefix."synch".$synchnum."a.bin ".$file_prefix."synch".$synchnum."b.bin and ".$file_prefix."synch".$synchnum.".bin\n";
	print "            YOU MUST USE ALL OF THEM!\n";
	print "            (if you wana generate 1 file anyway, execute again \n";
	print "             this script with -forceonefile param).\n\n";
	# first file
	print " * Create first file (".$file_prefix."synch".$synchnum."a.bin) .. please wait..\n";
	$surb = 0;
	$eurb = $epstoredurbs[$epMaxPos][0]-1;
	@args = ("perl", $0, $fin, "-synchnum=".$synchnum, "-chipset=$chipset", "-fext=a", "-surb=$surb", "-eurb=$eurb");
	system (@args);
	# second file
	print " * Create second file (".$file_prefix."synch".$synchnum."b.bin) .. please wait..\n";
	$surb = $epstoredurbs[$epMaxPos][0]+0;
	$eurb = $epstoredurbs[$epMaxPos][2]-1;
	$dheader = substr($epstoreddata[$epMax2ndPos+1], 4, 2);
	@args = ("perl", $0, $fin, "-synchnum=".$synchnum, "-chipset=$chipset", "-fext=b", "-surb=$surb", "-eurb=$eurb" , "-dheader=$dheader");
	system (@args);
	# third file
	print " * Create third file (".$file_prefix."synch".$synchnum.".bin) .. please wait..\n";
	$surb = $epstoredurbs[$epMax2ndPos+1][0]+1;
	$eurb = 99999999;
	$dheader = substr($epstoreddata[$epMax2ndPos+1], 4, 2);
	@args = ("perl", $0, $fin, "-synchnum=".$synchnum, "-chipset=$chipset", "-surb=$surb", "-eurb=$eurb");
	system (@args);	
	print " Process ended successfully\n";
}else{
 if ($ok){
 	print "Synch .bin '$file' generated, OK.\n" ;
 }else{
 	print "WARNING: interrupt packet not detected! File '$file' was generated anyway but is probably incorrect.\n";
 }
 close (BIN);
}
