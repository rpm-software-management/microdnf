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

#include "dnf-command-repolist.h"

#include <libsmartcols.h>
#include <unistd.h>

struct _DnfCommandRepolist
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_repolist_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandRepolist,
                                dnf_command_repolist,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_repolist_iface_init))

static void
dnf_command_repolist_init (DnfCommandRepolist *self)
{
}

// repository list table columns
enum { COL_REPO_ID, COL_REPO_NAME, COL_REPO_STATUS };

static struct libscols_table *
create_repolist_table (gboolean with_status)
{
  struct libscols_table *table = scols_new_table ();
  if (isatty (1))
    {
      scols_table_enable_colors (table, 1);
      scols_table_enable_maxout (table, 1);
    }
  struct libscols_column *cl = scols_table_new_column (table, "repo id", 0.4, 0);
  scols_column_set_cmpfunc(cl, scols_cmpstr_cells, NULL);
  scols_table_new_column (table, "repo name", 0.5, SCOLS_FL_TRUNC);
  if (with_status)
    scols_table_new_column (table, "status", 0.1, SCOLS_FL_RIGHT);
  return table;
}

static void
add_line_into_table (struct libscols_table *table,
                     gboolean               with_status,
                     const char            *id,
                     const char            *descr,
                     gboolean               enabled)
{
  struct libscols_line *ln = scols_table_new_line (table, NULL);
  scols_line_set_data (ln, COL_REPO_ID, id);
  scols_line_set_data (ln, COL_REPO_NAME, descr);
  if (with_status)
    {
      scols_line_set_data (ln, COL_REPO_STATUS, enabled ? "enabled" : "disabled");
      struct libscols_cell * cl = scols_line_get_cell (ln, COL_REPO_STATUS);
      scols_cell_set_color (cl, enabled ? "green" : "red");
    }
}

static gboolean
dnf_command_repolist_run (DnfCommand      *cmd,
                          int              argc,
                          char            *argv[],
                          GOptionContext  *opt_ctx,
                          DnfContext      *ctx,
                          GError         **error)
{
  gboolean opt_all = FALSE;
  gboolean opt_enabled = FALSE;
  gboolean opt_disabled = FALSE;
  g_auto(GStrv) opt_repos = NULL;
  const GOptionEntry opts[] = {
    { "all", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_all, "show all repos", NULL },
    { "disabled", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_disabled, "show disabled repos", NULL },
    { "enabled", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_enabled, "show enabled repos (default)", NULL },
    { G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_STRING_ARRAY, &opt_repos, NULL, NULL },
    { NULL }
  };
  g_option_context_add_main_entries (opt_ctx, opts, NULL);

  if (!g_option_context_parse (opt_ctx, &argc, &argv, error))
    return FALSE;

  if (opt_repos && opt_repos[0])
    {
      g_set_error (error,
                   G_OPTION_ERROR,
                   G_OPTION_ERROR_UNKNOWN_OPTION,
                   "Unknown argument %s", opt_repos[0]);
      return FALSE;
    }

  if (!opt_disabled)
    opt_enabled = TRUE;

  if (opt_enabled && opt_disabled)
    opt_all = TRUE;

  struct libscols_table *table = create_repolist_table (opt_all);

  GPtrArray *repos = dnf_context_get_repos (ctx);
  for (guint i = 0; i < repos->len; ++i)
    {
      DnfRepo * repo = g_ptr_array_index (repos, i);
      gboolean enabled = dnf_repo_get_enabled (repo) & DNF_REPO_ENABLED_PACKAGES;
      if (opt_all || (opt_enabled && enabled) || (opt_disabled && !enabled))
        {
          const gchar * id = dnf_repo_get_id (repo);
          g_autofree gchar * descr = dnf_repo_get_description (repo);
          add_line_into_table (table, opt_all, id, descr, enabled);
        }
    }

  struct libscols_column *cl = scols_table_get_column (table, COL_REPO_ID);
  scols_sort_table (table, cl);
  scols_print_table (table);
  scols_unref_table (table);

  return TRUE;
}

static void
dnf_command_repolist_class_init (DnfCommandRepolistClass *klass)
{
}

static void
dnf_command_repolist_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_repolist_run;
}

static void
dnf_command_repolist_class_finalize (DnfCommandRepolistClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_repolist_register_types (PeasObjectModule *module)
{
  dnf_command_repolist_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_REPOLIST);
}
