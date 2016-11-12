#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"

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

struct stat sb;

void writePrompt()
{
  fstat( 0, &sb );

  if ( ( sb.st_mode & S_IFMT ) == S_IFCHR )
  {
    writeOut( PROMPT_STR );
  }
}

#define INBUFFER_SIZE MAX_LINE_LENGTH * 2
char inBuffer[INBUFFER_SIZE];

char *inBufferIt = inBuffer;
char *inBufferBegin = inBuffer;
char *inBufferEnd = inBuffer;

int inBufferCurrentPos();
int warpBuffer();
int fillLine( char *dest, int maxSize );
void setPointersToEndOfLine();
void setPointersDefault();
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
          tooLongLine = true;
          break;
        }
      }

      if ( *inBufferIt == '\n' )
      {
        if ( tooLongLine )
        {
          errno = ENOMEM;
          return -1;
        }

        return fillLine( str, maxSize );
      }

      inBufferIt++;
    }

    int readBytes = read( 0, inBufferIt, INBUFFER_SIZE - inBufferCurrentPos() );

    if ( readBytes > 0 )
    {
      inBufferEnd += readBytes;
    }
    else
    {
      if ( readBytes < 0 && ( errno == EAGAIN || errno == EINTR ) ) continue;

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
    errno = ENOMEM;
    setPointersToEndOfLine();
    return -1;
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

void parseError()
{
  writeErr( SYNTAX_ERROR_STR );
  writeErr( "\n" );
}
