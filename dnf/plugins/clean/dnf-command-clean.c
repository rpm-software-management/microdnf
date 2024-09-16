/* dnf-command-clean.c
 *
 * Copyright © 2017 Jaroslav Rohel <jrohel@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dnf-command-clean.h"

struct _DnfCommandClean
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_clean_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandClean,
                                dnf_command_clean,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_clean_iface_init))

static void
dnf_command_clean_init (DnfCommandClean *self)
{
}

static gboolean
dnf_command_clean_run (DnfCommand      *cmd,
                       int              argc,
                       char            *argv[],
                       GOptionContext  *opt_ctx,
                       DnfContext      *ctx,
                       GError         **error)
{
  g_auto(GStrv) types = NULL;
  const GOptionEntry opts[] = {
    { G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_STRING_ARRAY, &types, NULL, NULL },
    { NULL }
  };
  g_option_context_add_main_entries (opt_ctx, opts, NULL);

  if (!g_option_context_parse (opt_ctx, &argc, &argv, error))
    return FALSE;

  if (types == NULL)
    {
      g_set_error_literal (error,
                           G_IO_ERROR,
                           G_IO_ERROR_FAILED,
                           "Clean requires argument: all");
      return FALSE;
    }

  if (g_strcmp0 (types[0], "all") != 0)
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_FAILED,
                  "Invalid clean argument: '%s'", *types);
      return FALSE;
    }

  if (types[1] != NULL)
    {
      g_set_error_literal (error,
                           G_IO_ERROR,
                           G_IO_ERROR_FAILED,
                           "Too many arguments. Clean accepts only one argument.");
      return FALSE;
    }

  /* lock cache */
  g_autoptr(DnfLock) lock = dnf_lock_new ();
  dnf_lock_set_lock_dir (lock, dnf_context_get_lock_dir (ctx));
  guint lock_id = dnf_lock_take (lock,
                                 DNF_LOCK_TYPE_METADATA,
                                 DNF_LOCK_MODE_PROCESS,
                                 error);
  if (lock_id == 0)
    return FALSE;

  /* Clean cached data */
  const gchar *cachedir  = dnf_context_get_cache_dir (ctx);
  if (!dnf_remove_recursive (cachedir, error))
    return FALSE;
  const gchar *solvdir = dnf_context_get_solv_dir (ctx);
  if (!dnf_remove_recursive (solvdir, error))
    return FALSE;

  if (!dnf_lock_release (lock, lock_id, error))
    return FALSE;

  g_print ("Complete.\n");

  return TRUE;
}

static void
dnf_command_clean_class_init (DnfCommandCleanClass *klass)
{
}

static void
dnf_command_clean_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_clean_run;
}

static void
dnf_command_clean_class_finalize (DnfCommandCleanClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_clean_register_types (PeasObjectModule *module)
{
  dnf_command_clean_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_CLEAN);
}
