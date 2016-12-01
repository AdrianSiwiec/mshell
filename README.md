Simple shell written in c. I features:

- pipelines

- file redirections

- background processes, with notification when process ends

- taking input from tty or file

Compilation:

- on linux: make clean && make in input_parse followed by make clean && make in ./

- on minix: change 'byacc' to 'yacc' in 17th line in input_parse/Makefile followed by make clean && make in input_parse followed by make clean && make in ./

To enable debug messages change _debug to True in 20th line of mshell.c

All reads/writes with exception of debug messeges are done with read/write functions, without printf/scanf.
