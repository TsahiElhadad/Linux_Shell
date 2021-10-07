// Tsahi Elhadad 206214165
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#define PROMPT "$ "
#define MAX_INPUT_USER 100
#define PATH_MAX 1024

// types of commands that user can pressed
typedef enum Commands{
    JOBS,
    HISTORY,
    CD,
    EXIT,
    OTHER
} Commands;

// struct that configure information about the user command
typedef struct info{
    char inputUser[MAX_INPUT_USER]; // the user input
    int isBackgroundSign; // flag that check if background sign '&' is pressed - 1 for pressed and else 0 
    pid_t pid; // the process id that run the command
} info;

static char previousPath[PATH_MAX]; // static variable that keep the previous path cwd for cd - command.

// function that implement the cd command.
cdCommand(char** argInput, int numOfArg){
    int flag = 0;
    if(numOfArg > 2) { // too many arguments
        printf("%s\n","Too many argument");
    }
    else {
        char pathCWD[PATH_MAX];
        getcwd(pathCWD,PATH_MAX); // get current cwd Path

        if(!strcmp(argInput[1],"~")) { // case '~'
            char homeDirectory[PATH_MAX];
            // pathDirectory = getcwd()
            strcpy(homeDirectory,getenv("HOME")); // get home directory path
            if(homeDirectory != NULL) { // if env succses
                if (chdir(homeDirectory) != 0) {
                    printf("%s\n","An error occurred");
                    flag = 1;
                }
            }
        }
        else if(!strcmp(argInput[1],"-")) { // case '-'
            if(!strcmp(previousPath, "")) { // OLDpwd not set
                printf("%s\n","An error occurred");
                flag = 1;
            }
            else if (chdir(previousPath) != 0) { // go to previous path
                printf("%s\n","An error occurred");
                flag = 1;
            }
        }
        else{ // regular case or '..'
            if (chdir(argInput[1]) != 0) { // go to previous path
                printf("%s\n","An error occurred");
                flag = 1;
            }
        }
        if(!flag){ // if cd command was valid
            strcpy(previousPath,pathCWD); // copy the older CWD to previous
        }
    }

}

// Function that implement the exit command.
void exitCommand(info infoCommands[MAX_INPUT_USER], int numCommand){
    int i = 0;
    int status;
    for(;i < numCommand; i++) { // kill all the processes that run.
        if(waitpid(infoCommands[i].pid, &status,WNOHANG) == 0){ // check if process is running
            if(kill(infoCommands[i].pid, SIGKILL) == -1){
                printf("%s\n", "An error occurred");
                exit(0);
            }
        }
    }
    exit(0);
}

// Function that handle with quotation and removed them for echo command
void handleQuotation(char strArg[], int lenStr) {
    char *pSrc, *pDst;
    for (pSrc = pDst = strArg; *pSrc != '\0'; pSrc++) { // pass over the characters of the string to remove the '"'
        *pDst = *pSrc;
        if (*pDst != '"'){
             pDst++;
        }
    }
    *pDst = '\0';
}

// Function that implement the commands the is no builtIn, create new child process and exec the command.
void otherCommand(char** argCommand, int numberOfArg, int ifBackground, info* infoCommands, int numCommand){
    int status;
    char* command = argCommand[0]; // command user is first
    pid_t pid;
    if(!strcmp(command,"echo")){
        int i;
        for(i = 1; i < numberOfArg; i++){
            handleQuotation(argCommand[i], strlen(argCommand[i]));
        }
    }
    if(ifBackground){ // need to delete sigh '&'
        argCommand[numberOfArg - 1] = '\0';
    }
    if((pid = fork()) == 0) { // child process
        if(execvp(command,argCommand) == - 1) {
            printf("%s\n","exec failed");
            exit(0);
        }
    }else if(pid < 0){ // fork failed
        printf("%s\n","fork failed");
        exit(0);
    }
    else { // father process
        if(!ifBackground) { // need to wait to child process
            if(waitpid(pid, &status, 0) != pid){
                printf("%s\n","An error occurred");
                exit(0);
            }
        }
    }

    infoCommands[numCommand].pid = pid; // set the pid of the current process that run the command.
}

// Function that implement the jobs command, pass ove the array of struct commands and print the runing processes.
void jobsCommand(info* infoCommands, int numCommands){
    int i = 0;
    int status;
    for(; i < numCommands; i++){
        if(infoCommands[i].pid != -1 && infoCommands[i].isBackgroundSign == 1)
            if(waitpid(infoCommands[i].pid, &status,WNOHANG) == 0){ // check if process is running
                printf("%s\n",infoCommands[i].inputUser);
            }
    }
}

// Function that execute the history command, pass over the array of commands and print to all the commands
// that pressed by user to the screen by her status - RUNNING OR DONE
void historyCommand(info infoCommands[MAX_INPUT_USER],int numCommand){
    int i = 0;
    int status;
    for(; i < numCommand; i++){
        // if command is initial and maybe run in the background
        if(infoCommands[i].pid != -1 && infoCommands[i].isBackgroundSign == 1){
        
            if(waitpid(infoCommands[i].pid, &status,WNOHANG) == 0){ // check if process is still running
                printf("%s %s\n",infoCommands[i].inputUser, "RUNNING");
            }
            else{ // process not running
                printf("%s %s\n",infoCommands[i].inputUser, "DONE");
            }
        }else{ // for sure the command done
            printf("%s %s\n",infoCommands[i].inputUser, "DONE");
        }
    }
    printf("%s %s\n",infoCommands[numCommand].inputUser, "RUNNING");
}

// Function that check which command was pressed and returns her by enum
Commands getNameCommand(char* userCommand){
    if(userCommand == NULL){
        printf("%s\n", "An error occurred");
        exit(1);
    }
    Commands command = OTHER;
    if(strcmp(userCommand,"jobs") == 0){
        command = JOBS;
    }
    else if(strcmp(userCommand,"history") == 0){
        command = HISTORY;
    }
    else if(strcmp(userCommand,"cd") == 0){
        command = CD;
    }
    else if(strcmp(userCommand,"exit") == 0){
        command = EXIT;
    }
    return command;
}

// Funciton that will execute the command the user pressed
int execUserCommand(char** argCommand, int numberOfArg, int ifBackground
                                            , info infoCommands[MAX_INPUT_USER], int numCommand){
    int flag = 1; // if exit pressed the flag replaced to 0 and we done
    Commands command = getNameCommand(argCommand[0]); // return the command name by enum
    switch(command){
        // built in commands
        case JOBS:
            jobsCommand(infoCommands, numCommand);
            break;
        case HISTORY:
            historyCommand(infoCommands,numCommand);
            break;
        case CD:
            cdCommand(argCommand,numberOfArg);
            break;
        case EXIT:
            exitCommand(infoCommands, numCommand);
            flag = 0;
            break;
        // non built in commands like: ls, sleep, echo
        case OTHER: 
            otherCommand(argCommand, numberOfArg, ifBackground, infoCommands, numCommand);
            break;
        default:
            break;
    }
    return flag;
}

/*
Function that check if sign '&' is typed by user for background process.
if typed return 1, else return 0.
*/
int isBackgroundProcess(char** argInputParse, int numOfArgs){
    if(strcmp(argInputParse[numOfArgs - 1],"&") == 0){
        return 1;
    }
    return 0;
}

/*
Function that Will parse the user input to saparate arguments by " " into arpInputParse array of strings.
*/
int parseUserInput(char** argInputParse,char* userInput){
    int i = 0;
    char* token = strtok(userInput," "); // parse by " "
    while(token != NULL){
        argInputParse[i] = token;
        token = strtok(NULL, " "); // move to next argument after " "
        ++i;
    }
    return i;
}

// Funciton that intial the array of struct
intialJobs(info infoCommands[MAX_INPUT_USER], int numberOfOrders){
    int i = 0;
    for(; i < numberOfOrders; i++){
        strcpy(infoCommands[i].inputUser,"");
        // infoCommands[i].isRunning = 0;
        infoCommands[i].pid = 0;
        infoCommands[i].isBackgroundSign = 0;
    }
}

int main(int argc, char const *argv[])
{
    int numCommand = 0; // count the commands
    char inputUser[MAX_INPUT_USER]; // the user input command
    char *argInputParse[MAX_INPUT_USER]; // parse the user command to array of strings
    int numberOfArgs = 0; // number of arguments that user pressed include the command itself
    int ifBackground = 0; // flag that check if background sign '&' is pressed 
    info infoCommands[MAX_INPUT_USER]; // array of commands for all the program
    intialJobs(infoCommands, MAX_INPUT_USER); // initial the array commands
    int flag = 1; // flag to check if exit pressed
    while(flag){
        printf(PROMPT);
        fflush(stdout); // if not using '/n'
        memset(argInputParse,'\0', sizeof(argInputParse)); // reset
        fgets(inputUser,sizeof(inputUser),stdin); // gets user command to inputUser
        inputUser[strlen(inputUser) - 1] = '\0'; // replace the '\n'
        if(!strcmp(inputUser,"")){ // if user press enter.
            continue;
        }
        char cpyInputUser[MAX_INPUT_USER]; // copy for delete the sign '&'
        strcpy(cpyInputUser, inputUser); // set copy of the input user
        numberOfArgs = parseUserInput(argInputParse,inputUser); // parse the user command to arguments
        ifBackground = isBackgroundProcess(argInputParse,numberOfArgs); // check if sign '&' background typed
        Commands command = getNameCommand(argInputParse[0]); // get the commands name
        if(ifBackground){
            cpyInputUser[strlen(cpyInputUser) - 1] = '\0'; // remove the sign '&'
            infoCommands[numCommand].isBackgroundSign = 1; // set background info 1 in the struct
        }
        strcpy(infoCommands[numCommand].inputUser,cpyInputUser); // set user input in the struct
        flag = execUserCommand(argInputParse, numberOfArgs,ifBackground, infoCommands, numCommand);
        numCommand++; // counter of command ++
   }
    return 0;
}
 
