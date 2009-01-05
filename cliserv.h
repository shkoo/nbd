#ifndef NBD_CLISERV_H
#define NBD_CLISERV_H 1

/* This header file is shared by client & server. They really have
 * something to share...
 * */

/* Client/server protocol is as follows:
   Password authentication if specified
   Send NBD_HELLO
   Send 64-bit cliserv_magic
   Send 64-bit size of exported device
   Send 128 bytes of zeros (reserved for future use)
 */

#include "config.h"

#include "lfs.h"

#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <stdlib.h>

#if SIZEOF_UNSIGNED_SHORT_INT==4
typedef unsigned short u32;
#elif SIZEOF_UNSIGNED_INT==4
typedef unsigned int u32;
#elif SIZEOF_UNSIGNED_LONG_INT==4
typedef unsigned long u32;
#else
#error I need at least some 32-bit type
#endif

#if SIZEOF_UNSIGNED_INT==8
typedef unsigned int u64;
#elif SIZEOF_UNSIGNED_LONG_INT==8
typedef unsigned long u64;
#elif SIZEOF_UNSIGNED_LONG_LONG_INT==8
typedef unsigned long long u64;
#else
#error I need at least some 64-bit type
#endif

#define __be32 u32
#define __be64 u64
#include "nbd.h"

#if NBD_LFS==1
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#endif

#ifndef NBD_AUTH_C
u64 cliserv_magic = 0x00420281861253LL;
#endif
#define NBD_HELLO "NBDMAGIC"

#define INFO(a) do { } while(0)

#ifndef NBD_AUTH_C
void setmysockopt(int sock) {
	int size = 1;
#if 0
	if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, sizeof(int)) < 0)
		 INFO("(no sockopt/1: %m)");
#endif
#ifdef	IPPROTO_TCP
	size = 1;
	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &size, sizeof(int)) < 0)
		 INFO("(no sockopt/2: %m)");
#endif
#if 0
	size = 1024;
	if (setsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, &size, sizeof(int)) < 0)
		 INFO("(no sockopt/3: %m)");
#endif
}
#endif /* NBD_AUTH_C */

#ifndef G_GNUC_NORETURN
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define G_GNUC_NORETURN __attribute__((__noreturn__))
#else
#define G_GNUC_NORETURN
#endif
#endif

#ifndef NBD_AUTH_C

void err(const char *s) G_GNUC_NORETURN;

void err(const char *s) {
	const int maxlen = 150;
	char s1[maxlen], *s2;

	strncpy(s1, s, maxlen);
	if ((s2 = strstr(s, "%m"))) {
		strcpy(s1 + (s2 - s), strerror(errno));
		s2 += 2;
		strcpy(s1 + strlen(s1), s2);
	}
#ifndef	sun
	/* Solaris doesn't have %h in syslog */
	else if ((s2 = strstr(s, "%h"))) {
		strcpy(s1 + (s2 - s), hstrerror(h_errno));
		s2 += 2;
		strcpy(s1 + strlen(s1), s2);
	}
#endif

	s1[maxlen-1] = '\0';
#ifdef ISSERVER
	syslog(LOG_ERR, "%s", s1);
	syslog(LOG_ERR, "Exiting.");
#endif
	fprintf(stderr, "Error: %s\nExiting.\n", s1);
	exit(1);
}

#ifdef WORDS_BIGENDIAN
u64 ntohll(u64 a) {
	return a;
}
#else
u64 ntohll(u64 a) {
	u32 lo = a & 0xffffffff;
	u32 hi = a >> 32U;
	lo = ntohl(lo);
	hi = ntohl(hi);
	return ((u64) lo) << 32U | hi;
}
#endif
#endif /* NBD_AUTH_C */
#define htonll ntohll

/* Flags used between the client and server */
#define NBD_FLAG_HAS_FLAGS	(1 << 0)	/* Flags are there */
#define NBD_FLAG_READ_ONLY	(1 << 1)	/* Device is read-only */

#endif /* NBD_CLISERV_H */
