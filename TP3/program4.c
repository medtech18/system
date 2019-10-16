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
char * split_string( char *s, const char *delimiter )//fonction recuperer du site stackoverflow.com elle fait le meme fonctionnement que strtock sauf que ca prend en charge une chaine de carractere comme delimiteur 
{                                                    // direct link :https://stackoverflow.com/questions/34675966/strtok-when-the-delimiter-is-string
    static char *p;

    if ( s ) p = s;

    size_t n = strlen( delimiter );

    if ( p && n != 0 )
    {
        while ( memcmp( p, delimiter, n ) == 0 ) p += n;
    }

    if ( !p || !*p ) return p = NULL;

    char *t = p;

    if ( n != 0 ) p = strstr( t, delimiter );

    if ( n == 0 || !p )
    {
        p = t + strlen( t );
    }
    else
    {
        *p = '\0';
        p += n;
    }

    return t;
}

int Lire_commande(char* commande,char* tab[10],char* sep){
    int i=0;
    char* token;
    reset_tab(tab);
    char* temp = calloc(strlen(commande)+1, sizeof(char));
    strcpy(temp, commande);

    token = split_string(temp, sep);
    
    while(token!=NULL){
        tab[i]=token;
        i++;
        token = split_string(NULL, sep);
    }
    
    return i;
}








int execcmdnextpipe(char* com[10],int f0[2]){//cette fonction execute la commande et cree un pipe pour la prochaine commande
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
          if(f0[1]!=1)
          close(f0[1]);
          close(f0[0]);
        }
        
      close(1);
      dup(f1[1]);
      close(f1[1]);
      close(f1[0]);
       if(execvp(com[0],com)!=0){
          perror("erreur commande\n");
          exit(-1); 
      }

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
int execcmdlastpipe(char* com[10],int f0[2],int f){//cette fonction execute la derniere commande avec le pipe f0 si il existe sinon ca sera une execution sans pipe normal
  
  switch (fork()){
    case -1 :
        perror("fork"); 
        exit(-1);
    case 0 :
        if(f0[0]!=0)
        {
          close(0);
          dup(f0[0]);
          close(f0[0]);
          if(f0[1]!=1)
          close(f0[1]);
        }
        if(f!=0)
        {
          close(1);
          dup(f);
          close(f);
        }
        if(execvp(com[0],com)!=0){
          perror("erreur commande\n");
          exit(-1); 
      }
          
    default :;
  }
  return 0;
}    

int execlignepipe(char* cmd,int f0[2],_Bool nextpipe,int f1){//fonction execute la commande cmd et determine si cest le dernier pipe en fonction de la valeur de nextpipe passé en parametre
 
  char* com[10];
  int retour =0;
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
       if(chdir(com[1])!=0)
      {
        printf("erreur commande cd dossier inexistant\n");
        retour=1;
      }
    }
    else 
    {
      perror("trop d'argument pour cd\n");
      retour=1;
    } 
  }
  else if(strcmp(com[0], "getenv")==0){
    if (com[1]!=(char *)0 && com[2]==(char *)0 )
    {
      char* var=getenv(com[1]);
      if(var!=NULL){
        printf("%s\n",var);
      }else{
        printf("la variable d'environnement '%s' n'est pas définie!",com[1]);
        retour=1;
      }
    }else
    {
      printf("erreur d'argument pour la fonction getenv!");
      retour=1;
    }
  }
  else if(strcmp(com[0], "setenv")==0)
  {
    if (com[1]==(char *)0)
    {
      com[0]="env";
      if(nextpipe)
      {
        execcmdnextpipe(com,f0);
      }else
      {
      execcmdlastpipe(com,f0,f1);
      }
    }else if(com[3]==(char *)0 && com[2]!=(char *)0)
    {
      if(f0[0]!=0)
        {
          close(f0[1]);
          close(f0[0]);
          f0[0]=0;
	        f0[1]=1;
          }
   
      setenv(com[1],com[2],1);
    } else{
      printf("erreur de parametre %s",com[0]);
      retour=1;
    }
    
  }
  else
  {
    if(nextpipe){
      execcmdnextpipe(com,f0);
    }else
    {
      execcmdlastpipe(com,f0,f1);
    }
  }
    return retour;            
}




int execpip(char* cmd,int f0,int f1){//execute cmd et fait le traitement si il y a un pipe
  char *com[10];
  int nb_pip;
  int vrcpu;
  int vretour=0;
  int f[2] = { f0, 1 };
  nb_pip=Lire_commande(cmd,com,"|");//ps ax | ls
  for (int i=0;i<nb_pip;i++){
    //printf(">%s\n",com[i]);
    if(execlignepipe(com[i],f,i<nb_pip-1,f1)!=0)
      vretour=1;
      
  }
  
  if(f[0]!=0){
    close(f[0]);
    if(f[1]!=1)
    close(f[1]);
  }
  if(f1!=0)
  close(f1);
  while(wait(&vrcpu)!=-1)
    if (vrcpu!=0)
      vretour=1;     
  return vretour;
}

int execcmdredirect(char* cmd,char* fichier,int mode,int cl){//fonction qui execute la commande passer en parametre dans un fichier en mode passer en paramettre et c1 pour definir la lecture ou ecriture 
  char* com[10];
  Lire_commande(cmd,com," ");
  int f,vrcpu;
  int vretour=0;
  int execcmd=0;
  f=open(fichier,mode,0777);
  if(cl==0)
  vretour=execpip(cmd,f,0);
  else
  {
    vretour=execpip(cmd,0,f);
  }
  
  close(f);

  return vretour;
}
int execligneredirect(char* cmd)//execute une commande qui contien des redirection et retour 0 si tout les commande sont bien passé sinon retourne 1
{
  int nbl;
  int nbr;
  int nbp;
  int var=0;
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
      var=1;
      nbp=Lire_commande(redir[l],point,".");
      if(nbp>1)
      {
        Lire_commande(redir[l],nomfichier," ");//supprimer les espace du fichier txt pour eviter les erreurs
        if(execcmdredirect(redir[l+1],nomfichier[0], O_RDONLY,0)!=0)retour=1;
      }else
      {
        nbp=Lire_commande(redir[l+1],point,".");
        if(nbp>1){
          Lire_commande(redir[l+1],nomfichier," ");//supprimer les espace du fichier txt pour eviter les erreurs
          if(execcmdredirect(redir[l],nomfichier[0], O_WRONLY | O_CREAT | O_TRUNC,1)!=0)retour=1;
        }else{
          printf("erreur redirection >\n");
          retour=1;
        }
      }
    }
    if(j<nbl-1)
    {
      var=1;
      char* redir2[10];
      Lire_commande(redil[j+1],redir2,">");
      nbp=Lire_commande(redir[nbr-1],point,".");
      
      if(nbp>1)
      {
        Lire_commande(redir[nbr-1],nomfichier," ");
        if(execcmdredirect(redir2[0],nomfichier[0],O_WRONLY | O_CREAT | O_TRUNC,1)!=0)retour=1;
      }else
      {
        nbp=Lire_commande(redir2[0],point,".");
        if(nbp>1)
        {
          Lire_commande(redir2[0],nomfichier," ");
          if(execcmdredirect(redir[nbr-1], nomfichier[0],O_RDONLY,0)!=0)retour=1;
        }else
        {
          printf("erreur redirection <\n");
          retour=1;
        }
      }
    }
  }
  if (var==0){
    retour=execpip(cmd,0,0);
  }
  return retour;       
}
int execor(char* cmd,int exec){
  char *com[10];
  int nb_or,retour;
  nb_or=Lire_commande(cmd,com,"||");
  int i=0;
  if (exec==0){
    exec=execligneredirect(com[i]); 
  }
  i++;
  while(i<nb_or && exec!=0){
    exec=execligneredirect(com[i]);
    
    i++;
  }
  return exec;
}
int execand(char* cmd){
  char *com[10];
  int nb_and,vrcpu;
  int vretour=0;
  int i=0;
  int exec=0;
  nb_and=Lire_commande(cmd,com,"&&");
   for (int i=0;i<nb_and;i++){
      exec=execor(com[i],exec);
  }
 
  return exec;
}
int execpvirg(char* cmd){
  char *com[10];
    int nb_pvirg;
    nb_pvirg=Lire_commande(cmd,com,";");
  for (int i=0;i<nb_pvirg;i++){
      execand(com[i]);
      
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
