#ifndef _REDIRECTIONS_H_
#define _REDIRECTIONS_H_

void redirectPipes( int *prevP, int *nextP, command **pcmd, pipeline *p );
int redirectFiles( command *cmd, pipeline *p );  // returns 0 if succes, otherwise -1

#endif /* !_REDIRECTIONS_H_ */
