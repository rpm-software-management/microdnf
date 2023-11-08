/* dnf-command.h
 *
 * Copyright © 2016 Igor Gnatenko <ignatenko@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
