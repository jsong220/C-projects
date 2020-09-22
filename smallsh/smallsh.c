//Name: Myung Jin Song
//Assignment: Program 3 CS344
//child cleanup incomplete

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h> /* segemptyset(), sigfillset(), sigaddset(), sigdelset()*/
#include <fcntl.h> /* for closing file descriptors after calling exec() -  last lecture*/

//forground background variable
int bgAllowed = 1;


void catchSIGTSTP(int signo) {
//use write for reentrancy
  if (bgAllowed == 1) {
    char* message = "Entering foreground-only mode (& is now ignored)\n";
    write(1, message, 49);
    fflush(stdout);
    bgAllowed = 0;
    //isbackground = 0;
  }
//use write for reentrancy
  else {
    char* message = "Exiting foreground-only mode\n";
    write (1, message, 29);
    fflush(stdout);
    bgAllowed = 1;
  }
}

int main (){
//setting up signals
  struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};
//ignore sigint signal
  SIGINT_action.sa_handler = SIG_IGN;
  SIGINT_action.sa_flags = SA_RESTART;
  sigfillset(&SIGINT_action.sa_mask);
//SIGINT_action.sa_flags = 0;

  SIGTSTP_action.sa_handler = catchSIGTSTP;
  SIGTSTP_action.sa_flags = SA_RESTART;
  sigfillset(&SIGTSTP_action.sa_mask);
//SIGTSTP_action.sa_flags = 0;

  sigaction(SIGINT, &SIGINT_action , NULL);
  sigaction(SIGTSTP, &SIGTSTP_action , NULL);

//variables to start main
  int myBash = 1;
  int isbackground = 0;
  int status = 1;
//setting up to store user commands
  int numCharsEntered = -5;
  size_t bufferSize=0;
  char* command = NULL;
  int i=0;

//max of 2048 characters and 512 arguments
  char* cd_dir = NULL;
  int numInputs = 0;
  char *mySentence[2048];
  char *bgCommad[2048] = {0};

//continue running until exit cmd is invoked
  while (myBash){

    printf(":");
    fflush(stdout);

    numCharsEntered = getline(&command, &bufferSize, stdin);
//setting up user input to be used
//get rid of newline that comes with getline
    strtok(command, "\n");
    memset(mySentence,0,sizeof(mySentence));
    memset(bgCommad,0,sizeof(bgCommad));
    char *input_token = strtok(command, " ");
    while (input_token != NULL){
//printf("%s\n", input_token);
      numInputs++;
//printf("%i\n", numInputs);
      mySentence[i++] = input_token;
      input_token = (strtok(NULL, " ")); 
    }
//printf("after all done..%s\n",cd_dir);

//checking to see if background process
    if(strcmp(mySentence[numInputs-1], "&") == 0){
//get rid of background variable so that its not passed onto exec
      int bgctr=0;
      for (bgctr=0; bgctr < numInputs-1; bgctr++){
        bgCommad[bgctr] = mySentence[bgctr];
      }
      isbackground=1;
// printf("it works!\n");
    }


//$$ should expand to process ID
    int process_id = getpid();
    char idBuffer[2048];
    char idBuffer2[] = "$$";
    char idStr[2048];
    char tempIDWord[2048] = {0};

    sprintf(idStr,"%d",process_id);
//printf("this is your pid %s\n",idStr);
//printf("process id: %i\n", process_id); 

    int idctr = 0;
    for (idctr=0; idctr < numInputs; idctr++){
      if (strstr(mySentence[idctr],idBuffer2)){
        int cutSize = strlen(mySentence[idctr]);
        cutSize = cutSize -2;
        strncpy(tempIDWord,mySentence[idctr],cutSize);
   //allocate enough memory for null terminator at end 
        mySentence[idctr] = malloc(cutSize+1);
        strcpy(mySentence[idctr],tempIDWord);
        strcat(mySentence[idctr],idStr);
   //printf("my sentence after done!! %s \n", mySentence[idctr]);
      }
    }



//my built in command choices
//exit - built in cmd
    if (strcmp(command, "exit") == 0){
//printf("exit case \n");
      myBash = 0;
      //exit(0);
    }

//cd built in cmd
    else if (strcmp(command, "cd") == 0){
//printf("cd case \n");

//cd - with no arguments directs to HOME directory
      if (numInputs == 1){
//printf("GO HOME! \n"); 
        chdir(getenv("HOME"));
//printf("%s\n",getcwd(currentDir,2045));
      }

//change directory to argument
      else if (numInputs == 2){
        chdir(mySentence[1]);
//printf("%s\n",getcwd(currentDir,2045));
// printf("after all done..%s\n",cd_dir);
      }

//more than one path shows error msg and no action
      else if (numInputs >= 3){
        printf("cd: too many arguments\n"); 
        fflush(stdout);
      }
    }

//status - built in cmd
    else if (strcmp(command, "status") == 0){
//printf("status case \n");
      printf("exit value: %i\n",status);
      fflush(stdout);

    }

//comments - # and blank spaces are ignored
    else if (strcmp(command, "#") == 0 || mySentence[0] == NULL || mySentence[0] == 0){
//printf("this is a comment or blank space do nothing..\n");
    }

    else if (strcmp(command, "#THIS") == 0){
//printf("this is a comment or blank space do nothing..\n");
    }

//handling non built in cmds with exec
    else{
      //printf("enterting exec functions! %s\n", mySentence[0]); 
      pid_t spawnPid = -5; //the child process ID or 0 or -1
      int childExitStatus = -5;

      spawnPid = fork();
      switch (spawnPid) {
        //for error case
        case -1: { perror("fork error!!\n"); exit(1); break; }
        //child process
        case 0: {

      //in case of foreground process
          if (isbackground == 0 && bgAllowed == 1){
      //taking care of redirection
            char outSource[2048];
            char inSource[2048];
            int inputRedirOn=0;
            int outputRedirOn=0;
            int foo=1;
            for (foo=1; foo < numInputs; foo++){
              if(strcmp(mySentence[foo], ">") == 0){
      //printf("redirection found:foo is:%i \n",foo);
                strcpy(outSource,mySentence[foo+1]);
                outputRedirOn=1;
              }
            }
            int bar=1;
            for (bar=1; bar < numInputs; bar++){
              if(strcmp(mySentence[bar], "<") == 0){
      //printf("redirection found:bar is:%i \n",bar);
                strcpy(inSource,mySentence[bar+1]);
                inputRedirOn=1;
              }
            }
      //in cases with redirection
            if(numInputs >= 3 && outputRedirOn == 1 || inputRedirOn == 1){
              int sourceFD, targetFD, result;

              if(inputRedirOn){
                sourceFD = open(inSource, O_RDONLY);
                if (sourceFD == -1) { perror("source open()"); exit(1); }
      //printf("sourceFD == %d\n", sourceFD); // Written to terminal
                result = dup2(sourceFD, 0);
                if (result == -1) { perror("source dup2()"); exit(2); }
                fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
              }

              if(outputRedirOn){
                targetFD = open(outSource, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (targetFD == -1) { perror("target open()"); exit(1); }
      //printf("targetFD == %d\n", targetFD); // Written to terminal
                result = dup2(targetFD, 1);
                if (result == -1) { perror("target dup2()"); exit(2); }
                fcntl(targetFD, F_SETFD, FD_CLOEXEC);
              }
              //printf("// 245!\n");
              execlp(mySentence[0], mySentence[0], NULL);
              break;
            }

      //if no redirection
            else{
              //printf("// 252!\n");
              execvp(mySentence[0], mySentence);
              perror("error! command does not exist, try again\n");
              if(perror){
                //printf("test\n");
                status=1;
              }
              exit(1); break;
            }
          }

      //in case of background process, pass new array without the "&"
          else if (isbackground == 1 && bgAllowed == 1){
      //taking care of redirection
            char outSource[2048];
            char inSource[2048];
            int inputRedirOn=0;
            int outputRedirOn=0;
            int foo=1;
            for (foo=1; foo < numInputs; foo++){
              if(strcmp(bgCommad[foo], ">") == 0){
      //printf("redirection found:foo is:%i \n",foo);
                strcpy(outSource,bgCommad[foo+1]);
                outputRedirOn=1;
              }
            }
            int bar=1;
            for (bar=1; bar < numInputs; bar++){
              if(strcmp(bgCommad[bar], "<") == 0){
      //printf("redirection found:bar is:%i \n",bar);
                strcpy(inSource,bgCommad[bar+1]);
                inputRedirOn=1;
              }
            }

            if(numInputs >= 3 && outputRedirOn == 1 || inputRedirOn == 1){
              int sourceFD, targetFD, result;

              if(inputRedirOn){
                sourceFD = open(inSource, O_RDONLY);
                if (sourceFD == -1) { perror("source open()"); exit(1); }
      //printf("sourceFD == %d\n", sourceFD); // Written to terminal
                result = dup2(sourceFD, 0);
                if (result == -1) { perror("source dup2()"); exit(2); }
              }

              if(outputRedirOn){
                targetFD = open(outSource, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (targetFD == -1) { perror("target open()"); exit(1); }
      //printf("targetFD == %d\n", targetFD); // Written to terminal
                result = dup2(targetFD, 1);
                if (result == -1) { perror("target dup2()"); exit(2); }
              }
              //printf("// 304!\n");
              execlp(bgCommad[0], bgCommad[0], NULL);
              break;
            }

      //if no redirection
            else{
              //printf("// 311!\n");
              execvp(bgCommad[0], bgCommad);
              perror("error! command does not exist, try again\n");
              if(perror){
                //printf("test\n");
                status=1;
              }
              exit(1); break;
            }
          }


      //in case of foreground process
          if (isbackground == 0 && bgAllowed == 0){
      //taking care of redirection
            char outSource[2048];
            char inSource[2048];
            int inputRedirOn=0;
            int outputRedirOn=0;
            int foo=1;
            for (foo=1; foo < numInputs; foo++){
              if(strcmp(mySentence[foo], ">") == 0){
      //printf("redirection found:foo is:%i \n",foo);
                strcpy(outSource,mySentence[foo+1]);
                outputRedirOn=1;
              }
            }
            int bar=1;
            for (bar=1; bar < numInputs; bar++){
              if(strcmp(mySentence[bar], "<") == 0){
      //printf("redirection found:bar is:%i \n",bar);
                strcpy(inSource,mySentence[bar+1]);
                inputRedirOn=1;
              }
            }
      //in cases with redirection
            if(numInputs >= 3 && outputRedirOn == 1 || inputRedirOn == 1){
              int sourceFD, targetFD, result;

              if(inputRedirOn){
                sourceFD = open(inSource, O_RDONLY);
                if (sourceFD == -1) { perror("source open()"); exit(1); }
      //printf("sourceFD == %d\n", sourceFD); // Written to terminal
                result = dup2(sourceFD, 0);
                if (result == -1) { perror("source dup2()"); exit(2); }
                fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
              }

              if(outputRedirOn){
                targetFD = open(outSource, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (targetFD == -1) { perror("target open()"); exit(1); }
      //printf("targetFD == %d\n", targetFD); // Written to terminal
                result = dup2(targetFD, 1);
                if (result == -1) { perror("target dup2()"); exit(2); }
                fcntl(targetFD, F_SETFD, FD_CLOEXEC);
              }
              //printf("// 366!\n");
              execlp(mySentence[0], mySentence[0], NULL);
              break;
            }

      //if no redirection
            else{
              //printf("// 373!\n");
              execvp(mySentence[0], mySentence);
              perror("error! command does not exist, try again\n");
              if(perror){
                //printf("test\n");
                status=1;
              }
              exit(1); break;
            }
          }

      //in case of background process, pass new array without the "&"
          else if (isbackground == 1 && bgAllowed == 0){
      //taking care of redirection
            //mySentence[numInputs-1] = 0;
            char outSource[2048];
            char inSource[2048];
            int inputRedirOn=0;
            int outputRedirOn=0;
            int foo=1;
            for (foo=1; foo < numInputs; foo++){
              if(strcmp(mySentence[foo], ">") == 0){
      //printf("redirection found:foo is:%i \n",foo);
                strcpy(outSource,mySentence[foo+1]);
                outputRedirOn=1;
              }
            }
            int bar=1;
            for (bar=1; bar < numInputs; bar++){
              if(strcmp(mySentence[bar], "<") == 0){
      //printf("redirection found:bar is:%i \n",bar);
                strcpy(inSource,mySentence[bar+1]);
                inputRedirOn=1;
              }
            }

            if(numInputs >= 3 && outputRedirOn == 1 || inputRedirOn == 1){
              int sourceFD, targetFD, result;

              if(inputRedirOn){
                sourceFD = open(inSource, O_RDONLY);
                if (sourceFD == -1) { perror("source open()"); exit(1); }
      //printf("sourceFD == %d\n", sourceFD); // Written to terminal
                result = dup2(sourceFD, 0);
                if (result == -1) { perror("source dup2()"); exit(2); }
              }

              if(outputRedirOn){
                targetFD = open(outSource, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (targetFD == -1) { perror("target open()"); exit(1); }
      //printf("targetFD == %d\n", targetFD); // Written to terminal
                result = dup2(targetFD, 1);
                if (result == -1) { perror("target dup2()"); exit(2); }
              }
              //printf("// 426!\n");
              execlp(mySentence[0], mySentence[0], NULL);
              break;
            }

      //if no redirection
            else{
              mySentence[numInputs-1] = '\0';
              //printf("// 433!\n");
              execvp(mySentence[0], mySentence);
              perror("error! command does not exist, try again\n");
              if(perror){
                //printf("test\n");
                status=1;
              }
              exit(1); break;
            }
          }



        } 
        //parent process
        default: {
      //if background process, dont wait for it to complete
          if (isbackground == 1 && bgAllowed == 1){
            pid_t actualPid = waitpid(spawnPid, &childExitStatus, WNOHANG);
            printf("background pid is %d\n", spawnPid);
            fflush(stdout);
            isbackground=0;
          }

          else if (isbackground == 0 && bgAllowed == 1){
            pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
          }

          else if (isbackground == 1 && bgAllowed == 0){
            pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
          }

          else{
            pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
      //printf("PARENT(%d): Child(%d) terminated, Exiting!\n", getpid(), actualPid);
      //break;
          }
        }
}
}
//reset variables for next input
i=0;
numInputs=0;
isbackground=0;

}
return 0;
}

