#ifndef __SYS_IPC_H__
#define __SYS_IPC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Copyright (c) 1984 AT&T */
/*   All Rights Reserved   */

/* THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/* The copyright notice above does not evidence any      */
/* actual or intended publication of such source code.   */

/*#ident "@(#)kern-port:sys/ipc.h   10.2"*/
#ident   "$Revision: 1.1.1.1 $"
/* Common IPC Access Structure */

#include <sys/types.h>
typedef unsigned short ushort;
typedef unsigned short key_t;

struct ipc_perm {
   ushort   uid;  /* owner's user id */
   ushort   gid;  /* owner's group id */
   ushort   cuid; /* creator's user id */
   ushort   cgid; /* creator's group id */
   ushort   mode; /* access modes */
   ushort   seq;  /* slot usage sequence number */
   key_t key;  /* key */
};

/* Common IPC Definitions. */
/* Mode bits. */
#define  IPC_ALLOC   0100000     /* entry currently allocated */
#define  IPC_CREAT   0001000     /* create entry if key doesn't exist */
#define  IPC_EXCL 0002000     /* fail if key exists */
#define  IPC_NOWAIT  0004000     /* error if request must wait */

/* Keys. */
#define  IPC_PRIVATE (key_t)0 /* private key */

/* Control Commands. */
#define  IPC_RMID 0  /* remove identifier */
#define  IPC_SET     1  /* set options */
#define  IPC_STAT 2  /* get options */

#ifndef _KERNEL

extern key_t   ftok(const char *, char);

#ifdef __cplusplus
}
#endif

#endif /* !_KERNEL */
#endif /* !__SYS_IPC_H__ */
