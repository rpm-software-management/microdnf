/* dnf-command-remove.h
 *
 * Copyright © 2016 Igor Gnatenko <ignatenko@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_REMOVE dnf_command_remove_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandRemove, dnf_command_remove, DNF, COMMAND_REMOVE, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_remove_register_types (PeasObjectModule *module);

G_END_DECLS
