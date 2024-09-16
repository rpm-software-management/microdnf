/* dnf-command-leaves.h
 *
 * Copyright © 2022 Emil Renner Berthing <esmil@mailme.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_LEAVES dnf_command_leaves_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandLeaves, dnf_command_leaves, DNF, COMMAND_LEAVES, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_leaves_register_types (PeasObjectModule *module);

G_END_DECLS
