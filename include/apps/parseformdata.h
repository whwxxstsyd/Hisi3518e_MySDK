/* parseformdata.h - parse form data
**
** Copyright ?1995 by Jef Poskanzer <jef@mail.acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#ifndef _PARSEFORMDATA_H_
#define _PARSEFORMDATA_H_

#ifdef	__cplusplus
extern "C" {
#endif

/****************************************************************/

extern char*	gogetenv(char *varname);
extern char*    getCGIstr(void);
extern char**	getCGIvars( void );
extern char*	findCGIvar( char *list[], char *name );
extern void 	freeCGIvars( char *list[] );
extern char*	getfilebuf( char *filename, int fnamelen, int *buflen );


#define HTTP_HEAD "Content-Type: text/plain\r\n\r\n"
#define	HTTP_TEXT_PLAIN		"Content-Type: text/plain\r\n\r\n"
#define XML_HEAD "<?xml version=\"1.0\" ?>\r\n"
#define XML_FRAME_D "<%s>%d</%s>\r\n"
#define XML_FRAME_S "<%s>%s</%s>\r\n"
#define MAX_TEXT_SIZE	32
#define MAX_SEND_BUF	8192

#ifdef	__cplusplus
}
#endif

#endif /* _PARSEFORMDATA_H_ */
