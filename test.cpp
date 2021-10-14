#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <setjmp.h>
#include <sys/time.h>

using namespace std;

#define PS1             "Simple-Shell$ "
#define LINE_LENGTH     1024
#define NUM_WORDS   16
#define SEPARATORS  " \t\r\n\a"

void execCommand(char** cmd);
void launchProgram(char** cmd, int log) ;

void exitShell(){

    //TODO: Liberar memoria dinámica

    exit(0);
}

struct command
{
  char **argv;
};

void  INThandler(int sig){

     char * c = (char*)malloc(sizeof(char)*2);
     signal(sig, SIG_IGN);
     printf("\nDesea Salir de la Shell? [y/n] ");
     cin.clear();
     cin.getline(c,2*sizeof(char));
     cin.clear();
  
    if (c[0] == 'y' || c[0] == 'Y'|| c[0] == '\n'){
        exitShell();
     }
    free(c);
    signal(SIGINT, INThandler);
    
    
}

char** splitLine(char* line) {
    char** cmd = (char**) malloc(NUM_WORDS * sizeof(char*));
    size_t max = NUM_WORDS;
    size_t pos = 0;

    cmd[pos++] = strtok(line, SEPARATORS);
    while ((cmd[pos++] = strtok(NULL, SEPARATORS)) != NULL) {
        if (pos == max - 1) {
            max += NUM_WORDS;
            cmd = (char**) realloc(cmd, max * sizeof(char*));
        }
    }

    return cmd;
}

char* getCommandLine() {
    char* line = (char*) malloc(LINE_LENGTH * sizeof(char));
    printf(PS1);
    cin.clear();
    cin.getline(line,LINE_LENGTH*sizeof(char));
    cin.clear();
    return line;
}

int commandExist(char** cmd, int create){// cmd[0] no nos sirve; cmd[1] nombre del nuevo comando; cmd[2] tipo de comando; cmd[3] repeticion cada x s; cmd[4] tiempo total;
    FILE* archivo;
    archivo = fopen ("comandos", "r+");
    if (archivo==NULL) {fputs ("File error",stderr); exit (1);}
    char* buffer = (char*) malloc(50 * sizeof(char));
    
    while(fgets(buffer,50,archivo)!= NULL && cmd[0]!=NULL){
        
        char** line = splitLine(buffer);
        if(cmd[1] != NULL && !strcmp(line[0],cmd[1]) && create){
            fclose(archivo);
            free(buffer);
            return 1;
        } 
        if(cmd[0] != NULL && !strcmp(line[0],cmd[0]) && !create){
            
            execCommand(line);
            free(buffer);
            fclose(archivo);
            return 1;
        }
    }
    free(buffer);
    fclose(archivo);
    return 0;
}

void execCommand(char** cmd){
    int rep = atoi(cmd[3])/atoi(cmd[2]);
    
    int segundos = atoi(cmd[2]);
    cmd[0] = cmd[1];
    cmd[1] = NULL;
    cmd[2] = NULL;
    cmd[3] = NULL;

    for(int i= 0; i<rep; i++){

        sleep(segundos);        
        launchProgram(cmd,1);
    }
}

void launchProgram(char** cmd, int log) {
    pid_t pid = fork();
    int fw;
    if(log==1){
        fw=open("resultado.txt",O_APPEND|O_WRONLY);
    }

    if (pid < 0) {
        // Fork falló
        fprintf(stderr, "Fallo al intentar llamar a fork()");
    } else if (pid == 0) {
        // Proceso hijo
        printf("\033[0;96m");
        fflush(stdout);
        if (log==1){
            dup2(fw,1);
        }
        execvp(cmd[0], cmd);
        fprintf(stderr, "%s: Comando no encontrado\n", cmd[0]);
        exit(0);
    } else {
        // Proceso padre
        wait(NULL);
        printf("\033[0;93m");
        fflush(stdout);
    }

    if(log==1) close(fw);
}

void createCommand(char** cmd){
    FILE* archivo;
    archivo = fopen ("comandos", "a+");
    if (archivo==NULL) {fputs ("File error",stderr); exit (1);}
    if((!strcmp(cmd[2],"vmstat") || !strcmp(cmd[2],"netstat")) && atoi(cmd[3]) <= atoi(cmd[4])){
        fprintf(archivo,"%s %s %s %s\n", cmd[1],cmd[2],cmd[3],cmd[4]);
        printf("COMANDO CREADO\n");
    }
    else{
        printf("ERROR, COMANDO NO CREADO\n");
    }
    fclose(archivo);
}

void doPipe(struct command *args,int npipe){

    if(npipe>2){
        cout << "No soportado, máximo 2 pipes"<<endl;
        return;
    }
    int pipes[4];
    pipe(pipes);
    if(npipe>1)pipe(pipes + 2); 

    pid_t pid = fork();
    pid_t pid2;
    pid_t pid3;

    if (pid == 0){

        printf("\033[0;96m");
        fflush(stdout);
        dup2(pipes[1], 1);// 1PIPE
        
        close(pipes[0]);  // 1PIPE
        close(pipes[1]);  // 1PIPE

        if(npipe>1){    // 2PIPE
            close(pipes[2]); 
            close(pipes[3]);  
        }

        execvp(args[0].argv[0],(char* const*)args[0].argv);
        
    }
    else{
        pid2 = fork();
        if (pid2 == 0){
            dup2(pipes[0], 0);  // 1PIPE
            

            if(npipe>1){ // 2PIPE
                dup2(pipes[3], 1);
                close(pipes[2]);
                close(pipes[3]); 
            }

            close(pipes[0]);  // 1PIPE
            close(pipes[1]);  // 1PIPE

            execvp(args[1].argv[0],(char* const*)args[1].argv);
	    }
        else{
            if(npipe>1){  // 2PIPE
                pid3 = fork();
                if (pid3 == 0){ // 2PIPE
                    dup2(pipes[2], 0);
                
                    close(pipes[2]);
                    close(pipes[3]);
                    close(pipes[0]);
                    close(pipes[1]);

                    execvp(args[2].argv[0],(char* const*)args[2].argv);
                }    
            }
            if(npipe==1){         // 1PIPE
                close(pipes[0]);  
                close(pipes[1]);  
            }
            
	    }
    }

  
    close(pipes[0]);
    close(pipes[1]);
    close(pipes[2]);
    close(pipes[3]);
    
    for (int i= 0; i < npipe+1; i++) {
        wait(NULL);
        
    }
    printf("\033[0;93m");
    kill(pid,SIGKILL);
    if(npipe>1)kill(pid2,SIGKILL);
    //if(npipe>1)kill(pid3,SIGKILL);
    
  }

int isPipe(char* linea){
    int max = 8;
    int posiciones = 0;
    int contador = 1;
    for(int i=0;linea[i]!='\0';i++){
        if(linea[i]=='|'){
            posiciones++;
        }
    }
    return posiciones;
}

command* argsPipe(char * line, int n){
    command * args = (command*) malloc((n+1) * sizeof(command));
    char ** args2 = (char**) malloc(NUM_WORDS * sizeof(char*));
    char * token;
    char * auxLine = line;
    for(int i=0;i<n+1;i++){
       token = strtok(auxLine, "|");
       if(token!=NULL){
            args2[i] = token;
       }
       auxLine = NULL;    
    }
    for(int i=0;i<n+1;i++){
        char** generado = splitLine(args2[i]);
        command aux;
        aux.argv = generado;
        args[i] = aux;
    }

    return args;
}

int main(){

    signal(SIGINT, INThandler);

    printf("\033[0;93m");

    while(1){
        
        char* line = getCommandLine();
        int n = isPipe(line);
        int flag = 0;
        if(!n){
            char** cmd = splitLine(line);
            if(cmd[0]!=0){
    
                if (strcmp(cmd[0], "exit") == 0) exitShell();
                if (strcmp(cmd[0], "cmdmonset") == 0)
                    if(!commandExist(cmd,1)){ 
                        createCommand(cmd);
                        flag = 1;
                    }
                    else{
                        printf("COMANDO EXISTENTE\n");
                        flag = 1;
                    }
            }
            if(!commandExist(cmd,0) && !flag) launchProgram(cmd,0);
        }
        else{
            command * args = argsPipe(line,n);
            doPipe(args,n);
        }
    }
    return 0;
}
