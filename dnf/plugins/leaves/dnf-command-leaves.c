/* dnf-command-leaves.c
 *
 * Copyright © 2022 Emil Renner Berthing <esmil@mailme.dk>
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

#include "dnf-command-leaves.h"

typedef struct {
  guint len;
  guint idx[];
} IdxArray;

static IdxArray *
idx_array_new (guint len)
{
  return g_malloc0 (G_STRUCT_OFFSET (IdxArray, idx) + len * sizeof (guint));
}

static void
idx_array_add (IdxArray *arr, guint idx)
{
  arr->idx[arr->len++] = idx;
}

static gboolean
idx_array_from_set_iter (gpointer key, gpointer value, gpointer user_data)
{
  IdxArray *arr = user_data;
  idx_array_add (arr, GPOINTER_TO_UINT (key));
  return TRUE;
}

static gint
idx_array_compare_func (gconstpointer a, gconstpointer b, gpointer user_data)
{
  guint x = *(const guint *)a;
  guint y = *(const guint *)b;

  if (x < y)
    return -1;
  return x > y;
}

static IdxArray *
idx_array_copy (const guint *idx, guint len)
{
  IdxArray *arr = idx_array_new (len);
  arr->len = len;
  for (guint i = 0; i < len; i++)
    arr->idx[i] = idx[i];
  g_qsort_with_data (arr->idx, arr->len, sizeof (*arr->idx), idx_array_compare_func, NULL);
  return arr;
}

static IdxArray *
idx_array_from_set (GHashTable *set)
{
  IdxArray *arr = idx_array_new (g_hash_table_size (set));
  g_hash_table_foreach_remove (set, idx_array_from_set_iter, arr);
  g_qsort_with_data (arr->idx, arr->len, sizeof (*arr->idx), idx_array_compare_func, NULL);
  return arr;
}

static gint
gtree_dnf_package_cmp (gconstpointer a, gconstpointer b)
{
  return dnf_package_cmp ((DnfPackage *)a, (DnfPackage *)b);
}

static GPtrArray *
build_graph (HyQuery query, const GPtrArray *pkgs)
{
  // create pkg2idx to map DnfPackages to their index in pkgs
  g_autoptr(GTree) pkg2idx = g_tree_new (gtree_dnf_package_cmp);
  for (guint i = 0; i < pkgs->len; i++)
    {
      DnfPackage *pkg = g_ptr_array_index (pkgs, i);
      g_tree_insert (pkg2idx, pkg, GUINT_TO_POINTER (i));
    }

  GPtrArray *graph = g_ptr_array_new_full (pkgs->len, g_free);
  g_autoptr(GHashTable) edges = g_hash_table_new (g_direct_hash, g_direct_equal);

  // for each package resolve its dependencies and add an edge if there is
  // exactly one package satisfying it
  for (guint i = 0; i < pkgs->len; i++)
    {
      DnfPackage *pkg = g_ptr_array_index (pkgs, i);
      g_autoptr(DnfReldepList) reqs = dnf_package_get_requires (pkg);

      const gint nreqs = dnf_reldep_list_count (reqs);
      for (gint j = 0; j < nreqs; j++)
        {
          DnfReldep *req = dnf_reldep_list_index (reqs, j);

          hy_query_filter_reldep (query, HY_PKG_PROVIDES, req);
          g_autoptr(GPtrArray) ppkgs = hy_query_run (query);
          hy_query_clear (query);
          dnf_reldep_free (req);

          if (ppkgs->len != 1)
            continue;

          DnfPackage *ppkg = g_ptr_array_index (ppkgs, 0);
          GTreeNode *node = g_tree_lookup_node (pkg2idx, ppkg);;
          g_assert (node);
          guint idx = GPOINTER_TO_UINT (g_tree_node_value (node));
          if (idx != i) // don't add self-edges
            g_hash_table_insert (edges, GUINT_TO_POINTER (idx), NULL);
        }

      g_ptr_array_add (graph, idx_array_from_set (edges));
    }

  return graph;
}

static GPtrArray *
reverse_graph (const GPtrArray *graph)
{
  g_autofree guint *len = g_malloc0 (graph->len * sizeof (*len));

  for (guint i = 0; i < graph->len; i++)
    {
      const IdxArray *edges = g_ptr_array_index (graph, i);

      for (guint j = 0; j < edges->len; j++)
        len[edges->idx[j]]++;
    }

  GPtrArray *rgraph = g_ptr_array_new_full (graph->len, g_free);
  for (guint i = 0; i < graph->len; i++)
    g_ptr_array_add (rgraph, idx_array_new (len[i]));

  for (guint i = 0; i < graph->len; i++)
    {
      const IdxArray *edges = g_ptr_array_index (graph, i);

      for (guint j = 0; j < edges->len; j++)
        {
          IdxArray *redges = g_ptr_array_index (rgraph, edges->idx[j]);
          idx_array_add (redges, i);
        }
    }

  return rgraph;
}

static GPtrArray *
kosaraju (const GPtrArray *graph)
{
  const guint N = graph->len;
  g_autofree guint *rstack = g_malloc (N * sizeof (*rstack));
  g_autofree guint *stack = g_malloc (N * sizeof (*stack));
  g_autofree gboolean *tag = g_malloc0 (N * sizeof (*tag));
  guint r = N;
  guint top = 0;

  // do depth-first searches in the graph and push nodes to rstack
  // "on the way up" until all nodes have been pushed.
  // tag nodes as they're processed so we don't visit them more than once
  for (guint i = 0; i < N; i++)
    {
      if (tag[i])
        continue;

      guint u = i;
      guint j = 0;
      tag[u] = TRUE;
      while (true)
        {
          const IdxArray *edges = g_ptr_array_index (graph, u);
          if (j < edges->len)
            {
              const guint v = edges->idx[j++];
              if (!tag[v])
                {
                  rstack[top] = j;
                  stack[top++] = u;
                  u = v;
                  j = 0;
                  tag[u] = TRUE;
                }
            }
          else
            {
              rstack[--r] = u;
              if (!top)
                break;
              u = stack[--top];
              j = rstack[top];
            }
        }
    }
  g_assert (r == 0);

  // now searches beginning at nodes popped from rstack in the graph with all
  // edges reversed will give us the strongly connected components.
  // this time all nodes are tagged, so let's remove the tags as we visit each
  // node.
  // the incoming edges to each component is the union of incoming edges to
  // each node in the component minus the incoming edges from component nodes
  // themselves.
  // if there are no such incoming edges the component is a leaf and we
  // add it to the array of leaves.
  g_autoptr(GPtrArray) rgraph = reverse_graph (graph);
  g_autoptr(GHashTable) sccredges = g_hash_table_new (g_direct_hash, g_direct_equal);
  GPtrArray *leaves = g_ptr_array_new_with_free_func (g_free);
  for (; r < N; r++)
    {
      guint u = rstack[r];
      if (!tag[u])
        continue;

      stack[top++] = u;
      tag[u] = FALSE;
      guint s = N;
      while (top)
        {
          u = stack[--s] = stack[--top];
          const IdxArray *redges = g_ptr_array_index (rgraph, u);
          for (guint j = 0; j < redges->len; j++)
            {
              const guint v = redges->idx[j];
              g_hash_table_insert (sccredges, GUINT_TO_POINTER (v), NULL);
              if (!tag[v])
                continue;

              stack[top++] = v;
              tag[v] = FALSE;
            }
        }

      for (guint i = s; i < N; i++)
        g_hash_table_remove (sccredges, GUINT_TO_POINTER (stack[i]));

      if (g_hash_table_size (sccredges) == 0)
        g_ptr_array_add (leaves, idx_array_copy (&stack[s], N - s));
      else
        g_hash_table_remove_all (sccredges);
    }

  return leaves;
}

struct _DnfCommandLeaves
{
  PeasExtensionBase parent_instance;
};

static void dnf_command_leaves_iface_init (DnfCommandInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (DnfCommandLeaves,
                                dnf_command_leaves,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE (DNF_TYPE_COMMAND,
                                                       dnf_command_leaves_iface_init))

static void
dnf_command_leaves_init (DnfCommandLeaves *self)
{
}

static void
disable_available_repos (DnfContext *ctx)
{
  const GPtrArray *repos = dnf_context_get_repos (ctx);

  for (guint i = 0; i < repos->len; ++i)
    {
      DnfRepo *repo = g_ptr_array_index (repos, i);
      dnf_repo_set_enabled (repo, DNF_REPO_ENABLED_NONE);
    }
}

static gint
gptrarr_dnf_package_cmp (gconstpointer a, gconstpointer b)
{
  DnfPackage *const *x = a;
  DnfPackage *const *y = b;
  return dnf_package_cmp (*x, *y);
}

static gint
gptrarr_first_package_cmp (gconstpointer a, gconstpointer b)
{
  IdxArray *const *x = a;
  IdxArray *const *y = b;
  guint i = (*x)->idx[0];
  guint j = (*y)->idx[0];

  if (i < j)
    return -1;
  return i > j;
}

static gboolean
dnf_command_leaves_run (DnfCommand      *cmd,
                        int              argc,
                        char            *argv[],
                        GOptionContext  *opt_ctx,
                        DnfContext      *ctx,
                        GError         **error)
{
  if (!g_option_context_parse (opt_ctx, &argc, &argv, error))
    return FALSE;

  // only look at installed packages
  disable_available_repos (ctx);
  dnf_context_setup_sack_with_flags (ctx,
                                     dnf_context_get_state (ctx),
                                     DNF_CONTEXT_SETUP_SACK_FLAG_NONE,
                                     error);

  // get a sorted array of all installed packages
  hy_autoquery HyQuery query = hy_query_create (dnf_context_get_sack (ctx));
  g_autoptr(GPtrArray) pkgs = hy_query_run (query);
  g_ptr_array_sort (pkgs, gptrarr_dnf_package_cmp);

  // build the directed graph of dependencies
  g_autoptr(GPtrArray) graph = build_graph (query, pkgs);

  // run Kosaraju's algorithm to find strongly connected components
  // withhout any incoming edges
  g_autoptr(GPtrArray) leaves = kosaraju (graph);
  g_ptr_array_sort (leaves, gptrarr_first_package_cmp);

  // print the packages grouped by their components
  for (guint i = 0; i < leaves->len; i++)
    {
      const IdxArray *scc = g_ptr_array_index (leaves, i);
      gchar mark = '-';

      for (guint j = 0; j < scc->len; j++)
        {
          DnfPackage *pkg = g_ptr_array_index (pkgs, scc->idx[j]);
          g_print ("%c %s\n", mark, dnf_package_get_nevra (pkg));
          mark = ' ';
        }
    }

  return TRUE;
}

static void
dnf_command_leaves_class_init (DnfCommandLeavesClass *klass)
{
}

static void
dnf_command_leaves_iface_init (DnfCommandInterface *iface)
{
  iface->run = dnf_command_leaves_run;
}

static void
dnf_command_leaves_class_finalize (DnfCommandLeavesClass *klass)
{
}

G_MODULE_EXPORT void
dnf_command_leaves_register_types (PeasObjectModule *module)
{
  dnf_command_leaves_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              DNF_TYPE_COMMAND,
                                              DNF_TYPE_COMMAND_LEAVES);
}
