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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * -------------
 */

/**
 * @defgroup Filesystem export management
 * @{
 */

/**
 * @file export_mgr.c
 * @author Jim Lieb <jlieb@panasas.com>
 * @brief export manager
 */

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <pthread.h>
#include <assert.h>
#include "nfs_core.h"
#include "log.h"
#include "avltree.h"
#include "osc_osd_dbus.h"
#include "export_mgr.h"
#include "server_stats_private.h"
#include "server_stats.h"
#include "abstract_mem.h"
#include "abstract_atomic.h"


/* Exports are stored in an AVL tree
 */

struct export_by_id {
	struct avltree t;
	pthread_rwlock_t lock;
};

static struct export_by_id export_by_id;

/**
 * @brief Export id comparator for AVL tree walk
 *
 */

static int
export_id_cmpf(const struct avltree_node *lhs,
	       const struct avltree_node *rhs)
{
	struct gsh_export *lk, *rk;

	lk = avltree_container_of(lhs, struct gsh_export, node_k);
	rk = avltree_container_of(rhs, struct gsh_export, node_k);
	if(lk->export_id != rk->export_id)
		return (lk->export_id < rk->export_id) ? -1 : 1;
	else
		return 0;
}

/**
 * @brief Lookup the export manager struct for this export id
 *
 * Lookup the export manager struct by export id.
 * Export ids are assigned by the config file and carried about
 * by file handles.
 *
 * @param export_id   [IN] the export id extracted from the handle
 * @param lookup_only [IN] if true, don't create a new entry
 *
 * @return pointer to ref locked stats block
 */

struct gsh_export *get_gsh_export(int export_id,
				  bool lookup_only)
{
	struct avltree_node *node = NULL;
	struct gsh_export *exp;
	struct export_stats *export_st;
	struct gsh_export v;

/* NOTE: If we call this in the general case, not from within stats
 * code we have to do the following.  We currently get away with it 
 * because the stats code has already done this and passed the export id
 * it found to the stats harvesting functions.  This table has a 1 - 1
 * relationship to an exportlist entry but no linkage because exportlist
 * is a candidate for rework when the pseudo fs "fsal" is done.
 *  Don't muddy the waters right now.
 */

/* 	pexport = nfs_Get_export_by_id(nfs_param.pexportlist, */
/* 					exportid)) == NULL) */
	v.export_id = export_id;

	PTHREAD_RWLOCK_rdlock(&export_by_id.lock);
	node = avltree_lookup(&v.node_k, &export_by_id.t);
	if(node) {
		exp = avltree_container_of(node, struct gsh_export, node_k);
		goto out;
	} else if(lookup_only) {
		PTHREAD_RWLOCK_unlock(&export_by_id.lock);
		return NULL;
	}
	PTHREAD_RWLOCK_unlock(&export_by_id.lock);

	export_st = gsh_calloc(sizeof(struct export_stats), 1);
	if(export_st == NULL) {
		return NULL;
	}
	exp = &export_st->export;
	exp->export_id = export_id;
	exp->refcnt = 0;  /* we will hold a ref starting out... */

	PTHREAD_RWLOCK_wrlock(&export_by_id.lock);
	node = avltree_insert(&exp->node_k, &export_by_id.t);
	if(node) {
		gsh_free(export_st); /* somebody beat us to it */
		exp = avltree_container_of(node, struct gsh_export, node_k);
	} else {
		pthread_mutex_init(&exp->lock, NULL);
	}

out:
	atomic_inc_int64_t(&exp->refcnt);
	PTHREAD_RWLOCK_unlock(&export_by_id.lock);
	return exp;
}

/**
 * @brief Release the export management struct
 *
 * We are done with it, let it go.
 */

void put_gsh_export(struct gsh_export *export)
{
	assert(export->refcnt > 0);
	atomic_dec_int64_t(&export->refcnt);
}

/**
 * @ Walk the tree and do the callback on each node
 *
 * @param cb    [IN] Callback function
 * @param state [IN] param block to pass
 */

int foreach_gsh_export(bool (*cb)(struct gsh_export *cl,
				  void *state),
		       void *state)
{
	struct avltree_node *export_node;
	struct gsh_export *exp;
	int cnt = 0;

	PTHREAD_RWLOCK_rdlock(&export_by_id.lock);
	for(export_node = avltree_first(&export_by_id.t);
	    export_node != NULL;
	    export_node = avltree_next(export_node)) {
		exp = avltree_container_of(export_node, struct gsh_export, node_k);
		if( !cb(exp, state))
			break;
		cnt++;
	}
	PTHREAD_RWLOCK_unlock(&export_by_id.lock);
	return cnt;
}

/**
 * @brief Initialize export manager
 */

void gsh_export_init(void)
{
	pthread_rwlockattr_t rwlock_attr;

	pthread_rwlockattr_init(&rwlock_attr);
#ifdef GLIBC
	pthread_rwlockattr_setkind_np(
		&rwlock_attr,
		PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
#endif
	pthread_rwlock_init(&export_by_id.lock, &rwlock_attr);
	avltree_init(&export_by_id.t, export_id_cmpf, 0);
}


/** @} */
