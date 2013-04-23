/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright (C) Panasas Inc., 2013
 * Author: Jim Lieb jlieb@panasas.com
 *
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * -------------
 */

/**
 * @defgroup Client host management
 * @{
 */

/**
 * @file client_mgr.h
 * @author Jim Lieb <jlieb@panasas.com>
 * @brief Client manager
 */

#ifndef CLIENT_MGR_H
#define CLIENT_MGR_H

#include "avltree.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define container_of(addr, type, member) ({                     \
        const typeof( ((type *)0)->member ) *__mptr = (addr);   \
                (type *)( (char *)__mptr - offsetof(type,member) );})

/* My habit with mutex */
#define P( _mutex_ ) pthread_mutex_lock( &_mutex_ )
#define V( _mutex_ ) pthread_mutex_unlock( &_mutex_ )


#define PTHREAD_RWLOCK_wrlock(state_lock)                           \
  do {                                                              \
       int rc;                                                      \
                                                                    \
       LogFullDebug(COMPONENT_RW_LOCK, "get wr lock %p", state_lock);   \
       rc = pthread_rwlock_wrlock(state_lock);                      \
       if (rc == 0)                                                 \
         LogFullDebug(COMPONENT_RW_LOCK, "got wr lock %p", state_lock); \
       else                                                         \
         LogCrit(COMPONENT_RW_LOCK, "error %d wr lock %p", rc, state_lock); \
     } while(0)                                                      \

#define PTHREAD_RWLOCK_rdlock(state_lock)                           \
  do {                                                              \
       int rc;                                                      \
                                                                    \
       LogFullDebug(COMPONENT_RW_LOCK, "get rd lock %p", state_lock);   \
       rc = pthread_rwlock_rdlock(state_lock);                      \
       if (rc == 0)                                                 \
         LogFullDebug(COMPONENT_RW_LOCK, "got rd lock %p", state_lock); \
       else                                                         \
         LogCrit(COMPONENT_RW_LOCK, "error %d rd lock %p", rc, state_lock); \
     } while(0)                                                      \

#define PTHREAD_RWLOCK_unlock(state_lock)                           \
  do {                                                              \
       int rc;                                                      \
                                                                    \
       rc = pthread_rwlock_unlock(state_lock);                      \
       if (rc == 0)                                                 \
         LogFullDebug(COMPONENT_RW_LOCK, "unlock %p", state_lock);      \
       else                                                         \
         LogCrit(COMPONENT_RW_LOCK, "error %d unlock %p", rc, state_lock); \
     } while(0)                                                      \

/* An elapsed time in nanosecs works because an unsigned
 * 64 bit can hold ~584 years of nanosecs.  If any code I have
 * ever written stays up that long, I would be amazed (and dead
 * a very long time...)
 */

typedef uint64_t nsecs_elapsed_t;

static const nsecs_elapsed_t NS_PER_USEC = 1000;
static const nsecs_elapsed_t NS_PER_MSEC = 1000000;
static const nsecs_elapsed_t NS_PER_SEC = 1000000000;

/**
 * @brief Buffer descriptor
 *
 * This structure is used to describe a counted buffer as an
 * address/length pair.
 */

struct gsh_buffdesc {
    void *addr;  /*< First octet/byte of the buffer */
    size_t len;  /*< Length of the buffer */
};

typedef struct sockaddr_in sockaddr_t;

struct gsh_client {
	struct avltree_node node_k;
	pthread_mutex_t lock;
	struct gsh_buffdesc addr;
	int64_t refcnt;
	nsecs_elapsed_t last_update;
	unsigned char addrbuf[];
};

void gsh_client_init(void);
struct gsh_client *get_gsh_client( sockaddr_t *client_ipaddr, 
        bool lookup_only);
void put_gsh_client(struct gsh_client *client);
int foreach_gsh_client(bool (*cb)(struct gsh_client *cl,
				   void *state),
		       void *state);
bool remove_gsh_client(sockaddr_t *);


#endif /* !CLIENT_MGR_H */
/** @} */
