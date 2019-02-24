

#include "myheader.h"
#include "symbol.h"

#include <math.h>

#include <string.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define noteorempty {\
    char *noeptr=p;\
    if(*noeptr==';')\
        continue;\
    else if(nline())\
        continue;\
    else\
        p=noeptr;\
}







/*-----------------------------------------------------------------------------------------------------*/

/*extern Symbol *phead;*/
extern Dword getlabel(void);
extern Dword getcmd();
extern int nline();

extern Dword addup();
extern void datafeed(int);

extern char translate32[];
extern char base32(int num);

extern void freelist(Symbol *ptr);

/*---------------------------------------------------------------------------------------------------*/



/*Area of definitions!*/

char *p;


Dword labelflag=0;
char line[MAXROWLEN] ;
char label[MAXLABEL] ;
char fname[MAXFILENAME];/*the name of the file without ".as".*/
char name[MAXFILENAME];/*the name of the file with ".as".*/
char extf[MAXFILENAME];/*the name of the file with ".ext".*/
char entf[MAXFILENAME];/*the name of the file with ".ent".*/

Dword dataarr[MAXLDATA];/*HOLD ALL FROM .DATA &.STRING.*/
Dword *codearr=NULL;

int SIC=STARTADDRESS,IC=STARTADDRESS,IS=ZERO;
int DC=ZERO;

Dword pass=ZERO;
Dword flinec=ZERO;
Dword warningfpass=ZERO;
Dword troubleline=(-1);
Dword foperand,soperand;

char numberflag=ZERO;
Dword globalnumber=ZERO;
Num13bits global13bits;

/*This function prints the code array to stdin.*/
/*This function state at the start that it been called.*/
void printcode(Dword num)
{
    int index;
    int value;
    puts("***\nstart of printcode-\n***\n");
    for(index=0 ; index <=num ; index++)
    {
        value=(*(codearr+index));
        printf(" (*codearr+(%d)) value is:%d.\n",index,value);
    }
}

/*This function create the object file of the current file been read.*/
/*It write the current file to a file with ending ".ob", at base 32.*/
/*All the addresses, commands & operands will write to .ob file.*/
/*It uses in function base32 and global array: translate32.*/
Dword cobjfile(char name[])
{
    int index=0;
    int counter=0;
    Dword temp=0;
    FILE *objfp=NULL;

    char objf[MAXFILENAME];
    strcpy(objf,name);
    strcat(objf,".ob");

    SIC=IC-IS;

    objfp=fopen(objf,"w+b");

    if(objfp)
    {
        fprintf(objfp,"%10s %10s\n","Base 32","Base 32");
        fprintf(objfp,"%10s %10s\n","Address","Machine-code");

        base32(IS);
        fprintf(objfp,"\t %10s \t",translate32);

        base32(DC);
        fprintf(objfp,"\t %10s \n",translate32);

        fflush(objfp);


        for(counter=(DC+IS) ; (counter) ; counter--)
        {

            base32((index+SIC));

            fprintf(objfp,"%10s ", translate32 );
            fflush(objfp);

            temp=(*(codearr+index));

            base32( temp );

            fprintf(objfp,"%10s\n" , translate32);
            fflush(objfp);

            index++;
        }

    }
    else
    {
        fclose(objfp);
        return 1;
    }

    fclose(objfp);
    return 0;
}/*End of cfile.*/



/*This function will be call by firstpass function only if warningfpass set to zero, meaning no error or warning in the current file.*/
/*This function responsible for creating & writing to output files with ending ".ext" & ".ent" .*/
/*In case of warning. information will be sent to stderr only.*/
/*Return:*/
Dword secondpass(FILE *stream)
{
    time_t t;
    FILE *pextf=NULL,*pentf=NULL;
    Dword *codehead=NULL;
    srand((unsigned)time(&t));

    puts("\n*****\nWelcome to second pass!.\n*****\n");

    strcpy(entf,fname);
    strcpy(extf,fname);
    strcat(entf,".ent");
    strcat(extf,".ext");

    flinec=0;

    if( ( pextf=fopen(extf,"a+b") ) && ( pentf=fopen(entf,"a+b") ) )/**/
    {

        fclose(pextf);
        fclose(pentf);

        codearr=(Dword*)calloc( (sizeof(Dword)) ,(IS + DC));

        codehead=codearr;

        addup();/*update all the internal addresses of labels by adding it IC.*/

        while(  (fgets(line, MAXROWLEN, stream)) != NULL )
        {
            p=line;

            noteorempty

            labelflag=0;

            getlabel();/*move p after the label*/

            SKIP

            getcmd();

            flinec++;

        }/*end of while loop: end of reading file.*/

        datafeed(DC);/*put all the data in the end of codearr.*/

    }
    else/*failed to create wanted files.*/
    {
        fprintf(stderr,"error: in file:%s, line:%d,", fname, flinec);
        fprintf(stderr,"Second pass cant create files.\n");
        printf("error: in file:%s, line:%d,Second pass cant create files.\n", fname, flinec);
    }


    /*call to a function that print the last file.*/
    cobjfile(fname);

    /*now free all memory!.*/

    freelist(phead);
    free(codehead);

    return warningfpass;
}




/*This function called by main only.*/
/*This function get an pointer to the stream of the current file.*/
/*This function is the only one that call secondpass. secondpass is the function for second pass only.*/
/*This function call secondpass only if the current file is without any errors.*/
Dword firstpass(FILE *stream)
{
    flinec=0;
    warningfpass=0;

    /*this while loop gets all the lines of the current file, one by one.*/
    while( ( fgets(line ,MAXROWLEN ,stream ) ) != NULL  )/*gets the line and put it in line array.return NULL if something went wrong.*/
    {

        printf("firstpass.c: Enter while loop.line: %d\n",flinec);/*BUG POINT.............................................*/

        p=line;

        noteorempty

        labelflag=0;

        getlabel(); /*if label exits-store it in label[].*/

        /*p is now right after the label.*/
        SKIP
        /*now, p is on first char of cmd.*/

        getcmd();

        flinec++;
    }/*end of while loop*/

    /*by now read the all file or something went wrong.*/

    if(warningfpass)/*enter if warningfpass equal to non zero value.meaning firstpass had a error.*/
    {
        /*warningfpass==1 only if there is a error in the file.*/
        /* in this case the program will not continue to second pass.*/

        printf("\nfirstpass.c: you have some warning.\nline:%d\nwarning is:%d.\nlast known char:\"%c\"\n",flinec,warningfpass,*p);/*BUG POINT....*/
        printf("The number of line with the error:%d",troubleline);
    }
    else/*all was good,in first pass, advance to second pass.*/
    {
        printf("firstpass.c: just before second pass.line:%d\n",flinec);/*BUG POINT.......................................*/

        rewind(stream);
        pass=SECONDPASS;

        IS=IC-SIC;
        secondpass(stream);
    }

    return (warningfpass); /*Return 1, if the file have non-valid input. 0 otherwise.*/
}/*end of firstpass*/




/*---- MAIN ----*/

int main(int argc, char *argv[])
{
    FILE *fp=NULL;
    static Dword fres=0;
    char *prog = argv[0];

    if(argc <= 1)
    {
        printf("no file name is entered !\n");
        return (-1);   /*return 1, if no file name is entered.*/
    }
    else
    {
        while( --argc > 0)
        {
            ++argv;
            strcpy(fname,*argv);
            strcpy(name,fname);
            strcat(name,".as");

            global13bits.number=0;
            warningfpass=ZERO;/*before call first pass, set warning to zero.*/

            if( ( fp = fopen(name,"r") ) == NULL )/*was *++argv.*/
            {
                /*fopen return null if cant open file or EOF.*/
                fprintf(stderr, "Warning:\n%s: cant open %s\n", prog, name);
                continue;

            }
            else/*If the open operation success */
            {

                printf("main.c: Enter to firstpass.\n");/*BUG POINT....................*/

                pass=FIRSTPASS;
                fres+=firstpass(fp);


            }

            fclose(fp);

        }/*End of while loop*/

    }

    return fres; /*return 0, if one or more valid file was entered.in case of warning return the number of files with warning in it.*/
}

