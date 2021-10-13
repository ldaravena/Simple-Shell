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

static volatile sig_atomic_t keep_running = 1;
/*
static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}*/

void  INThandler(int sig)
{
     char  c;

     signal(sig, SIG_IGN);
     printf("\nDesea Salir de la Shell? [Y/n] ");
     c = getchar();
     if (c == 'y' || c == 'Y'|| c == '\n')  //keep_running = 0;

         exit(0);

     else signal(SIGINT, INThandler);
     //getchar(); // Get new line character
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

    /*
     * for (int i = 0; cmd[i] != NULL; i++) {
     *     // Quita las comillas de cada argumento
     *     int len = strlen(cmd[i]);
     * }
     */

    return cmd;
}

char* getCommandLine() {
    char* line = (char*) malloc(LINE_LENGTH * sizeof(char));
    //printf("%d", getpid());
    printf(PS1);
    scanf("%[^\n]", line);
    getc(stdin);
    return line;
}

void launchProgram(char** cmd) {

    if (cmd[0] != NULL && strcmp(cmd[0], "exit") == 0) exit(0);

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
    return;
}

int main(){


    //signal(SIGINT, INThandler);


    printf("\033[0;93m") ;

//   printf("Shell $:\n");

    while(1){

        char* line = getCommandLine();

        char** cmd = splitLine(line);
        launchProgram(cmd);
    }


   // while(1);

    return 0;
}
