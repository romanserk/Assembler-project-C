


#ifndef GLOBAL_H
#define GLOBAL_H

extern char fname[];
extern char entf[];
extern char extf[];

/*global:*/
/*IC-counter for the code area.(first pass). */
/*SIC, second IC. used in second pass, for find out how many lines the code area need.*/
/*IS-defined at the end of first pass.(length of lines required).calculated by (IC-SIC) .*/
/*DC-counter for the data area.*/
extern int IC,IS;
extern int SIC;
extern int DC;

/*line, A global array that stores the current line been process.*/
/*flinec, A counter that counts the number of lines in a file. shows current line number.*/
/*troubleline, save the line number that cause the last error.*/
extern char line[] ;
extern Dword flinec;
extern Dword troubleline;

/*label,global array, that stores the last legal label.*/
/*labelflag,global, sign that label[], stores a legal label.*/
extern char label[] ;
extern Dword labelflag;

extern Num13bits global13bits;
extern Dword globalnumber;
extern char numberflag;

extern Dword foperand,soperand;

extern Dword *codearr;
extern Dword dataarr[];

extern Dword warningfpass;
extern Dword pass;
extern char *p;


/*internal functions:*/
extern int comma();
extern int nline();
extern Dword getnum();
extern Dword stars();
extern Dword regcomp();
extern Dword llabel();
extern Dword getlabel();
extern Dword operandtype();
extern Dword getcmd();





#endif /* GLOBAL_H */































/*A type of 2 bytes/16 bits*//*
typedef unsigned int Dword;



typedef struct bitmanip
{
    char *name;
    Dword word;
}Bitoper;


typedef struct oper
{
    char *name;
    Dword word;
    void (*func)(void);
}Operation;



typedef struct int13
{
    unsigned int data:13;
}Ioperand;



typedef struct guiding
{
    char *name;
    void (*func)(void);
}Guiding;



extern int IC;
extern int SIC;
extern int DC;

extern Operation operarr[];
extern Bitoper destoperarr[];
extern Bitoper souoperarr[];
extern Bitoper grparr[];
extern Bitoper rndarr[];
extern Bitoper AREarr[];
extern Bitoper regs[];
extern Guiding guide[];
*/
/*
Dword firstpass(FILE *stream);
Dword secondpass(FILE *stream);
*/
/*
extern char line[] ;
extern Dword flinec;

extern char label[] ;
extern Dword labelflag;

extern Dword globalnumber;
extern char numberflag;


extern char fname[];
extern char extf[];
extern char entf[];

extern Dword foperand,soperand;

extern Dword *codearr;
extern Dword dataarr[];


extern Dword pass;
extern char *p;


*/


