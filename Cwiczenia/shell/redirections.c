#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

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

int redirectFiles( command *cmd, pipeline *p )
{
  redirection **pRedir = cmd->redirs;

  while ( *pRedir != NULL )
  {
    if(processRedirection( *pRedir ) )
    {
      return -1; 
    }

    pRedir++;
  }
  
  return 0;
}

int processRedirection( redirection *redir )
{
  int redirectTo;
  int access, permission;

  if ( IS_RIN( redir->flags ) )
  {
    if ( _debug ) printf( "__in: %s\n", redir->filename );

    redirectTo = 0;
    access = O_RDONLY;
    permission = S_IREAD;
  }
  else if ( IS_ROUT( redir->flags ) )
  {
    if ( _debug ) printf( "__out: %s\n", redir->filename );

    redirectTo = 1;
    access = O_WRONLY | O_CREAT | O_TRUNC;
    permission = S_IRWXU;
  }
  else if ( IS_RAPPEND( redir->flags ) )
  {
    if ( _debug ) printf( "__append: %s\n", redir->filename );

    redirectTo = 1;
    access = O_WRONLY | O_APPEND | O_CREAT;
    permission = S_IRWXU;
  }

  int res = open( redir->filename, access, permission );

  if ( res == -1 )
  {
    printErrno( redir->filename, errno );
    return -1;
  }

  close( redirectTo );

  if ( dup( res ) != redirectTo )
  {
    return -1;
  }
  
  return 0;
}

