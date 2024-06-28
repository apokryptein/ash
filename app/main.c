#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include<sys/wait.h>

#define TOKEN_DELIM " \t\r\n\a\""
#define MAX_PATH 4096

typedef struct Command {
  char*  CommandName;
  size_t CommandLen;
} Command;

char* builtins[] = {
  "exit",
  "echo",
  "type",
  "pwd",
  "cd"
};

int numBuiltIns = sizeof(builtins) / sizeof(builtins[0]);

char* createFilePath(char* path, char* fileName);
char* inPath(char* command);
void executeCommand(char* command, char* args[]);

int main() {
  while (true) {
    printf("$ ");
    fflush(stdout);

    // Wait for user input
    char* input = NULL;
    size_t inputLen = 0;
    getline(&input, &inputLen, stdin);

    // continue if empty
    if (strlen(input) == 1) {
      continue;
    }

    char* token = strtok(input, TOKEN_DELIM);
    int numTokens = 0;
    char* command[inputLen];

    int i = 0;
    while (token != NULL) {
      command[i++] = token;
      token = strtok(NULL, TOKEN_DELIM);
      numTokens++;
    }
    command[i] = NULL;

    // EXIT
    if (!strcmp(command[0], "exit") && !strcmp(command[1], "0")) {
      return 0;
    }

    // ECHO
    if (!strcmp(command[0], "echo")) {
      for (int i = 1; i < numTokens; i++) {
        if (i == (numTokens - 1)) {
          printf("%s\n", command[i]);
    } else {
          printf("%s ", command[i]);
        }
      }
      continue;
    }

    // TYPE
    if (!strcmp(command[0], "type")) {
      int stat = 0;
      for (int i = 0; i < numBuiltIns; i++) {
        if (!strcmp(builtins[i], command[1])) {
          printf("%s is a shell builtin\n", command[1]);
          stat = 1;
        }
      }

      if (stat == 1) {
        continue;
      }

      char* commandInPath = inPath(command[1]);
      if (commandInPath != NULL) {
        printf("%s is %s\n", command[1], commandInPath);
        free(commandInPath);
        continue;
      }

      printf("%s: not found\n", command[1]);
      continue;
    }

    // PWD
    if (!strcmp(command[0], "pwd")) {
      char pwd[MAX_PATH];
      if (getcwd(pwd, sizeof(pwd)) != NULL) {
        printf("%s\n", pwd);
        continue;
      }
    }

    // CD
    if (!strcmp(command[0], "cd")) {
      char* homeDir = getenv("HOME");
      
      if (numTokens == 1) {
        if (chdir(homeDir) != 0) {
          printf("cd: %s: No such file or directory\n", homeDir);
        }
        continue;
      }
      if (!strcmp(command[1], "~")){
        if (chdir(homeDir)  != 0) {
          printf("cd: %s: No such file or directory\n", homeDir);
        }
        continue;
      }

      if (chdir(command[1]) != 0) {
        printf("cd: %s: No such file or directory\n", command[1]);
      }
      continue;
    }

    // EXEC COMMAND
    char* commandInPath = inPath(command[0]);
    if (commandInPath != NULL) {
      // execute the command
      executeCommand(commandInPath, command);

      fflush(stdout);
      free(commandInPath);
      continue;
    }
    
    // COMMAND NOT FOUND
    printf("%s: command not found\n", command[0]);
  }

  return 0;
}

char* createFilePath(char* path, char* fileName) {
  char* fullPath = malloc(strlen(path) + strlen(fileName) +2);
  sprintf(fullPath, "%s/%s", path, fileName);
  return fullPath;
}

char* inPath(char* command) {
  char* path = getenv("PATH");
  char* dPath = strdup(path);

  char* pathItems = strtok(dPath, ":");
  
  while (pathItems != NULL) {
    char* fullPath = createFilePath(pathItems, command);
    if (access(fullPath, X_OK) == 0) {
      return fullPath;
    }
    pathItems = strtok(NULL, ":");
  }

  free(dPath);

  return NULL;
}

void executeCommand(char* command, char* args[]) {
  pid_t pid = fork();
  if (pid == 0) {
    if(execvp(command, args) == -1) {
      perror("execvp");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("fork");
  } else {
    wait(NULL);
  }
}
