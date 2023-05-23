
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "command.h"
#include "executor.h"
#include <sysexits.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>

const int MAX_CONTENT_SIZE = 5000;
static int execute_aux(struct tree *t, int o_input_fd, int p_output_fd);

void replace_character(char* str, char old_char, char new_char) {
    if (str == NULL) {
        // Handle null pointer error, if needed
        return;
    }

    size_t length = strlen(str);

    for (size_t i = 0; i < length; i++) {
        if (str[i] == old_char) {
            str[i] = new_char;
        }
    }
}

int execute(struct tree *t) {
   execute_aux(t, STDIN_FILENO, STDOUT_FILENO); /*processing the root node first*/

  /* print_tree(t);*/ 

   return 0;
}
/*
static void print_tree(struct tree *t) {
   if (t != NULL) {
      print_tree(t->left);

      if (t->conjunction == NONE) {
         printf("NONE: %s, ", t->argv[0]);
      } else {
         printf("%s, ", conj[t->conjunction]);
      }
      printf("IR: %s, ", t->input);
      printf("OR: %s\n", t->output);

      print_tree(t->right);
   }
}
*/
static int execute_aux(struct tree *t, int p_input_fd, int p_output_fd) {
   int pid, status, new_pid, fd, fd2, success_status = 0, failure_status = -1, new_status;
   int pipe_fd[2], pid_1, pid_2;

   /*none conjuction processing*/
   if (t->conjunction == NONE){

     /*Check if exit command was entered, exit if it was*/
      if (strcmp(t->argv[0], "exit") == 0){
         exit(0);
      }
      /*Get the current working directory*/
      else if (strcmp(t->argv[0], "pwd") == 0) {
         char CWD[PATH_MAX];
         getcwd(CWD, sizeof(CWD));
         printf("%s\n", CWD);
         return 0;
      }
      /*Kill a process with -15 or -9 signals*/
      else if (strcmp(t->argv[0], "kill") == 0) {
         char* signal = t->argv[1];
         char* processID = t->argv[2];
         pid_t pID = (pid_t) atoi(processID);
         int s = 15;
         if (strcmp(signal, "SIGTERM") == 0) {
            s = 15;
         }
         else if (strcmp(signal, "SIGKILL") == 0) {
            s = 9;
         }
         int r = kill(pID, s);
         if (r == 0) {
            printf("Process Terminated Successfully\n");
         }
         else {
            fprintf(stderr, "Failed To Kill The Process\n");
         }
         return 0;
      }
      /*Create a new file*/
      else if (strcmp(t->argv[0], "touch") == 0) {
         FILE* fp = fopen(t->argv[1], "w");
         if (!fp) {
            fprintf(stderr, "Error Occured: %s", strerror(errno));
            printf("\n");
         }
         else {
            fclose(fp);
         }
         return 0;
      }
      else if (strcmp(t->argv[0], "his") == 0) {
         char* his = "";
         for (int i = 0; i < h; i++) {
            his = history[i];
            printf("%s\n", his);  
         }
         return 0;
      }
      else if (strcmp(t->argv[0], "tftxt") == 0) {
         FILE* fd1 = fopen(t->argv[1], "r");
         FILE* fd2 = fopen(t->argv[2], "w");
         char content[MAX_CONTENT_SIZE];
         if (!fd1 || !fd2) {
            fprintf(stderr, "Error Occured: %s", strerror(errno));
            printf("\n");
         }
         else {
            while (fgets(content, sizeof(content), fd1) != NULL) {
               fputs(content, fd2);
            }
            fclose(fd1);
            fclose(fd2);
         }
         return 0;
      }
      else if (strcmp(t->argv[0], "sbsq") == 0) {
         char* file = t->argv[1];
         char* sS = t->argv[2];
         char* line;
         size_t size = 0;
         FILE* fd = fopen(file, "r");
         char subSequence[strlen(sS) + 1];
         strcpy(subSequence, sS);
         int k = 0;
         int match = 0; 
         if (!fd) {
            fprintf(stderr, "Error Occured: %s", strerror(errno));
            printf("\n");
         }
         else {
            while (getline(&line, &size, fd) != -1) {
               char content[strlen(line) + 1];
               strcpy(content, line);
               for (int i = 0; i < strlen(content); i++) {
                  k = i;
                  for (int j = 0; j < strlen(subSequence); j++) {
                     if (content[k] == subSequence[j]) {
                        match++;
                        k++;
                     }
                     else {
                        match = 0;
                        break;
                     }
                  }
                  if (match == strlen(subSequence)) {
                     printf("Given Subsequence is found in the given file\n");
                     return 0;
                  }
               }
            }
            fclose(fd);
            printf("Given Subsequence is not found in the given file\n");
         }
         return 0;
      }
      else if(strcmp(t->argv[0], "rm") == 0){ // delete file [rm filename directory]
          // Create the file path by concatenating the directory and filename
         char file_path[256];  // Adjust the buffer size as per your needs

         if (t->argv[2] == NULL || strlen(t->argv[2]) == 0) {
            strncpy(file_path, t->argv[1], sizeof(file_path));
         } else {
            snprintf(file_path, sizeof(file_path), "%s/%s", t->argv[2], t->argv[1]);
         }

         if (remove(file_path) == 0) {
            printf("File '%s' deleted successfully.\n", file_path);
         } else {
            perror("Error deleting file");
         }

         return 0;
      }

      else if(strcmp(t->argv[0], "grepp") == 0){//command 5 grep
          FILE* file = fopen(t->argv[1], "r");
         if (file == NULL) {
            perror("Error opening file");
            return 0;
         }

         char line[256];  // Adjust the buffer size as per your needs
         int line_number = 0;
         int word_count = 0;

         while (fgets(line, sizeof(line), file) != NULL) {
            line_number++;

            // Split the line into words
            char* word;
            char* rest = line;
            while ((word = strtok_r(rest, " \t\n", &rest)) != NULL) {
                  if (strcmp(word, t->argv[2]) == 0) {
                     word_count++;
                  }
            }
         }

         fclose(file);

         printf("Word '%s' found %d time(s) in file '%s'.\n", t->argv[2], word_count, t->argv[1]);
         return 0;
      }

      else if(strcmp(t->argv[0], "find") == 0){ //command 4 find a file in current directory
         DIR* dir = opendir(".");
         if (dir == NULL) {
            perror("Error opening directory");
            return 0;
         }

         struct dirent* entry;
         while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && strcmp(entry->d_name, t->argv[1]) == 0) {
                  struct stat st;
                  if (stat(entry->d_name, &st) == 0) {
                     printf("%s/%s\n", getcwd(NULL, 0), entry->d_name);
                  }
            }
         }

         closedir(dir);
         return 0;
      }

      else if(strcmp(t->argv[0], "cr") == 0){// command 2 create file [cr filename directory]

            // Create the file path by concatenating the directory and filename
         char file_path[256];  // Adjust the buffer size as per your needs

         if (t->argv[2] == NULL || strlen(t->argv[2]) == 0) {
            strncpy(file_path, t->argv[1], sizeof(file_path));
         } else {
            snprintf(file_path, sizeof(file_path), "%s/%s", t->argv[2], t->argv[1]);
         }

         // Open/create the file for writing
         FILE* file = fopen(file_path, "w");
         if (file == NULL) {
            perror("Error creating file");
            return 0;
         }

         // Close the file
         fclose(file);
         return 0;
      }

      else if(strcmp(t->argv[0], "prf") == 0) { //command 6 print file prf filename
         FILE* file = fopen(t->argv[1], "r");
         if (file == NULL) {
            perror("Error opening file");
            return 0;
         }

         char line[256];  // Adjust the buffer size as per your needs

         while (fgets(line, sizeof(line), file) != NULL) {
            printf("%s", line);
         }
         printf("\n");
         fclose(file);
         return 0;
      }

      else if(strcmp(t->argv[0], "writef") == 0){ // command 7 write a line to file
         
         
         FILE* file = fopen(t->argv[1], "a");
         if (file == NULL) {
            perror("Error opening file");
            return 0;
         }
         
         int total_length = 0;
         int num_elements = sizeof(t->argv) / sizeof(t->argv[0]);
         for (int i = 2; i < num_elements; i++) {
            total_length += strlen(t->argv[i]);
         }
         char result[total_length-num_elements + 1];
         
         replace_character(t->argv[2], '_', ' ');
         for (int i = 2; i < num_elements; i++) {
            strcat(result, t->argv[i]);
         }

         fprintf(file, "%s\n", t->argv[2]);
         fclose(file);

         return 0;
      }

      else if (strcmp(t->argv[0], "mv") == 0){ //command 1 move file [mv file directory]
         // Extract the filename from the source path
         const char* filename = strrchr(t->argv[1], '/');
         if (filename == NULL) {
            filename = t->argv[1];  // If no directory separator found, assume filename only
         } else {
            filename++;  // Move past the directory separator
         }

         // Create the destination path by concatenating the destination directory and filename
         char destination_path[256];  // Adjust the buffer size as per your needs
         snprintf(destination_path, sizeof(destination_path), "%s/%s", t->argv[2], t->argv[1]);

         // Open the source file for reading
         FILE* source_file = fopen(t->argv[1], "rb");
         if (source_file == NULL) {
            perror("Error opening source file");
            return 0;
         }

         // Open/create the destination file for writing
         FILE* dest_file = fopen(destination_path, "wb");
         if (dest_file == NULL) {
            perror("Error opening destination file");
            fclose(source_file);
            return 0;
         }

         int ch;
         while ((ch = fgetc(source_file)) != EOF) {
            fputc(ch, dest_file);
         }

         fclose(source_file);
         fclose(dest_file);

         if (remove(t->argv[1]) == -1) {
            perror("Error deleting source file");
            return 0;
         }

         return 0;
      }
      /*check if user wants to change directory, if so change directory*/
      else if (strcmp(t->argv[0], "cd") == 0){
         if (strlen(t->argv[1]) != 0){
            if (chdir(t->argv[1]) == -1){
               perror(t->argv[1]);
            }
         }
         else {
            chdir(getenv("HOME")); /*change to home directory if no argument is provided*/
         }
      }

      /*process any entered linux commands*/
       if((pid = fork()) < 0){
          perror("fork");
          exit(-1);
       }
       /*parent processing*/
       if(pid != 0){
          wait(&status);   
          return status;  
       }

       /*child processing*/
       else {

          /*check if we have any input/output files*/
          if(t->input != NULL){
            if ((fd = open(t->input, O_RDONLY)) < 0) {
               perror("fd");
               exit(-1);
            } 
            /*get input from the file if it exists, and opening it has not failed*/
               if(dup2(fd, STDIN_FILENO) < 0){
                  perror("dup2");
                  exit(-1);
               }

               /*close the file descriptor*/
               if(close(fd) < 0){
                  perror("close");
                  exit(-1);
               }
          } 
         
          /*use provided output file if it exists*/
          if(t->output != NULL){
             if ((fd = open(t->output, O_CREAT | O_WRONLY | O_TRUNC)) < 0) {
                perror("open");
                exit(-1);
            } 

            /*change standard output to get output from provided file if exists*/
             if(dup2(fd, STDOUT_FILENO) < 0){
                perror("dup2");
                exit(-1);
             }

             /*close file descriptor associated with output file*/
              if(close(fd) < 0){
                  perror("close");
                  exit(-1);
               }  
          }
      
          /*execute the command using*/
          execvp(t->argv[0], t->argv);
          fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
          exit(-1);
       }
        
   }

   else if(t->conjunction == AND){

      /*Process left subtree, then right subtree if leftsubtree is processed correctly*/ 
      new_status = execute_aux(t->left, p_input_fd, p_output_fd);
      
      /*if the left_subtree was processed was succesfully, process the right subtree */
      if(new_status == 0){
        return execute_aux(t->right, p_input_fd, p_output_fd);
      }
      else {
         return new_status;
      }
   }

   /*of the current conjuction is of type pipe, process*/
   else if(t->conjunction == PIPE){
      
   if(t->left->output != NULL){
      printf("Ambiguous output redirect.\n");
      return failure_status;
   }

   if(t->right->input != NULL){
      printf("Ambiguous output redirect.\n");
      return failure_status;
   }

      /* create a pipe */
      if(pipe(pipe_fd) < 0){
         perror("pipe");
         exit(-1);
      }

      /*fork(creating child process)*/
      if((pid_1 = fork()) < 0){
         perror("fork");
      }
      

      if(pid_1 == 0) { /*child 1  process code*/
      
      close(pipe_fd[0]); /*close read end since were not using it*/

         if(t->input != NULL){

            if ((fd = open(t->input, O_RDONLY)) < 0) {
               perror("open");
               exit(-1);
            } 

            /*make the pipes write end the new standard output*/
            dup2(pipe_fd[1], STDOUT_FILENO);

            /*pass in input file if exists and the pipes write end for the output file descriptor*/
            execute_aux(t->left, fd, pipe_fd[1]);

            /*closed pipe write end*/
            if(close(pipe_fd[1]) < 0){
               perror("close");
               exit(-1);
            }

            /*close input file*/
            if(close(fd) < 0){
               perror("close");
               exit(-1);
            }
         }
         /*if no input file was provided, use parent file descriptor as the input file*/
         else {
            dup2(pipe_fd[1], STDOUT_FILENO);
            execute_aux(t->left, p_input_fd, pipe_fd[1]); /*process the left subtree*/
            if(close(pipe_fd[1] < 0)){
               perror("close");
               exit(-1);
            }
         } 
      }
      else {
         /*create second child to handle right subtree*/

         if((pid_2 = fork()) < 0){
            perror("fork");
            exit(-1);
          }

         if(pid_2 == 0){ /*child two code*/

            /*close write end of pipe, dont need it*/
            close(pipe_fd[1]);

            /*use output file if provided, else use parent output file descriptor*/
            if(t->output != NULL){

               /*open provided output file if exists*/
               if((fd = open(t->output, O_WRONLY | O_CREAT | O_TRUNC)) < 0){
                  perror("open");
                  exit(-1);
               }

               /*make the pipes read end the new standard input*/
               dup2(pipe_fd[0], STDIN_FILENO);
               execute_aux(t->right, pipe_fd[0], fd); /*process the right subtree*/
              
               /*close pipe read end*/
               if(close(pipe_fd[0]) < 0){
                  perror("close");
                  exit(-1);
                }

               /*closed output file*/
               if(close(fd) < 0){
                  perror("close");
                  exit(-1);
               }
            }

            else {
              dup2(pipe_fd[0], STDIN_FILENO);
              execute_aux(t->right, pipe_fd[0], p_output_fd); /*process the right subtree*/
            }

         } else {
           
           /* Parent has no need for the pipe */
           close(pipe_fd[0]);
           close(pipe_fd[1]);

           /* Reaping each child */
           wait(&status);
           wait(&status);
         }
      }
   }
   /*if current conjuction enumerator is of type subshell, process commands
     in a child process and return success/failure status after executing*/
   else if(t->conjunction == SUBSHELL){

     /*fork(creating child process)*/
      if((new_pid = fork()) < 0){
         perror("fork");
         exit(-1);
      }

     /*parent code*/
      if(new_pid != 0){
         wait(&status); /*wait for child*/
         exit(WEXITSTATUS(status)); /*return the status*/
      }
      else {
      /*child code*/

      /*get input from input file if it exists*/
      if(t->input != NULL){
         /*open input file*/
            if ((fd = open(t->input, O_RDONLY)) < 0) {
               perror("fd");
               exit(-1);
            } 
            /*changed standard input to use input from input file*/
            if(dup2(fd, STDIN_FILENO) < 0){
               perror("dup2");
               exit(-1);
            }
            /*close file descriptor(input file)*/
            if(close(fd) < 0){
               perror("fd");
               exit(-1);
            }
          }
          /*if there was no input file use provided parent file descriptor(the file parameter of the function)*/
          else {
             fd = p_input_fd;
          }

      /*use an output file if it exists*/
      if(t->output != NULL){
            if ((fd2 = open(t->output,  O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
               perror("fd");
               exit(-1);
            } 

            /*change standard output to the output file(output will be written to output file)*/
            if(dup2(fd2, STDOUT_FILENO)){
               perror("dup2");
               exit(-1);
            }
            /*close the output file descriptor*/
            if(close(fd2) < 0){
               perror("fd");
               exit(-1);
            }
          }
          /*if no outputfile exists write output to provided parent output file */
          else {
            fd2 = p_output_fd;       
          }
         /*execute left subtree and get status*/
        status =  execute_aux(t->left, fd, fd2);
        /*return with value of statujs(0: success, -1: failure*/
        exit(status);
      }

   }
 
  return success_status;
}

