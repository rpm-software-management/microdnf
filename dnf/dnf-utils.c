/* dnf-utils.c
 *
 * Copyright © 2010-2015 Richard Hughes <richard@hughsie.com>
 * Copyright © 2016 Colin Walters <walters@verbum.org>
 * Copyright © 2016 Igor Gnatenko <ignatenko@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include "dnf-utils.h"

void
dnf_utils_print_transaction (DnfContext *ctx)
{
  g_autoptr(GPtrArray) pkgs = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                     DNF_PACKAGE_INFO_INSTALL,
                                                     DNF_PACKAGE_INFO_REINSTALL,
                                                     DNF_PACKAGE_INFO_DOWNGRADE,
                                                     DNF_PACKAGE_INFO_UPDATE,
                                                     -1);
  g_print ("Transaction: ");
  if (pkgs->len == 0)
    g_print ("(empty)");
  else
    g_print ("%u packages", pkgs->len);
  g_print ("\n");

  for (guint i = 0; i < pkgs->len; i++)
    {
      DnfPackage *pkg = pkgs->pdata[i];
      g_print ("%s (%s)\n", dnf_package_get_nevra (pkg), dnf_package_get_reponame (pkg));
    }
}
