#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "siparse.h"
#include "utils.h"
#include "builtins.h"

void writeOut( char *str );
void writeErr( char *str );
void writePrompt();
int readLine( char *str, int maxSize );

void parseError();

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

      command *cmd = pickfirstcommand( ln );

      BuiltInPtr ptr = getBuiltIn( cmd->argv[0] );

      if ( ptr != NULL )
      {
        ptr( cmd->argv );
      }
      else
      {
        int childPid = fork();

        if ( childPid )
        {
          waitpid( childPid, NULL, 0 );
        }
        else
        {
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
