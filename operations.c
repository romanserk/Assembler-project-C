

#include "myheader.h"
#include "symbol.h"
#include "global.h"
#include "opcodes.h"
#include "operations.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


#define MAX32WORD 4

/*-----------------------------------------------------------------------------------------------------*/


extern void printcode(Dword num);
extern void mas(Dword oppara);
extern void manync(Dword oppara);
extern void rtstop(Dword oppara);



/*-----------------------------------------------------------------------------------------------------*/

/*THIS file contain the functions: */
/*.*/

/*THIS file contain the global arrays: */
/*regs[], guide[], operarr[], AREarr[], destoperarr[], souoperarr[], grparr[], rndarr[]. */


/*-----------------------------------------------------------------------------------------------------*/



/*An array that hold the names of the registers & their numbers*//*----NOT DONE!------*/
Bitoper regs[NUMREGS] =  { {"r0",0} , {"r1",1} , {"r2",2} , {"r3",3} , {"r4",4} , {"r5",5} , {"r6",6} , {"r7",7} , {"NULL",0} };

Guiding guide[NUMOFGUIDE]= { {".data",data} , {".string",mstring} , {".entry",entry} , {".extern",extrn} , {"NULL",NULL} };

/*NUMOFOPER=17*/
/*An array that hold all the operations & its opcode in the correct place in Dword*/
Operation operarr[NUMOFOPER] = {
{"mov",(0),mov} , {"cmp",(0|(1<<6)),cmp} , {"add",(0|(2<<6)),add} , {"sub",(0|(3<<6)),sub} , {"not",(0|(4<<6)),mnot} , {"clr",(0|(5<<6)),clr} ,
{"lea",(0|(6<<6)),lea} , {"inc",(0|(7<<6)),inc} , {"dec",(0|(8<<6)),dec} , {"jmp",(0|(9<<6)),jmp} , {"bne",(0|(10<<6)),bne} ,
{"red",(0|(11<<6)),red} , {"prn",(0|(12<<6)),prn} , {"jsr",(0|(13<<6)),jsr} , {"rts",(0|(14<<6)),rts} , {"stop",(0|(15<<6)),stop} , {"NULL",0,NULL}  };

/*An array that hold the different possibles for ARE bits and its Dword. */
Bitoper AREarr[] = { {"absolute",(0)} , {"external",(0|(1))} , {"relocatable",(0|(2))} , {"endofAREarr",0}  };

/*An array that holds the possibles for dest operand & Dword.*/
Bitoper destoperarr[] = { {"operand",(0)} , {"label",(0|(1<<2))} , {"random",(0|(2<<2))} ,{"reg",(0|(3<<2))} , {"endofdestoper",0} };
/*operand,label,random,register*/

/*An array that holds the possibilities for source operand & matching Dword.*/
Bitoper souoperarr[] = { {"operand",(0)} ,{"label",(0|(1<<4))} ,{"random",(0|(2<<4))} ,{"reg",(0|(3<<4))} ,{"endofsoper",0} };


Bitoper grparr[] = { {"nooperand",(0)} ,{"singleoperand",(0|(1<<10))} ,{"twooperand",(0|(2<<10))} , {"endofgrparr",0} };

Bitoper rndarr[] = { {"*",(0|(1<<12))} , {"**",(0|(2<<12))} , {"***",(0|(3<<12))} , {"endofrndarr",0}  };




/*global array(for conversions to base 32).*/
char translate32[MAX32WORD];


/*return a random number from min to max. */
Dword rnum(Dword min,Dword max)
{
    rand();

    return ( ( (rand())%(max-min) ) - min );
}


/*This function, get as a parameter a singed number & return its unsigned number identical, bits wise.*/
/*stou, signed to unsigned.*/
Dword stou(Dword num)
{

    Dword temp=0;
    Dword mask=0x7FFF;

    temp=abs(num);

    temp=~temp;

    temp=(Dword)( (temp) & (mask) );

    temp++;

    return ((Dword)temp);
}/*End of stou.*/

/*This function, get as a parameter a decimal number and store in translate32[], its representation in 32 base.*/
/*return values can be used for validation purposes.*/
char base32(int num)
{
    static int index=0;
    int zero=0;

    char base32arr[]="0123456789ABCDEFGHIJKLMNOPQRSTUV";

    if( num < zero )
    {
        num=(unsigned int)stou(num);
        base32(num);
        return 0;
    }




    if(num==0)
    {
        index=0;
        translate32[index]=base32arr[num];
        translate32[index+1]='\0';
        return 0;
    }

    base32(num/32);

    num=num%32;
    translate32[index++]=base32arr[num];
    translate32[index]='\0';

    return base32arr[num];
}

/*This function, used by operation function. It gets a parameters: indicator, label[],external & operation.  */
/*indicator, is the address. label[] is the name (stores in label global array). external & operation is a characteristic of the new wanted symbol.*/
/*It done so only if  the pass is the first pass,*/
/*And labelflag is up(set to 1), indicates a valid label in label[] global array*/
/**/
/*If the Symbol exits(name wise) the function set warningfpass to 1, and troubleline to the current file line number.*/
/*In case of warning. information will be sent to stderr.*/
void funclabelcreate(Dword indicator,char label[],Dword external, Dword operation)
{
    Symbol *lbl=NULL;
    if( (labelflag) && (pass==FIRSTPASS)  )
    {
        if( (lbl=findlabel()) )/*enter if find the same label in the Symbols linked list.*/
        {
            /*lbl==null - if didnt find it thus the control go to the "else" block.*/
            /*lbl!=null if the same label is exists all ready . in this case lbl will point to the first label.*/

            /*print an error statement & set label flag to zero.*/
            fprintf(stderr,"error: in file:%s, line:%d, label:%s, all ", fname, flinec,lbl->label);
            fprintf(stderr,"ready exits in line:%d in the code area.\n",lbl->adess);
            labelflag=ZERO;
            warningfpass=ONE;
            troubleline=flinec;

        }
        else
        {
            createsymbol(indicator,label,external,operation);
            labelflag=ZERO;
        }
        lbl=NULL;
    }

}


/*This function called if in the current line(stores in global array line[]) ".data" appears. */
/*It stores the data in dataarr[]. DC uses as index. */
/*If the data is not-valid the function set warningfpass to 1, and troubleline to the current file line number.*/
/*In case of warning. information will be sent to stdin & stderr.*/
void data(void)
{
    const char *tmp = p;
    Dword num;
    int res;

    if( (pass==SECONDPASS) || (nline()) )/*If secondpass or *p=='\n', this function is irrelevant.*/
    {
        return;
    }

    funclabelcreate(DC,label,ZERO,ZERO);

    if( isdigit(*p) || (*p=='-'))
    {
        res = strtol(tmp, &p, BASE10);

        if( (res<MAX15NUM) && (res>MIN15NUM) )/*if res is a legal number*/
        {
            if( res==ZERO && tmp==p)/*res==0 && tmp==p,meaning strtol return 0 & the pointer didnt move, meaning illegal input.*/
            {
                fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                fprintf(stderr,"there is an illegal number.\n");

                printf("****Data: warning.line: %d \n",flinec);/*bugs search.*/

                troubleline=flinec;
                warningfpass=ONE;
            }
            else
            {
                /*A legal number !.*/

                if(comma())
                {
                    num = res;
                    dataarr[DC++]=num;
                    data();
                }
                else if(nline())
                {
                    num=(Dword)res;
                    dataarr[DC++]=num;
                }
                else/*not a legal number !.*/
                {
                    fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                    fprintf(stderr,"there is an illegal number.\n");

                    printf("****Data: warning at line: %d \n",flinec);/*bugs search.*/

                    troubleline=flinec;
                    warningfpass=ONE;
                }

            }
        }


    }
    else if(*p==',')
    {
        p++;
        data();
    }
    else if(*p==' ' || *p=='\t')
    {
        SKIP
        data();
    }
    else/*In case of that is illegal char*/
    {
        fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
        fprintf(stderr,"there is an illegal number.\n");

        printf("****Data: warning.line: %d \n",flinec);/*bugs search.*/

        troubleline=flinec;
        warningfpass=ONE;
    }

}/*END OF DATA FUNCTION!*/


/*This function called if in the current line(stores in global array line[]) ".string" appears. */
/*It stores the data(characters) in dataarr[]. added with a 0 at the end. DC uses as index. */
/*If the data is not-valid the function set warningfpass to 1, and troubleline to the current file line number.*/
/*non- valid input will be: only one occurrence of '"' in the line or missing '\n' at the end.*/
/*In case of two double quotes one after another (like:"") the function will store on 0 (as at the end of a valid string).*/
/*In case that something went wrong the function set warningfpass to 1, and troubleline to the current file line number.*/
/*Information will be sent to stdin & stderr.*/
void mstring(void)
{
    const char *tmp=p;
    char *first=strchr(tmp,'\"');/*first occurrence of '"' in tmp.*/
    char *last=strrchr(tmp,'\"');/*last occurrence of '"' in tmp.*/

    if( (pass==SECONDPASS) || (nline()) )
    {
        return;
    }

    p=last;
    p++;

    funclabelcreate(DC,label,ZERO,ZERO);


    if(  first && last  && (first!=last) && (nline()) )/*all is good !*/
    {
        first++;

        for( ; (first!=last) ; first++ )
        {
            dataarr[DC++]=(*first);
        }

        dataarr[DC++]='\0';/* after string '\0'. */

    }
    else/*something went wrong. */
    {
        fflush(stderr);

        fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
        fprintf(stderr,"there is an illegal input.\n");

        printf("****String: warning.line: %d \n",flinec);/*bugs search.*/

        troubleline=flinec;
        warningfpass=ONE;

    }

}/*End of (m)string.*/


/*This function "work" only at the second pass.*/
/*This function write to a file ending with a .ent, all the Symbols that is internal with address indicated.*/
/*If the Symbol all-ready exitst the function set warningfpass to 1, and troubleline to the current file line number.*/
/*In case of warning. information will be sent to stdin & stderr.*/
void entry(void)
{
    /*work only in second pass!.*/
    /*p is on the label*/
    Symbol *lbl=NULL;
    FILE *fpentry=NULL;

    SKIP

    if( (pass==SECONDPASS) )
    {
        printf(".entry second pass.\n");
        if( (llabel()) && (nline()) )
        {
            lbl=findlabel();/*find the label in label[].*/
            if( lbl )/*find it*/
            {
                if( ((lbl->feature.ext)==0) )/*internal label.*/
                {
                    /*write in prog1.ent file & move to the next line.*/

                    fpentry=fopen(entf,"a+b");

                    if( (fpentry) )
                    {
                        base32(lbl->adess);
                        fprintf(fpentry,"%s %s\n",label,translate32);

                        fclose(fpentry);
                    }
                    else/*cant open the file.*/
                    {

                        fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                        fprintf(stderr,"cant open the file:%s to write.\n",entf);

                        printf("entry: warning.line: %d (cant access the file).\n",flinec);/*bugs search.*/
                        warningfpass=ONE;
                        troubleline=flinec;
                    }
                }
                else/*external label*/
                {
                    fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                    fprintf(stderr,"wrong type. internal label only.\n");

                    printf("Entry: warning.line: %d \n",flinec);/*bugs search.*/
                    warningfpass=ONE;
                    troubleline=flinec;
                }
            }
            else/*not exits label.*/
            {
                fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                fprintf(stderr,"the label is not found.\n");

                printf("Entry: warning.line: %d \n",flinec);/*bugs search.*/
                warningfpass=ONE;
                troubleline=flinec;
            }
        }
        else/*label name illegal.*/
        {
            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"there is an illegal input:");
            fprintf(stderr," label name is an illegal input.\n");

            printf("Entry: warning at line: %d \n",flinec);/*bugs search.*/
            warningfpass=ONE;
            troubleline=flinec;
        }

        /*end for second pass.*/
    }


}/*End of ".entry".*/



/*This function "work" only at the first pass.*/
/*This function been called only if ".extern" appears in the cuurent line.*/
/*This function add extern Symbol to the Symbols linked list.it checks if the Symbol exits. if so */
/*If the Symbol all-ready exits the function set warningfpass to 1, and troubleline to the current file line number.*/
/*In case of warning. information will be sent to stdin & stderr.*/
void extrn(void)
{
    Symbol *lbl=NULL;
    int index=0;
    labelflag=0;

    if( (pass==SECONDPASS) )
    {
        return;
    }

    /*first pass only!.*/

    SKIP

    if( isalpha(*p) )
    {
        do
        {
            if(p)
                label[index++]=*p;

            p++;
        }while( (isalnum(*p)) );


        if(nline())
        {
            label[index]='\0';

            lbl=findlabel();

            if( (lbl) )/*find a label store in label[], in linked list.*/
            {
                /*error!*/
                fflush(stderr);
                fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                fprintf(stderr,"label defined twice. first in line:%d.\n",lbl->adess);
                warningfpass=ONE;
                troubleline=flinec;

                printf("****Extern: warning at line: %d \n",flinec);/*bugs search.*/
            }
            else/*add the new symbol.*/
            {
                createsymbol(ZERO,label,ONE,ZERO);
            }
        }
        else/*illegal name.*/
        {
            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal label name.\n");
            warningfpass=ONE;
            troubleline=flinec;

            printf("****Extern: warning at line: %d \n",flinec);/*bugs search.*/
        }
    }
    else
    {
        fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
        fprintf(stderr,"illegal label name.\n");
        warningfpass=ONE;
        troubleline=flinec;

        printf("****Extern: warning at line: %d \n",flinec);/*bugs search.*/
    }

}/*extern end.*/


/*This function insert the value(bit wise) from a type of 13 bits to a type of 15 bits.*/
/*This function get the source and destination address as parameters.*/
void insertnum(Dword *toword,Num13bits *from)
{
    Num13bits tmp;
    Dword mask=1;
    tmp.number=from->number;
    *toword=0;

    while(( tmp.number ) != 0 )
    {
        if((tmp.number) & mask )
        {
            *toword=(*toword)|mask;
        }
        tmp.number=tmp.number&(~mask);
        mask<<=1;
    }

}


/*This function been called if "mov" operation is declared in the current line.*/
/*If in first pass it calling to mas() function. with 0 as a parameter.*/
/*If in the second pass it will call mas() function, with mov opcode as a parameter.*/
void mov(void)
{
    if(pass==FIRSTPASS)
    {
        mas(0);
    }
    else
    {
        mas(MOVOPCODE);
    }
}

/*This function been called if "add" operation is declared in the current line.*/
/*If in first pass it calling to mas() function. with 0 as a parameter.*/
/*If in the second pass it will call mas() function, with add opcode as a parameter.*/
void add(void)
{
    if(pass==FIRSTPASS)
    {
        mas(0);
    }
    else
    {
        mas(ADDOPCODE);
    }
}/*end of add*/

/*This function been called if "sub" operation is declared in the current line.*/
/*If in first pass it calling to mas() function. with 0 as a parameter.*/
/*If in thw second pass it will call mas() function, with sub opcode as a parameter.*/
void sub(void)
{
    if(pass==FIRSTPASS)
    {
        mas(0);
    }
    else
    {
        mas(SUBOPCODE);
    }
}



/*This function called if "cmp" operation is declared in the current line. In first pass & in second pass.*/
/*In first pass:*/
/*--Call to funclabelcreate at the beginning.*/
/*The function checks that the operands is valid & suitable for it.*/
/*If so, IC (global indicator to the code area) increased by the required times.*/
/*If not,meaning non-valid info. than:*/
/*The function set warningfpass to 1, and troubleline to the current file line number.*/
/*In case of warning. information will be sent to stdin & stderr.*/
/**/
/*In second pass:*/
/*The function doing what it done in the first pass(all the above!). And in addition: */
/*It insert the appropriate operands to the code area. (global array: codearr)*/
/*In case of warning. information will be sent to stderr only.*/
void cmp(void)
{
    int index=0;
    Dword command=0,suorce=0,dest=0;
    Num13bits tmpoperand;
    char *assp=p;
    Symbol *lbl=NULL;
    FILE *fpext=NULL;

    SKIP
    assp=p;

    funclabelcreate(IC,label,ZERO,ONE);/*do something only in first pass.*/

    if( (foperand=operandtype()) && comma() )/*The first operand is known & the pattern is good up to the comma. p after the comma. */
    {
        SKIP

        if( (soperand=operandtype()) && nline() )/*the pattern is good & second operand is a known one's */
        {
            if(  (soperand==eoperand) || (soperand==elabel) || (soperand==ereg) )/*the second operand is legal for THIS operation.*/
            {
                if( (foperand==esreg) && (soperand==ereg)  )/*if the first operand is a random reg & the second one is a register=two Dwords*/
                {
                    if(pass==FIRSTPASS)
                    {
                        IC=IC+2;
                    }
                }
                else if( (foperand==ereg) && (soperand==ereg) )
                {
                    if(pass==FIRSTPASS)
                    {
                        IC=IC+2;
                    }
                }
                else
                {
                    if(pass==FIRSTPASS)
                    {
                        IC=IC+3;
                    }
                }
            }
            else/*second operand illegal!*/
            {
                fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                fprintf(stderr,"illegal second operand.\n");
                warningfpass=ONE;
                troubleline=flinec;

                printf("****CMP: warning at line: %d \n",flinec);/*bugs search.*/
            }
        }
        else/*the pattern is not good or second operand is unknown*/
        {
            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal input.\n");
            warningfpass=ONE;
            troubleline=flinec;

            printf("****CMP: warning at line: %d \n",flinec);/*bugs search.*/
        }
    }
    else/*the pattern is illegal up to the first comma or first operand is unknown.*/
    {
        fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
        fprintf(stderr,"illegal first operand.\n");
        warningfpass=ONE;
        troubleline=flinec;

        printf("****CMP: warning at line: %d \n",flinec);/*bugs search.*/
    }



    if(pass==SECONDPASS)
    {
        p=assp;/*put p pointer at the start of the first operand.*/
        SKIP
        /*p point to the start of the first operand.*/

        command = ( (CMPOPCODE) | (grparr[2].word)  );/*two operands to cmp*/

        if( (foperand==eoperand) || (foperand==esoperand) )/*if the first operand is regular number or random number */
        {
            if( (foperand==esoperand) )/*rnd bits.*//*first operand is random number*/
            {
                command=command|(rndarr[1].word)|(souoperarr[2].word);

                tmpoperand.number=rnum(MIN13NUM,MAX13NUM);/*create a random number and store it.*/
                insertnum(&suorce,(&tmpoperand) );
                suorce<<=2;
            }
            else/*given number.*/
            {
                command=command|(souoperarr[0].word);

                operandtype();
                insertnum(&suorce,&global13bits);/*enter the number to suorce Dword*/
                suorce<<=2;
            }
        }
        else if ( (foperand==elabel) || (foperand==eslabel) )/*first operand is a label or random label*/
        {
            if( (foperand==eslabel) )/*rnd bits*//*first operand is random label*/
            {
                command=command|(rndarr[2].word)|(souoperarr[2].word);

                suorce=rndsymboladd();/*create a random number & choose a random label.*/
                suorce=( (suorce<<2)|(AREarr[2].word) );
            }
            else/*given label*/
            {
                command=command|(souoperarr[1].word);/*label is 01*/

                /*check if the label legit & store it in label[].*/
                llabel();
                lbl=findlabel();

                if(lbl)/*find the Symbol that match the operand label.*/
                {
                    if( (lbl->feature.ext) == 1)/*the label is external*/
                    {
                        suorce = ( (suorce) | (AREarr[1].word) );

                        if( (fpext=fopen(extf,"a+b")) )
                        {
                            base32(SIC+1);

                            fprintf(fpext,"%s %s\n",label,translate32);
                            fclose(fpext);
                        }
                        else/*failed to open file.*/
                        {
                            printf("cant open...\n");/*DEBUG POINT...............*/
                            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                            fprintf(stderr,"cant open the file:%s to write.\n",extf);
                        }
                    }
                    else/*the label is internal.*/
                    {
                        suorce = ( (lbl->adess) );
                        suorce = ( (suorce<<2) | (AREarr[2].word) );
                    }
                }
                else/*The operand label name isnt in the linked list of symbols.*/
                {
                    fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                    fprintf(stderr,"unable to find the wanted label.\n");
                }
            }

        }
        else if( (foperand==ereg) || (foperand==esreg) )/*first operand is a register or random register*/
        {
            if( (foperand==esreg) )/*rnd*//*first operand is a random register*/
            {
                command=command|(rndarr[0].word)|(souoperarr[2].word);

                suorce= rnum(0,MAXREG);/*choose a random register.*/
                suorce=suorce<<8;
            }
            else/*first operand is a given register.*/
            {
                command=command|(souoperarr[3].word);
                suorce=regcomp();
                suorce=suorce<<8;
            }
        }
        else
        {
            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal first operand.\n");
        }
        /*end handling first operand.*/


        /*Managing the second operand type */
        p=assp;/*put p pointer at the start of the first operand.*/
        SKIP
        operandtype(); comma();
        SKIP
        /*p is now point the second operand.*/

        if( (soperand==elabel) )
        {
            command=((command)|(destoperarr[1].word));

            if(llabel())
            {
                lbl=NULL;
                lbl=findlabel();

                if(lbl)/*enter if find the matching label.lbl is pointer to it.*/
                {
                    if( (lbl->feature.ext)==1 )/*If external.*/
                    {
                        dest = ( (dest)|(AREarr[1].word) );

                        if( (fpext=fopen(extf,"a+b")) )
                        {
                            base32(SIC+2);

                            fprintf(fpext,"%s %s\n",label,translate32);
                            fclose(fpext);
                        }
                        else/*failed to open file.*/
                        {
                            printf("cant open...\n");/*DEBUG POINT...............*/
                            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                            fprintf(stderr,"cant open the file:%s to write.\n",extf);
                        }
                    }
                    else
                    {
                        dest =( (lbl->adess) );
                        dest = ( (dest<<2) | (AREarr[2].word) );
                    }
                }
                else/*The operand label name isnt in the linked list of symbols.*/
                {
                    fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                    fprintf(stderr,"unable to find the wanted label.\n");
                }
            }
            else
            {
                fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                fprintf(stderr,"illegal label name.\n");
            }
        }
        else if( (soperand==ereg) )
        {
            command=((command)|(destoperarr[3].word));

            dest=regcomp();
            dest=dest<<2;

            if( (foperand==ereg) || (foperand==esreg) )
            {
                suorce =( (suorce)|(dest) );
            }
        }
        else if( (soperand==eoperand) )
        {
            command=((command)|(destoperarr[0].word));

            operandtype();
            insertnum(&dest,&global13bits);
            dest=dest<<2;
        }
        else
        {
            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal second operand.\n");
        }


        /*now enter the Dwords to dataarr[]*/

        /*printf("***CMP() ENTER:\ncommand:%d.suorce:%d.dest:%d\n\n",command,suorce,dest);*/

        index=IS-(IC-SIC);
        *(codearr+(index))=command;
        SIC++;

        index=IS-(IC-SIC);
        *(codearr+(index))=suorce;
        SIC++;


        if( (soperand==ereg) && ( (foperand==ereg)||(foperand==esreg) ) )/*If the two operands is registers than do nothing.*/
        {
        }
        else/*If not enter dest. Dword.*/
        {
            index=IS-(IC-SIC);
            *(codearr+(index))=dest;
            SIC++;
        }

    }/*end of second pass in cmp().*/


}/*END OF CMP OPERATION*/



/*The next 8 functions do the same operations*/

/*This functions been called if (name) operation is declared in the current line.*/
/*If in first pass it calling to manync() function. with 0 as a parameter.*/
/*If in the second pass it will call manync() function, with the suitable opcode as a parameter.*/
/*The (*name) is: "not" , "clr" , "inc" , "dec" , "jmp" , "bne" , "red" and "jsr".*/
void mnot(void)
{
    if(pass==FIRSTPASS)
    {
        manync(0);
    }
    else
    {
        manync(NOTOPCODE);
    }
}
void clr(void)
{
    if(pass==FIRSTPASS)
    {
        manync(0);
    }
    else
    {
        manync(CLROPCODE);
    }
}
void inc(void)
{
    if(pass==FIRSTPASS)
    {
        manync(0);
    }
    else
    {
        manync(INCOPCODE);
    }
}
void dec(void)
{
    if(pass==FIRSTPASS)
    {
        manync(0);
    }
    else
    {
        manync(DECOPCODE);
    }
}
void jmp(void)
{
    if(pass==FIRSTPASS)
    {
        manync(0);
    }
    else
    {
        manync(JMPOPCODE);
    }
}
void bne(void)
{
    if(pass==FIRSTPASS)
    {
        manync(0);
    }
    else
    {
        manync(BNEOPCODE);
    }
}
void red(void)
{
    if(pass==FIRSTPASS)
    {
        manync(0);
    }
    else
    {
        manync(REDOPCODE);
    }
}
void jsr(void)
{
    if(pass==FIRSTPASS)
    {
        manync(0);
    }
    else
    {
        manync(JSROPCODE);
    }
}




/*This function called if "lea" operation is declared in the current line. In first pass & in second pass.*/
/*In first pass:*/
/*--Call to funclabelcreate at the beginning.*/
/*The function checks that the operands is valid & suitable for it.*/
/*If so, IC (global indicator to the code area) increased by the required times.*/
/*If not,meaning non-valid info. than:*/
/*The function set warningfpass to 1, and troubleline to the current file line number.*/
/*In case of warning. information will be sent to stdin & stderr.*/
/**/
/*In second pass:*/
/*The function doing what it done in the first pass(all the above!). And in addition: */
/*It insert the appropriate operands to the code area. (global array: codearr)*/
/*In case of warning. information will be sent to stderr only.*/
void lea(void)
{
    int index=0;
    Dword command=0,suorce=0,dest=0;
    char *assp=p;
    Symbol *lbl=NULL;
    FILE *fpext=NULL;

    SKIP
    assp=p;

    funclabelcreate(IC,label,ZERO,ONE);

    if( (foperand=operandtype()) && comma() )/*legal pattern (up to comma) & known first operand.*/
    {
        if( (foperand==elabel) || (foperand==eslabel) )/*first operand is legal for this operation: elabel or eslabel.*/
        {
            SKIP
            if( (soperand=operandtype()) && nline() )/*legal pattern & known second operand.*/
            {
                if( (soperand==elabel) || (soperand==ereg) )/*second operand is legal: elabel , ereg.*/
                {
                    if(pass==FIRSTPASS)
                    {
                        IC=IC+3;
                    }
                }
                else/*second operand illegal for this operation. */
                {
                    fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                    fprintf(stderr,"illegal second operand.\n");
                    troubleline=flinec;
                    soperand=emistake;
                    warningfpass=ONE;

                    printf("****LEA: warning at line: %d \n",flinec);/*bugs search.*/
                }
            }
            else/*ILLEGAL PATTERN OR UNKOWN SECOND OPERAND*/
            {
                fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                fprintf(stderr,"illegal second operand.\n");
                troubleline=flinec;
                soperand=emistake;
                warningfpass=ONE;

                printf("****LEA: warning at line: %d \n",flinec);/*bugs search.*/
            }
        }
        else/*known first operand, but illegal for this operation.*/
        {
            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal first operand.\n");
            troubleline=flinec;
            foperand=emistake;
            warningfpass=ONE;

            printf("****LEA: warning at line: %d \n",flinec);/*bugs search.*/
        }
    }
    else/*ILLEGAL PATTERN OR UNKOWN FIRST OPERAND*/
    {
        fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
        fprintf(stderr,"illegal first operand.\n");
        troubleline=flinec;
        foperand=emistake;
        warningfpass=ONE;

        printf("****LEA: warning at line: %d \n",flinec);/*bugs search.*/
    }


    if(pass==SECONDPASS)
    {
        p=assp;/*put p pointer at the start of the first operand.*/
        SKIP
        /*p point to the start of the first operand.*/

        command = ( (LEAOPCODE) | (grparr[2].word)  );/*two operands to lea()*/

        if ( (foperand==elabel) || (foperand==eslabel) )/*first operand is a label or random label*/
        {
            if( (foperand==eslabel) )/*rnd bits*//*first operand is random label*/
            {
                command=command|(rndarr[2].word)|(souoperarr[2].word);

                suorce=rndsymboladd();/*create a random number & choose a random label.*/
                suorce=( (suorce<<2)|(AREarr[2].word) );
            }
            else/*given label*/
            {
                command=command|(souoperarr[1].word);/*label is 01*/

                /*check if the label legit & store it in label[].*/
                llabel();

                if((lbl=findlabel()))/*find the Symbol that match the operand label.*/
                {
                    if( (lbl->feature.ext) == 1)/*the label is external*/
                    {
                        suorce = ( (suorce) | (AREarr[1].word) );

                        if( (fpext=fopen(extf,"a+b")) )
                        {
                            base32(SIC+1);

                            fprintf(fpext,"%s %s\n",label,translate32);
                            fclose(fpext);
                        }
                        else/*failed to open file.*/
                        {
                            printf("cant open...\n");/*DEBUG POINT...............*/
                            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                            fprintf(stderr,"cant open the file:%s to write.\n",extf);
                        }
                    }
                    else/*the label is internal.*/
                    {
                        suorce =( (lbl->adess) );
                        suorce= ( (suorce<<2) | (AREarr[2].word) );
                    }
                }
                else/*The operand label name isnt in the linked list of symbols.*/
                {
                    fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                    fprintf(stderr,"unable to find the wanted label.\n");

                }
            }
        }
        else
        {
            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal first operand.\n");
        }
        /*end handling first operand.*/


        /*Managing the second operand type */

        p=assp;/*put p pointer at the start of the first operand.*/
        SKIP
        operandtype(); comma();
        SKIP
        /*p is now point the second operand.*/

        if( (soperand==elabel) )
        {
            command=((command)|(destoperarr[1].word));

            if(llabel())
            {
                lbl=NULL;

                if( (lbl=findlabel()) )/*enter if find the matching label.lbl is pointer to it.*/
                {
                    if( (lbl->feature.ext)==1 )/*If external.*/
                    {
                        dest = ( (dest)|(AREarr[1].word) );

                        if( (fpext=fopen(extf,"a+b")) )
                        {
                            base32(SIC+2);

                            fprintf(fpext,"%s %s\n",label,translate32);
                            fclose(fpext);
                        }
                        else/*failed to open file.*/
                        {
                            printf("cant open...\n");/*DEBUG POINT...............*/
                            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                            fprintf(stderr,"cant open the file:%s to write.\n",extf);
                        }
                    }
                    else
                    {
                        dest =( (lbl->adess) );
                        dest = ( (dest<<2) | (AREarr[2].word) );
                    }
                }
                else/*The operand label name isnt in the linked list of symbols.*/
                {
                    fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                    fprintf(stderr,"unable to find the wanted label.\n");
                }
            }
            else
            {
                fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                fprintf(stderr,"illegal label name.\n");
            }
        }
        else if( (soperand==ereg) )
        {
            command=((command)|(destoperarr[3].word));

            dest=regcomp();
            dest=dest<<2;
        }
        else/*unknown second operand.*/
        {
            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal second operand.\n");
        }

        /*now enter the Dwords to dataarr[]*/

        /*printf("LEA() ENTER:\ncommand:%d.suorce:%d.dest:%d\n\n",command,suorce,dest);*/

        index=IS-(IC-SIC);
        *(codearr+(index))=command;
        SIC++;

        index=IS-(IC-SIC);
        *(codearr+(index))=suorce;
        SIC++;

        index=IS-(IC-SIC);
        *(codearr+(index))=dest;
        SIC++;

    }/*end of second pass in lea().*/

}/*end of lea().*/


/*The next 2 functions do the same operations*/

/*This functions(one of  them) been called if "rts" or "stop" operation is declared in the current line.*/
/*If in first pass it calling to rtstop() function. with 0 as a parameter.*/
/*If in the second pass it will call rtstop() function, with the suitable opcode as a parameter.*/
void rts(void)
{
    if(pass==FIRSTPASS)
    {
        rtstop(0);
    }
    else
    {
        rtstop(RTSOPCODE);
    }
}
void stop(void)
{
    if(pass==FIRSTPASS)
    {
        rtstop(0);
    }
    else
    {
        rtstop(STOPOPCODE);
    }
}


/*This function called if "prn" operation is declared in the current line. In first pass & in second pass.*/
/*In first pass:*/
/*--Call to funclabelcreate at the beginning.*/
/*The function checks that the operands is valid & suitable for it.*/
/*If so, IC (global indicator to the code area) increased by the required times.*/
/*If not,meaning non-valid info. than:*/
/*The function set warningfpass to 1, and troubleline to the current file line number.*/
/*In case of warning. information will be sent to stdin & stderr.*/
/**/
/*In second pass:*/
/*The function doing what it done in the first pass(all the above!). And in addition: */
/*It insert the appropriate operands to the code area. (global array: codearr)*/
/*In case of warning. information will be sent to stderr only.*/
void prn(void)
{
    int index=0;
    Dword command=0,dest=0;
    char *assp=p;
    Symbol *lbl=NULL;
    FILE *fpext=NULL;

    SKIP
    assp=p;

    funclabelcreate(IC,label,ZERO,ONE);

    if( (foperand=operandtype()) && nline() )/*KNOWN OPERAND & PATTERN IS GOOD*/
    {
        if( (foperand==eoperand) || (foperand==elabel) || (foperand==ereg) )/*LEGAL OPERAND FOR THIS OPERATION*/
        {
            if(pass==FIRSTPASS)
            {
                IC=IC+2;
            }
        }
        else/*ILLEGAL OPERAND FOR THIS OPERATION.*/
        {
            troubleline=flinec;
            warningfpass=ONE;
            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal operand.\n");

            printf("****PRN: warning at line: %d \n",flinec);/*bugs search.*/
        }
    }
    else/*WORNG PATTERN OR UNKOWN OPERAND.*/
    {
        fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
        fprintf(stderr,"illegal first operand.\n");
        warningfpass=ONE;
        troubleline=flinec;

        printf("****PRN: warning at line: %d \n",flinec);/*bugs search.*/
    }


    if(pass==SECONDPASS)
    {
        p=assp;/*put p pointer at the start of the first operand.*/
        SKIP
        /*p point to the start of the operand.*/

        command = ( (PRNOPCODE) | (grparr[1].word)  );/*one operand & opcode.*/

        /*Managing the dest. operand type */
        /*p is now point the dest. operand.*/

        if( (foperand==elabel) )
        {
            command=((command)|(destoperarr[1].word));

            if(llabel())
            {
                lbl=NULL;

                if( (lbl=findlabel()) )/*enter if find the matching label.lbl is pointer to it.*/
                {
                    if( (lbl->feature.ext)==1 )/*If external.*/
                    {
                        dest = ( (dest)|(AREarr[1].word) );

                        if( (fpext=fopen(extf,"a+b")) )
                        {
                            base32(SIC+1);

                            fprintf(fpext,"%s %s\n",label,translate32);
                            fclose(fpext);
                        }
                        else/*failed to open file.*/
                        {
                            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                            fprintf(stderr,"cant open the file:%s to write.\n",extf);
                        }
                    }
                    else
                    {
                        dest =( (lbl->adess) );
                        dest = ( (dest<<2) | (AREarr[2].word) );
                    }
                }
                else/*The operand label name isnt in the linked list of symbols.*/
                {
                    fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                    fprintf(stderr,"unable to find the wanted label.\n");
                }
            }
            else
            {
                fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
                fprintf(stderr,"illegal label name.\n");
            }
        }
        else if( (foperand==ereg) )
        {
            command=((command)|(destoperarr[3].word));

            dest=regcomp();
            dest=dest<<2;
        }
        else if(foperand==eoperand)
        {
            command=((command)|(destoperarr[0].word));
            operandtype();
            insertnum(&dest,&global13bits);

            dest=dest<<2;
        }

        /*now enter the Dwords to dataarr[]*/

        index=IS-(IC-SIC);
        *(codearr+(index))=command;
        SIC++;

        index=IS-(IC-SIC);
        *(codearr+(index))=dest;
        SIC++;

    }/*end of second pass in prn().*/

}/*End of prn().*/



/*getcmd:*/
/*This function called by the firstpass & secondpass function.*/
/*This function is a control flow function.*/
/*It identify the next operation & send the flow to the correct function.*/
/*A legal name contain '.' and lower letters only. '.' only in the start of the command name.*/
/*Return:*/
/*In case that the command name is illegal, the function will return (-1).*/
/*In case that the name is assembled in a legal characters, but such command name is not exits, will return (-2).*/
/*In case of a legal & exits command, will return its index.(in the matching array).*/
/*There are two arrays for commands. one is for operations like mov,add,inc,prn and more..*/
/* The second is for .data , .string , .entry and.extern*/
/**/
/*If something went wrong, The function set warningfpass to 1, and troubleline to the current file line number.*/
/*In case of warning. information will be sent to stdin & stderr.*/
Dword getcmd()
{
    const char *fccmd=p;/*fccmd = first char-command.*/
    int flag=0,index=0;
    char tmp=0;

    if( *p=='.' )/*guiding*/
    {
        flag=ONE;
        p++;
    }

    while(islower(*p))
        p++;

    if( (*p==' ') || (*p=='\t') || (*p=='\n') )/*After the command/guiding name a legal space is required: ' ' or '\t' ONLY.*/
    {
        tmp=*p;
        *p='\0';
    }
    else/*Something is wrong , cmd name illegal */
    {
        fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
        fprintf(stderr,"Illegal command name.\n");

        printf("****Warning at line: %d \n",flinec);/*bugs search.*/
        troubleline=flinec;
        warningfpass=ONE;

        return (-1);
    }

    if(flag)/*guiding !*/
    {
        while(  (strcmp( (guide[index].name) , (guide[NUMOFGUIDE-1].name) ) ) )/*( guide[index].func ) != NULL*/
        {
            if( ! ( strcmp( fccmd , (guide[index].name) ) ) )/*enter if the same name, meaning found the func*/
            {
                *p=tmp;
                SKIP

                printf("GETCMD() FUNCTION:End of getcmd(index).guide[]--1.index:%d\n",index);

                guide[index].func();
                return index;
            }
            index++;
        }
    }
    else/*operation*//* ( operarr[index].func ) == NULL*//* */
    {
        while( (strcmp( (operarr[index].name) , (operarr[NUMOFOPER-1].name) ) ) )  /*If the two strings is equal - return zero & quit.*//*strcmp return 0 if two const equal*/
        {
            if( ! (strcmp( fccmd , (operarr[index].name) ) ) )
            {
                *p=tmp;
                SKIP

                printf("GETCMD() FUNCTION:End of getcmd(index).BEFORE OPERARR[]---1.index:%d\n",index);

                operarr[index].func();
                return index;
            }
            index++;
        }
    }

    fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
    fprintf(stderr,"Command name not exits.\n");
    printf("****Warning at line: %d \n",flinec);/*bugs search.*/

    return (-2);/**/
}/*end of getcmd*/


