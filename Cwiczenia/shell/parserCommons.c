#include <unistd.h>

#include "siparse.h"

int isLastPCmd( command **pcmd )
{
  return ( *( pcmd + 1 ) ) == NULL;
}
int isFirstPCmd( command **pcmd, pipeline *p )
{
  return pcmd == *p;
}

int isLineInvalid( line *ln )
{
  if ( ln == NULL ) return 1;

  pipeline *p = ln->pipelines;

  if ( p == NULL ) return 1;

  while ( *p != NULL )
  {
    command **pcmd = *p;

    if ( pcmd == NULL ) return 1;

    while ( *pcmd != NULL )
    {
      command *cmd = *pcmd;

      if ( cmd == NULL || ( cmd->argv[0] == NULL ) )
      {
        if ( !isFirstPCmd( pcmd, p ) ) return 1;

        break;
      }

      pcmd++;
    }

    p++;
  }

  return 0;
}
int isPipelineEmpty( pipeline *p )
{
  command **pcmd = *p;

  return ( *pcmd )->argv[0] == NULL;
}
