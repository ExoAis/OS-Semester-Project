# Mini_Shell
This program is a collection of C and header files for the implemenation of a Shell. A shell is a program that accepts commands from a user and executes them. 
This shell implemenation thus far supports file redirection, pipes, and boolean operations with Linux commands on a Linux operating system. The shell will also work with Linux distributions like Ubuntu.The shell command prompt is **d8sh%.**

## Examples of commands supported:
1. cd - to change directories in the file system
2. exit - to exit the shell
3. ls && cd .. - lists the files in the current directory, and will goto the parent directory
4. input.txt << sum.c >> output.txt - uses input and output redirection so that sum.c can execute using input from the input.txt file and output its results to the output.txt file|
5. random_numbers.c | sum.c  - the standard output of a program called randon_numbers.c will be connected to the standard input for the sum.c program, thus creating a pipeline

## Video Walkthrough:
<img src='shell_walkthrough.gif' title='Video Walkthrough' width='' alt='Video Walkthrough' />

...

a complex example:
1. echo hello && echo world && grep main sum.c - prints hello world to the console and displays where the text "main" is in sum.c on to the console

## Concepts:
1. C Language at the low level
2. Linux
3. Computer Systems
