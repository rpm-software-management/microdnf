/* dnf-utils.c
 *
 * Copyright © 2010-2015 Richard Hughes <richard@hughsie.com>
 * Copyright © 2016 Colin Walters <walters@verbum.org>
 * Copyright © 2016-2017 Igor Gnatenko <ignatenko@redhat.com>
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


static gint
dnf_package_cmp_cb (DnfPackage **pkg1, DnfPackage **pkg2)
{
  return dnf_package_cmp (*pkg1, *pkg2);
}


gboolean
dnf_utils_print_transaction_packages (GPtrArray *pkgs)
{
  // sort packages by NEVRA
  g_ptr_array_sort (pkgs, (GCompareFunc) dnf_package_cmp_cb);
  for (guint i = 0; i < pkgs->len; i++)
    {
      DnfPackage *pkg = pkgs->pdata[i];
      g_print (" %s (%s, %s)\n", dnf_package_get_nevra (pkg), dnf_package_get_reponame (pkg), g_format_size (dnf_package_get_size (pkg)));
    }
}


gboolean
dnf_utils_print_transaction (DnfContext *ctx)
{
  g_autoptr(GPtrArray) pkgs = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                     DNF_PACKAGE_INFO_INSTALL,
                                                     DNF_PACKAGE_INFO_REINSTALL,
                                                     DNF_PACKAGE_INFO_DOWNGRADE,
                                                     DNF_PACKAGE_INFO_UPDATE,
                                                     DNF_PACKAGE_INFO_REMOVE,
                                                     -1);

  if (pkgs->len == 0)
    {
      g_print ("Nothing to do.\n");
      return FALSE;
    }

  g_autoptr(GPtrArray) pkgs_install = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                             DNF_PACKAGE_INFO_INSTALL,
                                                             -1);
  if (pkgs_install->len != 0)
    {
      g_print ("Installing:\n");
      dnf_utils_print_transaction_packages (pkgs_install);
    }


  g_autoptr(GPtrArray) pkgs_reinstall = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                               DNF_PACKAGE_INFO_REINSTALL,
                                                               -1);
  if (pkgs_reinstall->len != 0)
    {
      g_print ("Reinstalling:\n");
      dnf_utils_print_transaction_packages (pkgs_reinstall);
    }

  g_autoptr(GPtrArray) pkgs_upgrade = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                             DNF_PACKAGE_INFO_UPDATE,
                                                             -1);
  if (pkgs_upgrade->len != 0)
    {
      g_print ("Upgrading:\n");
      dnf_utils_print_transaction_packages (pkgs_upgrade);
    }

  g_autoptr(GPtrArray) pkgs_remove = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                            DNF_PACKAGE_INFO_REMOVE,
                                                            -1);
  if (pkgs_remove->len != 0)
    {
      g_print ("Removing:\n");
      dnf_utils_print_transaction_packages (pkgs_remove);
    }

  g_autoptr(GPtrArray) pkgs_downgrade = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                               DNF_PACKAGE_INFO_DOWNGRADE,
                                                               -1);
  if (pkgs_downgrade->len != 0)
    {
      g_print ("Downgrading:\n");
      dnf_utils_print_transaction_packages (pkgs_downgrade);
    }

  g_print ("Transaction Summary:\n");
  g_print (" %-10s %4d packages\n", "Install", pkgs_install->len);
  g_print (" %-10s %4d packages\n", "Reinstall", pkgs_reinstall->len);
  g_print (" %-10s %4d packages\n", "Upgrade", pkgs_upgrade->len);
  g_print (" %-10s %4d packages\n", "Remove", pkgs_remove->len);
  g_print (" %-10s %4d packages\n", "Downgrade", pkgs_downgrade->len);

  return TRUE;
}
