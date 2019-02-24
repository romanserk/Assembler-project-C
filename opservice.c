
#include "myheader.h"
#include "symbol.h"
#include "global.h"
#include "opcodes.h"





/*--------------------------------------------------------------------------------------------*/

extern void funclabelcreate(Dword indictor,char label[],Dword external, Dword operation);
extern Dword rnum(Dword min,Dword max);
extern void insertnum(Dword *toword,Num13bits *from);

extern char base32(int num);
extern char translate32[];

/*--------------------------------------------------------------------------------------------*/



/*This function called by mov,add & sub functions. In first pass & in second pass.*/
/**/
/*In first pass:*/
/*--Call to funclabelcreate at the beginning.*/
/**/
/*The function checks that the operands is valid & suitable for mov,add & sub operations.*/
/*If so, IC (global indicator to the code area) increased by the required times.*/
/*      by 3 ,if the code area will need three lines for this input line*/
/*      by 2 ,if two operands is register operands.*/
/*If not, meaning non-valid info. IC will stay unchanged & than:*/
/*      The function set warningfpass to 1, and troubleline to the current file line number.*/
/*      In case of warning. information will be sent to stdin & stderr.*/
/**/
/*In second pass:*/
/*The function doing what it done in the first pass(all the above!). And in addition: */
/*It insert the appropriate command & operands to the code area. (global array: codearr)*/
/*In case of warning. information will be sent to stderr only.*/
void mas(Dword oppara)
{
    Num13bits tmpoperand;
    int index=0;
    Dword command=0,suorce=0,dest=0;
    char *assp=p;
    Symbol *lbl=NULL;
    FILE *fpext=NULL;

    SKIP
    assp=p;

    funclabelcreate(IC,label,ZERO,ONE);

    if( (foperand=operandtype()) && comma() )/*pattern is good up to first comma & first operand is a known one.*/
    {
        SKIP

        if( (soperand=operandtype()) && nline() )/* all pattern is good & second operand is known one.*/
        {
            if( (soperand==elabel) || (soperand==ereg) )/*second operand legal for THIS operation.*/
            {
                if(soperand==ereg)/*chance for only two Dwords */
                {
                    if( (foperand==esreg) || (foperand==ereg) )/*two registers:one Dword*/
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
                else
                {
                    if(pass==FIRSTPASS)
                    {
                        IC=IC+3;
                    }
                }
            }
            else/*second operand is known, but illegal for THIS operation.*/
            {
                /*ILLEGAL SECOND OPERAND include file name & line number.*//* dont go to second pass.*/
                fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
                fprintf(stderr,"illegal second operand.\n");
                warningfpass=ONE;
                troubleline=flinec;

                printf("MAS: Warning at line: %d \n",flinec);/*bugs search.*/
            }
        }
        else/*pattern after comma is not good or second operand is unknown*/
        {
            /*SECOND OPERAND IS UNKONWN*/
            fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal second operand.\n");
            warningfpass=ONE;
            troubleline=flinec;

            printf("MAS: Warning at line: %d \n",flinec);/*bugs search.*/
        }
    }
    else/*pattern is not good up to first comma or first operand is unknown */
    {
        fprintf(stderr,"error: in file:%s, line:%d, ", fname, flinec);
        fprintf(stderr,"illegal first operand.\n");
        warningfpass=ONE;
        troubleline=flinec;

        printf("MAS: Warning at line: %d \n",flinec);/*bugs search.*/
    }

    p=assp;/*put p pointer at the start of the first operand.*/

    /*The code from here is only if second pass*/
    /*now I will build the Dword's & store it in codearr[].*/

    if(pass==SECONDPASS)
    {
        p=assp;/*put p pointer at the start of the first operand.*/
        SKIP
        /*p point to the start of the first operand.*/

        command = ( (oppara) | (grparr[2].word)  );

        if( (foperand==eoperand) || (foperand==esoperand) )/*if the first operand is regular number or random number */
        {
            if( (foperand==esoperand) )/*rnd bits.*//*first operand is random number*/
            {
                command=command|(rndarr[1].word)|(souoperarr[2].word);

                tmpoperand.number=rnum(MIN13NUM,MAX13NUM);/*lior: check if this good...............................<<<<...............<<<<...........*/
                insertnum(&suorce,(&tmpoperand) );
                suorce<<=2;

            }
            else/*given number.*/
            {
                command=command|(souoperarr[0].word);

                operandtype();
                insertnum(&suorce,&global13bits);
                suorce<<=2;
            }
        }
        else if ( (foperand==elabel) || (foperand==eslabel) )/*first operand is a label or random label*/
        {
            if( (foperand==eslabel) )/*rnd bits*//*first operand is random label*/
            {
                command=command|(rndarr[2].word)|(souoperarr[2].word);

                suorce=rndsymboladd();/*rndsymboladd: choose a random internal label & return its address.*/
                suorce=( (suorce<<2)|(AREarr[2].word) );/*store the address in the left 13 bits.*/

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
                            fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
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
                    fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
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
        else/*unknown first operand*/
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
                            fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
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
                    fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
                    fprintf(stderr,"unable to find the wanted label.\n");
                }
            }/*label name illegal.*/
            else
            {
                fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
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
        else/*unknown second operand.*/
        {
            fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal second operand.\n");
        }

        /*now enter the Dwords to dataarr[]*/

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

    }/*end of second pass in mas().*/


    return;
}/*end of mas().*/




/*This function called by not, clr, inc, dec, jmp, bne, red and jsr functions. In first pass & in second pass.*/
/**/
/*In first pass:*/
/*--Call to funclabelcreate at the beginning.*/
/**/
/*The function checks that the operand is valid & suitable for the operation.*/
/*If so, IC (global indicator to the code area) increased by the required times.*/
/*      by 2 ,if the code area will need two lines for this input line*/
/*If not, meaning non-valid info. IC will stay unchanged & than:*/
/*      The function set warningfpass to 1, and troubleline to the current file line number.*/
/*      In case of warning. information will be sent to stdin & stderr.*/
/**/
/*In second pass:*/
/*The function doing what it done in the first pass(all the above!). And in addition: */
/*It insert the appropriate command & operand to the code area. (global array: codearr)*/
/*In case of warning. information will be sent to stderr only.*/
void manync(Dword oppara)/*no source operand.destination operand is label or register only*/
{
    int index=0;
    Dword command=0,dest=0;
    char *assp=p;
    Symbol *lbl=NULL;
    FILE *fpext=NULL;

    SKIP

    assp=p;

    funclabelcreate(IC,label,ZERO,ONE);

    if( (foperand=operandtype())  && nline() )/*The pattern is good & operand known*/
    {
        if( (foperand==elabel) || (foperand==ereg) )/*The operand is a legal for those operations*/
        {
            if(pass==FIRSTPASS)
            {
                IC=IC+2;
            }
        }
        else/*ILLEGAL OPERAND FOR THIS OPERATIONS.*/
        {
            /*print error . dont go to second pass. provide file name & line number !.*/
            /*say that ILLEGAL OPERAND FOR THIS OPERATIONS*/
            fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal operand.\n");
            warningfpass=ONE;
            troubleline=flinec;

            printf("****MANYNC: Warning at line: %d \n",flinec);/*bugs search.*/
        }
    }
    else/*the pattern not good or unknown operand. */
    {
        /*print error . dont go to second pass. provide file name & line number !.*/
        /* say that:the pattern not good or unknown operand*/

        fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
        fprintf(stderr,"illegal operand.\n");
        warningfpass=ONE;
        troubleline=flinec;

        printf("****MANYNC: Warning at line: %d \n",flinec);/*bugs search.*/

    }


    if(pass==SECONDPASS)
    {
        p=assp;
        SKIP
        /*p point to the start of the operand.*/

        command = ( (oppara) | (grparr[1].word)  );

        /*Managing the dest. operand type */
        /*p is now point the dest. operand.*/

        if( (foperand==elabel) )
        {
            command=((command)|(destoperarr[1].word));

            if(llabel())
            {
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
                            fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
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
                    fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
                    fprintf(stderr,"unable to find the wanted label.\n");
                }
            }
            else/*label name illegal.*/
            {
                fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
                fprintf(stderr,"illegal label name.\n");
            }
        }
        else if( (foperand==ereg) )
        {
            command=((command)|(destoperarr[3].word));
            dest=regcomp();
            dest=dest<<2;
        }
        else
        {
            fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
            fprintf(stderr,"illegal operand.\n");
        }

        /*now enter the Dwords to dataarr[]*/

        index=IS-(IC-SIC);
        *(codearr+(index))=command;
        SIC++;

        index=IS-(IC-SIC);
        *(codearr+(index))=dest;
        SIC++;

    }/*end of second pass in manync().*/

}/*End of manync().*/



/*This function called by rts & stop functions. In first pass & in second pass.*/
/**/
/*In first pass:*/
/*--Call to funclabelcreate at the beginning.*/
/**/
/*IC (global indicator to the code area) increased by the required times.*/
/*      by 1 ,if the input line is legal.*/
/*If not, meaning non-valid info. IC will stay unchanged & than:*/
/*      The function set warningfpass to 1, and troubleline to the current file line number.*/
/*      In case of warning. information will be sent to stdin & stderr.*/
/**/
/*In second pass:*/
/*The function doing what it done in the first pass(all the above!). And in addition: */
/*It insert the appropriate command to the code area. (global array: codearr)*/
/*In case of warning. information will be sent to stdin & stderr.*/
void rtstop(Dword oppara)
{
    int index=0;
    Dword command=0;

    funclabelcreate(IC,label,ZERO,ONE);


    if((nline()))
    {
        if(pass==FIRSTPASS)
        {
            IC=IC+1;
        }
    }
    else
    {
        /*print error . dont go to second pass. provide file name & line number !.*/
        /*say that ILLEGAL INPUT FOR THIS OPERATIONS*/
        fprintf(stderr,"Error: in file:%s, line:%d, ", fname, flinec);
        fprintf(stderr,"illegal input.\n");
        warningfpass=ONE;
        troubleline=flinec;

        if(pass==FIRSTPASS)
            printf("****RTS/STOP: Warning at line: %d \n",flinec);/*bugs search.*/
    }


    if(pass==SECONDPASS)
    {
        command=( (command) | (oppara) | (grparr[0].word) );

        index=IS-(IC-SIC);
        *(codearr+(index))=command;
        SIC++;

    }
}/*End of rtstop.*/




