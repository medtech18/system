#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>



int main(int argc)
{
    int f[2];
    pipe(f);
    switch (fork()){
        case -1 :
            perror("fork"); 
            exit(-1);
        case 0 :
            close(1);
            dup(f[1]);
            close(f[1]);
            close(f[0]);
            execlp("ps","ps","ax",(char *)0);
        default :
            int f1[2];
            pipe(f1);
            switch (fork()){
                case -1 :
                    perror("fork");
                    exit(-1);
                case 0 :
                    close(0);
                    close(1);
                    dup(f[0]);
                    dup(f1[1]);
                    close(f[1]);
                    close(f[0]);
                    close(f1[1]);
                    close(f1[0]);
                    execlp("grep","grep","bash",(char *)0);
                default :
                    close(f[0]); 
                    close(f[1]);
                    switch (fork()){
                    case -1 :
                        perror("fork");
                        exit(-1);
                    case 0 :
                        close(0);
                        dup(f1[0]);
                        close(f1[1]);
                        close(f1[0]);
                        execlp("wc","wc","-l",(char *)0);
                    default :
                        close(f1[0]); 
                        close(f1[1]);
                        while (wait(NULL)!=-1);
                    }
            }
    }
    return 0;
}