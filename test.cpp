#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

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

void  INThandler(int sig){

     char  c;
     signal(sig, SIG_IGN);
     printf("\nDesea Salir de la Shell? [Y/n] ");
     cin.clear();
     cin>>c;
     cin.clear();
     if (c == 'y' || c == 'Y'|| c == '\n'){
         exitShell();
     }      

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
    int pid = fork();
    int fw;
    if(log==1){
        fw=open("pato.txt",O_APPEND|O_WRONLY);
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
    //return;
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




int main(){

    signal(SIGINT, INThandler);

    printf("\033[0;93m");

    while(1){

        char* line = getCommandLine();
        int flag = 0;
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

    return 0;
}
