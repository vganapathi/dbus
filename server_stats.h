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
 * @defgroup Server statistics management
 * @{
 */

/**
 * @file server_stats.h
 * @author Jim Lieb <jlieb@panasas.com>
 * @brief Server statistics
 */

#ifndef SERVER_STATS_H
#define SERVER_STATS_H

#include <sys/types.h>
#include "client_mgr.h"

struct req_op_context {
    sockaddr_t *caller_addr; /*< IP connection info */
    const uint64_t*clientid; /*< Client ID of caller, NULL if
                               unknown/not applicable. */
    struct gsh_client *client; /*< mostly stats but other bits too */
    nsecs_elapsed_t start_time; /*< start time of this op/request */
    nsecs_elapsed_t queue_wait; /*< time in wait queue */
    /* add new context members here */
};

#if 0
void server_stats_nfs_done(struct req_op_context *req_ctx,
			   int export_id,
			   request_data_t *reqdata,
			   int rc,
			   bool dup);
#endif

void server_stats_io_done(struct req_op_context *req_ctx,
			  int export_id,
			  size_t requested,
			  size_t transferred,
			  bool success,
			  bool is_write);
void server_stats_compound_done(struct req_op_context *req_ctx,
				int export_id,
				int num_ops,
				int status);
void server_stats_nfsv4_op_done(struct req_op_context *req_ctx,
				int export_id,
				int proto_op,
				nsecs_elapsed_t start_time,
				int status);

#endif /* !SERVER_STATS_H */
/** @} */
