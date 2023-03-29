#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <dirent.h>
#include <fnmatch.h>
#include <errno.h>


#define MAX_INPUT 128 //Max size of input
#define MAX_TOKENS 20 //Max amount of tokens on a single line to parse
#define READ_BUFFER 1
#define TOKEN_INIT 8
#define PWD_INIT 128

static int isInitialized = 0; //Static variable to keep track of if the shell has been initialized


//TokenList linked list
typedef struct tokenList{
   char *token;
   struct tokenList *command;
   struct tokenList *next;
   struct tokenList *prev;
}tokenList_t;

typedef struct wildcardList{
    char *token;
    struct wildcardList *next;
}wildcardList_t;



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
int shellExit(tokenList_t *command);
int pwd(tokenList_t *command, int in, int out);
int cd(tokenList_t *command, int in, int out);

void tokenizer(int argc, char **argv);
void addToken(char *token, tokenList_t *command);
void freeTokenList();
void alterAndSetCommand(tokenList_t **c);
void checkExit();
int executeTokens(tokenList_t *start, tokenList_t *end, int in, int out);
void wildcardHandler();
char* findPathCommand(char *command);

//Testing functions
int echoSyn();
int echoCommand();
int echoNext();
int echoPrev();
int executeTestCommands();
int testFile = 0;
// Command struct
struct command
{
    const char* name;
    int (*func)();
};


// Command list
struct command commandList[] = {
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

        if (tokenList == NULL)
        {
            continue;
        }
        
        // Handle Wildcards 
        wildcardHandler();

        //Execute command
        //echoSyn();
        
        // Reach end of token list to send that ptr
        tokenList_t *ptr = tokenList;
        while(ptr->next != NULL)
        {
            ptr = ptr->next;
        }
        
        // Check if exit was called
        checkExit();
        // Execute
        

        //Checking if test file is being run
        if(argc > 1)
        {
            if(strcmp(argv[1], "SyntaxTest.txt") == 0) //It is the test file
            {
                testFile = 1;
            }
            else{
                testFile = 0;
            }
            
        }
        
        
        if(testFile == 0)
        {
            int cmdStatus = executeTokens(tokenList, ptr, STDIN_FILENO, STDOUT_FILENO);
            if (cmdStatus == 1)
            {
                errStatus = 1;
            }
        }
        //If test file is found for Syntax, only run functions to show syntax of tokens, processes and redirection will not work for that file
        else{
            executeTestCommands();
        }
        // Free token list
        freeTokenList(); 
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

int shellExit(tokenList_t *command)
{
    exitStatus = 1;
    return 0; 
};

void echo()
{
    tokenList_t *ptr = tokenList->next;
    while(ptr != NULL)
    {
        printf("%s\n",ptr->token);
        ptr = ptr->next;
    }
    free(ptr);

};

int cd(tokenList_t *command, int in, int out){
    
    tokenList_t *ptr = command->next;
    if(ptr == NULL){
        perror("No Directory Specified");
        return 1;
    }

    char *dir = ptr->token;
    if(chdir(dir) == -1){ 
        perror("Error Changing Directory");
        return 1;
    }

    return 0;
}

int pwd(tokenList_t *command, int in, int out)
{
    //char cwd[255]; //Think of a size
    int cwdSize = PWD_INIT;
    char* cwd = malloc(sizeof(char)*PWD_INIT);

    // Get current working directory, realloc if needed
    while(getcwd(cwd, cwdSize) == NULL)
    {
        if (errno == ENOMEM){
            perror("Error getting current working directory");
            free(cwd);
            return 1;
        }

        if (errno == ERANGE){
            // Buffer too small, realloc and try again
            cwdSize *= 2;
            cwd = realloc(cwd, sizeof(char)*cwdSize);

            if(cwd == NULL){
                perror("Error reallocating memory for current working directory");
                free(cwd);
                return 1;
            }
        }
        
    }

    // write to stdout
    write(out, cwd, strlen(cwd));
    write(out, "\n", 1);
    free(cwd);
    
    return 0;

};
/*Testing functions*/
int echoSyn()
{
    tokenList_t *ptr = tokenList->next;
    while(ptr != NULL)
    {
        printf("T: %s ",ptr->token);
        ptr = ptr->next;
    }
    printf("\n");
    free(ptr);
    return 0;

}

int echoCommand()
{
    tokenList_t *ptr = tokenList->next;
    while(ptr != NULL)
    {
        printf("%s ",ptr->command->token);
        ptr = ptr->next;
    }
    printf("\n");
    free(ptr);
    return 0;

}

int echoNext()
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
    free(ptr);
    return 0;

}

int echoPrev()
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
    free(ptr);
    return 0;

}

int executeTestCommands()
{
    tokenList_t *command = tokenList->command; //Pointer to the command

    for(int i = 0; i < sizeof(commandList)/sizeof(commandList[0]); i++){
        if(strcmp(commandList[i].name, command->token) == 0){
            int success = commandList[i].func(command);
            return success;
        }
    }
    free(command);
}

void wildcardHandler(){
    tokenList_t *ptr = tokenList;

    while(ptr != NULL){
        // new list to hold wildcard results
        wildcardList_t *wildList = NULL;

        // Check if wildcard is present in given string
        if(strchr(ptr->token, '*') != NULL){
            // Wildcard found
            
            // Using working directory
            char *dir = ".";
            DIR *d;
            struct dirent *dirInfo;
            d = opendir(dir);
            
            // Check if failed
            if(d == NULL){
                perror("Error opening directory");
                return;
            }

            // Loop through directory
            wildcardList_t *previousWild = NULL;

            dirInfo = readdir(d);
            
            while(dirInfo != NULL){
                // Check if file matches wildcard
                // Allows for * and ?* wildcards
                if (fnmatch(ptr->token, dirInfo->d_name, FNM_PERIOD) == 0) { // fnmatch(pattern, string, flags)
                    // File matches wildcard
                    //printf("Match Found!\n");

                    // Create new wildcard list
                    wildcardList_t *newWildList = malloc(sizeof(wildcardList_t));
                    newWildList->next = NULL;

                    // Copy name 
                    newWildList->token = malloc((strlen(dirInfo->d_name) + 1) * sizeof(char));
                    strcpy(newWildList->token, dirInfo->d_name);

                    // Add to list
                    if(wildList == NULL){
                        wildList = newWildList;
                    }else{
                        previousWild->next = newWildList;
                    }
                    previousWild = newWildList;

                }

                // Get next file
                dirInfo = readdir(d);
            }

            // Close directory
            closedir(d);
            
        }

        // Check if wildcard list is empty
        if(wildList == NULL){
            // No wildcards found
            ptr = ptr->next;
            continue;
        }

        // Convert wildcard list to token list
        wildcardList_t *wildPtr = wildList;
        tokenList_t *wildTokens = NULL;
        tokenList_t *previousWildToken = NULL;

        while(wildPtr != NULL){
            // Create new token
            tokenList_t *newToken = malloc(sizeof(tokenList_t));
            newToken->next = NULL;
            newToken->prev = NULL;
            newToken->command = NULL;

            // Copy name 
            newToken->token = malloc((strlen(wildPtr->token) + 1) * sizeof(char));
            strcpy(newToken->token, wildPtr->token);

            // Add to list
            if(wildTokens == NULL){
                wildTokens = newToken;
            }else{
                previousWildToken->next = newToken;
                newToken->prev = previousWildToken;
            }
            previousWildToken = newToken;

            // Get next wildcard
            wildPtr = wildPtr->next;
        }

        // See if ptr is a command 
        if(ptr->command == ptr){
            // First wildcard is the command, every other wildcard points to first
            wildTokens->command = wildTokens;
            
            tokenList_t *tokenWildPtr = wildTokens->next;
            while(tokenWildPtr != NULL){
                tokenWildPtr->command = wildTokens;
                tokenWildPtr = tokenWildPtr->next;
            }

            // Also change all ptr->command to wildTokens->command if they are the same
            tokenList_t *tokenPtr = ptr->next;
            while(tokenPtr != NULL && tokenPtr->command == ptr){
                tokenPtr->command = wildTokens;
                tokenPtr = tokenPtr->next;
            }
            
        }else{
            // Put wild commands as ptr->command
            tokenList_t *tokenWildPtr = wildTokens;
            while(tokenWildPtr != NULL){
                tokenWildPtr->command = ptr->command;
                tokenWildPtr = tokenWildPtr->next;
            }
        }

        // Set prev
        wildTokens->prev = ptr->prev;
        if(ptr->prev != NULL){
            ptr->prev->next = wildTokens;
        }

        // Set next
        tokenList_t *lastWildToken = wildTokens;
        while(lastWildToken->next != NULL){
            lastWildToken = lastWildToken->next;
        }
        lastWildToken->next = ptr->next;
        if(ptr->next != NULL){
            ptr->next->prev = lastWildToken;
        }

        // If ptr is the first token, change tokenList
        if(ptr == tokenList){
            tokenList = wildTokens;
        }

        // Free ptr
        free(ptr->token);
        free(ptr);

        // Free wildcard list
        wildcardList_t *wildFreePtr = wildList;
        while(wildFreePtr != NULL){
            wildcardList_t *temp = wildFreePtr;
            wildFreePtr = wildFreePtr->next;
            free(temp->token);
            free(temp);
        }

        // Get next token
        ptr = wildTokens->next;

    }
}

int executeCommand(tokenList_t *tokenListStartPtr, tokenList_t *tokenListEndPtr, int in, int out){
    
    // Redirects
    // Create args AND handle redirects 
    char **args = malloc(sizeof(char*));
    args[0] = tokenListStartPtr->token;
    int argCount = 1;
    tokenList_t *argPtr = tokenListStartPtr->next;
    int lastWasRedirect = 0;
    char* lastRedirectIn = NULL;
    char* lastRedirectOut = NULL;

    while(argPtr != NULL && argPtr != tokenListEndPtr && tokenListStartPtr != tokenListEndPtr){

        // Check if redirect
        if (strcmp(argPtr->token, "<") == 0 || strcmp(argPtr->token, ">") == 0)
        {

            // Log redirect
            if(strcmp(argPtr->token, "<") == 0){
                lastRedirectIn = argPtr->next->token;
            }else{
                lastRedirectOut = argPtr->next->token;
            }

            // Skip
            argPtr = argPtr->next;
            lastWasRedirect = 1;

            // Skip again
            if (argPtr != tokenListEndPtr)
            {
                argPtr = argPtr->next;
                //printf("Skipping %s", argPtr->token);
                lastWasRedirect = 0;
            }
        }else{
            // Add to args
            args = realloc(args, (argCount + 1) * sizeof(char*));
            args[argCount] = argPtr->token;
            argCount++;

            // Get next arg
            argPtr = argPtr->next;
        }
    }

    // Do for tokenlistend
    if(tokenListEndPtr != tokenListStartPtr){
        if (strcmp(tokenListEndPtr->token, "<") == 0 || strcmp(tokenListEndPtr->token, ">") == 0)
        {
            // Not valid
            perror("No File Redirected");
            return 1;

        }else{
            // Add to args unless prev is < or >
            if (lastWasRedirect == 0)
            {
                args = realloc(args, (argCount + 1) * sizeof(char*));
                args[argCount] = tokenListEndPtr->token;
                argCount++;
            }
        }
    }

    // Add null to end of args
    args = realloc(args, (argCount + 1) * sizeof(char*));
    args[argCount] = NULL;

    // Handle redirects
    if(lastRedirectIn != NULL){
        // Open file
        in = open(lastRedirectIn, O_RDONLY);
        if(in < 0){
            perror("Error opening file");
            return 1;
        }
    }

    if(lastRedirectOut != NULL){
        // Open file
        out = open(lastRedirectOut, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if(out < 0){
            perror("Error opening file");
            return 1;
        }
    }

    // Check if command was exit
    if(strcmp(tokenListStartPtr->token, "exit") == 0){

        // Free args
        free(args);

        // Close files
        if(in != 0){
            close(in);
        }
        if(out != 1){
            close(out);
        }

        return 0; // already handled
    }

    // Check if command is builtin
    tokenList_t *command = tokenListStartPtr->command; //Pointer to the command

    for(int i = 0; i < sizeof(commandList)/sizeof(commandList[0]); i++){
        if(strcmp(commandList[i].name, command->token) == 0){
            int success = commandList[i].func(tokenListStartPtr, in, out);

            // Free args
            free(args);

            // Close files
            if(in != 0){
                close(in);
            }
            if(out != 1){
                close(out);
            }

            return success;
        }
    }

    // Check if command is in path (contains /)
    char* commandPath = NULL;
    //char pathInDir[255];
    int dirSize = -1;
    char* pathInDir = malloc(sizeof(char)*PWD_INIT);
    
    if(strchr(tokenListStartPtr->token, '/') != NULL){
        // Get current path 
        dirSize = PWD_INIT;
        while(getcwd(pathInDir, dirSize) == NULL)
        {
            // Safety check
            if (errno == ENOMEM){
                perror("Error getting current working directory");
                free(pathInDir);
                return 1;
            }

            // If too small buffer, realloc and try again
            if (errno == ERANGE){
                dirSize *= 2;
                pathInDir = realloc(pathInDir, sizeof(char)*dirSize);

                if(pathInDir == NULL){
                    perror("Error reallocating memory for current working directory");
                    free(pathInDir);
                    return 1;
                }
            }
        }

        commandPath = pathInDir;
    }


    // Check if command is in pre-listed paths
    if(commandPath == NULL){
        commandPath = findPathCommand(tokenListStartPtr->token);
    }

    // No command was found, so return
    if(commandPath == NULL){
        // Command not found
        perror("Command not found");
        return 1;
    }

    // Add command to path
    char *newPath = malloc(strlen(commandPath) + strlen(tokenListStartPtr->token) + 2);
    strcpy(newPath, commandPath);
    strcat(newPath, "/");
    strcat(newPath, tokenListStartPtr->token);

    free(pathInDir);

    // Now we can actually execute which sounds fun
    // Fork
    pid_t pid = fork();
    if(pid == 0){
        // Child
        // Execute
        dup2(in, 0);
        dup2(out, 1);

        execv(newPath, args);

        // Free
        free(args);
        free(newPath);

        perror("Error executing command");
        return 1;
    }else if(pid < 0){
        // Error
        // Free
        free(args);
        free(newPath);

        perror("Error forking");
        return 1;
    }else{
        // Parent
        // Wait for child
        int status;
        waitpid(pid, &status, 0);

        if(status != 0){
            perror(newPath);
            return 1;
        }

        if (in != 0)
        {
            close(in);
        }

        if (out != 1)
        {
            close(out);
        }

    }

    // Free 
    free(args);
    free(newPath);

    return 0;
    
}

void checkExit(){
    tokenList_t* ptr = tokenList;
    while(ptr != NULL){
        if(strcmp(ptr->token, "exit") == 0){
            // Exit called
            shellExit(NULL);
            break;
        }

        ptr = ptr->next;
        // We dont need to remove, it will just be a dead pipe (returning 0 in the command execution)

    }
}

int executeTokens(tokenList_t *tokenListStartPtr, tokenList_t *tokenListEndPtr, int in, int out){
    
    // Just print out tokens for now
    tokenList_t *ptr = tokenListStartPtr;
    // printf("Tokens: ");
    // while(ptr != NULL){
    //     printf("[%s] ", ptr->token);
    //     ptr = ptr->next;
    // }   
    // puts("");

    // Go through tokens for pipes
    ptr = tokenListEndPtr;
    tokenList_t *pipePtr = NULL;
    while (ptr != NULL)
    {
        // Check if pipe
        if(strcmp(ptr->token, "|") == 0){
            // Pipe found
            pipePtr = ptr;
            break;
        }

        // Get next token
        ptr = ptr->prev;
    }

    // Check if pipe found
    if(pipePtr == NULL){ // kinda the base case
        // No pipe found, execute normally
        int status = executeCommand(tokenListStartPtr, tokenListEndPtr, in, out);
        return status;
    }else{
        // Pipe found, execute with pipe

        // Create pipe
        int pipefd[2];
        if(pipe(pipefd) == -1){
            perror("Error creating pipe");
            return 1;
        }

        // Fork
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error forking");
            return 1;
        } else if (pid == 0) {
            // Child
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            int status = executeTokens(tokenListStartPtr, pipePtr->prev,in, pipefd[1]);
            close(pipefd[1]);
            exit(status);
        }else{
            // Parent
            close(pipefd[1]);

            int status = executeCommand(pipePtr->next, tokenListEndPtr, pipefd[0], out);
            close(pipefd[0]);

            int childStatus;
            waitpid(pid, &childStatus, 0);

            if(status != 0){
                return status;
            }

            if(childStatus != 0){
                return childStatus;
            }

            return 0;
        }        
        
    }

    return 0;
    
}

char* findPathCommand(char* commandName){

    // Go through path options, use stat, find if file is exe
    for(int i = 0; i < sizeof(pathOptions)/sizeof(pathOptions[0]); i++){
        struct stat fileInfo;
        char *myPath = pathOptions[i];
        char *command = commandName;
        char *newPath = malloc(strlen(myPath) + strlen(command) + 2);

        // Check if malloc failed
        if(newPath == NULL){
            perror("Error allocating memory for path");
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
            free(holder); //Free the holder string for no seg fault
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
            if(((buff[0] != ' ' && buff[0] != '|' && buff[0] != '>' && buff[0] != '<' && buff[0] != '\n' && buff[0] != '\n') || quoteTracker == 1) || (escapeTracker == 1 && buff[0] != '\n')) //Only add valid, nonspecial chars; technically reading past entered bytes, so don't add the last one
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
            else if(buff[0] == '|' || buff[0] == '>' || buff[0] == '<') 
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

    free(holder); //Freeing the holder buffer

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