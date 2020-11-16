/* dnf-command.h
 *
 * Copyright © 2016 Igor Gnatenko <ignatenko@redhat.com>
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

#include <glib-object.h>
#include <libdnf/libdnf.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND dnf_command_get_type ()
G_DECLARE_INTERFACE (DnfCommand, dnf_command, DNF, COMMAND, GObject)

struct _DnfCommandInterface
{
  GTypeInterface parent_iface;

  gboolean (*run) (DnfCommand      *cmd,
                   int              argc,
                   char            *argv[],
                   GOptionContext  *opt_ctx,
                   DnfContext      *ctx,
                   GError         **error);
};

gboolean dnf_command_run (DnfCommand      *cmd,
                          int              argc,
                          char            *argv[],
                          GOptionContext  *opt_ctx,
                          DnfContext      *ctx,
                          GError         **error);

G_END_DECLS
