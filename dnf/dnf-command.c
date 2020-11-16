/* dnf-command.c
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
