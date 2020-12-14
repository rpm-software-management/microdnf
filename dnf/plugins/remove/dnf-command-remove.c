/* dnf-command-remove.c
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

#include "dnf-command-remove.h"
#include "dnf-utils.h"

struct _DnfCommandRemove
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_remove_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandRemove,
                                dnf_command_remove,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_remove_iface_init))

static void
dnf_command_remove_init (DnfCommandRemove *self)
{
}

static void
disable_available_repos (DnfContext *ctx)
{
  GPtrArray *repos = dnf_context_get_repos (ctx);
  for (guint i = 0; i < repos->len; ++i)
    {
      DnfRepo * repo = g_ptr_array_index (repos, i);
      dnf_repo_set_enabled (repo, DNF_REPO_ENABLED_NONE);
    }
}

static gboolean
dnf_command_remove_run (DnfCommand      *cmd,
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

  disable_available_repos (ctx);

  /* Remove each package */
  for (GStrv pkg = pkgs; *pkg != NULL; pkg++)
    {
      if (!dnf_context_remove (ctx, *pkg, error))
        return FALSE;
    }
  if (!dnf_goal_depsolve (dnf_context_get_goal (ctx), DNF_ERASE, error))
    return FALSE;
  dnf_utils_print_transaction (ctx);
  if (!dnf_utils_userconfirm ())
    return FALSE;
  if (!dnf_context_run (ctx, NULL, error))
    return FALSE;
  g_print ("Complete.\n");

  return TRUE;
}

static void
dnf_command_remove_class_init (DnfCommandRemoveClass *klass)
{
}

static void
dnf_command_remove_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_remove_run;
}

static void
dnf_command_remove_class_finalize (DnfCommandRemoveClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_remove_register_types (PeasObjectModule *module)
{
  dnf_command_remove_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_REMOVE);
}
