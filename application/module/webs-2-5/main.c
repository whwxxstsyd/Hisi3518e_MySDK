/*
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/

/*
 *	Main program for for the GoAhead WebServer.
 */

/********************************* Includes ***********************************/

#include	"uemf.h"
#include	"wsIntrn.h"
#include	<signal.h>
#include	<unistd.h>
#include	<sys/types.h>

#ifdef WEBS_SSL_SUPPORT
#include	"../websSSL.h"
#endif

#ifdef USER_MANAGEMENT_SUPPORT
#include	"../um.h"
void	formDefineUserMgmt(void);
#endif


/* add by liwei */
//#include "debug.h"          /* 与<syslog.h>中的log函数冲突 */
#include "web_main.h"		/* add by liwei */
#include "common.h"
#include "normal.h"
#include "av_buffer.h"
#include "task.h"
#include "protocol/protocol_base.h"
#include "protocol/h264_protocol.h"
#include "socket.h"
#include <sys/ioctl.h>
/* end add */


int getpwd(char *pszBuf);
/*********************************** Locals ***********************************/
/*
  *	Change configuration here
  */


/* add by liwei */
static int stop_flag = 0;  
CGI_INTERFACE gWebCgiInf;
/* end add */

static char_t		*rootWeb = T("www");			/* Root web directory */
static char_t		*demoWeb = T("wwwdemo");		/* Root web directory */
static char_t		*password = T("");				/* Security password */
static int			port = WEBS_DEFAULT_PORT;		/* Server port */
static int			retries = 5;					/* Server port retries */
static int			finished = 0;					/* Finished flag */

/****************************** Forward Declarations **************************/
static int 	initWebs(void);
static int	aspTest(int eid, webs_t wp, int argc, char_t **argv);
static void formTest(webs_t wp, char_t *path, char_t *query);
static void formUploadFileTest(webs_t wp, char_t *path, char_t *query); // add by gyr 2011.10.15
static int  websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
				int arg, char_t *url, char_t *path, char_t *query);
static void	sigintHandler(int);
#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif

/*********************************** Code *************************************/
/*
 *	Main -- entry point from LINUX
 */

int main_enter(int argc, char** argv)
{
/*
 *	Initialize the memory allocator. Allow use of malloc and start
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */
	bopen(NULL, (60 * 1024), B_USE_MALLOC);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigintHandler);
	signal(SIGTERM, sigintHandler);

/*
 *	Initialize the web server
 */
	if (initWebs() < 0) {
		printf("initWebs error.\n");	// added by gyr 2011.09.17
		return -1;
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLOpen();
/*	websRequireSSL("/"); */	/* Require all files be served via https */
#endif

/*
 *	Basic event loop. SocketReady returns true when a socket is ready for
 *	service. SocketSelect will block until an event occurs. SocketProcess
 *	will actually do the servicing.
 */
	finished = 0;
	while (!finished) 
	{
		if (socketReady(-1) || socketSelect(-1, 1000)) {
			socketProcess(-1);
		}
		websCgiCleanup();
		emfSchedProcess();
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLClose();
#endif

#ifdef USER_MANAGEMENT_SUPPORT
	umClose();
#endif

/*
 *	Close the socket module, report memory leaks and close the memory allocator
 */
	websCloseServer();
	socketClose();
#ifdef B_STATS
	memLeaks();
#endif
	bclose();
	return 0;
}

/*
 *	Exit cleanly on interrupt
 */
static void sigintHandler(int unused)
{
	finished = 1;
}

/*
 *	Get local host IP
 */
char* GetLocalHostIP ()
{  
    int  MAXINTERFACES = 16;
    char *ip=NULL;
    int fd, intrface, retn = 0;
    struct ifreq buf[MAXINTERFACES]; ///if.h
    struct ifconf ifc; ///if.h

    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) //socket.h
    {
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) //ioctl.h
        {
            intrface = ifc.ifc_len / sizeof (struct ifreq);

            while (intrface-->0)
            {
                if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))
                {
                    ip=(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));//types
                    break;
                }
            }
        }
        close (fd);
    }
    return ip;
}



/******************************************************************************/
/*
 *	Initialize the web server.
 */

static int initWebs()
{
	struct in_addr	intaddr;
	char			host[128], dir[128], webdir[128];
	char			*cp;
	char_t			wbuf[128];
	char      *pszTmp = NULL;

/*
 *	Initialize the socket subsystem
 */
	socketOpen();

#ifdef USER_MANAGEMENT_SUPPORT
/*
 *	Initialize the User Management database
 */
	umOpen();
	umRestore(T("./"));

#endif

/*
 *	Define the local Ip address, host name, default home page and the
 *	root web directory.
 */
	 if (gethostname(host, sizeof(host)) < 0) {
		 error(E_L, E_LOG, T("Can't get hostname"));
		 return -1;
	 }

 /* del by gyr 2011.09.17
	if ((hp = gethostbyname(host)) == NULL) {
		error(E_L, E_LOG, T("Can't get host address"));
		printf("initWebs::Can't get hostname...\n");	// added by gyr 2011.09.17
		return -1;
	}
	memcpy((char *) &intaddr, (char *) hp->h_addr_list[0],
		(size_t) hp->h_length);
*/

	cp = GetLocalHostIP();

/*
 *	Set ../web as the root web. Modify this to suit your needs
 *	A "-demo" option to the command line will set a webdemo root
 */
	//getcwd(dir, sizeof(dir));// Get the name of the current working directory, note | gyr 2011.09.17

	if (0 != getpwd(dir)) 
	{
		return -1;
	}

	if ((pszTmp = strrchr(dir, '/'))) {
		*pszTmp = '\0';
	}
	
	sprintf(webdir, "./%s", rootWeb);
	printf("webdir: %s\n", webdir);


/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultDir(webdir);
	ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
	websSetIpaddr(wbuf);
	ascToUni(wbuf, host, min(strlen(host) + 1, sizeof(wbuf)));
	websSetHost(wbuf);

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultPage(T("default.asp"));
	websSetPassword(password);

/*
 *	Open the web server on the given port. If that port is taken, try
 *	the next sequential port for up to "retries" attempts.
 */
	websOpenServer(port, retries);

/*
 * 	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security
 *	handler, forms handler and the default web page handler.
 */
	websUrlHandlerDefine(T(""), NULL, 0, websSecurityHandler,WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
	websUrlHandlerDefine(T("/cgi-bin"), NULL, 0, websCgiHandler, 0);
	websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler,WEBS_HANDLER_LAST);

/*
 *	Now define two test procedures. Replace these with your application
 *	relevant ASP script procedures and form functions.
 */
	websAspDefine(T("aspTest"), aspTest);
	websFormDefine(T("formTest"), formTest);
	websFormDefine(T("formUploadFileTest"), formUploadFileTest);// add by gyr 2011.10.15

/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
	formDefineUserMgmt();
#endif

/*
 *	Create a handler for the default home page
 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0);

	return 0;
}

/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to
 *	"localhost/asp.asp" to test.
 */

static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;

	if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	return websWrite(wp, T("Name: %s, Address %s"), name, address);
}

/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

static void formTest(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T("Joe Smith"));
	address = websGetVar(wp, T("address"), T("1212 Milky Way Ave."));

	websHeader(wp);
	websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2>\n"), name, address);
	websFooter(wp);
	websDone(wp, 200);
}

/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
	int arg, char_t *url, char_t *path, char_t *query)
{
/*
 *	If the empty or "/" URL is invoked, redirect default URLs to the home page
 */
	if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
		websRedirect(wp, WEBS_DEFAULT_HOME);
		return 1;
	}
	return 0;
}

/******************************************************************************/

#ifdef B_STATS
static void memLeaks()
{
	int		fd;

	if ((fd = gopen(T("leak.txt"), O_CREAT | O_TRUNC | O_WRONLY, 0666)) >= 0) {
		bstats(fd, printMemStats);
		close(fd);
	}
}

/******************************************************************************/
/*
 *	Print memory usage / leaks
 */

static void printMemStats(int handle, char_t *fmt, ...)
{
	va_list		args;
	char_t		buf[256];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	write(handle, buf, strlen(buf));
}
#endif

/******************************************************************************/
/*first test the goahead
/* add by HTQ 2017.7.19
  */
static void helloform(webs_t wp, char_t *path, char_t *query)

{

	char_t *str;

	str = websGetVar(wp, T("echo"), T("Joe Smith"));

	websHeader(wp);

	websWrite(wp, T("<h2>%s</h2>"), str);

	websFooter(wp);

	websDone(wp, 200);

}





/******************************************************************************/
/* for test html upload file to web server
/* add by gyr 2011.10.15
/******************************************************************************/


static void formUploadFileTest(webs_t wp, char_t *path, char_t *query)
{
    FILE *       fp;
    char_t *     fn;
    char_t *     bn = NULL;
    int          locWrite;
    int          numLeft;
    int          numWrite;

	printf("\n...................formUploadFileTest...................\n\n");

    a_assert(websValid(wp));
    websHeader(wp);

    fn = websGetVar(wp, T("filename"), T(""));
    if (fn != NULL && *fn != '\0') {
        if ((int)(bn = gstrrchr(fn, '/') + 1) == 1) {
            if ((int)(bn = gstrrchr(fn, '\\') + 1) == 1) {
                bn = fn;
            }
        }
    }

	printf("fn=%s, bn=%s  .......\n", fn, bn);

    websWrite(wp, T("Filename = %s<br>Size = %d bytes<br>"), bn, wp->lenPostData);

    if ((fp = fopen((bn == NULL ? "upldForm.bin" : bn), "w+b")) == NULL) {
        websWrite(wp, T("File open failed!<br>"));
    } else {
        locWrite = 0;
        numLeft = wp->lenPostData;
        while (numLeft > 0) {
            numWrite = fwrite(&(wp->postData[locWrite]), sizeof(*(wp->postData)), numLeft, fp);
            if (numWrite < numLeft) {
                websWrite(wp, T("File write failed.<br>  ferror=%d locWrite=%d numLeft=%d numWrite=%d Size=%d bytes<br>"), ferror(fp), locWrite, numLeft, numWrite, wp->lenPostData);
            break;
            }
            locWrite += numWrite;
            numLeft -= numWrite;
        }

        if (numLeft == 0) {
            if (fclose(fp) != 0) {
                websWrite(wp, T("File close failed.<br>  errno=%d locWrite=%d numLeft=%d numWrite=%d Size=%d bytes<br>"), errno, locWrite, numLeft, numWrite, wp->lenPostData);
            } else {
                websWrite(wp, T("File Size Written = %d bytes<br>"), wp->lenPostData);
            }
        } else {
            websWrite(wp, T("numLeft=%d locWrite=%d Size=%d bytes<br>"), numLeft, locWrite, wp->lenPostData);
        }
    }

    websFooter(wp);
    websDone(wp, 200);

}

/******************************************************************************/


#define MAXBUFSIZE 1024

int getpwd(char *pszBuf)
{
	int count;
	
	count = readlink("/proc/self/exe", pszBuf, MAXBUFSIZE);
	if ( count < 0 || count >= MAXBUFSIZE ) {
		perror("readlink");
		
		return(EXIT_FAILURE);
	}
	
	pszBuf[count] = '\0';
	
	return(EXIT_SUCCESS);
}


/* add by liwei */
void restart_web_server(void)
{
    finished = 1;
}

int start_web_server(void)
{   
    while (!stop_flag)
    {
        if (main_enter() < 0)
        {
            return -1;
        }
    }

    return 0;
}

void stop_web_server(void)
{
    stop_flag = 1;
    finished = 1;
}

void web_set_interface(SET_PARAM_STR_FUNC set_param_str_inf, 
	                        SET_PARAM_INT_FUNC set_param_int_inf, 
	                        GET_PARAM_FUNC get_param_inf, 
	                        CMD_PROC_FUNC cmd_proc_inf)
{
    gWebCgiInf.set_param_str_func = set_param_str_inf;
    gWebCgiInf.set_param_int_func = set_param_int_inf;
    gWebCgiInf.get_param_func = get_param_inf;
    gWebCgiInf.cmd_proc_func = cmd_proc_inf;
}
/* end add */


