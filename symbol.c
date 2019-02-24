

#include "myheader.h"
#include "opcodes.h"
#include "symbol.h"
#include <string.h>
#include <stdlib.h>



extern char label[] ;
extern int IC;



/*-------------------Symbols functions & pointers to the symbols table--------------*/

Symbol *phead=NULL,*plast=NULL;/*pointers that hold the first symbol(phead) & the last symbol(plast).*/


/*This function prints the linked list to stdin.*/
/*This function state at the start that it been called.*/
void printlist(void)
{
    Symbol *ptr=NULL;

    puts("***\nstart of printlist-\n***\n");

    if(phead==NULL)
        return;

    ptr=phead;

    while((ptr->next)!=NULL )
    {
        printf("\nThe current node is:%s, its address is:%d.\n",ptr->label,ptr->adess);
        ptr=ptr->next;
    }
    if(ptr!=NULL)
    {
        printf("\nThe current node is:%s, its address is:%d.\n",ptr->label,ptr->adess);
    }
}


void createsymbol(Dword address, char *name, Dword ext, Dword ope)
{
    Symbol *unused;
    unused=(Symbol *)malloc(sizeof(Symbol));

    unused->adess=address;
    strcpy(unused->label,name);
    unused->feature.ext=ext;
    unused->feature.oper=ope;
    unused->next=NULL;

    /**/


    if(phead==NULL)
    {
        phead=unused;
        plast=phead;
    }
    else if(phead==plast)
    {
        plast=unused;
        phead->next=plast;
        plast->next=NULL;
    }
    else
    {
        plast->next=unused;
        plast=plast->next;
        plast->next=NULL;
    }

    printlist();

}



/*This function is called only when label[], store a valid label.otherwise it a waste.*/
/*This function use global variables: label[], phead & plast */
/*This function search a Symbol in the Symbol linked list that contain the same label name as label[] contain*/
/*This function returns: */
/*A Symbol pointer to the matched Symbol node,meaning its label match the one label[] store.*/
/*A null pointer - otherwise.*/
Symbol *findlabel(void)
{
    Symbol *curr=NULL;

    if(  (phead==NULL) || (strlen(label)==0)  )/*no Symbol exits, meaning no labels exits in this program for current time!.*/
    {
        return NULL;
    }
    else
    {

        curr=phead;

        /*to be sure that the last symbol(node) point to null as next node!.*/
        for(  ;  (curr!=NULL) && (curr!=plast) ; curr=curr->next  )
        {

            if( (strlen(label) ) && (strlen(curr->label)) )
            {

                if(  (!(strcmp(label, curr->label))) )
                    {
                        return curr;
                    }
            }

        }/*pos. will stop on the Symbol that i wanted or null if didnt find.*/

        if( (curr!=NULL) && (strlen(curr->label)) && (strlen(label)) )
            if( !(strcmp(label, curr->label)) )/*for plast.*/
            {
                puts("findlabel:find & return!.");
                return curr;
            }

    }
    /*puts("findlabel:End.(didnt find)\n");*/
    return NULL;/*if didnt find it will be NULL,because ..*/
}/*End of findlabel().*/



/*This function update all internal symbols addresses, by adding its address IC.*/
/*this is made before second pass, and before adding the data area to the end of the code area.*/
/*Reminder: This is the linked list that hold all the symbols\labels. The data area been strcat to the end of the code area is a different act.*/
/*return: */
/*Return 0, if no symbols to update.*/
/*Return >0, if update symbols.*/
/*ans count the number of Symbols update & return at the end of the function.*/
/*Return (-1), if didnt search the all list.*/
Dword addup(void)
{
    Symbol *curr=NULL;
    Dword ans=0;
    plast->next=NULL;

    if( (phead==NULL) || (plast==NULL) )/*checks that the global pointer to the head & tail is not null.*/
        return 0;

    curr=phead;

    for( ; curr!=NULL ; curr=curr->next )
    {

        if(curr!=NULL)
        {

            if(  (!(curr->feature.oper))  &&  (!(curr->feature.ext))  )/*crash here !*/
            {

                curr->adess=(curr->adess)+IC;
                ans++;
            }
        }


        if((curr->next)==NULL)
            break;
    }



    if(  (curr!=NULL) && (plast!=NULL) )
    {
        if(curr==plast)/*meaning all the list been searched.*/
            return ans;
    }


    return (-1);
}

/*this function choose a random internal symbol & return its address.*/
/*dont use label[].*/
Dword rndsymboladd(void)
{
    Symbol *ptr=NULL;
    Dword counter=0;

    rand();

    if(phead==NULL)/*no symbols\labels.*/
        return counter;
    else if (plast==phead)/*only one label.*/
    {
        if( (phead->feature.ext) == 0 )
            return ( ((phead->adess)<<2)|(AREarr[2].word) );
    }
    else/*more then one.*/
    {
        plast->next=phead;/*do cycle*/
        ptr=phead;

        counter = ((Dword)( rand()% MAXLDATA ) );
        counter+=MAXLDATA;

        while( (counter--) )
        {
            if( ((ptr->feature.ext)==0) )
            {
                counter=ptr->adess;
                break;
            }

            ptr=ptr->next;
        }

        plast->next=NULL;/*un-cycle*/
    }

    return counter;
}


void freelist(Symbol *ptr)
{
    Symbol *curr=NULL;

    if(phead==NULL)
    {
        plast=NULL;
        return;
    }
    else if(phead==plast)
    {
        plast=NULL;
        free(phead);
        phead=NULL;
        return;
    }

    while( phead!=plast  )
    {
        curr=phead;
        phead=phead->next;
        free(curr);
    }

    free(plast);

    phead=NULL;
    plast=NULL;

    return;
}


