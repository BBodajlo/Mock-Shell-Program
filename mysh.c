#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


#define MAX_INPUT 128 //Max size of input
#define MAX_TOKENS 20 //Max amount of tokens on a single line to parse
#define READ_BUFFER 1
#define TOKEN_INIT 8

static int isInitialized = 0; //Static variable to keep track of if the shell has been initialized


//TokenList linked list
typedef struct tokenList{
   char *token;
   struct tokenList *command;
   struct tokenList *next;
   struct tokenList *prev;
}tokenList_t;



// Variables Main Needs 
int exitStatus = 0;
char programPath[MAX_INPUT] = "mysh"; 
tokenList_t *tokenList; //General token list
int batchMode = 0; //To stop the execution loop
int batch = 0; //For tokenizer to know that batch mode is being used and to keep reading the file
int input; //Var for file descriptor for input in tokenizer
int errStatus = 0;
tokenList_t *previousToken = NULL;

// Functions 
void initialize();
void shellExit();
void echo();
void pwd();
void cd();
void tokenizer(int argc, char **argv);
void addToken(char *token, tokenList_t *command);
void freeTokenList();
void alterAndSetCommand(tokenList_t **c);
void batchTokenizer(int argc, char **argv);
int executeTokens();
char* findPath(char *command);

//Testing functions
void echoSyn();
void echoCommand();
void echoNext();
void echoPrev();

// Command struct
struct command
{
    const char* name;
    void (*func)();
};



// Command list
struct command commandList[] = {
    {"echo", echo},
    {"exit", shellExit},
    {"pwd", pwd},
    {"cd", cd},
    {"echoSyn", echoSyn},
    {"echoCommand", echoCommand},
    {"echoNext", echoNext},
    {"echoPrev", echoPrev},
};

// Path Options
char *pathOptions[] = {"/usr/local/sbin", "/usr/local/bin", "/usr/sbin", "/usr/bin", "/sbin", "/bin"};


int main (int argc, char** argv)
{
    if(argc < 2)
    {
        initialize(); //This will need to move once we get the batch mode working
    }
    
    char buff[MAX_INPUT]; //Simple start buffer for user input
   
    while(exitStatus == 0 && batchMode == 0) // Will probably just be set to while(1) later after logic is implemented
    {

        if(errStatus && argc < 2){
            printf("!%s> ", programPath); // Errored last time, so give the error prompt
            errStatus = 0;
        }else if(argc < 2){
            printf("%s> ", programPath); 
        }
        
        tokenList = NULL; //Just to make sure tokenList truly is null before using it again

        //Parse input (For interactive mode only atm)
        tokenizer(argc, argv); 
        
        //Execute command
        //echoSyn();
        executeTokens();
        
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

void cd(){
    tokenList_t *ptr = tokenList->next;
    if(ptr == NULL){
        printf("No directory specified");
        return;
    }

    char *dir = ptr->token;
    if(chdir(dir) == -1){ 
        perror("Error changing directory");
    }
}

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
/*Testing functions*/
void echoSyn()
{
    tokenList_t *ptr = tokenList->next;
    while(ptr != NULL)
    {
        printf("T: %s ",ptr->token);
        ptr = ptr->next;
    }
    printf("\n");

}

void echoCommand()
{
    tokenList_t *ptr = tokenList->next;
    while(ptr != NULL)
    {
        printf("%s ",ptr->command->token);
        ptr = ptr->next;
    }
    printf("\n");

}

void echoNext()
{
    tokenList_t *ptr = tokenList;
    while(ptr != NULL)
    {
        if(ptr->next == NULL)
        {
            printf("NULL ");
            ptr = ptr->next;
        }
        else{
            printf("%s ",ptr->next->token);
            ptr = ptr->next;
        }
    }
    printf("\n");

}

void echoPrev()
{
    tokenList_t *ptr = tokenList;
    while(ptr != NULL)
    {
        if(ptr->prev == NULL)
        {
            printf("NULL ");
            ptr = ptr->next;
        }
        else{
            printf("%s ",ptr->prev->token);
            ptr = ptr->next;
        }
    }
    printf("\n");

}



char* specialArgs[] = {"<", ">", "|"};
int executeTokens(){
    tokenList_t *ptr = tokenList; //Pointer to the token list
    while(ptr != NULL){
        
        tokenList_t *command = ptr->command; //Pointer to the command
        
        // Skip special args for now
        if(command == NULL){
            ptr = ptr->next;
            continue;
        }

        // See if command is in command list
        int found = 0;
        for(int i = 0; i < sizeof(commandList)/sizeof(commandList[0]); i++){
            if(strcmp(commandList[i].name, command->token) == 0){
                commandList[i].func();
                found = 1;
                // DO THIS STUFF LATER!
                break;
            }
        }

        if(!found){

            // Run command in child process
            pid_t pid = fork();
            
            if(pid == 0){ // Child process

                // Find path
                char *cmdDir = findPath(command->token);
                if(cmdDir == NULL){
                    perror("Command not found");
                    exit(EXIT_FAILURE);
                }

                // Make path
                char *cmdPath = malloc(strlen(cmdDir) + strlen(command->token) + 2);
                strcpy(cmdPath, cmdDir);
                strcat(cmdPath, "/");
                strcat(cmdPath, command->token);

                // Make arg list
                tokenList_t *tempptr = ptr;
                int count = 0;
                while(tempptr != NULL && tempptr->command == command){
                    count++;
                    tempptr = tempptr->next;
                }

                char *args[count+1];
                for(int i = 0; i < count; i++){
                    args[i] = ptr->token;
                    ptr = ptr->next;
                }
                args[count] = NULL;

                // Execute command                
                int status = execv(cmdPath, args);                
                free(cmdPath);

                if(status == -1){
                    perror("Error executing command");
                    exit(EXIT_FAILURE);
                }

                exit(EXIT_SUCCESS);

            }else if(pid > 0){ // Parent process
                // Wait for child to finish
                int status;
                waitpid(pid, &status, 0);

                if(WEXITSTATUS(status) == EXIT_FAILURE){
                    errStatus = 1;
                }
                
            }else{
                printf("Error forking process");
                return EXIT_FAILURE;
            }

        }

        // Move to next command
        while(ptr != NULL && ptr->command == command){
            ptr = ptr->next;
        }
    }

    return 0;
}

char* findPath(char* commandName){

    // Go through path options, use stat, find if file is exe
    for(int i = 0; i < sizeof(pathOptions)/sizeof(pathOptions[0]); i++){
        struct stat fileInfo;
        char *myPath = pathOptions[i];
        char *command = commandName;
        char *newPath = malloc(strlen(myPath) + strlen(command) + 2);

        // Check if malloc failed
        if(newPath == NULL){
            printf("Error allocating memory for path");
            return NULL;
        }

        // Zeros
        memset(newPath, 0, strlen(myPath) + strlen(command) + 2);

        // Append command to path
        strcat(newPath, myPath);
        strcat(newPath, "/");
        strcat(newPath, command);
        
        if(stat(newPath, &fileInfo) == -1){
            // No file found
            //printf("File %s not found... Countinuing\n", newPath);
            free(newPath);
            continue;
        }

        // See if file is executable
        if(fileInfo.st_mode & S_IXUSR){
            // File is executable
            free(newPath);
            return pathOptions[i];
        }

        // Dont continue if not executable
        //printf("File %s is not executable\n", newPath);
        free(newPath);
        return NULL;

    }

    return NULL;
}

/*Used when a token should be comeplete and added to the list */
void pushToken(tokenList_t **command, char *holder)
{
    
    if(tokenList == NULL) //In case: | <> is the first input
    {
     *command = tokenList;
     addToken(holder, *command); //Adding the thing to the list
     alterAndSetCommand(command);
    }
    else{
        addToken(holder, *command); //Adding the thing to the list
    } 
}
/*Used to clear the string that holds the constructed token*/
void clearHolder(int *holderSpot, char *holder)
{
    *holderSpot = 0; 
    memset(holder, 0, sizeof(holder)); //Reset the holder string
    holder[0] = -1;
}
/*Gets rid of garbage associated with realloc*/
void memsetBuffer(char *holder, int holderspot, int holderBufferSize)
{
    for(int i = holderspot; i < holderBufferSize; i++) //Starts at the spot directly after the content already added
    {
        holder[i] = 0;
    }
}



void tokenizer(int argc, char **argv)
{

    char buff[READ_BUFFER]; //To store the line buffer into
    int bytes; //Number of bytes from the buffer
    int isCommand = 1; //To keep track of |s and to set a new command for args
    tokenList_t *command = tokenList; //The command for the args following it which is a pointer to the command in the list
    //char holder[MAX_INPUT]; //Holder string to keep track of the current token being made
    char *holder = malloc(sizeof(char)*TOKEN_INIT); //Holder string to keep track of the current token being made
    int holderBufferSize = TOKEN_INIT;
    memset(holder, 0, sizeof(holder)); //Make sure the string holder is all 0's so no weird data
    holder[0] = -1; //So empty input is not accepted
    int holderSpot = 0; //To increment the holder spot
    int quoteTracker = 0; 
    int escapeTracker = 0;
    previousToken = NULL; // Resetting previous token for new stream
    
    

    
    
    if(argc < 2) //Checking to see if the program was given arguments meaning batch mode was employed
    {
        input = STDIN_FILENO; //If no arguments, standard input is the input
        fflush(STDIN_FILENO); //Make sure std in is empty before getting input
    }
    else if(batch == 0){
        batch = 1; //Telling the tokeizer loop to coninute reading the file given instead of reopening it
        if(access(argv[1], F_OK) == 0) //Probably instill some error checking
        {
            input = open(argv[1], O_RDONLY);
            //printf("here");
        }
        else{
            printf("Could not open file %s\n", argv[1]); //Place holder error message to not opening file
            free(tokenList); //Free the token list for no seg fault
            exit(EXIT_FAILURE);
        }
    }
    
    while((bytes = read(input, buff, READ_BUFFER)) > 0) 
    {
          //  if((char)buff[0] == '\n')
          //  {
          //      printf("true\n");
          //  }
          //  else{
           //     printf("false\n");
           // }
           // printf("char: %d\n",buff[0]);
            //printf("Escape Traker == %d\n", escapeTracker);
           //If the \ is seen, the next character should be added no matter what
            if(((buff[0] != ' ' && buff[0] != '|' && buff[0] != '>' && buff[0] != '<' && buff[0] != '\n' && buff[0] != '*' && buff[0] != '\n') || quoteTracker == 1) || (escapeTracker == 1 && buff[0] != '\n')) //Only add valid, nonspecial chars; technically reading past entered bytes, so don't add the last one
            {
                
                if((char)buff[0] == '"' && quoteTracker == 0 && escapeTracker == 0) //First occurence of a quote
                {
                    quoteTracker = 1; //When set to one, everything will be added to one token
                }
                else if((char)buff[0] == '"' && quoteTracker == 1 && escapeTracker == 0) //The second occurence of a quote
                {
                    quoteTracker = 0; 
                    pushToken(&command, holder); //Push the entire token made from the quotes
                    clearHolder(&holderSpot, holder);
                }
                else if((char)buff[0] == '\\' && escapeTracker == 0) //If the \ is reached, then the tracker is set and next character will be added to token
                {
                    //printf("Here in escape\n");
                    escapeTracker = 1;
                }
                else if(buff[0] != 13)
                {
                   // printf("here in push\n");
                    escapeTracker = 0; //After the escape tracker is set, the next character should be added here and this is set to 0 to stop that
                    //Logically should be holderSpot ==  holderBufferSize -1 but weird issue with it adding garbage data at holderspot == 7
                    //Will make the buffer bigger by 2x each time it needs to be increased
                    if(holderSpot ==  holderBufferSize -1) 
                    {
                        holder = realloc(holder, holderBufferSize*2); 
                        holderBufferSize = holderBufferSize*2;
                        holder[holderSpot++] = (char)buff[0];
                        memsetBuffer(holder, holderSpot, holderBufferSize); //Getting rid of garbage data from realloc
                    }
                    else{
                    holder[holderSpot++] = (char)buff[0];
                    }
                }
                
            }
            //Special character cases:
            //If in the middle of a token, push current token, then push special character
            //If just special character add it
            //Both set is command to 1 to look for a new command
            else if(buff[0] == '|' || buff[0] == '>' || buff[0] == '<' || buff[0] == '*') 
            {
                isCommand = 1; //Next token should be a command
                if(holder[0] == -1) //Looking for a special character as the first and only argument
                {
                    holder[holderSpot] = buff[0];
                    pushToken(&command, holder);      
                    clearHolder(&holderSpot, holder);
                }
                else{
                    pushToken(&command, holder); //current token is not a special character and needs to be pushed
                    clearHolder(&holderSpot, holder);
                     
                    holder[holderSpot] = buff[0]; //adding the special character next

                    pushToken(&command, holder);     
                    clearHolder(&holderSpot, holder);
                    }
                }
            //Token needs to be pushed at every space or end of the line
            //Buffer should contain an actual character to be pushed i.e. not -1
            //Should not push if in the middle of a quote and it sees a space or another else
            //Should push if seeing the second quote no matter what
            else if((((buff[0] == '\n' && escapeTracker == 0)|| buff[0] == ' ') && holder[0] !=-1 && quoteTracker == 0) || ((char)buff[0] == '"' && quoteTracker == 1)) 
            {   
                //holder[holderSpot] = '\0'; //Completing the strings; Not sure if actually needed
                addToken(holder, command); //Adding token to the list with the possible wrong command
                clearHolder(&holderSpot, holder);
                if(isCommand == 1) //This token should be a command and tokens after should point to it
                {
                    //SHOULD ONLY BE CALLED AFTER A TOKEN HAS BEEN ADDED
                    alterAndSetCommand(&command); //Needs to set the token's command pointer to the current node being made in the list done through addressing
                    isCommand = 0; //Will not make a new command until a | < > has been reached
                }
                quoteTracker = 0; //Should always be set to 0 since should only reach when it is 1 and relavent
                
                

            }
        
        //break;

        //In interactive mode, there is always a \n at the end of a command so it should stop reading input
        
        if(buff[0] == '\n' && escapeTracker == 1 && batch == 1){
            escapeTracker = 0;
            //printf("here");
        }
        else if(buff[0] == '\n')
        {
            if(holder[0] != -1 && quoteTracker == 0) //In the case of an escape charactera at the end of a stream, the token should be pushed 
            {
                addToken(holder, command); 
                if(isCommand == 1)
                {
                    alterAndSetCommand(&command); 
                }
            }
            break;
        }
        
    }
    //In batch mode, if at the end of the input the token should be pushed
    //Should be a valid character and not -1
    //Should not push if it was in the middle of a string, so only one "
    if(argc >= 2 && bytes == 0) //The end of a file reaches here; should be batch mode and end of file with 0 bytes
    {
        batchMode = 1; //Indicating to outer execution loop to stop after this command
        if(holder[0] != -1 && quoteTracker == 0) //If there is still a valid token, push it
        {
            addToken(holder, command); 
            if(isCommand == 1)
            {
                alterAndSetCommand(&command); 
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
    ptr->prev = previousToken;
    previousToken = ptr;
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






