/*
GS7070.h
Header file for gs7070 int response handler

*/
#define CONTROLCODEBUFFERSIZE 20 /*Piucked at random, I never get more that three controlcodes in a row so 20, if there are 20 then somethings gone badly wrong!*/
/*
* Error Codes
*
*/
#define GSLINEDROP -1 /*line dropped (e.g. you unpluged the modem from the phone line)*/
#define GSSYNCERROR -2 /*can happed after a lkine drop?*/

typedef struct  /*GSCONTROLREG*/ { /* nood to think of a better name for this, it's really the GS7070 int control maybe GSControlReg*/
	int count;
	unsigned char match[2];
	unsigned char replace[2];
} GSControlReg;/* GSControlReg;*/
/*
Structure to store the information about the current state of the the S7070
*/
typedef struct /*GS7070INT*/{
	
	/*Current control codes.
	 this should -> registers but I don't know hat they are at the moment
	 */
	unsigned short controlcodes[CONTROLCODEBUFFERSIZE];

	int controlcodecount;/*how many cuntrol codes on in the buffer*/	
	int controlseqcount;	
	GSControlReg* replace73;/* what sequence possition are we at (for 73's) */
	GSControlReg* replace53; /* what sequence possition are we at (for 53's) */
	/*Just incase we need information about the current connection*/
	int count734D;
	int count7341;
	int sequencecount;/*not used, I have seperate counters for different sequences at the moment!*/
	short vid;/*Modem vendor/product codes not in use at the moment*/
	short pid;
	short vci;
	short vpi;	
	short annax;
	
} GS7070int ;/* GS7070int;*/

int gs7070SetControl(GS7070int* gs, unsigned char* buffer);

void gs7070GetResponse(GS7070int* gs,unsigned char* buffer);

GS7070int* allocateGS7070int(void);
GS7070int* allocateGS7070int(void);


