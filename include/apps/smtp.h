#ifndef		__SMTP_H__
#define		__SMTP_H__

#ifdef		__cplusplus
extern	"C" {
#endif

#include "common.h"
#include "param.h"
#include "linux_list.h"

#define     ARPADATE_LENGTH			(32)
#define     SMTP_BUF_SIZE   		(3072)
#define     SMTP_SEND_TIMEOUT       (3000)      /* 3000ms */
#define     SMTP_CONNECT_TIMEOUT    (3000)
							
#define     EMAIL_BODY_MSG		\
    			"From: %s\r\n"			\
    			"To: %s\r\n"			\
    			"CC: %s,%s,%s\r\n"			\
    			"Subject: %s\r\n"		\
    			"Date: %s\r\n"			\
    			"MIME-Version: 1.0\r\n"		\
    			"Content-Type: multipart/mixed;\r\n"		\
    			"\tboundary=\"camera_smtp_boundary\"\r\n\r\n"		\
    			"X-Priority: 0\r\n"		\
    			"X-MSMail-Priority: Normal\r\n"		\
    			"X-Mailer: Microsoft Outlook Express 6.00.2800.1437\r\n"			\
    			"X-MimeOLE: Produced By Microsoft MimeOLE V6.00.2800.1441\r\n"		\
    			"This is a multi-part message in MIME format.\r\n\r\n"				\
    			"--camera_smtp_boundary\r\n"		\
    			"Content-Type: text/plain;\r\n"		\
    			"Content-Transfer-Encoding: quoted-printable\r\n\r\n"		\
    			"%s\r\n\r\n"

#define     EMAIL_ATTACHMENT_MSG   \
    			"--camera_smtp_boundary\r\n"			\
    			"Content-Type: video/x-msvideo;\r\n"		\
    			"Content-Transfer-Encoding: base64\r\n"		\
    			"Content-Disposition: attachment;\r\n"		\
    			"\tfilename=\"%s\"\r\n\r\n"

typedef struct ATTACHMENT_LIST_T
{
	struct list_head  list;		/*用于维护链表*/
	char              fileName[STR_512_LEN];
}ATTACHMENT_LIST;

int send_mail_real(SMTP_CONF *sconf, const char *subject, const char *context, ATTACHMENT_LIST *list);
int send_mail(SMTP_CONF *sconf, const char *subject, const char *context, char **file_array, int file_cnt);

#ifdef		__cplusplus
}
#endif

#endif
