#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX_INPUT 128 //Max size of input
#define MAX_TOKENS 20 //Max amount of tokens on a single line to parse

static int isInitialized = 0; //Static variable to keep track of if the shell has been initialized


// Variables Main Needs 
int exitStatus = 0;
char path[MAX_INPUT] = "mysh"; 
char** tokenArray; //Tokens that are parsed
int numOfTokens; //Number of tokens parsed

// Functions 
void initialize();
void shellExit();
void echo();
void pwd();
void tokenizer(char**, char*);
void freeTokens(char**);

// Command struct
struct command
{
    const char* name;
    int (*func)();
};

// Command list
struct command commandList[] = {
    {"echo", echo},
    {"exit", shellExit},
    {"pwd", pwd}
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
        //128 to indicate number of chars
        //scanf("%128[^\n]%*c", buff); // fetches the command (input for command would not have been read yet)
        tokenArray = malloc(sizeof(char*)*MAX_INPUT);
        for(int i = 0; i < MAX_TOKENS; i++)
        {
            tokenArray[i] = (char*)malloc(MAX_TOKENS + 1);
            memset(tokenArray[i], 0, sizeof(tokenArray[i]));
        }

        tokenizer(tokenArray, buff);
        printf("%s\n",tokenArray[0]);
        printf("%s\n",tokenArray[1]);
        int a = strcmp(tokenArray[0], tokenArray[1]);
        printf("%d\n", a);
        // Search for command in command list
        int found = 0;
        for (int i = 0; i < sizeof(commandList) / sizeof(commandList[0]); i++)
        {
            if (strcmp(tokenArray[0], commandList[i].name) == 0)
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
        
        }
        freeTokens(tokenArray); //Free token array 
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

    for(int i = 1; i < numOfTokens; i++) //Searches through every token that is not the first command
    {
        printf("%s ", tokenArray[i]);
    }
    printf("\n");

};
void pwd()
{
    char cwd[255]; //Think of a size
    if(getcwd(cwd, sizeof(cwd)) == NULL) //Simply uses the getcwd command to get the current directory
    {
        printf("Error getting directory");
    }
    else
    {
        printf("%s\n", cwd);
    }

    
};

void tokenizer(char** tokens, char* buff)
{

    int bytes;
    int tokenSpot = 0; //To increment to token array when a new token should be made
    //Probably don't need a holder array; keeping for now
    char holder[MAX_INPUT]; //Holder string to keep track of the current token being made
    memset(holder, 0, sizeof(holder));
    int holderSpot = 0; //To increment the holder spot
    fflush(STDIN_FILENO);
    while((bytes = read(STDIN_FILENO, buff, MAX_INPUT)) > 0)
    {
        for(int i = 0; i <= bytes-1; i++)
        {
            if(buff[i] == ' ' || i == bytes-1) //If the buffer has a space, or is at the end, finish the token and move to the next one
            {
                strcpy(tokens[tokenSpot], holder); //Copies the built token into the token array
                holderSpot = 0; 
                memset(holder, 0, sizeof(holder)); //Reset the holder string
                tokenSpot +=1; //Next token
                numOfTokens++;

            }
            else{
            holder[holderSpot++] = buff[i];
            }
        }
        break;
    }


};

//Takes the std in buffer and converts it into an array of tokens
/*void tokenizer(char** tokens, char* buff)
{
    int tokenSpot = 0; //To increment to token array when a new token should be made
    //Probably don't need a holder array; keeping for now
    char holder[MAX_INPUT]; //Holder string to keep track of the current token being made
    int holderSpot = 0; //To increment the holder spot

    //MAKE SURE NUMOFTOKENS IS CORRECT AMOUNT WHEN DEBUGGING!!!!!!!!
    numOfTokens = 0; //May increment more than needed on edge cases

    for(int i = 0; i <= strlen(buff); i++) //Goes through the std in buffer
    {
       
        if(buff[i] == ' ' || i == strlen(buff)) //If the buffer has a space, or is at the end, finish the token and move to the next one
        {
            strcpy(tokens[tokenSpot], holder); //Copies the built token into the token array
            holderSpot = 0; 
            memset(holder, 0, sizeof(holder)); //Reset the holder string
            tokenSpot +=1; //Next token
            numOfTokens++;
            
        }
        else{
            holder[holderSpot++] = buff[i];
        }
        
    }

}*/

void freeTokens(char** arr) //Frees the tokens array
{
    for(int i = 0; i < MAX_TOKENS; i++)
    {
        free(arr[i]);
    }
    free(arr);
}