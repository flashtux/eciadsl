/*
gs7070.c
GS7070 ep_int response handeling

Changes:
Oliverthered: oliverthered:oliverthered.com

Corrected a few typos and added some aditional comments about the expected code sequences and there responses. This version now responds to the modem exactly the same as the Windows driver Annax A, G.DMT vci=0 vpi=38.

 
*/
#include "gs7070.h"
#include <stdlib.h>
#include <stdio.h>
/*
Manages commands sent from the modem
*/
int gs7070SetControl(GS7070int* gs, unsigned char* buffer){
	unsigned short controlcode=(buffer[0]<<8 )| buffer[1];
	/*Check for junk/unknown stuff, because it probbly means that the line has dropped*/
	if(controlcode==0x0C0C) return(0); 	
	if(controlcode==0x7311 || controlcode==0x5311){
		if(controlcode==0x7311 && gs->replace73->count==5){
			
			gs->replace73->count=0;
			if(gs->controlcodecount==1 && gs->controlcodes[1]==0x7341){
				gs->count7341++;
				if(gs->count7341==3){
					gs->replace73->replace[0]=0x73;
					gs->replace73->replace[1]=0x0F;
				}	
			}
		}else if(controlcode==0x5311 && gs->replace53->count==5){
			gs->replace53->count=0;
			if(gs->controlcodecount==1 && gs->controlcodes[1]==0x7341){
				if(gs->replace53->replace[0]==0xC3 && gs->replace53->replace[1]==0x39){
					gs->replace53->replace[0]=0x43;
					gs->replace53->replace[1]=0x01;
				}
			}
			
		}
		return(0);/*nothing to do*/
	}	
	
	/*check to see if we have a new control code to put into the buffer*/
	if(gs->controlcodecount==-1){
		gs->controlcodecount++;
		gs->controlcodes[gs->controlcodecount]=controlcode;
		gs->controlseqcount=1;
	}else{
		
		if(gs->controlcodes[gs->controlcodecount]!=controlcode){
		
			if(gs->controlcodecount>=CONTROLCODEBUFFERSIZE-1){
				printf("Too many control codes without a reset, expect a fatal error!\n");
				return(GSSYNCERROR);
			}
			
			if(gs->controlseqcount!=3){
				printf("Unexpected control code change %d\n",gs->controlseqcount);
			}
			/*Se if the we have recieved a reset command*/
			/*also a 0xF343?*/
			if(controlcode==0xF301){ /*F301 is some kind of reset command*/
				printf("Command change recieved\n");
				gs->controlcodecount=0;
/*				gs->controlseqcount=0;*/
			}else{
			/*No reset so add the new command onto the end of the control code list*/
				gs->controlcodecount++;
			}
			gs->controlcodes[gs->controlcodecount]=controlcode;
			gs->controlseqcount=1;
		}else{
			gs->controlseqcount++;
		}
	}
/*Big nasty state catcher comming up!!
One day, maybe sometime in the future this can be replaced with some propper cntrol code!!!
*/	
	if(gs->controlseqcount==1){
		/*
		 I think the first down the line is a f343? hmm... 
		 I get the odd F343 code that needs handeling
		 0xF343 looks like a 'opps I didn't expet that, start again code', I get lots of them if I don't generate the correct responses
		 I should imagine the correct response is 63 01 and 43 01
		setup the expected responses for the controlregs;	
		the list of known control codes are as follows
		
		also sometimes if the control sequence is mucked up I get a single 0xF301
		always 0xF301 (reset)
		followed by
		0xf3 13
		0xf3 4f --0x7311 should responed 0x63 53(possibly the first time) or 0xE3 0x51
		or
		0x73 4D  -- 0x7311 should responed 0x63 0B(possibley the first time) or 0xE3 0B 
		or
		0x73 41 --This is usually the first control seq I get after the modem has connrected to the ISP

		When the line drops the modem sends junk....
		
		And I should respond with ????? something to drop the connection?
		response to 0x73 41 is usually
			73 11 (63 01)
			53 11 (c3 39)
			73 11 (63 01)
			53 11 (43 01)
			73 11 (63 01)
			53 11 (43 01)
			73 11 (73 0f)	
		*/
		if(gs->controlcodecount==0){
			gs->replace53->replace[0]=0x43;
			gs->replace53->replace[1]=0x01;
			gs->replace53->count=0;
			gs->replace73->replace[0]=0x63;
			gs->replace73->replace[1]=0x01;
			gs->replace73->count=0;
		}else if(gs->controlcodecount==1){
			switch(gs->controlcodes[0]){
			case 0xf301:
				printf("0xF301\n");
				switch(gs->controlcodes[1]){
				case 0x734D:
					printf("0x734D\n");
					/*this is sometime 0x63 53 (only ever the first time for me and only sometimes)*/
					if(gs->count734D==0){
						gs->replace73->replace[0]=0x63;
						gs->replace73->replace[1]=0x0B;
						
						gs->count734D=1;
					}else{
						gs->replace73->replace[0]=0xE3;
						gs->replace73->replace[1]=0x0B;
					}
					gs->replace73->count=0;
				break;
				case 0x7341:
					printf("0x7341\n");
					gs->count7341=0;
					gs->replace73->replace[0]=0x63;
					gs->replace73->replace[1]=0x01;
					gs->replace73->count=0;
					gs->replace53->replace[0]=0xc3;
					gs->replace53->replace[1]=0x39;
					gs->replace53->count=0;
				break;
					case 0xF313:
					printf("0xF313\n");
					/*This isn't an end code and should be followed by F3 4f so don't report*/
				/* default: (ANSI C fix) */
					/*
					ahhhh I don't know what the modem is trying to tell me!
					*/
				};
			break;
			 default:
				printf("Unknown control code sequence count1 %04x\n",gs->controlcodes[1]);
			};
		}else if(gs->controlcodecount==2){/*need to add the rest of the codes in here!!*/
			switch(gs->controlcodes[2]){
			case 0xF34F:
			printf("0xF34F\n");
				gs->replace73->replace[0]=0xE3;
				gs->replace73->replace[1]=0x51;	
				gs->replace73->count=0;
			break;
			 default:
				printf("More control codes than expected count%d %04x \n",gs->controlcodecount,gs->controlcodes[gs->controlcodecount]);
			};
		}else {
			printf("Unknown control code sequence count0 %04x\n",gs->controlcodes[0]);
		}
	
	}else if(gs->controlseqcount==4){
		printf("Expected change of control code %04x\n",gs->controlcodes[3]);
	}
	return(0);
}


void replaceme(GSControlReg* rp,unsigned char* data){	
	rp->count++;
	if(rp->count <=2) return; /*don't need to replace anything the first two times around*/
	data[0]=rp->replace[0];
	data[1]=rp->replace[1];
}

		
		
/*
Returns the correct response to a control command sent by the modem,
 gs7070SetControl must be called first so that we are in the correct mode for the response.
*/
void gs7070GetResponse(GS7070int* gs,unsigned char* buffer){
	if((buffer[0] !=0x73 && buffer[0]!=0x53) || buffer[1] !=0x11){
		return; /*nothing to do*/
	}
	switch(buffer[0]){
	case 0x53:
		replaceme(gs->replace53,buffer);
	break;
	case 0x73:
		replaceme(gs->replace73,buffer);
	break;
	};
}
/*
 * Allocate a regster and setup some values
 * 
 * 
 */
GSControlReg* allocategsctlreg(char matchhi,char matchlow,char replacehi,char replacelow){
	GSControlReg* rp=(GSControlReg*) malloc(sizeof(GSControlReg));
	rp->count=0;
	rp->match[0]=matchhi;
	rp->match[1]=matchlow;
	rp->replace[0]=replacehi;
	rp->replace[1]=replacelow;
	return rp;	
}
/*
 * Clean up a register
 * 
 */
void deallocategsctlreg(GSControlReg* gscr){
	free(gscr);
}
/*
 *
 *Allocate the gs7070 interrupt handler and do some initilisation
 *
 */
GS7070int* allocateGS7070int(void){
	GS7070int* gs = (GS7070int*)malloc(sizeof(GS7070int)); /*which way round should a malloc be?*/
	gs->controlcodecount=-1;
	gs->replace73=allocategsctlreg(0x73,0x11,0x63,0x01);
	gs->replace53=allocategsctlreg(0x53,0x11,0x43,0x01);
	gs->controlseqcount=0;
	gs->count7341=0;
	gs->count734D=0;
	return gs;
	
}
/*
 *
 *Deallocate the gs7070 interrupt handler and clean up
 *
 */
void deallocateGS7070int(GS7070int* gs){
	deallocategsctlreg(gs->replace73);
	gs->replace73=0;
	deallocategsctlreg(gs->replace53);
	gs->replace53=0;
	gs->controlcodecount=-1;
	free(gs);
}


