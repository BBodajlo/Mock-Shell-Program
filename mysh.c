#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int isInitialized = 0;

void initialize()
{
    if(isInitialized == 0)
    {
        system("clear");
        printf("Welcome to the shell!!!!!!\nGood luck!!!!\n");
    }
    isInitialized = 1;
}



int main (int argc, char** argv)
{

    initialize();
    int i = 0;
    char buff[128];
    while(i == 0)
    {
        printf("mysh> ");
        scanf("%s", buff);

        if(strcmp(buff, "exit") == 0)
        {
            i = 1;
        }
        else
        {
            printf("ehco: %s\n", buff);
        }

        
    }

    initialize();

}