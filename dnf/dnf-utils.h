/* dnf-utils.h
 *
 * Copyright © 2016 Colin Walters <walters@verbum.org>
 * Copyright © 2016-2017 Igor Gnatenko <ignatenko@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <libdnf/libdnf.h>

G_BEGIN_DECLS

gboolean dnf_utils_print_transaction (DnfContext *ctx);
gboolean dnf_utils_conf_main_get_bool_opt (const gchar *name, enum DnfConfPriority *priority);
gboolean dnf_utils_userconfirm (void);

G_END_DECLS
