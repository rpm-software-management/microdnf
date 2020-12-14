/*
 * Copyright (C) 2019 Red Hat, Inc.
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

#include "dnf-command-reinstall.h"
#include "dnf-utils.h"

struct _DnfCommandReinstall
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_reinstall_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandReinstall,
                                dnf_command_reinstall,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_reinstall_iface_init))

static void
dnf_command_reinstall_init (DnfCommandReinstall *self)
{
}

static gint
compare_nevra (gconstpointer a, gconstpointer b, gpointer user_data)
{
  return g_strcmp0 (a, b);
}

static void
packageset_free (gpointer pkg_set)
{
  dnf_packageset_free (pkg_set);
}

static gboolean
dnf_command_reinstall_arg (DnfContext *ctx, const char *arg, GError **error)
{
  DnfSack *sack = dnf_context_get_sack (ctx);

  g_auto(HySubject) subject = hy_subject_create (arg);
  HyNevra out_nevra;
  hy_autoquery HyQuery query = hy_subject_get_best_solution (subject, sack, NULL, &out_nevra,
                                                             FALSE, TRUE, TRUE, TRUE, TRUE);
  if (out_nevra)
    hy_nevra_free (out_nevra);

  hy_autoquery HyQuery query_installed = hy_query_clone (query);
  hy_query_filter (query_installed, HY_PKG_REPONAME, HY_EQ, HY_SYSTEM_REPO_NAME);
  g_autoptr(GPtrArray) installed_pkgs = hy_query_run (query_installed);

  if (installed_pkgs->len == 0)
    {
      g_print ("No match for argument: %s\n", arg);
      return TRUE;
    }
  
  hy_query_filter (query, HY_PKG_REPONAME, HY_NEQ, HY_SYSTEM_REPO_NAME);
  g_autoptr(GPtrArray) available_pkgs = hy_query_run (query);

  g_autoptr(GTree) available_nevra2pkg_set = g_tree_new_full (compare_nevra, NULL,
                                                              g_free, packageset_free);
  for (guint i = 0; i < available_pkgs->len; ++i)
    {
      DnfPackage *package = available_pkgs->pdata[i];
      const char *nevra = dnf_package_get_nevra (package);
      DnfPackageSet *pkg_set = g_tree_lookup (available_nevra2pkg_set, nevra);
      if (!pkg_set)
        {
          pkg_set = dnf_packageset_new (sack);
          /* dnf_package_get_nevra() returns pointer to temporary buffer -> copy is needed */
          g_tree_insert (available_nevra2pkg_set, g_strdup (nevra), pkg_set);
        }
      dnf_packageset_add (pkg_set, package);
    }

  HyGoal goal = dnf_context_get_goal (ctx);
  for (guint i = 0; i < installed_pkgs->len; ++i)
    {
      DnfPackage *package = installed_pkgs->pdata[i];
      const char *nevra = dnf_package_get_nevra (package);
      DnfPackageSet *available_pkgs = g_tree_lookup (available_nevra2pkg_set, nevra);
      if (available_pkgs) {
          g_auto(HySelector) selector = hy_selector_create (sack);
          hy_selector_pkg_set (selector, available_pkgs);
          if (!hy_goal_install_selector (goal, selector, error))
            return FALSE;
        }
      else 
        g_print ("Installed package %s not available.\n", nevra);
    }
    
  return TRUE;
}

static gboolean
dnf_command_reinstall_run (DnfCommand      *cmd,
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

  DnfState * state = dnf_context_get_state (ctx);
  dnf_context_setup_sack_with_flags (ctx, state, DNF_CONTEXT_SETUP_SACK_FLAG_NONE, error);

  for (GStrv pkg = pkgs; *pkg != NULL; pkg++)
    {
      if (!dnf_command_reinstall_arg (ctx, *pkg, error))
        return FALSE;
    }

  DnfGoalActions flags = DNF_INSTALL;
  if (!dnf_context_get_install_weak_deps ())
    flags |= DNF_IGNORE_WEAK_DEPS;  
  if (!dnf_goal_depsolve (dnf_context_get_goal (ctx), flags, error))
    return FALSE;
  
  DnfTransaction *transaction = dnf_context_get_transaction (ctx);
  int tsflags = dnf_transaction_get_flags (transaction);
  dnf_transaction_set_flags(transaction, tsflags | DNF_TRANSACTION_FLAG_ALLOW_REINSTALL);

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
dnf_command_reinstall_class_init (DnfCommandReinstallClass *klass)
{
}

static void
dnf_command_reinstall_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_reinstall_run;
}

static void
dnf_command_reinstall_class_finalize (DnfCommandReinstallClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_reinstall_register_types (PeasObjectModule *module)
{
  dnf_command_reinstall_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_REINSTALL);
}
