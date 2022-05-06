/* dnf-command-leaves.h
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

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_LEAVES dnf_command_leaves_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandLeaves, dnf_command_leaves, DNF, COMMAND_LEAVES, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_leaves_register_types (PeasObjectModule *module);

G_END_DECLS
