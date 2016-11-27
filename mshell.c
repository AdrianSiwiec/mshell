#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <sys/wait.h>

#include "builtins.h"
#include "config.h"
#include "utils.h"

void processLine( line *ln );
int processPipeline( pipeline *p, int isBackground );  // returns if there was builtIn in pipeline
void onExecError( command *cmd );
void setSIGINTHandler();
sigset_t blockSignal( int how, int signum );  // returns old mask

extern volatile int foregroundChildren;
struct sigaction oldSIGINTHandler;

int _debug = 0;

int main( int argc, char *argv[] )
{
  setChildHandler();
  setSIGINTHandler();

  line *ln;
  char input[MAX_LINE_LENGTH + 10];

  while ( 1 )
  {
    if ( isInTty() )
    {
      writeZombies();
      writePrompt();
    }

    int length = readLine( input, MAX_LINE_LENGTH );

    if ( _debug ) printf( "__read returned %d\n", length );

    if ( length > 0 && length < MAX_LINE_LENGTH )
    {
      input[length] = NULL;
      ln = parseline( input );

      processLine( ln );
    }
    else if ( length == -2 )
    {
      parseError();
    }
    else if ( length == 0 )
    {
      if ( isInTty() )
      {
        writeOut( "\n" );
      }

      exit( 0 );
    }
    else
    {
      printErrno( NULL, errno );
      exit( 1 );
    }

    if ( _debug ) printf( "\n\n" );
  }
}

void processLine( line *ln )
{
  if ( isLineInvalid( ln ) )
  {
    parseError();
    return;
  }

  int isBackground = ln->flags & LINBACKGROUND;

  if ( _debug ) printf( "__isBackground = %d\n", isBackground );

  pipeline *p = ln->pipelines;

  sigset_t oldMask;

  oldMask = blockSignal( SIG_BLOCK, SIGCHLD );

  while ( *p != NULL )
  {
    int wasBuiltIn = processPipeline( p, isBackground );

    while ( !wasBuiltIn && !isBackground && foregroundChildren > 0 )
    {
      sigsuspend( &oldMask );
    }

    p++;
  }

  sigprocmask( SIG_SETMASK, &oldMask, NULL );
}

int processPipeline( pipeline *p, int isBackground )
{
  if ( isPipelineEmpty( p ) )
  {
    if ( _debug ) printf( "__empty pipeline is not executed\n" );

    return 0;
  }

  int prevP[2];
  int nextP[2];
  command **pcmd = *p;

  int wasBuiltIn = 0;

  while ( *pcmd != NULL )
  {
    command *cmd = *pcmd;

    if ( runBuildIn( cmd->argv[0], cmd->argv ) )
    {
      wasBuiltIn = 1;
    }
    else
    {
      if ( !isLastPCmd( pcmd ) )
      {
        if ( pipe( nextP ) < 0 )
        {
          if ( _debug ) printf( "could not create pipe\n" );

          exit( 1 );
        }
      }

      int childPid = fork();

      if ( childPid )  // father
      {
        if ( !isBackground )
        {
          foregroundChildren++;
          addForegroundChild( childPid );
        }

        if ( !isLastPCmd( pcmd ) ) close( nextP[1] );

        if ( !isFirstPCmd( pcmd, p ) ) close( prevP[0] );
      }
      else  // son
      {
        if ( isBackground )
        {
          setsid();
        }

        blockSignal( SIG_UNBLOCK, SIGINT );

        redirectPipes( prevP, nextP, pcmd, p );

        int redirectionError;
        redirectionError = redirectFiles( cmd, p );

        if ( !redirectionError )
        {
          execvp( cmd->argv[0], cmd->argv );
          onExecError( cmd );
        }

        exit( EXEC_FAILURE );
      }

      prevP[0] = nextP[0];
      prevP[1] = nextP[1];
    }

    pcmd++;
  }

  return wasBuiltIn;
}
void onExecError( command *cmd )
{
  printErrno( cmd->argv[0], errno );
  exit( EXEC_FAILURE );
}

void setSIGINTHandler()
{
  blockSignal( SIG_BLOCK, SIGINT );
}

sigset_t blockSignal( int how, int signum )
{
  sigset_t oldMask, newMask;
  sigemptyset( &newMask );
  sigaddset( &newMask, signum );
  sigprocmask( how, &newMask, &oldMask );
  return oldMask;
}
