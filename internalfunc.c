
#include "myheader.h"
#include "opcodes.h"
#include "global.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>



#define ONESTAR 1
#define TWOSTARS 2
#define THREESTARS 3
#define MINSTARS 1
#define MAXSTARS 3

/*-----------------------------------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------------------------------*/

/*THIS file contain the functions: */
/*comma(), nline(), getnum(), stars(), regcomp(), llabel(), getlabel(), operandtype(), datafeed().*/


/*-----------------------------------------------------------------------------------------------------*/

int comma(void)
{
    SKIP

    if(*p==',')
    {
        p++;
        return 1;
    }

    return 0;
}

int nline(void)
{
    while(*p==' ' || *p=='\t')
        p++;

    if(*p=='\n')
        return 1;

    return 0;
}


/*This function get the pointer p , and return the next number in it.*/
/*If illegal number: return 0 & set numberflag to 1.*/
Dword getnum(void)
{
    Num13bits tnum;
    const char *tmp=p;
    int num=0;
    tnum.number=0;

    if( (isdigit(*p)) || (*p=='-') )
    {

        num = strtol(tmp,&p,BASE10);


        if( (num>MIN13NUM) &&  (num<MAX13NUM)  )/*If sum can be store in 13 bits,because getnum(), is for operation fonctions.*/
        {
            tnum.number=num;

            if( (num==ZERO) && (tmp==p) )/*illegal number: not a number at all. !*/
            {
                numberflag=ONE;
                warningfpass=ONE;
                printf("****Warning: illegal number(pointer didnt move):\n");
                /*printf("****Number is:%d, and TNumber is:%d\n",num,tnum.number);*/
            }
            else/*legal number.*/
            {
                return ((Dword)tnum.number) ;
            }
        }
        else/*illegal number: too big or too small.*/
        {
            numberflag=ONE;
            warningfpass=ONE;
            printf("****Warning: number is illegal.\n");
            printf("****Number is:%d , and TNumber is:%d \n",num,tnum.number);
            printf("MAX13: %d ,MIN13: %d\n",MAX13NUM,MIN13NUM);
        }
    }

    return 0;
}


/*This function counts the number of stars & return it.*/
/*return the number of stars.*/
/*put p at the next char*/
Dword stars(void)
{
    Dword count=0;

    while( (*p=='*') )
    {
        count++;
        p++;
    }

    return count;
}



/*This function get the pointer p on char 'r' only. it passes the first 'r' & all the digits after it*/
/*It stop when the current char at p is not a digit, save it , put instead '\0' and compare the string first to*/
/*before return,it putting the last char in place.*/
/*all known registers names.*/
/*return the number of the register index in regs[] or -1 on failure=not such register.*/
Dword regcomp(void)
{
    const char *first=p;
    char last;
    Dword index=0;

    if(*p=='r')
    {
        p++;

        while(isdigit(*p))
            p++;

        last=*p;
        *p='\0';

        while( strcmp( (regs[index].name) , (regs[NUMREGS-1].name) ) )/*(regs[index].name!=NULL)*/
        {
            if( !(strcmp( first , regs[index].name )) )
            {
                *p=last;
                return index;
            }
            index++;
        }


    }

    *p=last;
    return ((Dword)(-1));
}

/*This function checks if the label is legal,if so: store it in label array and return 1.*/
/*This function need to called with p on the first char that belongs to the label*/
/*this function check if the first char is alpha, if so it stores all the alpha & numbers after it in label[].*/
/*stop when *p isnt alpha or a number*/
/*return: */
/*return 1: if index is less then 30 chars. also put '\0' in the end of label[].*/
/*return 0: If the label isnt legal.*/
/*THIS FUNCTION DOESNT CHECK IF LINE IS LEGAL !*/
/*p.s: for the first label return 0.*/
Dword llabel(void)
{
    int index=0;

    if(nline())
        return 0;

    if(isalpha(*p))
    {
        while(isalnum(*p))
        {
            label[index++]=*p;
            p++;
        }

        if(index<MAXLABEL)
        {
            label[index]='\0';
            return 1;
        }
    }

    return 0;
}

/*getlabel check if the global line there is ':',if so: it checks if ':' in the first 30 bytes\char's.*/
/*If there a legal label - labelflag will set to one.*/
/*Return (-1),If illegal label.*/
/*Return 0,If ':' doesnt exits.*/
/*Return 0,If ':' exits, but not in the first 30.*/
/*Return 1,If ':' exits in the first 30.*/
/*p will be on the ':' & put there ' '*/
Dword getlabel(void)
{
    char *temp=line;
    p=line;

    if( (temp = strpbrk(line,":"))  )/*find character ':' , and return a pointer to it. NULL otherwise.*/
    {
        if((temp = memchr(line, ':' , (MAXLABEL-1)) ) )/*check if ':' is in the first 30 char's , return pointer to the first ':'. NULL otherwise.*/
        {
            /*p=temp;*/
            llabel();/*checks if the label is legal */
                            /*& store it in label[], if it less than 30 chars & allowed chars*/
            if( *p==':' )/*If legal label: copy the 30 first chars to label[] & return 1 . 0 otherwise. */
            {
                *p =' ';
            }
            else/*illegal label in the first 30 chars!.*/
            {
                warningfpass=ONE;
                labelflag=0;
                return (-1);
            }
        }
        else/*there is ':', but not in the first 30 char's!. still no label*/
        {
            labelflag=0;
            p=line;
            return 0;
        }
    }
    else/*didnt find ':' in the string = no label. p will point to the start of line*/
    {
        p=line;
        labelflag=0;
        return 0;
    }


    labelflag=1;
    return 1;
}



/*----operandtype (enum) function----------*/

/*This function called when p (global pointer) is on the first char of the operand in line[].*/
/*It find out which operand is in front of it & return its type by enum array:operand  */
/*CASES: */
/*If the operand is a number(start with #) than getnum() function is called and return its value to Dword named: globalnumber*/
/*getnum():*/
/* if getnum() is called & a legal number is next. getnum() will return the number & set glblwar to 0*/
/*If illegal number getnum() will return 0 & set glblwar to 1. */
/*If glblwar will be set to 1 operandtype() will return mistake(==0)*/
/**/
/*If the operand is stars then operandtype call stars() function */
/*stars():*/
/*count the number of stars & return it.*/
/*If the number of stars is legit than operandtype() will return the proper index in operand enum.*/
/*For illegal number of stars operandtype() will return mistake(==0).*/
/**/
/**/
/*If the operand is a label operandtype() will call llabel()*/
/*llabel():*/
/*return 1: if the operand is a legal label & store it in label[]. legal label is less then 30 chars, start with alpha & contain*/
/*only alpha or digits.*/
/*return 0: If the label isnt legal.*/



/*This function return the operand type as described in enum operand.*/
/*If the operand is a number, operandtype() store it in Dword globalnumber*/
/*If the operand is a label,*/
/*return 0, if the operand is not known or illegal.*/
Dword operandtype(void)
{
    Dword res;
    char *tp=NULL;

    if(*p=='#')
    {
        /*puts("operandtype(): a number.");*/
        p++;
        numberflag=0;
        global13bits.number=getnum();

        if( (numberflag) )/*numberflag is 1, if illegal number!.*/
        {
            printf("****OPERANDTYPE() function: warningfpass.\n");
            warningfpass=ONE;
            res=emistake;
        }
        else/*numberflag==0, meaning legal number!.*/
        {
            res=eoperand;
        }

    }
    else if(*p=='*')
    {
        /*puts("operandtype(): a star.\n");*/
        res=stars();

        if( (res<=MAXSTARS) && (res>=MINSTARS) )/*legal number of stars.*/
        {
            if(res==ONESTAR)
            {
                res=esreg;
            }
            else if(res==TWOSTARS)
            {
                res=esoperand;
            }
            else if(res==THREESTARS)
            {
                res=eslabel;
            }
        }
        else/*illegal number of stars.*/
        {
            /*printf("****OPERANDTYPE() function: warningfpass.\n");*/
            warningfpass=ONE;
            res=emistake;
        }
    }
    else if(isalpha(*p))
    {
        /*puts("operandtype(): a alpha.\n");*/
        if(*p=='r')
        {
            tp=p;
            res = regcomp();

            if( (res==0) || (res>0)  )
            {
                res=ereg;
                return res;
            }
            else
            {
                p=tp;

                if(llabel())
                {
                    res=elabel;
                }
                else
                {
                    /*printf("****OPERANDTYPE() function: warningfpass.\n");*/
                    warningfpass=ONE;
                    res=emistake;
                }
            }
        }
        else/*label*/
        {
            if(llabel())
            {
                res=elabel;
            }
            else
            {
                /*printf("****OPERANDTYPE() function: warningfpass.\n");*/
                warningfpass=ONE;
                res=emistake;
            }
        }
    }
    else
    {
        warningfpass=ONE;
        res=emistake;
    }

    return res;
}/*End of operandtype().*/


/*This function concatenate the ending of the code array (codearr) with the data from the data array(dataarr).*/
/*other words: put all of the data array at the ending of the code array.*/
void datafeed(int counter)
{
    int codeindex=IS-(IC-SIC);
    int dataindex=0;

    while(counter--)
    {
        /*printf("DATAFEED: codeindex:%d,IS:%d , IC:%d, SIC:%d, counter:%d.\n",codeindex,IS,IC,SIC,counter);*/

        *(codearr+(codeindex))=dataarr[dataindex];

        /*printf("codearr value:%d,dataarr value:%d\n",*(codearr+(codeindex)),dataarr[dataindex]);*/

        codeindex++;
        dataindex++;
    }
}/*End of datafeed.*/

