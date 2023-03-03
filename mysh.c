#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX_INPUT 128 //Max size of input
#define MAX_TOKENS 20 //Max amount of tokens on a single line to parse

static int isInitialized = 0; //Static variable to keep track of if the shell has been initialized


//TokenList linked list
typedef struct tokenList{
    char *token;
    struct tokenList *command;
    struct tokenList *next;
}tokenList_t;



// Variables Main Needs 
int exitStatus = 0;
char path[MAX_INPUT] = "mysh"; 
tokenList_t *tokenList; //General token list

// Functions 
void initialize();
void shellExit();
void echo();
void pwd();
void tokenizer();
void addToken(char *token, tokenList_t *command);
void freeTokenList();
void alterAndSetCommand(tokenList_t **c);
void batchTokenizer(int argc, char **argv);


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
    if(argc < 2)
    {
        initialize(); //This will need to move once we get the batch mode working
    }
    
    char buff[MAX_INPUT]; //Simple start buffer for user input
   
    while(exitStatus == 0) // Will probably just be set to while(1) later after logic is implemented
    {
        int errStatus = 0;

        if(errStatus && argc < 2){
            printf("!%s> ", path); // Errored last time, so give the error prompt
            errStatus = 0;
        }else if(argc < 2){
            printf("%s> ", path); 
        }
        
        tokenList = NULL; //Just to make sure tokenList truly is null before using it again

        //Parse input (For interactive mode only atm)
        
        if(argc < 2) //If there are commandline arguments, batch mode should run
        {
            tokenizer(); 
        }
        else{
            
            batchTokenizer(argc, argv);
            exitStatus = 1; //Only one iteration needs to be run for batch mode
        }

        
        // Search for command in command list
        int found = 0;
        for (int i = 0; i < sizeof(commandList) / sizeof(commandList[0]); i++)
        {
            if (strcmp(tokenList->command->token, commandList[i].name) == 0)
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
        freeTokenList(); //Free token array 
    }


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
    tokenList_t *ptr = tokenList->next;
    while(ptr != NULL)
    {
        printf("%s\n",ptr->token);
        ptr = ptr->next;
    }

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


void tokenizer()
{
    char buff[MAX_INPUT]; //To store the line buffer into
    int bytes; //Number of bytes from the buffer
    int isCommand = 1; //To keep track of |s and to set a new command for args
    tokenList_t *command = tokenList; //The command for the args following it which is a pointer to the command in the list
    char holder[MAX_INPUT]; //Holder string to keep track of the current token being made
    memset(holder, 0, sizeof(holder)); //Make sure the string holder is all 0's so no weird data
    int holderSpot = 0; //To increment the holder spot

    fflush(STDIN_FILENO); //Make sure std in is empty before getting input
    
    //ALTER TO CHECK IF AN ARGUMENT PASSED IS A FILE TO READ FROM AND CHANGE THE INPUT TO THAT FILE AND NOT STDIN
    
    while((bytes = read(STDIN_FILENO, buff, MAX_INPUT)) > 0) //Will read the line given in iteract mode
    {
        for(int i = 0; i < bytes; i++) //Goes through every byte read
        {
            if(buff[i] != ' ' && buff[i] != '|' && buff[i] != '>' && buff[i] != '<' && buff[i] != '\n') //Maybe make into a function if more are needed; to check which character are not to be 
            {
                holder[holderSpot++] = (char)buff[i];
            }
            else if(buff[i] == '|' || buff[i] == '>' || buff[i] == '<') //Moving to new command so set new command to next token
            {
                isCommand = 1; //Next token should be a command
                holder[holderSpot] = buff[i];
                if(tokenList == NULL) //In case: | <> is the first input
                {
                    command = tokenList;
                    addToken(holder, command); //Adding the thing to the list
                    alterAndSetCommand(&command);
                } //To make | < > point to its self, take away else statement and take away the extra second add token
                else{
                    addToken(holder, command); //Adding the thing to the list
                }    
                holderSpot = 0; 
                memset(holder, 0, sizeof(holder)); //Reset the holder string
                holder[0] = -1;
                     //Probably change how to identify is string is empty :) (Yes I copy and pasted this, sue me)
                }
            else if(holder[0] !=-1 || i == bytes) //If the buffer has a space, or buffer is not empty, finish the token and move to the next one
            {   
                
                holder[holderSpot] = '\0'; //Completing the strings; Not sure if actually needed
                addToken(holder, command); //Adding token to the list with the possible wrong command
                holderSpot = 0; 
                memset(holder, 0, sizeof(holder)); //Reset the holder string
                holder[0] = -1; //Probably change how to identify is string is empty :)
                if(isCommand == 1) //This token should be a command and tokens after should point to it
                {
                    //SHOULD ONLY BE CALLED AFTER A TOKEN HAS BEEN ADDED
                    alterAndSetCommand(&command); //Needs to set the token's command pointer to the current node being made in the list done through addressing
                    isCommand = 0; //Will not make a new command until a | < > has been reached
                }
                
                

            }
        }
        break;
    }


};

//Read commandline arguments 
void batchTokenizer(int argc, char **argv)
{
    int isCommand = 1; //To keep track of |s and to set a new command for args
    tokenList_t *command = tokenList; //The command for the args following it which is a pointer to the command in the list
    char holder[MAX_INPUT]; //Holder string to keep track of the current token being made
    memset(holder, 0, sizeof(holder)); //Make sure the string holder is all 0's so no weird data
    int holderSpot = 0; //To increment the holder spot
   
    for(int argNum = 1; argNum < argc; argNum++)//Goes through every commandline arg
    {
        //ADD A CHECK TO SEE IF ARGV IS A FILE AND IF IT IS PASS IT OFF TO TOKENIZER
        //CHECK TO SEE IF YOU CAN OPEN A FILE BY SAID NAME

        for(int i = 0; i <= strlen(argv[argNum]); i++) //Goes through every spot in each argument string
        {
            if(argv[argNum][i] != ' ' && argv[argNum][i] != '|' && argv[argNum][i] != '>' && argv[argNum][i] != '<' && argv[argNum][i] != '\n' && i != strlen(argv[argNum])) //Maybe make into a function if more are needed; to check which character are not to be 
            {
                holder[holderSpot++] = (char)argv[argNum][i];
            }
            else if(argv[argNum][i] == '|' || argv[argNum][i] == '>' || argv[argNum][i] == '<') //Moving to new command so set new command to next token
            {
                isCommand = 1; //Next token should be a command
                holder[holderSpot] = argv[argNum][i];
                if(tokenList == NULL) //In case: | <> is the first input
                {
                    command = tokenList;
                    addToken(holder, command); //Adding the thing to the list
                    alterAndSetCommand(&command);
                } //To make | < > point to its self, take away else statement and take away the extra second add token
                else{
                    addToken(holder, command); //Adding the thing to the list
                }    
                holderSpot = 0; 
                memset(holder, 0, sizeof(holder)); //Reset the holder string
                holder[0] = -1;
                     //Probably change how to identify is string is empty :) (Yes I copy and pasted this, sue me)
                }
            else if(holder[0] !=-1 || i == strlen(argv[argNum]-1)) //If the buffer has a space, or buffer is not empty, finish the token and move to the next one
            {   
                
                holder[holderSpot] = '\0'; //Completing the strings; Not sure if actually needed
                addToken(holder, command); //Adding token to the list with the possible wrong command
                holderSpot = 0; 
                memset(holder, 0, sizeof(holder)); //Reset the holder string
                holder[0] = -1; //Probably change how to identify is string is empty :)
                if(isCommand == 1) //This token should be a command and tokens after should point to it
                {
                    //SHOULD ONLY BE CALLED AFTER A TOKEN HAS BEEN ADDED
                    alterAndSetCommand(&command); //Needs to set the token's command pointer to the current node being made in the list done through addressing
                    isCommand = 0; //Will not make a new command until a | < > has been reached
                }
                
                

            }
        }
        
    }


};


//Need to set the command pointer when a new command is tokenized. Since it has to be the node currently being created, will use double pointers and set the memory spot outside of scope
//Will only be called when a new command has already added to the token list
void alterAndSetCommand(tokenList_t **c)
{
    if(*c == NULL) //Checking to see if this is the first command being made
    {
        *c = tokenList; //Set to the first token
    }
    else{
        while((*c)->next != NULL) //Going through the token list until the last one which was currently added
        {
            *c = (*c)->next; //Also sets the command out side of this scope as well
        }
    }
    (*c)->command = *c; //Setting the last token's command to itself
}


//Adding token to the end of the list; Token will always be insterted at the end 
//Everything in a single function since all the tokenlist needs to do is add tokens one by one start from empty every iteration
void addToken(char *token, tokenList_t *command)
{
    tokenList_t *ptr;
    tokenList_t *tListP = NULL;
    if(tokenList == NULL)
    {
        tokenList = (tokenList_t*)malloc(sizeof(tokenList_t));
        ptr = tokenList;
    }
    else
    {
        tListP = tokenList;
        while(tListP->next != NULL) //Find the next spot to add token in list
        {
            tListP = tListP->next;
        }
        ptr = (tokenList_t*)malloc(sizeof(tokenList_t));//Allocating size for the tokenList spot
    }
    
    
    

    ptr->token = calloc(strlen(token)+1, 1); //Allocating space for string; +1 for terminator; 1 for size of items; initialized to 0 for strings
    strcpy(ptr->token, token); //Copying string into the spot allocated

    //memcpy(ptr->token, token, strlen(token)+1);

    ptr->command = command;

    ptr->next = NULL; //
    if(tListP != NULL)
    {
        tListP->next = ptr;
    }
}


void freeTokenList() //Need to free the strings allocated inside of each tokenList
{
    
    if(tokenList == NULL) //There were no tokens at all
    {
        return;
    }

    while(tokenList != NULL)
    {
        tokenList_t *temp = tokenList;
        free(temp->token);
        tokenList = tokenList->next;
        free(temp);
    }

}




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

