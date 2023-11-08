/* dnf-command-upgrade.h
 *
 * Copyright © 2016 Igor Gnatenko <ignatenko@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_UPGRADE dnf_command_upgrade_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandUpgrade, dnf_command_upgrade, DNF, COMMAND_UPGRADE, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_upgrade_register_types (PeasObjectModule *module);

G_END_DECLS
