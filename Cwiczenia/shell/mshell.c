#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "siparse.h"
#include "utils.h"

void writeOut( char *str );
void writeErr( char *str );
void writePrompt();
int readLine( char *str, int maxSize );

void parseError();

int main( int argc, char *argv[] )
{
  line *ln;
  char input[2001];

  while ( 1 )
  {
    writePrompt();
    int length = readLine( input, 2000 );

    if ( length > 0 && length < MAX_LINE_LENGTH )
    {
      input[length] = NULL;
      ln = parseline( input );

      if ( ln == NULL )
      {
        parseError();
      }

      int childPid = fork();

      if ( childPid )
      {
        waitpid( childPid, NULL, 0 );
      }
      else
      {
        command *cmd = pickfirstcommand( ln );
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
    else if ( length >= MAX_LINE_LENGTH )
    {
      parseError();
    }
    else if ( length == 0 )
    {
      write( 1, "\n", 1 );
      exit( 0 );
    }
    else
    {
      exit( 1 );
    }
  }
}

// --- IN OUT

void writeErr( char *str )
{
  write( 2, str, strlen( str ) );
}
void writeOut( char *str )
{
  write( 1, str, strlen( str ) );
}

struct stat sb;

void writePrompt()
{
    fstat(0, &sb);
    if()
    {
        TODO
        writeOut( PROMPT_STR );
    }
}

#define INBUFFER_SIZE 10000
char inBuffer[INBUFFER_SIZE];

char *inBufferIt = inBuffer;
char *inBufferBegin = inBuffer;
char *inBufferEnd = inBuffer;

int inBufferCurrentPos();
void warpBuffer();
int fillLine( char *dest, int maxSize );

int readLine( char *str, int maxSize )
{
    while( 1 ) 
    {
        while( inBufferIt < inBufferEnd )
        {
            if( inBufferCurrentPos() >= INBUFFER_SIZE )
            {
                warpBuffer();   
            }
            
            if( *inBufferIt == '\n' )
            {
                return fillLine( str, maxSize );   
            }
            
            inBufferIt ++;
        }
        
        int readBytes = read( 0, inBufferIt, INBUFFER_SIZE - inBufferCurrentPos() );

        if( readBytes >= 0 )
        {
            inBufferEnd += readBytes;   
        }
        else
        {
            return readBytes;   
        }
    }
}

int inBufferCurrentPos()
{
    return inBufferIt - inBuffer;   
}
void warpBuffer()
{
    if(inBufferBegin == inBuffer)
    {
        //TODO: error   
    }
    
    char *ptr = inBuffer;
    for(char *c = inBufferBegin; c < inBufferEnd; c++)
    {
        *ptr = *c;
        ptr++;
    }

    int moved = inBufferBegin - inBuffer;
    inBufferBegin -= moved;
    inBufferIt -= moved;
    inBufferEnd -= moved;
}
int fillLine( char *dest, int maxSize )
{
    if( inBufferIt - inBufferBegin > maxSize )
    {
        return -2; //TODO: errnumber 
    }

    for( char* c = inBufferBegin; c < inBufferIt; c++ )
    {
        *dest = *c;   
        dest++;
    }
    *dest = NULL;

    int res = inBufferIt - inBufferBegin;

    inBufferIt ++;
    inBufferBegin = inBufferIt;

    return res;
}

// --- IN OUT END

void parseError()
{
  writeErr( SYNTAX_ERROR_STR );
  writeErr( "\n" );
}
