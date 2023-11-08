/* dnf-command-swap.h
 *
 * Copyright (C) 2022 Red Hat, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_SWAP dnf_command_swap_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandSwap, dnf_command_swap, DNF, COMMAND_SWAP, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_swap_register_types (PeasObjectModule *module);

G_END_DECLS
