#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define PS1             "Simple-Shell$ "
#define LINE_LENGTH     1024
#define NUM_WORDS   16
#define SEPARATORS  " \t\r\n\a"

void exitShell(){

    //TODO: Liberar memoria dinámica

    exit(0);
}

void  INThandler(int sig)
{
     char  c;

     signal(sig, SIG_IGN);
     printf("\nDesea Salir de la Shell? [Y/n] ");
     c = getchar();
     if (c == 'y' || c == 'Y'|| c == '\n')

         exitShell();

     else signal(SIGINT, INThandler);
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
    scanf("%[^\n]", line);
    getc(stdin);
    return line;
}

int commandExist(char** cmd, int create){// cmd[0] no nos sirve; cmd[1] nombre del nuevo comando; cmd[2] tipo de comando; cmd[3] repeticion cada x s; cmd[4] tiempo total;
    FILE* archivo;
    archivo = fopen ("comandos", "r+");
    if (archivo==NULL) {fputs ("File error",stderr); exit (1);}
    char* buffer = (char*) malloc(50 * sizeof(char));
    while(fgets(buffer,50,archivo)!= NULL){
        char** line = splitLine(buffer);
        if(cmd[1] != NULL && !strcmp(line[0],cmd[1]) && create) return 1;
        if(cmd[0] != NULL && !strcmp(line[0],cmd[0]) && !create){
            execCommand(line);
            return 1;
        }
    }
    fclose(archivo);
    return 0;
}

void launchProgram(char** cmd) {
    if(cmd[0] != NULL){ // Entrada distinta de null
        if (strcmp(cmd[0], "exit") == 0) exitShell();
        if (strcmp(cmd[0], "cmdmonset") == 0){
            if(!commandExist(cmd,1)) createCommand(cmd);//Consultamos si el comando existe (una funcion booleana (con int por el maldito c)), y si existe se reemplaza
            //Si es que no existe lo creamos 
            exitShell();
        }
    }
    //Debemos revisar si vamos a ejecutar un comando creado
    int pid = fork();

    if (pid < 0) {
        // Fork falló
        fprintf(stderr, "Fallo al intentar llamar a fork()");
    } else if (pid == 0) {
        // Proceso hijo
        printf("\033[0;96m");
        fflush(stdout);
        execvp(cmd[0], cmd);
        fprintf(stderr, "%s: Comando no encontrado\n", cmd[0]);
        exit(0);
    } else {
        // Proceso padre
        wait(NULL);
        printf("\033[0;93m");
        fflush(stdout);
    }
    //return;
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
        launchProgram(cmd);
    }
}




void createCommand(char** cmd){
    FILE* archivo;
    archivo = fopen ("comandos", "a+");
    if (archivo==NULL) {fputs ("File error",stderr); exit (1);}
    printf("%s\n", cmd[2]);
    fprintf(archivo,"%s %s %s %s\n", cmd[1],cmd[2],cmd[3],cmd[4]);
    fclose(archivo);
}



int main(){

    signal(SIGINT, INThandler);

    printf("\033[0;93m");

    while(1){

        char* line = getCommandLine();

        char** cmd = splitLine(line);
        if(!commandExist(cmd,0)) launchProgram(cmd);
    }

    return 0;
}
