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

#include "dnf-command-repoquery.h"

struct _DnfCommandRepoquery
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_repoquery_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandRepoquery,
                                dnf_command_repoquery,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_repoquery_iface_init))

static void
dnf_command_repoquery_init (DnfCommandRepoquery *self)
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

static gint
gptrarr_dnf_package_cmp (gconstpointer a, gconstpointer b)
{
  return dnf_package_cmp(*(DnfPackage**)a, *(DnfPackage**)b);
}

static gboolean
dnf_command_repoquery_run (DnfCommand     *cmd,
                          int              argc,
                          char            *argv[],
                          GOptionContext  *opt_ctx,
                          DnfContext      *ctx,
                          GError         **error)
{
  gboolean opt_available = FALSE;
  gboolean opt_installed = FALSE;
  g_auto(GStrv) opt_key = NULL;
  const GOptionEntry opts[] = {
    { "available", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_available, "display available packages (default)", NULL },
    { "installed", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_installed, "display installed packages", NULL },
    { G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_STRING_ARRAY, &opt_key, NULL, NULL },
    { NULL }
  };
  g_option_context_add_main_entries (opt_ctx, opts, NULL);

  if (!g_option_context_parse (opt_ctx, &argc, &argv, error))
    return FALSE;

  // --available is default (compatibility with YUM/DNF)
  if (!opt_available && !opt_installed)
    opt_available = TRUE;

  if (opt_available && opt_installed)
    opt_available = opt_installed = FALSE;

  if (opt_installed)
    disable_available_repos (ctx);

  DnfState * state = dnf_context_get_state (ctx);
  DnfContextSetupSackFlags sack_flags = opt_available ? DNF_CONTEXT_SETUP_SACK_FLAG_SKIP_RPMDB
                                                      : DNF_CONTEXT_SETUP_SACK_FLAG_NONE;
  dnf_context_setup_sack_with_flags (ctx, state, sack_flags, error);
  DnfSack *sack = dnf_context_get_sack (ctx);

  hy_autoquery HyQuery query = hy_query_create (sack);

  if (opt_key)
    {
      hy_query_filter_empty (query);
      for (char **pkey = opt_key; *pkey; ++pkey)
        {
          g_auto(HySubject) subject = hy_subject_create (*pkey);
          HyNevra out_nevra;
          hy_autoquery HyQuery key_query = hy_subject_get_best_solution (subject, sack, NULL,
            &out_nevra, TRUE, TRUE, FALSE, TRUE, TRUE);
          if (out_nevra)
            hy_nevra_free(out_nevra);
          hy_query_union (query, key_query);
        }
    }

  g_autoptr(GPtrArray) pkgs = hy_query_run (query);

  g_ptr_array_sort (pkgs, gptrarr_dnf_package_cmp);

  // print packages without duplicated lines
  const char *prev_line = "";
  for (guint i = 0; i < pkgs->len; ++i)
    {
      DnfPackage *package = g_ptr_array_index (pkgs, i);
      const char * line = dnf_package_get_nevra (package);
      if (strcmp (line, prev_line) != 0)
        {
          g_print ("%s\n", line);
          prev_line = line;
        }
    }

  return TRUE;
}

static void
dnf_command_repoquery_class_init (DnfCommandRepoqueryClass *klass)
{
}

static void
dnf_command_repoquery_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_repoquery_run;
}

static void
dnf_command_repoquery_class_finalize (DnfCommandRepoqueryClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_repoquery_register_types (PeasObjectModule *module)
{
  dnf_command_repoquery_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_REPOQUERY);
}
