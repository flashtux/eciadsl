#include <stdio.h>

/* TODO: detect other OSes: Mingw32, DOS, win16, other unices, Darwin
   other GNU/linux (*BSD, etc.), QNX, etc.
*/

#ifdef _WIN32 
#ifndef __GNUC__
/* MSVC/Borland */
#define OS "Win32 non-GNU"
#else
/* Cygwin */
#define OS "Win32+Cygwin"
#endif
#elif defined(__MACOS__)
/* MacOS */
#define OS "MacOS"
#elif defined(__MACOSX__)
/* MacOS/X*/
#define OS "MacOS/X"
#elif defined(__BEOS__)
/* BeOS */
#define OS "BeOS"
#elif defined(__EMX__)
/* OS/2 GCC */
#define OS "OS/2"
#elif defined(__QNX__)
/* QNX */
#define OS "QNX"
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define OS "BSD"
#else
#ifdef __GNUC__
#define OS "GNU/Linux"
#else
#define OS "undetermined"
#endif
#endif
#endif

int main(int argc, char** argv)
{
	short int word;

	word=0x00FF;
	fprintf(stdout, OS " (%s endian)", *(&word)?"little":"big");
	return(0);
}
