#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>

#include "builtins.h"
#include "config.h"
#include "utils.h"

void processLine( line *ln );
void processPipeline( pipeline *p );
void onExecError( command *cmd );

void childHandler( int sigNb );
void setChildHandler();

int livingChildren = 0;
int _debug = 1;

int main( int argc, char *argv[] )
{
  setChildHandler();

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
  if ( isLineInvalid( ln ) )
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
  if ( isPipelineEmpty( p ) )
  {
    if ( _debug ) printf( "__empty pipeline is not executed\n" );

    return;
  }

  int prevP[2];
  int nextP[2];
  command **pcmd = *p;

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
        if ( !isLastPCmd( pcmd ) )
        {
          close( nextP[1] );  // wtf why?
        }

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

void childHandler( int sigNb )
{
  pid_t child;

  do
  {
    child = waitpid( -1, NULL, WNOHANG );

    if ( child > 0 )
    {
      livingChildren--;

      if ( _debug ) printf( "__living children: %d\n", livingChildren );
    }
  } while ( child > 0 );
}
void setChildHandler()
{
  struct sigaction act;
  act.sa_handler = childHandler;
  act.sa_flags = 0;
  sigemptyset( &act.sa_mask );
  sigaction( SIGCHLD, &act, NULL );
}
