#ifndef _IO_LIB_H_
#define _IO_LIB_H_

void writeOut( char *str );
void writeIntOut( int i );
void writeErr( char *str );
void writePrompt();
int readLine( char *str, int maxSize );  // reads line to str, returns number of read bytes, -2 if line was too long or
                                         // -1 if read fails unexpectedly
void parseError();
void printErrno( char *filename, int errno );

#endif /* _IO_LIB_H_ */
