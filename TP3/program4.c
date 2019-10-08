#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void reset_tab(char* tab[10])//reinitiliser le tableau donnee en paramettre
{
    for (int i=0;i<10;i++)
        tab[i]=(char *)0;

}

int Lire_commande(char* commande,char* tab[10],char* sep)
{
    int i=0;
    char* token;
    reset_tab(tab);
    token = strtok(commande, sep);
    
    while(token!=NULL){
        tab[i]=token;
        i++;
        token = strtok(NULL, sep);
    }
    return i;
}

int execcmd(char* com[10])
{
    switch (fork())
    {
    case -1 :
        perror("fork"); 
        exit(-1);
    case 0 :
        execvp(com[0],com);
          
    default : 
        while(wait(NULL)!=-1);
    }         
}

int execligne(char* cmd)
{
    char* com[10];
    Lire_commande(cmd,com," ");
    if (strcmp(com[0], "cd")==0)
    {
        if (com[1]==(char *)0)
        {
            chdir(getenv("HOME"));
        }
        else if (com[2]==(char *)0)
        {
            chdir(com[1]);
        }
        else 
        {
            perror("trop d'argument pour cd\n");
        } 
    }
    else
    {
        execcmd(com);
    }
    return 0;            
}

int execand(char* cmd)
{
    char *com[10];
    int nb_and;
    nb_and=Lire_commande(cmd,com,"&&");
    for (int i=0;i<nb_and;i++){
        execligne(com[i]);     
        }
}

int execor(char* cmd)
{
    char *com[10];
    int nb_or;
    nb_or=Lire_commande(cmd,com,"||");
    for (int i=0;i<nb_or;i++){
        execligne(com[i]);         
        }
}

int execpip(char* cmd)
{
    char *com[10];
    int nb_pip;
    nb_pip=Lire_commande(cmd,com,"|");
    for (int i=0;i<nb_pip;i++){
        execligne(com[i]);             
        }
}

void mon_shell()
{
    int no, status,nbpipe;
    char cmd[100];
    char *com[10];
    printf("Voici mon shell, taper Q pour sortir\n");
    printf("> ");
    gets(cmd);
    while (strcmp(cmd,"Q")!=0){
        nbpipe=Lire_commande(cmd,com,"|");
        for (int i=0;i<nbpipe;i++){
        
              execligne(com[i]);
         
          }
          
          printf("\n> ");
          
          gets(cmd);
    }
}
void mafonction(){
    
    int f[2];
    int f1[2];
    pipe(f);
    pipe(f1);
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
}
int main(int argc)
{
    mon_shell();
    return 0;
}