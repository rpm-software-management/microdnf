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

#include <libsmartcols.h>

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

static void
package_info_add_line (struct libscols_table *table, const char *key, const char *value)
{
  struct libscols_line *ln = scols_table_new_line (table, NULL);
  scols_line_set_data (ln, 0, key);
  scols_line_set_data (ln, 1, value);
}

static void
print_package_info (DnfPackage *package)
{
  struct libscols_table *table = scols_new_table ();
  scols_table_enable_noheadings (table, 1);
  scols_table_set_column_separator (table, " : ");
  scols_table_new_column (table, "key", 5, 0);
  struct libscols_column *cl = scols_table_new_column (table, "value", 10, SCOLS_FL_WRAP);
  scols_column_set_safechars (cl, "\n");
  scols_column_set_wrapfunc (cl, scols_wrapnl_chunksize, scols_wrapnl_nextchunk, NULL);

  package_info_add_line (table, "Name", dnf_package_get_name (package));
  guint64 epoch = dnf_package_get_epoch (package);
  if (epoch != 0)
    {
      g_autofree gchar *str_epoch = g_strdup_printf ("%ld", epoch);
      package_info_add_line (table, "Epoch", str_epoch);
    }
  package_info_add_line (table, "Version", dnf_package_get_version (package));
  package_info_add_line (table, "Release", dnf_package_get_release (package));
  package_info_add_line (table, "Architecture", dnf_package_get_arch (package));
  g_autofree gchar *size = g_format_size_full (dnf_package_get_size (package),
                                               G_FORMAT_SIZE_LONG_FORMAT | G_FORMAT_SIZE_IEC_UNITS);
  package_info_add_line (table, "Size", size);
  package_info_add_line (table, "Source", dnf_package_get_sourcerpm (package));
  package_info_add_line (table, "Repository", dnf_package_get_reponame (package));
  package_info_add_line (table, "Summanry", dnf_package_get_summary (package));
  package_info_add_line (table, "URL", dnf_package_get_url (package));
  package_info_add_line (table, "License", dnf_package_get_license (package));
  package_info_add_line (table, "Description", dnf_package_get_description (package));

  scols_print_table (table);
  scols_unref_table (table);
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
  gboolean opt_info = FALSE;
  gboolean opt_installed = FALSE;
  gboolean opt_nevra = FALSE;
  g_auto(GStrv) opt_key = NULL;
  const GOptionEntry opts[] = {
    { "available", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_available, "display available packages (default)", NULL },
    { "info", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_info, "show detailed information about the packages", NULL },
    { "installed", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_installed, "display installed packages", NULL },
    { "nevra", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_nevra,
      "use name-epoch:version-release.architecture format for displaying packages (default)", NULL },
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

  const char *prev_line = "";
  for (guint i = 0; i < pkgs->len; ++i)
    {
      DnfPackage *package = g_ptr_array_index (pkgs, i);
      if (opt_nevra || !opt_info)
        {
          const char * line = dnf_package_get_nevra (package);
          // print nevras without duplicated lines
          if (opt_info || strcmp (line, prev_line) != 0)
            {
              g_print ("%s\n", line);
              prev_line = line;
            }
        }
      if (opt_info)
        {
          print_package_info (package);
          g_print ("\n");
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
