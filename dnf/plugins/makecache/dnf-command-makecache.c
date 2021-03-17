/* dnf-command-makecache.c
 *
 * Copyright (C) 2021 Red Hat, Inc.
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

#include "dnf-command-makecache.h"

struct _DnfCommandMakecache
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_makecache_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandMakecache,
                                dnf_command_makecache,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_makecache_iface_init))

static void
dnf_command_makecache_init (DnfCommandMakecache *self)
{
}

static gboolean
dnf_command_makecache_run (DnfCommand      *cmd,
                           int              argc,
                           char            *argv[],
                           GOptionContext  *opt_ctx,
                           DnfContext      *ctx,
                           GError         **error)
{
  const GOptionEntry opts[] = {
    { NULL }
  };
  g_option_context_add_main_entries (opt_ctx, opts, NULL);

  if (!g_option_context_parse (opt_ctx, &argc, &argv, error))
    return FALSE;

  if (argc > 1)
    {
      g_set_error (error, G_OPTION_ERROR, G_OPTION_ERROR_UNKNOWN_OPTION, "Unknown argument %s", argv[1]);
      return FALSE;
    }

  DnfState * state = dnf_context_get_state (ctx);
  DnfContextSetupSackFlags sack_flags = DNF_CONTEXT_SETUP_SACK_FLAG_SKIP_RPMDB;
  dnf_context_setup_sack_with_flags (ctx, state, sack_flags, error);

  g_print ("Metadata cache created.\n");

  return TRUE;
}

static void
dnf_command_makecache_class_init (DnfCommandMakecacheClass *klass)
{
}

static void
dnf_command_makecache_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_makecache_run;
}

static void
dnf_command_makecache_class_finalize (DnfCommandMakecacheClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_makecache_register_types (PeasObjectModule *module)
{
  dnf_command_makecache_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_MAKECACHE);
}
