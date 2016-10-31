#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins.h"
#include "config.h"
#include "siparse.h"
#include "utils.h"

void writeOut( char *str );
void writeErr( char *str );
void writePrompt();
int readLine( char *str, int maxSize );

bool isLastPCmd( command **pcmd );
bool isFirstPCmd( command **pcmd, pipeline *p );
bool lineIsInvalid( line *ln );

void processLine( line *ln );
void processPipeline( pipeline *p );
void redirectPipes( int *prevP, int *nextP, command **pcmd, pipeline *p );
void redirectFiles( command *cmd, pipeline *p );
void processRedirection( redirection *redir );
void onExecError( command *cmd );

void childHandler( int sigNb );

int livingChildren = 0;
bool _debug = true;

int main( int argc, char *argv[] )
{
  struct sigaction act;
  act.sa_handler = childHandler;
  act.sa_flags = 0;
  sigemptyset( &act.sa_mask );
  sigaction( SIGCHLD, &act, NULL );

  line *ln;
  char input[MAX_LINE_LENGTH + 10];

  while ( 1 )
  {
    writePrompt();
    int length = readLine( input, MAX_LINE_LENGTH );

    if ( length > 0 && length < MAX_LINE_LENGTH )
    {
      input[length] = NULL;
      ln = parseline( input );

      processLine( ln );
    }
    else if ( length >= MAX_LINE_LENGTH )
    {
      parseError();
    }
    else if ( length == 0 )
    {
      exit( 0 );
    }
    else
    {
      if ( _debug ) printf( "ERROR, errno = %s, input = %s, length = %d", strerror( errno ), input, length );

      exit( 2 );
    }
  }
}

void processLine( line *ln )
{
  if ( lineIsInvalid( ln ) )
  {
    parseError();
    return;
  }

  pipeline *p = ln->pipelines;

  while ( *p != NULL )
  {
    processPipeline( p );

    while ( livingChildren > 0 )
    {
      pause();
    }

    p++;
  }
}

void processPipeline( pipeline *p )
{
  command **pcmd = *p;

  if ( ( *pcmd )->argv[0] == NULL )
  {
    return;
  }

  int prevP[2];
  int nextP[2];

  while ( *pcmd != NULL )
  {
    command *cmd = *pcmd;

    BuiltInPtr ptr = getBuiltIn( cmd->argv[0] );

    if ( ptr != NULL )  // if is builtIn
    {
      ptr( cmd->argv );
    }
    else
    {
      if ( !isLastPCmd( pcmd ) )
      {
        if ( pipe( nextP ) < 0 )
        {
          printf( "could not create pipe\n" );
          // TODO cannot create pipe
        }
      }

      livingChildren++;
      int childPid = fork();

      if ( childPid )
      {
        close( nextP[1] );  // wtf why?

        if ( !isFirstPCmd( pcmd, p ) )
        {
          close( prevP[0] );
        }
      }
      else
      {
        redirectPipes( prevP, nextP, pcmd, p );

        redirectFiles( cmd, p );

        execvp( cmd->argv[0], cmd->argv );

        onExecError( cmd );
      }

      prevP[0] = nextP[0];
      prevP[1] = nextP[1];
    }

    pcmd++;
  }
}
void onExecError( command *cmd )
{
  switch ( errno )
  {
    case ENOENT:
      writeOut( cmd->argv[0] );
      writeOut( ": no such file or directory\n" );
      break;

    case EACCES:
      writeOut( cmd->argv[0] );
      writeOut( ": permission denied\n" );
      break;

    default:
      writeOut( cmd->argv[0] );
      writeOut( ": exec error\n" );
  }

  exit( EXEC_FAILURE );
}

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

bool isLastPCmd( command **pcmd )
{
  return *( pcmd + 1 ) == NULL;
}
bool isFirstPCmd( command **pcmd, pipeline *p )
{
  return pcmd == *p;
}
void childHandler( int sigNb )
{
  pid_t child;

  do
  {
    child = waitpid( -1, NULL, WNOHANG );

    if ( child > 0 )
    {
      livingChildren--;

      if ( _debug ) printf( "living children: %d\n", livingChildren );
    }
  } while ( child > 0 );
}
bool lineIsInvalid( line *ln )
{
  if ( ln == NULL ) return true;

  pipeline *p = ln->pipelines;

  if ( p == NULL ) return true;

  while ( *p != NULL )
  {
    command **pcmd = *p;

    if ( pcmd == NULL ) return true;

    while ( *pcmd != NULL )
    {
      command *cmd = *pcmd;

      if ( cmd == NULL || ( cmd->argv[0] == NULL ) )
      {
        if ( !isFirstPCmd( pcmd, p ) ) return true;

        break;
      }

      pcmd++;
    }

    p++;
  }

  return false;
}
