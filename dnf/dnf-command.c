/* dnf-command.c
 *
 * Copyright © 2016 Igor Gnatenko <ignatenko@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dnf-command.h"

G_DEFINE_INTERFACE (DnfCommand, dnf_command, G_TYPE_OBJECT)

static void
dnf_command_default_init (DnfCommandInterface *iface)
{
}

gboolean
dnf_command_run (DnfCommand      *cmd,
                 int              argc,
                 char            *argv[],
                 GOptionContext  *opt_ctx,
                 DnfContext      *ctx,
                 GError         **error)
{
  g_return_val_if_fail (DNF_IS_COMMAND (cmd), FALSE);

  DnfCommandInterface *iface = DNF_COMMAND_GET_IFACE (cmd);
  g_return_val_if_fail (iface->run, TRUE);
  return iface->run (cmd, argc, argv, opt_ctx, ctx, error);
}
