/* dnf-utils.h
 *
 * Copyright © 2016 Colin Walters <walters@verbum.org>
 * Copyright © 2016-2017 Igor Gnatenko <ignatenko@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <glib.h>
#include <libdnf/libdnf.h>

G_BEGIN_DECLS

gboolean dnf_utils_print_transaction (DnfContext *ctx);
gboolean dnf_utils_conf_main_get_bool_opt (const gchar *name, enum DnfConfPriority *priority);
gboolean dnf_utils_userconfirm (void);

G_END_DECLS
