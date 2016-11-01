void redirectPipes( int *prevP, int *nextP, command **pcmd, pipeline *p );
void redirectFiles( command *cmd, pipeline *p );
void processRedirection( redirection *redir );
