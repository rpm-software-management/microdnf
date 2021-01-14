/* dnf-main.c
 *
 * Copyright © 2010-2015 Richard Hughes <richard@hughsie.com>
 * Copyright © 2016 Colin Walters <walters@verbum.org>
 * Copyright © 2016-2017 Igor Gnatenko <ignatenko@redhat.com>
 * Copyright © 2017-2021 Jaroslav Rohel <jrohel@redhat.com>
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

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <libpeas/peas.h>
#include <libdnf/libdnf.h>
#include "dnf-command.h"
#include "dnf-utils.h"

typedef enum { ARG_DEFAULT, ARG_FALSE, ARG_TRUE } BoolArgs;

static BoolArgs opt_install_weak_deps = ARG_DEFAULT;
static BoolArgs opt_allow_vendor_change = ARG_DEFAULT;
static BoolArgs opt_keepcache = ARG_DEFAULT;
static gboolean opt_no = FALSE;
static gboolean opt_yes = FALSE;
static gboolean opt_nodocs = FALSE;
static gboolean opt_best = FALSE;
static gboolean opt_nobest = FALSE;
static gboolean opt_test = FALSE;
static gboolean opt_refresh = FALSE;
static gboolean show_help = FALSE;
static gboolean dl_pkgs_printed = FALSE;
static GSList *enable_disable_repos = NULL;
static gboolean disable_plugins_loading = FALSE;
static gboolean config_used = FALSE;
static gboolean enable_disable_plugin_used = FALSE;
static gboolean installroot_used = FALSE;
static gboolean cachedir_used = FALSE;
static gboolean reposdir_used = FALSE;
static gboolean varsdir_used = FALSE;

static gboolean
process_global_option (const gchar  *option_name,
                       const gchar  *value,
                       gpointer      data,
                       GError      **error)
{
  g_autoptr(GError) local_error = NULL;
  DnfContext *ctx = DNF_CONTEXT (data);

  gboolean ret = TRUE;
  if (g_strcmp0 (option_name, "--config") == 0)
    {
      config_used = TRUE;
      dnf_context_set_config_file_path (value);
    }
  else if (g_strcmp0 (option_name, "--disablerepo") == 0)
    {
      enable_disable_repos = g_slist_append (enable_disable_repos, g_strconcat("d", value, NULL));
    }
  else if (g_strcmp0 (option_name, "--enablerepo") == 0)
    {
      enable_disable_repos = g_slist_append (enable_disable_repos, g_strconcat("e", value, NULL));
    }
  else if (g_strcmp0 (option_name, "--disableplugin") == 0)
    {
      g_auto(GStrv) patterns = g_strsplit (value, ",", -1);
      for (char **it = patterns; *it; ++it)
        dnf_context_disable_plugins (*it);
      enable_disable_plugin_used = TRUE;
    }
  else if (g_strcmp0 (option_name, "--enableplugin") == 0)
    {
      g_auto(GStrv) patterns = g_strsplit (value, ",", -1);
      for (char **it = patterns; *it; ++it)
        dnf_context_enable_plugins (*it);
      enable_disable_plugin_used = TRUE;
    }
  else if (g_strcmp0 (option_name, "--installroot") == 0)
    {
      installroot_used = TRUE;
      if (value[0] != '/')
        {
          local_error = g_error_new (G_OPTION_ERROR,
                                     G_OPTION_ERROR_BAD_VALUE,
                                     "Absolute path must be used");
          ret = FALSE;
        }
      else
        {
          dnf_context_set_install_root (ctx, value);
        }
    }
  else if (g_strcmp0 (option_name, "--releasever") == 0)
    {
      dnf_context_set_release_ver (ctx, value);
    }
  else if (g_strcmp0 (option_name, "--setopt") == 0)
    {
      g_auto(GStrv) setopt = g_strsplit (value, "=", 2);
      if (!setopt[0] || !setopt[1])
        {
          local_error = g_error_new (G_OPTION_ERROR,
                                     G_OPTION_ERROR_BAD_VALUE,
                                     "Missing value in: %s", value);
          ret = FALSE;
        }
      else if (strchr (setopt[0], '.'))
        { /* repository option, pass to libdnf */
          ret = dnf_conf_add_setopt (setopt[0], DNF_CONF_COMMANDLINE, setopt[1], &local_error);
        }
      else if (strcmp (setopt[0], "tsflags") == 0)
        {
          g_auto(GStrv) tsflags = g_strsplit (setopt[1], ",", -1);
          for (char **it = tsflags; *it; ++it)
            {
              if (strcmp (*it, "nodocs") == 0)
                opt_nodocs = TRUE;
              else if (strcmp (*it, "test") == 0)
                opt_test = TRUE;
              else
                {
                  local_error = g_error_new (G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
                                            "Unknown tsflag: %s", *it);
                  ret = FALSE;
                  break;
                }
            }
        }
      else if (strcmp (setopt[0], "module_platform_id") == 0)
        {
          const char *module_platform_id = setopt[1];
          if (module_platform_id[0] != '\0')
            {
              dnf_context_set_platform_module (ctx, module_platform_id);
            }
          else
            {
              local_error = g_error_new (G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
                                         "Empty value in: %s", value);
              ret = FALSE;
            }
        }
      else if (strcmp (setopt[0], "cachedir") == 0)
        {
          cachedir_used = TRUE;
          const char *cachedir = setopt[1];
          if (cachedir[0] != '\0')
            {
              g_autofree gchar *metadatadir = g_build_path ("/", cachedir, "metadata", NULL);
              dnf_context_set_cache_dir (ctx, metadatadir);
              g_autofree gchar *solvdir = g_build_path ("/", cachedir, "solv", NULL);
              dnf_context_set_solv_dir (ctx, solvdir);
              g_autofree gchar *lockdir = g_build_path ("/", cachedir, "lock", NULL);
              dnf_context_set_lock_dir (ctx, lockdir);
            }
          else
            {
              local_error = g_error_new (G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
                                         "Empty value in: %s", value);
              ret = FALSE;
            }
        }
      else if (strcmp (setopt[0], "install_weak_deps") == 0)
        {
          const char *setopt_val = setopt[1];
          if (setopt_val[0] == '1' && setopt_val[1] == '\0')
            opt_install_weak_deps = ARG_TRUE;
          else if (setopt_val[0] == '0' && setopt_val[1] == '\0')
            opt_install_weak_deps = ARG_FALSE;
          else
            {
              local_error = g_error_new (G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
                                         "Invalid boolean value \"%s\" in: %s", setopt[1], value);
              ret = FALSE;
            }
        }
      else if (strcmp (setopt[0], "allow_vendor_change") == 0)
        {
          const char *setopt_val = setopt[1];
          if (setopt_val[0] == '1' && setopt_val[1] == '\0')
            opt_allow_vendor_change = ARG_TRUE;
          else if (setopt_val[0] == '0' && setopt_val[1] == '\0')
            opt_allow_vendor_change = ARG_FALSE;
          else
            {
              local_error = g_error_new (G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
                                         "Invalid boolean value \"%s\" in: %s", setopt[1], value);
              ret = FALSE;
            }
        }
      else if (strcmp (setopt[0], "keepcache") == 0)
        {
          const char *setopt_val = setopt[1];
          if (setopt_val[0] == '1' && setopt_val[1] == '\0')
            opt_keepcache = ARG_TRUE;
          else if (setopt_val[0] == '0' && setopt_val[1] == '\0')
            opt_keepcache = ARG_FALSE;
          else
            {
              local_error = g_error_new (G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
                                         "Invalid boolean value \"%s\" in: %s", setopt[1], value);
              ret = FALSE;
            }
        }
      else if (strcmp (setopt[0], "reposdir") == 0)
        {
          reposdir_used = TRUE;
          g_auto(GStrv) reposdir = g_strsplit (setopt[1], ",", -1);
          dnf_context_set_repos_dir (ctx, (const gchar * const *)reposdir);
        }
      else if (strcmp (setopt[0], "varsdir") == 0)
        {
          varsdir_used = TRUE;
          g_auto(GStrv) varsdir = g_strsplit (setopt[1], ",", -1);
          dnf_context_set_vars_dir (ctx, (const gchar * const *)varsdir);
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
  { "assumeno", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_no, "Automatically answer no for all questions", NULL },
  { "assumeyes", 'y', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_yes, "Automatically answer yes for all questions", NULL },
  { "best", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_best, "Try the best available package versions in transactions", NULL },
  { "config", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Configuration file location", "<config file>" },
  { "disablerepo", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Disable repository by an id", "ID" },
  { "disableplugin", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Disable plugins by name", "name" },
  { "enablerepo", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Enable repository by an id", "ID" },
  { "enableplugin", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Enable plugins by name", "name" },
  { "nobest", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_nobest, "Do not limit the transaction to the best candidates", NULL },
  { "installroot", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Set install root", "PATH" },
  { "nodocs", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_nodocs, "Install packages without docs", NULL },
  { "noplugins", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &disable_plugins_loading, "Disable loading of plugins", NULL },
  { "refresh", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &opt_refresh, "Set metadata as expired before running the command", NULL },
  { "releasever", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option, "Override the value of $releasever in config and repo files", "RELEASEVER" },
  { "setopt", '\0', G_OPTION_FLAG_NONE, G_OPTION_ARG_CALLBACK, process_global_option,
    "Override a configuration option (install_weak_deps=0/1, allow_vendor_change=0/1, keepcache=0/1, module_platform_id=<name:stream>, cachedir=<path>, reposdir=<path1>,<path2>,..., tsflags=nodocs/test, varsdir=<path1>,<path2>,..., repo_id.option_name=<value>)", "<option>=<value>" },
  { NULL }
};

static DnfContext *
context_new (void)
{
  DnfContext *ctx = dnf_context_new ();

#define CACHEDIR "/var/cache/yum"
  dnf_context_set_cache_dir (ctx, CACHEDIR"/metadata");
  dnf_context_set_solv_dir (ctx, CACHEDIR"/solv");
  dnf_context_set_lock_dir (ctx, CACHEDIR"/lock");
#undef CACHEDIR
  dnf_context_set_check_disk_space (ctx, FALSE);
  dnf_context_set_check_transaction (ctx, TRUE);

  /* Sets a maximum cache age in seconds. It is an upper limit.
   * The lower value between this value and "metadata_expire" value from repo/global
   * configuration file is used.
   * The value G_MAXUINT has a special meaning. It means that the cache never expires
   * regardless of the settings in the configuration files. */
  dnf_context_set_cache_age (ctx, G_MAXUINT - 1); 

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
        g_print ("Downloading metadata...\n");
        break;
      case DNF_STATE_ACTION_DOWNLOAD_PACKAGES:
        if (!dl_pkgs_printed)
          {
            g_print ("Downloading packages...\n");
            dl_pkgs_printed = TRUE;
          }
        break;
      case DNF_STATE_ACTION_TEST_COMMIT:
        g_print ("Running transaction test...\n");
        break;
      case DNF_STATE_ACTION_INSTALL:
        if (action_hint)
          g_print ("Installing: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_REMOVE:
        if (action_hint)
          g_print ("Removing: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_UPDATE:
        if (action_hint)
          g_print ("Updating: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_OBSOLETE:
        if (action_hint)
          g_print ("Obsoleting: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_REINSTALL:
        if (action_hint)
          g_print ("Reinstalling: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_DOWNGRADE:
        if (action_hint)
          g_print ("Downgrading: %s\n", action_hint);
        break;
      case DNF_STATE_ACTION_CLEANUP:
        if (action_hint)
          g_print ("Cleanup: %s\n", action_hint);
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

/*
 * The first non-option is the command/subcommand.
 * Get it and remove it from arguments.
 */
static const gchar *
get_command (int *argc,
             char *argv[])
{
  const gchar *cmd_name = NULL;
  for (gint in = 1; in < *argc; in++)
    {
      if (cmd_name != NULL)
        argv[in-1] = argv[in];
      else if (argv[in][0] != '-')
        cmd_name = argv[in];
    }
  if (cmd_name != NULL) --*argc;
  return cmd_name;
}

static gint
compare_strings (gconstpointer a,
                 gconstpointer b)
{
  return strcmp (a, b);
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
  GSList *cmds_with_subcmds = NULL;  /* list of commands with subcommands */
  /* dictionary of aliases for commands */
  g_autoptr(GHashTable) cmds_aliases = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

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
        {
          g_autofree gchar *command_name = g_strdup (peas_plugin_info_get_name (info));
          g_autofree gchar *command_alias_name = g_strdup (peas_plugin_info_get_external_data (info, "Alias-Name"));

          /* Plugins with a '_' character in command name implement subcommands.
             E.g. the "command_module_enable" plugin implements the "enable" subcommand of the "module" command. */
          for (gchar *ptr = command_name; *ptr != '\0'; ++ptr)
            {
              if (*ptr == '_')
                {
                  *ptr = ' ';
                  cmds_with_subcmds = g_slist_append (cmds_with_subcmds, g_strndup (command_name, ptr - command_name));
                  break;
                }
            }

          /* Add command alias to the dictionary. */
          if (command_alias_name)
              g_hash_table_insert (cmds_aliases, g_strdup (command_alias_name), g_strdup (command_name));

          /*
           * At least 2 spaces between the command and its description are needed
           * so that help2man formats it correctly.
           */
          g_string_append_printf (cmd_summary, "\n  %-16s     %s", command_name, peas_plugin_info_get_description (info));

          /* If command has an alias with a description, add it to the help. */
          const gchar *command_alias_description = peas_plugin_info_get_external_data (info, "Alias-Description");
          if (command_alias_name && command_alias_description)
            g_string_append_printf (cmd_summary, "\n  %-16s     %s", command_alias_name, command_alias_description);
        }
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
      if (installroot_used &&
          !(config_used && disable_plugins_loading && cachedir_used && reposdir_used && varsdir_used))
        {
          error = g_error_new_literal (G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
            "The \"--installroot\" argument must be used together with \"--config\", "
            "\"--noplugins\", \"--setopt=cachedir=<path>\", \"--setopt=reposdir=<path>\", "
            "\"--setopt=varsdir=<path>\" arguments.");
          goto out;
        }
        
      if (disable_plugins_loading)
        dnf_context_set_plugins_all_disabled (disable_plugins_loading);

      if (enable_disable_plugin_used && dnf_context_get_plugins_all_disabled ())
        {
          if (disable_plugins_loading)
            g_print ("Loading of plugins is disabled by command line argument \"--noplugins\". "
                     "Use of \"--enableplugin\" and \"--disableplugin\" has no meaning.\n");
          else
            g_print ("Loading of plugins is disabled by configuration file. "
                     "Use of \"--enableplugin\" and \"--disableplugin\" has no meaning.\n");
        }

      if (opt_refresh)
       dnf_context_set_cache_age (ctx, 0);

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

      /* set transaction flags, allow downgrades for all transaction types */
      DnfTransaction *txn = dnf_context_get_transaction (ctx);
      int flags = dnf_transaction_get_flags (txn) | DNF_TRANSACTION_FLAG_ALLOW_DOWNGRADE;
      if (opt_nodocs)
        flags |= DNF_TRANSACTION_FLAG_NODOCS;
      if (opt_test)
        flags |= DNF_TRANSACTION_FLAG_TEST;
      dnf_transaction_set_flags (txn, flags);
      
      /* Disable calling dnf_goal_depsolve() during dnf_context_run().
       * The calling is done with hardcoded parameters. We dont want it. */
      dnf_transaction_set_dont_solve_goal(txn, TRUE);

      if (opt_install_weak_deps == ARG_TRUE)
        dnf_context_set_install_weak_deps (TRUE);
      else if (opt_install_weak_deps == ARG_FALSE)
        dnf_context_set_install_weak_deps (FALSE);

      if (opt_allow_vendor_change == ARG_TRUE)
        dnf_context_set_allow_vendor_change (TRUE);
      else if (opt_allow_vendor_change == ARG_FALSE)
        dnf_context_set_allow_vendor_change (FALSE);

      if (opt_keepcache == ARG_TRUE)
        dnf_context_set_keep_cache (ctx, TRUE);
      else if (opt_keepcache == ARG_FALSE)
        dnf_context_set_keep_cache (ctx, FALSE);

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

      if (opt_no)
        {
          dnf_conf_main_set_option ("assumeno", DNF_CONF_COMMANDLINE, "1", NULL);
        }

      if (opt_yes)
        {
          dnf_conf_main_set_option ("assumeyes", DNF_CONF_COMMANDLINE, "1", NULL);
        }
    }

  const gchar *cmd_name = get_command (&argc, argv);

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
  const gchar *subcmd_name = NULL;
  gboolean with_subcmds = FALSE;

  /* Find the plugin that implements the command cmd_name or its subcommand.
   * Command name (cmd_name) can not contain '_' character. It is reserved for subcomands. */
  if (cmd_name != NULL && strchr(cmd_name, '_') == NULL)
    {
      const gchar *original_cmd_name = g_hash_table_lookup (cmds_aliases, cmd_name);
      const gchar *search_cmd_name = original_cmd_name ? original_cmd_name : cmd_name;
      with_subcmds = g_slist_find_custom (cmds_with_subcmds, search_cmd_name, compare_strings) != NULL;
      g_autofree gchar *mod_name = g_strdup_printf ("command_%s", search_cmd_name);
      plug = peas_engine_get_plugin_info (engine, mod_name);
      if (plug == NULL && with_subcmds)
        {
          subcmd_name = get_command (&argc, argv);
          if (subcmd_name != NULL)
            {
              g_autofree gchar *submod_name = g_strdup_printf ("command_%s_%s", search_cmd_name, subcmd_name);
              plug = peas_engine_get_plugin_info (engine, submod_name);
            }
        }
      if (plug != NULL)
        exten = peas_extension_set_get_extension (cmd_exts, plug);
    }
  if (exten == NULL)
    {
      if (cmd_name == NULL)
        error = g_error_new_literal (G_IO_ERROR,
                                     G_IO_ERROR_FAILED,
                                     "No command specified");
      else if (!with_subcmds)
        error = g_error_new (G_IO_ERROR,
                             G_IO_ERROR_FAILED,
                             "Unknown command: '%s'", cmd_name);
      else if (subcmd_name)
        error = g_error_new (G_IO_ERROR,
                             G_IO_ERROR_FAILED,
                             "Unknown subcommand: '%s'", subcmd_name);
      else
        error = g_error_new (G_IO_ERROR,
                             G_IO_ERROR_FAILED,
                             "Missing subcommand for command: '%s'", cmd_name);

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
  g_slist_free_full(cmds_with_subcmds, g_free);

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
