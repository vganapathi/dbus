/*
 *
 *
 * Copyright CEA/DAM/DIF  (2008)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * ---------------------------------------
 */

/**
 * @file    nfs_core.h
 * @brief   Prototypes for the different threads in the nfs core
 */

#ifndef NFS_CORE_H
#define NFS_CORE_H

#include <pthread.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>

#define RQCRED_SIZE 400 /* this size is excessive */

/* Arbitrary string buffer lengths */
#define AUTH_STR_LEN 30
#define PWENT_MAX_LEN 81

/* Hard and soft limit for nfsv4 quotas */
#define NFS_V4_MAX_QUOTA_SOFT 4294967296LL /*  4 GB */
#define NFS_V4_MAX_QUOTA_HARD 17179869184LL /* 16 GB */
#define NFS_V4_MAX_QUOTA      34359738368LL /* 32 GB */

#define XATTR_BUFFERSIZE 4096

typedef enum request_type {
	NFS_CALL,
	NFS_REQUEST,
} request_type_t;

typedef struct request_data {
	request_type_t rtype;
	pthread_mutex_t mtx;
	pthread_cond_t cv;
	struct timespec time_queued; /*< The time at which a request was added
				     *  to the worker thread queue. */
} request_data_t;

/**
 * @todo Matt: this is automatically redundant, but in fact upstream
 * TI-RPC is not up-to-date with RFC 5665, will fix (Matt)
 *
 * @copyright 2012, Linux Box Corp
*/
enum rfc_5665_nc_type {
	_NC_ERR,
	_NC_TCP,
	_NC_TCP6,
	_NC_RDMA,
	_NC_RDMA6,
	_NC_SCTP,
	_NC_SCTP6,
	_NC_UDP,
	_NC_UDP6,
};
typedef enum rfc_5665_nc_type nc_type;

typedef struct gsh_addr {
	nc_type nc;
	struct sockaddr_storage ss;
	uint32_t port;
} gsh_addr_t;

/* ServerEpoch is ServerBootTime unless overriden by -E command line option */
struct timespec ServerBootTime;

/* used in DBUS-api diagnostic functions (e.g., serialize sessionid) */
int b64_ntop(u_char const *src, size_t srclength, char *target,
	     size_t targsize);
int b64_pton(char const *src, u_char *target, size_t targsize);

/**
 * @brief Logging mutex lock
 *
 * Based on Marc Eshel's error checking read-write lock macros, check
 * the return value of pthread_mutex_lock and log any non-zero value.
 *
 * @param[in,out] mtx The mutex to acquire
 */

#define PTHREAD_MUTEX_lock(mtx)						\
	do {								\
		int rc;							\
									\
		LogFullDebug(COMPONENT_RW_LOCK,				\
			     "Lock mutex %p", mtx);			\
		rc = pthread_mutex_lock(mtx);				\
		if (rc == 0) {						\
			LogFullDebug(COMPONENT_RW_LOCK,			\
				     "Got mutex %p", mtx);		\
		} else{							\
			LogCrit(COMPONENT_RW_LOCK,			\
				"Error %d acquiring mutex %p",		\
				rc, mtx);				\
		}							\
	} while(0)

/**
 * @brief Logging mutex unlock
 *
 * Based on Marc Eshel's error checking read-write lock macros, check
 * the return value of pthread_mutex_unlock and log any non-zero
 * value.
 *
 * @param[in,out] mtx The mutex to relinquish
 */

#define PTHREAD_MUTEX_unlock(mtx)					\
	do {								\
		int rc;							\
									\
		LogFullDebug(COMPONENT_RW_LOCK,				\
			     "Unlock mutex %p", mtx);			\
		rc = pthread_mutex_unlock(mtx);				\
		if (rc == 0) {						\
			LogFullDebug(COMPONENT_RW_LOCK,			\
				     "Released mutex %p", mtx);		\
		} else {						\
			LogCrit(COMPONENT_RW_LOCK,			\
				"Error %d relinquishing mutex %p",	\
				rc, mtx);				\
		}							\
	} while(0)

#endif /* !NFS_CORE_H */
