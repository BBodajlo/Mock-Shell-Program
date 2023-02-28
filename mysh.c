#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int isInitialized = 0; //Static variable to keep track of if the shell has been initialized

void initialize()
{
    if(isInitialized == 0)
    {
        system("clear"); //Will clear the current terminal
        printf("Welcome to the shell!!!!!!\nGood luck!!!!\n");
    }
    isInitialized = 1; //Won't run any other time
}



int main (int argc, char** argv)
{

    initialize(); //This will need to move once we get the batch mode working
    int i = 0;
    char buff[128]; //Simple start buffer for user input
    while(i == 0) // Will probably just be set to while(1) later after logic is implemented
    {
        printf("mysh> "); //After every command is input, the shell will give the newline prompt
        scanf("%s", buff);

        if(strcmp(buff, "exit") == 0) //Will probably move to a different logic for exiting; simple for now
        {
            i = 1;
        }
        else
        {
            printf("ehco: %s\n", buff); //Just to make sure spacing worked out, the shell just ehcos back what is input
        }

        
    }

    initialize(); //Checking to see if the initilization was called again (it wasn't)

}