#ifndef _IO_LIB_H_
#define _IO_LIB_H_

void writeOut( char *str );
void writeErr( char *str );
void writePrompt();
int readLine( char *str, int maxSize );
void parseError();

#endif
