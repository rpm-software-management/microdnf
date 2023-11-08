/* dnf-command-clean.h
 *
 * Copyright © 2017 Jaroslav Rohel <jrohel@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_CLEAN dnf_command_clean_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandClean, dnf_command_clean, DNF, COMMAND_CLEAN, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_clean_register_types (PeasObjectModule *module);

G_END_DECLS
