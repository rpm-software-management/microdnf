/*
 * Copyright (C) 2019 Red Hat, Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_REPOLIST dnf_command_repolist_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandRepolist, dnf_command_repolist, DNF, COMMAND_REPOLIST, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_repolist_register_types (PeasObjectModule *module);

G_END_DECLS
