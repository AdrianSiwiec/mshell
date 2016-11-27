#ifndef _BUILTINS_H_
#define _BUILTINS_H_

#define BUILTIN_ERROR 2

typedef struct {
	char* name;
	int (*fun)(char**); 
} builtin_pair;

typedef int ( *BuiltInPtr )( char *argv[] );

extern builtin_pair builtins_table[];

int runBuildIn( char *name, char **arg ); //return whether name is builtIn

#endif /* !_BUILTINS_H_ */
