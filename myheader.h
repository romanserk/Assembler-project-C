

#include <stdio.h>

#define STARTADDRESS 100
#define MAXLDATA 1000
#define NUMOFOPER 17
#define MAXROWLEN 81
#define MAXLABEL 31
#define MAXFILENAME 30
#define NUMOFGUIDE 5
#define MAXCMDNAME 10

#define MAX15NUM 16383
#define MIN15NUM -16384

#define MAX13NUM 4095
#define MIN13NUM -4096

#define FIRSTPASS 1
#define SECONDPASS 2

#define MINREG 0
#define MAXREG 8
#define NUMREGS 9

#define ZERO 0
#define ONE 1
#define TWO 2
#define THREE 3
#define BASE10 10

#define SKIP while( (*p==' ') || (*p=='\t') ) p++;


#ifndef MYHEADER_H
#define MYHEADER_H


/*A type of 2 bytes/16 bits*/
typedef unsigned int Dword;


typedef struct bitmanip
{
    const char *name;
    Dword word;
}Bitoper;


typedef struct oper
{
    const char *name;
    Dword word;
    void (*func)(void);
}Operation;


typedef struct int13
{
    int number:13;
}Num13bits;


typedef struct guiding
{
    const char *name;
    void (*func)(void);
}Guiding;



enum operand{emistake=0,eoperand=1,elabel=2,ereg=3,esoperand=4,eslabel=5,esreg=6};


#endif /* MYHEADER_H*/




