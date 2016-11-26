void redirectPipes( int *prevP, int *nextP, command **pcmd, pipeline *p );
int redirectFiles( command *cmd, pipeline *p );
int processRedirection( redirection *redir );
