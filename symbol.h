

/*will be included in files that use Symbol type & functions*/


#ifndef SYMBOL_H
#define SYMBOL_H

/*Symbol type for Symbols/Labels*/
/*A STRUCTURE FOR THE SYMBOLS LINKED LIST*/
typedef struct snode
{
    char label[MAXLABEL];

    Dword adess;

    struct
    {
        unsigned int ext:1;
        unsigned int oper:1;
    }feature;

    struct snode *next;
}Symbol;

extern Symbol *findlabel();
extern Symbol *phead,*plast;
extern void createsymbol(Dword address, char *name, Dword ext, Dword ope);
extern void csymbl(Dword,char *,Dword,Dword);
extern void addsymbol();
extern Dword rndsymboladd();
extern void printlist(void);


#endif /* SYMBOL_H*/

