/* dnf-main.c
 *
 * Copyright © 2010-2015 Richard Hughes <richard@hughsie.com>
 * Copyright © 2016 Colin Walters <walters@verbum.org>
 * Copyright © 2016-2017 Igor Gnatenko <ignatenko@redhat.com>
 * Copyright © 2017 Jaroslav Rohel <jrohel@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include <locale.h>
#include <stdlib.h>
#include <glib.h>
#include <libpeas/peas.h>
#include <libdnf/libdnf.h>
#include "dnf-command.h"

static gboolean opt_yes = TRUE;
static gboolean opt_nodocs = FALSE;
static gboolean opt_best = FALSE;
static gboolean opt_nobest = FALSE;
static gboolean show_help = FALSE;
static gboolean dl_pkgs_printed = FALSE;
static GSList *enable_disable_repos = NULL;

static gboolean
process_global_option (const gchar  *option_name,
                       const gchar  *value,
                       gpointer      data,
                       GError      **error)
{
  g_autoptr(GError) local_error = NULL;
  DnfContext *ctx = DNF_CONTEXT (data);

  gboolean ret = TRUE;
  if (g_strcmp0 (option_name, "--disablerepo") == 0)
    {
      enable_disable_repos = g_slist_append (enable_disable_repos, g_strconcat("d", value, NULL));
    }
  else if (g_strcmp0 (option_name, "--enablerepo") == 0)
    {
      enable_disable_repos = g_slist_append (enable_disable_repos, g_strconcat("e", value, NULL));
    }
  else if (g_strcmp0 (option_name, "--releasever") == 0)
    {
      dnf_context_set_release_ver (ctx, value);
    }
  else if (g_strcmp0 (option_name, "--setopt") == 0)
    {
      if (g_strcmp0 (value, "tsflags=nodocs") == 0)
        {
          opt_nodocs = TRUE;
        }
      else
        {
          local_error = g_error_new (G_OPTION_ERROR,
                                     G_OPTION_ERROR_BAD_VALUE,
                                     "Unable to handle: %s", value);
          ret = FALSE;
        }
    }
  else
    g_assert_not_reached ();

  if (local_error != NULL)
    g_set_error (error,
                 G_OPTION_ERROR,
                 G_OPTION_ERROR_BAD_VALUE,
                 "(%s) %s", option_name, local_error->message);

  return ret;
}

static const GOptionEntry global_opts[] = {
  { "assumeyes", 'y', G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, &opt_yes, "Does nothing, we always assume yes", NULL },
  { "best", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_best, "Try the best available package versions in transactions", NULL },
  { "disablerepo", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Disable repository by an id", "ID" },
  { "enablerepo", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Enable repository by an id", "ID" },
  { "nobest", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_nobest, "Do not limit the transaction to the best candidates", NULL },
  { "nodocs", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_nodocs, "Install packages without docs", NULL },
  { "releasever", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Override the value of $releasever in config and repo files", "RELEASEVER" },
  { "setopt", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Set transaction flag, like tsflags=nodocs", "FLAG" },
  { NULL }
};

static DnfContext *
context_new (void)
{
  DnfContext *ctx = dnf_context_new ();

  dnf_context_set_repo_dir (ctx, "/etc/yum.repos.d/");
#define CACHEDIR "/var/cache/yum"
  dnf_context_set_cache_dir (ctx, CACHEDIR"/metadata");
  dnf_context_set_solv_dir (ctx, CACHEDIR"/solv");
  dnf_context_set_lock_dir (ctx, CACHEDIR"/lock");
#undef CACHEDIR
  dnf_context_set_check_disk_space (ctx, FALSE);
  dnf_context_set_check_transaction (ctx, TRUE);
  dnf_context_set_keep_cache (ctx, FALSE);
  dnf_context_set_cache_age (ctx, 0);

  return ctx;
}

static void
state_action_changed_cb (DnfState       *state,
                         DnfStateAction  action,
                         const gchar    *action_hint)
{
  switch (action)
    {
      case DNF_STATE_ACTION_DOWNLOAD_METADATA:
        g_print("Downloading metadata...\n");
        break;
      case DNF_STATE_ACTION_DOWNLOAD_PACKAGES:
        if (!dl_pkgs_printed)
          {
            g_print("Downloading packages...\n");
            dl_pkgs_printed = TRUE;
          }
        break;
      case DNF_STATE_ACTION_TEST_COMMIT:
        g_print("Running transaction test...\n");
        break;
      case DNF_STATE_ACTION_INSTALL:
        g_print("Installing: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_REMOVE:
        g_print("Removing: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_UPDATE:
        g_print("Updating: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_OBSOLETE:
        g_print("Obsoleting: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_REINSTALL:
        g_print("Reinstalling: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_DOWNGRADE:
        g_print("Downgrading: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_CLEANUP:
        g_print("Cleanup: %s\n", action_hint);
        break;
      default:
        break;
    }
}

static GOptionGroup *
new_global_opt_group (DnfContext *ctx)
{
  GOptionGroup *opt_grp = g_option_group_new ("global",
                                              "Global Options:",
                                              "Show global help options",
                                              ctx,
                                              NULL);
  g_option_group_add_entries (opt_grp, global_opts);
  return opt_grp;
}

int
main (int   argc,
      char *argv[])
{
  g_autoptr(GError) error = NULL;
  g_autoptr(DnfContext) ctx = context_new ();
  g_autoptr(PeasEngine) engine = peas_engine_get_default ();
  g_autoptr(PeasExtensionSet) cmd_exts = NULL;
  g_autoptr(GOptionContext) opt_ctx = g_option_context_new ("COMMAND");
  g_autoptr(GOptionContext) subcmd_opt_ctx = NULL;
  g_autofree gchar *subcmd_opt_param = NULL;

  setlocale (LC_ALL, "");

  if (g_getenv ("DNF_IN_TREE_PLUGINS") != NULL)
    peas_engine_prepend_search_path (engine,
                                    BUILDDIR"/plugins",
                                    SRCDIR"/plugins");
  else
    peas_engine_prepend_search_path (engine,
                                     PACKAGE_LIBDIR"/plugins",
                                     PACKAGE_DATADIR"/plugins");

  peas_engine_prepend_search_path (engine,
                                   "resource:///org/fedoraproject/dnf/plugins",
                                   NULL);

  g_autofree gchar *path = g_build_filename (g_get_user_data_dir (),
                                             "dnf",
                                             "plugins",
                                             NULL);
  peas_engine_prepend_search_path (engine,
                                   path,
                                   NULL);

  cmd_exts = peas_extension_set_new (engine,
                                     DNF_TYPE_COMMAND,
                                     NULL);

  GString *cmd_summary = g_string_new ("Commands:");
  for (const GList *plugin = peas_engine_get_plugin_list (engine);
       plugin != NULL;
       plugin = plugin->next)
    {
      PeasPluginInfo *info = plugin->data;
      if (!peas_engine_load_plugin (engine, info))
        continue;
      if (peas_engine_provides_extension (engine, info, DNF_TYPE_COMMAND))
        /*
         * At least 2 spaces between the command and its description are needed
         * so that help2man formats it correctly.
         */
        g_string_append_printf (cmd_summary, "\n  %-16s     %s", peas_plugin_info_get_name (info), peas_plugin_info_get_description (info));
    }
  g_option_context_set_summary (opt_ctx, cmd_summary->str);
  g_string_free (cmd_summary, TRUE);
  g_option_context_set_ignore_unknown_options (opt_ctx, TRUE);
  g_option_context_set_help_enabled (opt_ctx, FALSE);
  g_option_context_set_main_group (opt_ctx, new_global_opt_group (ctx));

  /* Is help option in arguments? */
  for (gint in = 1; in < argc; in++)
    {
      if (g_strcmp0 (argv[in], "--") == 0)
        break;
      if (g_strcmp0 (argv[in], "-h") == 0 ||
          g_strcmp0 (argv[in], "--help") == 0 ||
          g_strcmp0 (argv[in], "--help-all") == 0 ||
          g_strcmp0 (argv[in], "--help-global") == 0)
        {
          show_help = TRUE;
          break;
        }
    }

  /*
   * Parse the global options.
   */
  if (!g_option_context_parse (opt_ctx, &argc, &argv, &error))
    goto out;

  /*
   * Initialize dnf context only if help is not requested.
   */
  if (!show_help)
    {
      if (!dnf_context_setup (ctx, NULL, &error))
        goto out;
      DnfState *state = dnf_context_get_state (ctx);
      g_signal_connect (state, "action-changed",
                        G_CALLBACK (state_action_changed_cb),
                        NULL);

      for (GSList * item = enable_disable_repos; item; item = item->next)
        {
          gchar * item_data = item->data;
          int ret;
          if (item_data[0] == 'd')
            ret = dnf_context_repo_disable (ctx, item_data+1, &error);
          else
            ret = dnf_context_repo_enable (ctx, item_data+1, &error);
          if (!ret)
            goto out;
        }

      /* allow downgrades for all transaction types */
      DnfTransaction *txn = dnf_context_get_transaction (ctx);
      int flags = dnf_transaction_get_flags (txn) | DNF_TRANSACTION_FLAG_ALLOW_DOWNGRADE;

      if (opt_nodocs)
        {
          flags |= DNF_TRANSACTION_FLAG_NODOCS;
        }

      dnf_transaction_set_flags (txn, flags);
      if (opt_best && opt_nobest)
        {
          error = g_error_new_literal(G_OPTION_ERROR,
                                      G_OPTION_ERROR_BAD_VALUE,
                                      "Argument --nobest is not allowed with argument --best");
          goto out;
        }
      if (opt_best)
        {
          dnf_context_set_best(TRUE);
        }
      else if (opt_nobest)
        {
          dnf_context_set_best(FALSE);
        }
    }

  /*
   * The first non-option is the command.
   * Get it and remove it from arguments.
   */
  const gchar *cmd_name = NULL;
  for (gint in = 1; in < argc; in++)
    {
      if (cmd_name != NULL)
        argv[in-1] = argv[in];
      else if (argv[in][0] != '-')
        cmd_name = argv[in];
    }
  if (cmd_name != NULL) --argc;

  g_option_context_set_help_enabled (opt_ctx, TRUE);

  if (cmd_name == NULL && show_help)
    {
      const char *prg_name = strrchr(argv[0], '/');
      prg_name = prg_name ? prg_name + 1 : argv[0];

      g_set_prgname (prg_name);
      g_autofree gchar *help = g_option_context_get_help (opt_ctx, TRUE, NULL);
      g_print ("%s", help);
      goto out;
    }

  PeasPluginInfo *plug = NULL;
  PeasExtension *exten = NULL;
  if (cmd_name != NULL)
    {
      g_autofree gchar *mod_name = g_strdup_printf ("command_%s", cmd_name);
      plug = peas_engine_get_plugin_info (engine, mod_name);
      if (plug != NULL)
        exten = peas_extension_set_get_extension (cmd_exts, plug);
    }
  if (exten == NULL)
    {
      if (cmd_name == NULL)
        error = g_error_new_literal (G_IO_ERROR,
                                     G_IO_ERROR_FAILED,
                                     "No command specified");
      else
        error = g_error_new (G_IO_ERROR,
                             G_IO_ERROR_FAILED,
                             "Unknown command: '%s'", cmd_name);

      g_autofree gchar *help = g_option_context_get_help (opt_ctx, TRUE, NULL);
      g_printerr ("This is microdnf, which implements subset of `dnf'.\n"
                  "%s", help);
      goto out;
    }

  subcmd_opt_param = g_strdup_printf ("%s - %s",
    peas_plugin_info_get_external_data (plug, "Command-Syntax"),
    peas_plugin_info_get_description (plug));
  subcmd_opt_ctx = g_option_context_new (subcmd_opt_param);
  g_option_context_add_group (subcmd_opt_ctx, new_global_opt_group (ctx));
  if (!dnf_command_run (DNF_COMMAND (exten), argc, argv, subcmd_opt_ctx, ctx, &error))
    goto out;

out:
  if (error != NULL)
    {
      const gchar *prefix = "";
      const gchar *suffix = "";
      if (isatty (1))
        {
          prefix = "\x1b[31m\x1b[1m"; /* red, bold */
          suffix = "\x1b[22m\x1b[0m"; /* bold off, color reset */
        }
      g_printerr ("%serror: %s%s\n", prefix, suffix, error->message);
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
