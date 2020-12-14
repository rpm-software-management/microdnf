/* dnf-command-install.c
 *
 * Copyright © 2010-2015 Richard Hughes <richard@hughsie.com>
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

#include "dnf-command-install.h"
#include "dnf-utils.h"

struct _DnfCommandInstall
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_install_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandInstall,
                                dnf_command_install,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_install_iface_init))

static void
dnf_command_install_init (DnfCommandInstall *self)
{
}

static gboolean
dnf_command_install_run (DnfCommand      *cmd,
                         int              argc,
                         char            *argv[],
                         GOptionContext  *opt_ctx,
                         DnfContext      *ctx,
                         GError         **error)
{
  g_auto(GStrv) pkgs = NULL;
  const GOptionEntry opts[] = {
    { G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_STRING_ARRAY, &pkgs, NULL, NULL },
    { NULL }
  };
  g_option_context_add_main_entries (opt_ctx, opts, NULL);

  if (!g_option_context_parse (opt_ctx, &argc, &argv, error))
    return FALSE;

  if (pkgs == NULL)
    {
      g_set_error_literal (error,
                           G_IO_ERROR,
                           G_IO_ERROR_FAILED,
                           "Packages are not specified");
      return FALSE;
    }

  /* Install each package */
  for (GStrv pkg = pkgs; *pkg != NULL; pkg++)
    {
      if (!dnf_context_install (ctx, *pkg, error))
        return FALSE;
    }
  DnfGoalActions flags = DNF_INSTALL;
  if (dnf_context_get_best())
    {
      flags |= DNF_FORCE_BEST;
    }
  if (!dnf_context_get_install_weak_deps ())
    flags |= DNF_IGNORE_WEAK_DEPS;  
  if (!dnf_goal_depsolve (dnf_context_get_goal (ctx), flags, error))
    return FALSE;
  if (!dnf_utils_print_transaction (ctx))
    return TRUE;
  if (!dnf_utils_userconfirm ())
    return FALSE;
  if (!dnf_context_run (ctx, NULL, error))
    return FALSE;
  g_print ("Complete.\n");

  return TRUE;
}

static void
dnf_command_install_class_init (DnfCommandInstallClass *klass)
{
}

static void
dnf_command_install_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_install_run;
}

static void
dnf_command_install_class_finalize (DnfCommandInstallClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_install_register_types (PeasObjectModule *module)
{
  dnf_command_install_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_INSTALL);
}
