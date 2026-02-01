#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 3;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}
int list();
int touchme(char *filename);
int makedir(char *dir_name);
int cdme(char *path);
int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int echo(char *string);
int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();    
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);
    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);
    } 
    else if (strcmp(command_args[0], "echo") == 0){
	    if (args_size != 2)
		    return badcommand();
	    return echo(command_args[1]);
    } else if (strcmp(command_args[0], "my_ls") == 0) {
      if (args_size !=1)
	return badcommand();
      return list();
    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
      if (args_size !=2)
	return badcommand();
      return makedir(command_args[1]);
    } else if (strcmp(command_args[0], "my_touch") == 0) {
      if (args_size !=2)
	return badcommand();
      return (touchme(command_args[1]));
    } else if (strcmp(command_args[0], "my_cd") == 0) {
      if (args_size !=2)
	return badcommand();
      return (cdme(command_args[1]));
    }
    else
        return badcommand();
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n \
echo STRING		Prints string \
my_ls                   Lists all files and folders in the current directory \
my_mkdir dirname creates a new directory with the name dirname in the current directory.";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}


int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}

int source(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT - 1, p);
    while (1) {
        errCode = parseInput(line);     // which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT - 1, p);
    }

    fclose(p);

    return errCode;
}

int echo(char *string){
        int errCode = 0;
        if (string[0] == '$'){
                if (strlen(string) == 1){
                        printf("\n");
                        errCode = 1;
                }
                else
                {
                        int len = strlen(string);
                        char var[len-1];
                        strncpy(var, string+1, len-1);
                        char *val = mem_get_value(var);
                        if(strcmp(val, "Variable does not exist") == 0){
                                printf("\n");
                                errCode = 2;
                        }
                        else{
                                printf("%s\n", val);
                        }
                }
        }
        else{
                printf("%s\n", string);
        }
        return errCode;
}


int list(){
  // open current directory
  DIR *cur_dir;
  cur_dir = opendir("."); // open current dir
  struct dirent *dp;
  
  
  char *filenames[1000];  
  int count = 0;
  
  while((dp = readdir(cur_dir)) != NULL){
    filenames[count] = strdup(dp->d_name);
    count++;
  }
  closedir(cur_dir);
  
  // sort file names with bubble sort
  for (int i = 0; i < count - 1; i++){
    for (int j = i + 1; j < count; j++){
      if (strcmp(filenames[i], filenames[j]) > 0){
        
        char *temp = filenames[i];
        filenames[i] = filenames[j];
        filenames[j] = temp;
      }
    }
  }
  
  // Print sorted filenames
  for (int i = 0; i < count; i++){
    printf("%s\n", filenames[i]);
    free(filenames[i]);
  }
  
  return 0;
}

int makedir(char *dir_name){
  mode_t permissions = S_IRWXU | S_IRWXG | S_IRWXO;
  char actual_dir_name[256];

  // uh oh! check the dir name
  if (dir_name[0] == '$'){
    // if its a varialbe name place look fo rhte variable in meme
    char var_name[256];
    strcpy(var_name, dir_name + 1);  // skip the $
    
    char *var_value = mem_get_value(var_name);


    // variable exist?
    if (strcmp(var_value, "Variable does not exist") == 0){
      printf("Bad command: my_mkdir\n");
      return 1;
    }
    
    
    int valid = 1;
    int len = strlen(var_value);
    if (len == 0){
      valid = 0;
    }
    for (int i = 0; i < len; i++){
      if (!((var_value[i] >= 'a' && var_value[i] <= 'z') ||
            (var_value[i] >= 'A' && var_value[i] <= 'Z') ||
            (var_value[i] >= '0' && var_value[i] <= '9'))){
        valid = 0;
        break;
      } //ensuring resitrctions of names (unsure if neccesary)
    }
    
    if (!valid){
      printf("Bad command: my_mkdir\n");
      return 1;
    }
    
    strcpy(actual_dir_name, var_value);
  } else {
    // Direct directory name
    strcpy(actual_dir_name, dir_name);
  }
  
  if (mkdir(actual_dir_name, permissions) == -1){
    return 2;
  } else {
    return 0;
  }
}


int touchme(char *filename){
  mode_t permissions = S_IRWXU | S_IRWXG | S_IRWXO;
  FILE *f = fopen(filename, "a");
  if (f == NULL){
    return 2;
  } 
  fclose(f);
  return 0;
}

int cdme(char *path){
    //lowkey just chdir the path with error handling
    if (chdir(path) != 0){
        printf("Bad command: my_cd\n");
        return 1;
    }
    return 0;
}