#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "siparse.h"

extern int _debug;

void redirectPipes( int *prevP, int *nextP, command **pcmd, pipeline *p )
{
  if ( !isLastPCmd( pcmd ) )
  {
    close( 1 );  // close stdOut
    dup( nextP[1] );
    close( nextP[0] );
  }

  if ( !isFirstPCmd( pcmd, p ) )
  {
    close( 0 );
    dup( prevP[0] );
    close( prevP[1] );
  }
}

void redirectFiles( command *cmd, pipeline *p )
{
  redirection **pRedir = cmd->redirs;

  while ( *pRedir != NULL )
  {
    processRedirection( *pRedir );

    pRedir++;
  }
}

void processRedirection( redirection *redir )
{
  int redirectTo;
  int access, permission;

  if ( IS_RIN( redir->flags ) )
  {
    if ( _debug ) printf( "in: %s\n", redir->filename );

    redirectTo = 0;
    access = O_RDONLY;
    permission = S_IREAD;
  }
  else if ( IS_ROUT( redir->flags ) )
  {
    if ( _debug ) printf( "out: %s\n", redir->filename );

    redirectTo = 1;
    access = O_WRONLY | O_CREAT | O_TRUNC;
    permission = S_IRWXU;
  }
  else if ( IS_RAPPEND( redir->flags ) )
  {
    if ( _debug ) printf( "append: %s\n", redir->filename );

    redirectTo = 1;
    access = O_APPEND | O_CREAT;
    permission = S_IRWXU;
  }

  int res = open( redir->filename, access, permission );

  if ( res == -1 )
  {
    // TODO handle
  }

  close( redirectTo );

  if ( dup( res ) != redirectTo )
  {
    // TODO handle
  }
}
