/* dnf-command-makecache.h
 *
 * Copyright (C) 2021 Red Hat, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_MAKECACHE dnf_command_makecache_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandMakecache, dnf_command_makecache, DNF, COMMAND_MAKECACHE, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_makecache_register_types (PeasObjectModule *module);

G_END_DECLS
