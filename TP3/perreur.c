#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void reset_tab(char* tab[10]){//reinitiliser le tableau donnee en paramettre
for (int i=0;i<10;i++)
      tab[i]=(char *)0;

}

int Lire_commande(char* commande,char* tab[10],char* sep){
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





int execcmd(char* com[10],int* f0,int f1[2]){
  pipe(f1);
    switch (fork()){
    case -1 :
        perror("fork");
        exit(-1);
    case 0 :
    printf("test");
        if(f0!=NULL){//probleeeeeeeeeeeeeeme
        close(0);
        dup(f0[0]);
        close(f0[1]);
        close(f0[0]);
        }
        close(1);
        dup(f1[1]);
        execvp(com[0],com);

  default :;}
  return 0;
}
int execligne(char* cmd,int* f0){
    int f1[2];
  char* com[10];
  Lire_commande(cmd,com," ");
  if (strcmp(com[0], "cd")==0){
   if(f0!=NULL)
        {
          close(f0[1]);
          close(f0[0]);
          f0=NULL;}

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
  execcmd(com,f0,f1);
  wait(NULL);
    if(f0!=NULL)
    {
    close(f0[0]);
    close(f0[1]);
    }

  f0=f1;
    }
  return 0;
}


int execand(char* cmd){
char *com[10];
int nb_and;
 nb_and=Lire_commande(cmd,com,"&&");
        for (int i=0;i<nb_and;i++){
             // execligne(com[i]);

        }

}
int execor(char* cmd){
char *com[10];
int nb_or;
 nb_or=Lire_commande(cmd,com,"||");
        for (int i=0;i<nb_or;i++){
              //execligne(com[i]);

        }

}
int execpip(char* cmd){
char *com[10];
int nb_pip;
int* f;
f=NULL;
 nb_pip=Lire_commande(cmd,com,"|");
        for (int i=0;i<nb_pip;i++){
              printf(">%s\n",com[i]);
              execligne(com[i],f);
        }
        if(f!=NULL){
        close(f[0]);
        close(f[1]);
        }
    while(wait(NULL)!=-1);
}
int execpvirg(char* cmd){
char *com[10];
int nb_pvirg;
 nb_pvirg=Lire_commande(cmd,com,";");
        for (int i=0;i<nb_pvirg;i++){
              //printf(">%s\n",com[i]);
              //execligne(com[i]);
              execpip(com[i]);
        }

}

void mon_shell(){
    int no, status,nbpipe;
    char cmd[100];
    char *com[10];
    printf("Voici mon shell, taper Q pour sortir\n");
   printf("> ");
    gets(cmd);
    while (strcmp(cmd,"Q")!=0){
        execpvirg(cmd);

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

