#include <stdio.h>
#include <sys/wait.h>

#define maxForegroundChildren 60
#define maxBackgroundChildren 60

int isForegroundChild( int pid );
int removeForegroundChild( int pid );
int addForegroundChild( int pid );

volatile int foregroundPids[maxForegroundChildren];
volatile int *foregroundPtr = foregroundPids;

volatile int zombiePids[maxBackgroundChildren];
volatile int zombieStatuses[maxBackgroundChildren];
volatile int zombieIt = 0;

extern int _debug;

volatile int foregroundChildren = 0;


void childHandler( int sigNb )
{
  pid_t child;
  
  do
  {
    int childStatus;
    child = waitpid( -1, &childStatus, WNOHANG );

    if(_debug) printf("__Got child: %d\n", child);

    if ( child > 0 )
    {
      if ( isForegroundChild( child ) )
      {
        foregroundChildren--;

        if ( _debug ) printf( "__living foreground children: %d\n", foregroundChildren );

        removeForegroundChild( child );

      }
      else
      {
        if ( _debug ) printf( "__living BACKground children: %d\n", zombieIt );

        if ( zombieIt < maxBackgroundChildren )
        {
          zombiePids[zombieIt] = child;
          zombieStatuses[zombieIt] = childStatus;
          zombieIt++;
        }
      }
    }
  } while ( child > 0 );
}

void writeZombies()
{
  int i;

  for ( i = 0; i < zombieIt; i++ )
  {
    writeOut( "Background process " );
    writeIntOut( zombiePids[i] );
    writeOut( " terminated. " );

    if ( WIFEXITED( zombieStatuses[i] ) )
    {
      writeOut( "(exited with status " );
      writeIntOut( WEXITSTATUS( zombieStatuses[i] ) );
      writeOut( ")\n" );
    }
    else
    {
      writeOut( "(killed by signal " );
      writeIntOut( WTERMSIG( zombieStatuses[i] ) );
      writeOut( ")\n" );
    }
  }

  zombieIt = 0;
}

void setChildHandler()
{
  struct sigaction act;
  act.sa_handler = childHandler;
  act.sa_flags = 0;
  sigemptyset( &act.sa_mask );
  sigaction( SIGCHLD, &act, NULL );
}

int addForegroundChild( int pid )
{
  *foregroundPtr = pid;
  foregroundPtr++;
}

int isForegroundChild( int pid )
{
  int *p = foregroundPids;

  while ( p != foregroundPtr )
  {
    if ( *p == pid ) return 1;

    p++;
  }

  return 0;
}
int removeForegroundChild( int pid )
{
  if ( foregroundPids == foregroundPtr ) return -1;

  int *p = foregroundPids;

  while ( p != foregroundPtr )
  {
    if ( *p == pid )
    {
      *p = *( foregroundPtr - 1 );
      foregroundPtr--;

      return 0;
    }

    p++;
  }

  return -1;
}
