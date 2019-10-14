#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <string.h>

void reset_tab(char* tab[10]){//reinitiliser le tableau donnee en paramettre
for (int i=0;i<10;i++)
      tab[i]=(char *)0;

}

int Lire_commande(char* commande,char* tab[10],char* sep){
    int i=0;
    char* token;
    reset_tab(tab);
    char* temp = calloc(strlen(commande)+1, sizeof(char));
    strcpy(temp, commande);

    token = strtok(temp, sep);
    
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
    return 0;      
}

int execcmdredirect(char* cmd,char* fichier,int mode,int cl){
  char* com[10];
  Lire_commande(cmd,com," ");
  int f;
  switch (fork())
  {
  case -1 :
      perror("fork"); 
      exit(-1);
  case 0 :
      f=open(fichier,mode);
      close(cl);
      dup(f);
      execvp(com[0],com);
      close(f);
  default : 
      while(wait(NULL)!=-1);
  }   
  printf("ok\n");
  
  return 0;
}

int execligneredirect(char* cmd)
{
  int nbl;
  int nbr;
  int nbp;
  int retour=0;
  char* redil[10];
  char* redir[10];
  char* point[10];
  char* nomfichier[10];
  nbl=Lire_commande(cmd,redil,"<");
  for (int j=0;j<nbl;j++)
  {
    nbr=Lire_commande(redil[j],redir,">");
    for(int l=0;l<nbr-1;l++)
    {
      retour=1;
      nbp=Lire_commande(redir[l],point,".");
      if(nbp>1)
      {
        Lire_commande(redir[l],nomfichier," ");//supprimer les espace du fichier txt pour eviter les erreurs
        execcmdredirect(redir[l+1],nomfichier[0], O_RDONLY,0);
      }else
      {
        nbp=Lire_commande(redir[l+1],point,".");
        if(nbp>1){
          Lire_commande(redir[l+1],nomfichier," ");//supprimer les espace du fichier txt pour eviter les erreurs
          execcmdredirect(redir[l],nomfichier[0], O_WRONLY,1);
        }else{
          printf("erreur redirection >\n");
        }
      }
    }
    if(j<nbl-1)
    {
      retour=1;
      char* redir2[10];
      Lire_commande(redil[j+1],redir2,">");
      nbp=Lire_commande(redir[nbr-1],point,".");
      
      if(nbp>1)
      {
        Lire_commande(redir[nbr-1],nomfichier," ");
        execcmdredirect(redir2[0],nomfichier[0],O_WRONLY,1);
      }else
      {
        nbp=Lire_commande(redir2[0],point,".");
        if(nbp>1)
        {
          Lire_commande(redir2[0],nomfichier," ");
          execcmdredirect(redir[nbr-1], nomfichier[0],O_RDONLY,0);
        }else
        {
          printf("erreur redirection <\n");
        }
      }
    }
  }
  return retour;       
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


int execcmdnextpipe(char* com[10],int f0[2]){//ps ax|ls|ps ax
  int f1[2];
  pipe(f1);
  int dip=fork();

  switch (dip){
    case -1 :
        perror("fork"); 
        exit(-1);
    case 0 :
      if(f0[0]!=0)
        {
          close(0);
          dup(f0[0]);
          close(f0[1]);
          close(f0[0]);
        }
      close(1);
      dup(f1[1]);
      close(f1[1]);
      close(f1[0]);
      execvp(com[0],com);

    default : ;  
    if(f0[0]!=0)
    {
      close(f0[0]); 
      close(f0[1]);
    }
  }

  f0[0]=f1[0];
  f0[1]=f1[1];
  return 0; 
}    
int execcmdlastpipe(char* com[10],int f0[2]){

  switch (fork()){
    case -1 :
        perror("fork"); 
        exit(-1);
    case 0 :
        if(f0[0]!=0)
        {
          close(0);
          dup(f0[0]);
          close(f0[1]);
          close(f0[0]);
        }
        execvp(com[0],com);
          
    default :;
  }
  return 0;
}    

int execlignepipe(char* cmd,int f0[2],_Bool nextpipe){
 
  char* com[10];-

  Lire_commande(cmd,com," ");
  if (strcmp(com[0], "cd")==0){
   if(f0[0]!=0)
        {
          close(f0[1]);
          close(f0[0]);
          f0[0]=0;
	  f0[1]=1;}
   
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
  }else if(strcmp(com[0], "getenv")==0){
    if (com[1]!=(char *)0 && com[2]==(char *)0 )
    {
      char* var=getenv(com[1]);
      if(var!=NULL){
        printf("%s\n",var);
      }else{
        printf("la variable d'environnement '%s' n'est pas définie!",com[1]);
      }
    }else
    {
      printf("erreur d'argument pour la fonction getenv!");
    }
  }else if(strcmp(com[0], "setenv")==0)
  {
    if (com[1]==(char *)0)
    {
      com[0]="env";
      if(nextpipe)
      {
        execcmdnextpipe(com,f0);
      }else
      {
      execcmdlastpipe(com,f0);
      }
    }else if(com[3]==(char *)0 && com[2]!=(char *)0)
    {
      setenv(com[1],com[2],1);
    } else{
      printf("erreur de parametre %s",com[0]);
    }
    
  }
  else
  {
    if(nextpipe){
      execcmdnextpipe(com,f0);
    }else
    {
      execcmdlastpipe(com,f0);
    }
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
  return 0;
}
int execor(char* cmd){
  char *com[10];
  int nb_or;
  nb_or=Lire_commande(cmd,com,"||");
  for (int i=0;i<nb_or;i++){
        //execligne(com[i]);
        
  }
  return 0;
}
int execpip(char* cmd){
  char *com[10];
  int nb_pip;
  int f[2] = { 0, 1 };
  nb_pip=Lire_commande(cmd,com,"|");//ps ax | ls
  for (int i=0;i<nb_pip;i++){
    //printf(">%s\n",com[i]);
    execlignepipe(com[i],f,i<nb_pip-1);
      
  }
  
  if(f[0]!=0){
    close(f[0]);
    close(f[1]);
  }
  while(wait(NULL)!=-1);     
  return 0;
}
int execpvirg(char* cmd){
  char *com[10];
    int nb_pvirg;
    nb_pvirg=Lire_commande(cmd,com,";");
  for (int i=0;i<nb_pvirg;i++){
    
       if(execligneredirect(com[i])==0){
         execpip(com[i]);
       }

  }
  return 0;
}
void mon_shell(){
    int no, status,nbpipe;
    char cmd[100];
    char *com[10];
     signal(SIGINT,SIG_IGN);
    printf("Voici mon shell, taper Q pour sortir\n");
    printf("> ");
    gets(cmd);
    while (strcmp(cmd,"Q")!=0){
        execpvirg(cmd);
          
          printf("\n> ");
          
          gets(cmd);
    }
}

int main(int argc)
{
    mon_shell();
    return 0;
}
