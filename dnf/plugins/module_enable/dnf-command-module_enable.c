/*
 * Copyright (C) 2020 Red Hat, Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "dnf-command-module_enable.h"
#include "dnf-utils.h"

struct _DnfCommandModuleEnable
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_module_enable_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandModuleEnable,
                                dnf_command_module_enable,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_module_enable_iface_init))

static void
dnf_command_module_enable_init (DnfCommandModuleEnable *self)
{
}

static gboolean
dnf_command_module_enable_run (DnfCommand      *cmd,
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

  if (!dnf_context_module_enable (ctx, (const char **)pkgs, error))
    {
      return FALSE;
    }
  if (!dnf_context_module_switched_check (ctx, error))
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
  if (!dnf_utils_userconfirm ())
    {
      return FALSE;
    }
  if (!dnf_context_run (ctx, NULL, error))
    {
      return FALSE;
    }
  return TRUE;
}

static void
dnf_command_module_enable_class_init (DnfCommandModuleEnableClass *klass)
{
}

static void
dnf_command_module_enable_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_module_enable_run;
}

static void
dnf_command_module_enable_class_finalize (DnfCommandModuleEnableClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_module_enable_register_types (PeasObjectModule *module)
{
  dnf_command_module_enable_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_MODULE_ENABLE);
}
