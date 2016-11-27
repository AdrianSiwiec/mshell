#ifndef _PARSER_COMMONS_H_
#define _PARSER_COMMONS_H_

int isLastPCmd( command **pcmd );
int isFirstPCmd( command **pcmd, pipeline *p );
int isLineInvalid( line *ln );
int isPipelineEmpty( pipeline *p );

#endif /* !_PARSER_COMMONS_H_ */
