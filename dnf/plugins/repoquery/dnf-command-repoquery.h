/*
 * Copyright (C) 2019 Red Hat, Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_REPOQUERY dnf_command_repoquery_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandRepoquery, dnf_command_repoquery, DNF, COMMAND_REPOQUERY, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_repoquery_register_types (PeasObjectModule *module);

G_END_DECLS
