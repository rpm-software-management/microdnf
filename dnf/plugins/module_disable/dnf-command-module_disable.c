/*
 * Copyright (C) 2020 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "dnf-command-module_disable.h"
#include "dnf-utils.h"

struct _DnfCommandModuleDisable
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_module_disable_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandModuleDisable,
                                dnf_command_module_disable,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_module_disable_iface_init))

static void
dnf_command_module_disable_init (DnfCommandModuleDisable *self)
{
}

static gboolean
dnf_command_module_disable_run (DnfCommand      *cmd,
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
                           "Modules are not specified");
      return FALSE;
    }

  if (!dnf_context_module_disable (ctx, (const char **)pkgs, error))
    {
      return FALSE;
    }

  if (!dnf_goal_depsolve (dnf_context_get_goal (ctx), DNF_NONE, error))
    {
      if (g_error_matches (*error, DNF_ERROR, DNF_ERROR_NO_PACKAGES_TO_UPDATE))
        {
          g_clear_error (error);
        } else {
          return FALSE;
        }
    }
  if (!dnf_utils_print_transaction (ctx))
    {
      return TRUE;
    }
  if (!dnf_context_run (ctx, NULL, error))
    {
      return FALSE;
    }
  return TRUE;
}

static void
dnf_command_module_disable_class_init (DnfCommandModuleDisableClass *klass)
{
}

static void
dnf_command_module_disable_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_module_disable_run;
}

static void
dnf_command_module_disable_class_finalize (DnfCommandModuleDisableClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_module_disable_register_types (PeasObjectModule *module)
{
  dnf_command_module_disable_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_MODULE_DISABLE);
}
