#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "builtins.h"
#include "config.h"
#include "siparse.h"
#include "utils.h"

void writeOut( char *str );
void writeErr( char *str );
void writePrompt();
int readLine( char *str, int maxSize );

void parseError();
bool isLastPCmd( command **pcmd );
bool isFirstPCmd( command **pcmd, pipeline *p );

int main( int argc, char *argv[] )
{
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

      if ( ln == NULL )
      {
        parseError();
      }

      pipeline *p = ln->pipelines;

      while ( *p != NULL )
      {
        command **pcmd = *p;

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


            int childPid = fork();

            if ( childPid )
            {
              close( nextP[1] ); //wtf why?

              waitpid( childPid, NULL, 0 );
            }
            else
            {
              if ( !isLastPCmd( pcmd ) )  // if not last in pipeline
              {
                printf( " not last\n" );
                close( 1 );  // close stdOut
                dup( nextP[1] );
                close( nextP[0] );
              }

              if ( !isFirstPCmd( pcmd, p ) )  // if not first in pipeline
              {
                printf( " not first\n" );
                close( 0 );
                dup( prevP[0] );
                close( prevP[1] );
              }

              execvp( cmd->argv[0], cmd->argv );

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

            prevP[0] = nextP[0];
            prevP[1] = nextP[1];
          }

          pcmd++;
        }

        p++;
      }
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

bool isLastPCmd( command **pcmd )
{
  return *( pcmd + 1 ) == NULL;
}
bool isFirstPCmd( command **pcmd, pipeline *p )
{
  return pcmd == *p;
}
