/* dnf-command-distrosync.h
 *
 * Copyright © 2021 Neal Gompa <ngompa13@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_DISTROSYNC dnf_command_distrosync_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandDistroSync, dnf_command_distrosync, DNF, COMMAND_DISTROSYNC, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_distrosync_register_types (PeasObjectModule *module);

G_END_DECLS
