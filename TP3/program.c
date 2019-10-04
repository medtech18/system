#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>


void mon_shell()
{
    int status;   
    switch (fork()){
        case -1 :
           fprintf(stderr, "Erreur de fork\n");
           exit(-1);
        case 0 :
           if (execlp("who","who" ,(char *)0) == -1) {
               perror("execvp");
               exit(-1);
            }
        default :
          wait(NULL);
          chdir(getenv("HOME"));
          
    }  
    switch (fork()){
        case -1 :
           fprintf(stderr, "Erreur de fork\n");
           exit(-1);
        case 0 :
           if (execlp("ls","ls","-l", (char *)0 )== -1) {
               perror("execvp");
               exit(-1);
            }
        default :
          wait(NULL);
          
    }
}

int main(int argc,char *argv[])
{
    mon_shell();
}