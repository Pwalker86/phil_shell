#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

void phil_shell_loop();
char *phil_shell_read_line(void);
char **phil_shell_split_line(char *line);
int phil_shell_execute(char **args);
int phil_shell_launch(char **args);

int main() {
    phil_shell_loop();
    return EXIT_SUCCESS;
}

void phil_shell_loop(void) {
    char *line;
    char **args;
    int status;
    
    do {
        printf("$> ");
        line = phil_shell_read_line();
        args = phil_shell_split_line(line);
        status = phil_shell_execute(args);
        
        free(line);
        free(args);
    } while (status);
    
}

#define PHIL_SHELL_RL_BUFSIZE 1024

char *phil_shell_read_line(void) {
    int bufsize = PHIL_SHELL_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    
    if (!buffer) {
        fprintf(stderr, "phil_shell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        // Read a character
        c = getchar();
        
        // If we hit EOF, replace it with a null character and return.
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        
        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += PHIL_SHELL_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "phil_shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define PSHP_TOK_BUFSIZE 64
#define PSHP_TOK_DELIM " \t\r\n\a"

char **phil_shell_split_line(char *line) {
    int bufsize = PSHP_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;
    
    if (!tokens) {
        fprintf(stderr, "pshp: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(line, PSHP_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        
        if (position >= bufsize) {
            bufsize += PSHP_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "pshp: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        
        token = strtok(NULL, PSHP_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/*
 Function Declarations for builtin shell commands:
 */
int pshp_cd(char **args);
int pshp_help(char **args);
int pshp_exit(char **args);
/*
 List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[])(char **) = {
    &pshp_cd,
    &pshp_help,
    &pshp_exit
};

int pshp_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

/*
 Builtin function implementations.
 */
int pshp_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "pshp: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("pshp");
        }
    }
    return 1;
}

int pshp_help(char **args) {
    int i;
    printf("Stephen Brennan's PSHP\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");
    
    for (i = 0; i < pshp_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    
    printf("Use the man command for information on other programs.\n");
    return 1;
}

int pshp_exit(char **args) {
    return 0;
}

int phil_shell_launch(char **args) {
    pid_t pid, wpid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("pshp");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("pshp");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}

int phil_shell_execute(char **args) {
    int i;
    
    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }
    
    for (i = 0; i < pshp_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    
    return phil_shell_launch(args);
}
