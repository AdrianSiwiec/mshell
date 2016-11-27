#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"

extern int _debug;

void writeErr( char *str )
{
  write( 2, str, strlen( str ) );
}
void writeOut( char *str )
{
  write( 1, str, strlen( str ) );
}
void writeIntOut( int i )
{
  char buf[12] = {0};
  sprintf( buf, "%d", i );
  writeOut( buf );
}

int isInTty()
{
  struct stat sb;
  fstat( 0, &sb );

  return ( sb.st_mode & S_IFMT ) == S_IFCHR;
}

void writePrompt()
{
  writeOut( PROMPT_STR );
}

void parseError()
{
  writeErr( SYNTAX_ERROR_STR );
  writeErr( "\n" );
}

void printErrno( char *filename, int errnum )
{
  if( filename != NULL )
  {
    writeErr( filename );
    writeErr(": ");
  }
  switch ( errnum )
  {
    case ENOENT:
      writeErr( "no such file or directory\n" );
      break;

    case EACCES:
      writeErr( "permission denied\n" );
      break;

    default:
      writeErr( "unhandled errno\n" );
  }
}

// readLine start

#define INBUFFER_SIZE MAX_LINE_LENGTH * 2
int inBufferCurrentPos();
int warpBuffer();
int fillLine( char *dest, int maxSize );
void setPointersToEndOfLine();
void setPointersDefault();

char inBuffer[INBUFFER_SIZE];
char *inBufferIt = inBuffer;
char *inBufferBegin = inBuffer;
char *inBufferEnd = inBuffer;


int readLine( char *str, int maxSize )
{
  bool tooLongLine = false;

  while ( 1 )
  {
    while ( inBufferIt < inBufferEnd )
    {
      if ( inBufferCurrentPos() >= INBUFFER_SIZE - 1 )
      {
        if ( warpBuffer() )
        {
          if(_debug) printf("__line longer than buffer\n");
          tooLongLine = true;
          break;
        }
      }

      if ( *inBufferIt == '\n' )
      {
        if(_debug) printf("__got endl!, before then was: %c\n", *(inBufferIt-1));
        if ( tooLongLine )
        {
          fillLine( str, maxSize );
          return -2;
        }

        return fillLine( str, maxSize );
      }

      inBufferIt++;
    }

    int readBytes = read( 0, inBufferIt, INBUFFER_SIZE - inBufferCurrentPos() );
    if(_debug) printf("__read: %d\n", readBytes);

    if ( readBytes > 0 )
    {
      inBufferEnd += readBytes;
    }
    else if(readBytes == 0 )
    {
      if( !isInTty() && inBufferIt - inBufferBegin > 0)
      {
        *(inBufferIt) = '\n';
        inBufferEnd++;
      }
      else
      {
        return 0;
      }
    }
    else
    {
      if ( errno == EAGAIN || errno == EINTR ) continue;

      return readBytes;
    }
  }
}

int inBufferCurrentPos()
{
  return inBufferIt - inBuffer;
}

int warpBuffer()
{
  if ( inBufferBegin == inBuffer )
  {
    setPointersDefault();
    return -1;
  }

  char *ptr = inBuffer;
  char *c;

  for ( c = inBufferBegin; c < inBufferEnd; c++ )
  {
    *ptr = *c;
    ptr++;
  }

  int moved = inBufferBegin - inBuffer;
  inBufferBegin -= moved;
  inBufferIt -= moved;
  inBufferEnd -= moved;

  return 0;
}

int fillLine( char *dest, int maxSize )
{
  if ( inBufferIt - inBufferBegin > maxSize )
  {
    setPointersToEndOfLine();
    return -2;
  }

  char *c;

  for ( c = inBufferBegin; c < inBufferIt; c++ )
  {
    *dest = *c;
    dest++;
  }

  *dest = NULL;

  int res = inBufferIt - inBufferBegin;

  if ( res == 0 )
  {
    *dest = ' ';
    dest++;
    *dest = NULL;
    res++;
  }

  setPointersToEndOfLine();

  return res;
}

void setPointersToEndOfLine()
{
  inBufferIt++;
  inBufferBegin = inBufferIt;
}

void setPointersDefault()
{
  inBufferIt = inBuffer;
  inBufferBegin = inBuffer;
  inBufferEnd = inBuffer;
}


