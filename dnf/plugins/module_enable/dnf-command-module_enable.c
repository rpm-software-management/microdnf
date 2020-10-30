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

    return dnf_context_enable_modules (ctx, (const char **)pkgs, error);
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
