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
  #define OS "Win32+Cygwin/MingW"
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
 #if defined(__FreeBSD__)
  #define OS "FreeBSD"
 #elif defined(__NetBSD__)
  #define OS "NetBSD"
 #elif defined(__OpenBSD__)
  #define OS "OpenBSD"
 #else
  #ifdef __GNUC__
   #define OS "GNU/Linux"
  #else
   #define OS "undetermined"
  #endif
 #endif
#endif

int main()
{
    const int i = 1;
    const char * ptr = (const char *) &i;

	fprintf(stdout, OS " (%s endian), c:%d si:%d i:%d li:%d",
            (*ptr)?"little":"big",
            sizeof(char), sizeof(short int), sizeof(int), sizeof(long int));

    return 0;
}
