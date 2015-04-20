Name: Cesar Acosta

Date: 11/10/14

Class: CSCI 1730 (Maria Hybinette)


!!!!!!IMPORTANT!!!!!!
Make sure this program runs in the cluster node as that is where readline
is installed, otherwise this program will not compile correctly.


Purpose: For this assignment you will write one program, a shell, yosh, that needs to compile and run on a computer system on {nike}. [as a side note if you develop your program on a separate platform, before you turn it in you need to make sure it compiles and runs on nike.]

A typical flow of commands of a shell is listed below - at the high level a shell is an infinite loops, a big while loop, that infinitely waits for your command.

1.)print a prompt
2.)read a line of input
3.)parse the line to determine
4.)the name and parameters of programs to execute
5.)any pipes or input/output redirections
6.)set up any pipes or input/output redirections
7.)use the fork system call to create a child process:
8.)the child process calls the exec system call to execute the specified program
9.)the parent process (your shell) waits for the child process to complete (with the waitpid system call) or continues, if the child process is to be run in the background
10.)repeat


Compile: This program needs to be compiled with the following command:
	gcc -DUNIX -lreadline -lcurses  -g shell.c parse.c -o shell
This should compile the file correctly.
Also use the command: [make] to compile and then [make clean] to remove the
executable.

How To Use: The following are built-in commands:
exit
help
history
cd [directory] (Unfortunately doesn't work)
jobs 
! [argument(it is a number)] (There MUST be a space between the ! and the number)

Other than the above commands, other commonly used commands should execute correctly as well such as(with flags): ls, ps, who, wc, grep, etc.

Unfortunately, this program does NOT support the use of multiple pipes, it
can only do one pipe at the current moment.

IMPORTANT!: When grading, please ignore the "No such File or Directory"
message. The commands still work, for some reason it keeps printing out 
that error even though the commands work. Please double check to
make sure that the command worked.
Thank you for your cooperation.

Resources:
stackoverflow.com, gnu.org
