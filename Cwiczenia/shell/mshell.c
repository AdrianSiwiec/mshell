#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>

#include "builtins.h"
#include "config.h"
#include "utils.h"

void processLine( line *ln );
void processPipeline( pipeline *p, int isBackground );
void onExecError( command *cmd );
void setSIGINTHandler();

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
    writeZombies();
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
      exit( 1 );
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

  int isBackground = ln->flags & LINBACKGROUND;

  if ( _debug ) printf( "__isBackground = %d\n", isBackground );

  pipeline *p = ln->pipelines;

  sigset_t oldMask, newMask;

  sigemptyset( &newMask );
  sigaddset( &newMask, SIGCHLD );
  sigprocmask( SIG_BLOCK, &newMask, &oldMask );

  while ( *p != NULL )
  {
    processPipeline( p, isBackground );

    while ( !isBackground && foregroundChildren > 0 )
    {
      sigsuspend( &oldMask );
    }

    p++;
  }

  sigprocmask( SIG_SETMASK, &oldMask, NULL );
}

void processPipeline( pipeline *p, int isBackground )
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
          exit( 1 );
        }
      }

      int childPid = fork();

      if ( childPid )
      {
        if ( !isBackground )
        {
          foregroundChildren++;
          addForegroundChild( childPid );
        }

        if ( !isLastPCmd( pcmd ) ) close( nextP[1] );

        if ( !isFirstPCmd( pcmd, p ) ) close( prevP[0] );
      }
      else
      {
        if ( isBackground )
        {
          setsid();
        }

        sigset_t newMask;
        sigemptyset( &newMask );
        sigaddset( &newMask, SIGINT );
        sigprocmask( SIG_UNBLOCK, &newMask, NULL );

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

void SIGINTHandler()
{
  writeOut( "\n" );
  writePrompt();
}

void setSIGINTHandler()
{
  //  struct sigaction act;
  //  act.sa_handler = SIGINTHandler;
  //  act.sa_flags = 0;
  //  sigemptyset( &act.sa_mask );
  //  sigaction( SIGINT, &act, &oldSIGINTHandler );

  sigset_t newMask;
  sigemptyset( &newMask );
  sigaddset( &newMask, SIGINT );
  sigprocmask( SIG_BLOCK, &newMask, NULL );
}
