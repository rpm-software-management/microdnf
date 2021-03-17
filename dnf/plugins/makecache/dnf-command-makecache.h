/* dnf-command-makecache.h
 *
 * Copyright (C) 2021 Red Hat, Inc.
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

#define DNF_TYPE_COMMAND_MAKECACHE dnf_command_makecache_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandMakecache, dnf_command_makecache, DNF, COMMAND_MAKECACHE, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_makecache_register_types (PeasObjectModule *module);

G_END_DECLS
