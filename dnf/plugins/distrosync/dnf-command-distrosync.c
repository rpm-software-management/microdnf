/* dnf-command-distrosync.c
 *
 * Copyright © 2021 Neal Gompa <ngompa13@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dnf-command-distrosync.h"
#include "dnf-utils.h"

struct _DnfCommandDistroSync
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_distrosync_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandDistroSync,
                                dnf_command_distrosync,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_distrosync_iface_init))

static void
dnf_command_distrosync_init (DnfCommandDistroSync *self)
{
}

static gboolean
dnf_command_distrosync_run (DnfCommand      *cmd,
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
      if (!dnf_context_distrosync_all (ctx, error))
        return FALSE;
    }
  else
    {
      /* Sync each package */
      for (GStrv pkg = pkgs; *pkg != NULL; pkg++)
        {
          if (!dnf_context_distrosync (ctx, *pkg, error))
            return FALSE;
        }
    }
  DnfGoalActions flags = 0;
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
dnf_command_distrosync_class_init (DnfCommandDistroSyncClass *klass)
{
}

static void
dnf_command_distrosync_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_distrosync_run;
}

static void
dnf_command_distrosync_class_finalize (DnfCommandDistroSyncClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_distrosync_register_types (PeasObjectModule *module)
{
  dnf_command_distrosync_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_DISTROSYNC);
}
