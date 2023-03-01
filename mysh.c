#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INPUT 128 //Max size of input

static int isInitialized = 0; //Static variable to keep track of if the shell has been initialized


// Variables Main Needs 
int exitStatus = 0;
char path[MAX_INPUT] = "mysh"; 


// Functions 
void initialize();
void shellExit();
void echo();

// Command struct
struct command
{
    const char* name;
    int (*func)();
};

// Command list
struct command commandList[] = {
    {"echo", echo},
    {"exit", shellExit}
};

int main (int argc, char** argv)
{

    initialize(); //This will need to move once we get the batch mode working
    char buff[MAX_INPUT]; //Simple start buffer for user input

    while(exitStatus == 0) // Will probably just be set to while(1) later after logic is implemented
    {
        int errStatus = 0;

        if(errStatus){
            printf("!%s> ", path); // Errored last time, so give the error prompt
            errStatus = 0;
        }else{
            printf("%s> ", path); 
        }
        scanf("%s", buff); // fetches the command (input for command would not have been read yet)

        // Search for command in command list
        int found = 0;
        for (int i = 0; i < sizeof(commandList) / sizeof(commandList[0]); i++)
        {
            if (strcmp(buff, commandList[i].name) == 0)
            {
                errStatus = commandList[i].func();
                found = 1;
                break;
            }
        }

        // If command not found
        if (!found)
        {
            printf("Command not found\n");
            
            // Clear stdin input since cmd non-existant 
            while (getchar() != '\n'){};
        }
        
    }

    initialize(); //Checking to see if the initilization was called again (it wasn't)

    return EXIT_SUCCESS;

}

void initialize()
{
    if(isInitialized == 0)
    {
        system("clear"); //Will clear the current terminal
        printf("Welcome to the shell!!!!!!\nGood luck!!!!\n");
    }
    isInitialized = 1; //Won't run any other time
}

void shellExit()
{
    exitStatus = 1;
};

void echo()
{
    char ch = getchar(); // skip the space
    ch = getchar();
    
    while (ch != '\n'){
        printf("%c", ch);
        ch = getchar();
    };
    printf("\n");

};