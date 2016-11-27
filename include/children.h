#ifndef _CHILDREN_H_
#define _CHILDREN_H_

void childHandler( int sigNb );
void setChildHandler();
void addForegroundChild( int pid );
void writeZombies();

#endif /* _CHILDREN_H_ */
