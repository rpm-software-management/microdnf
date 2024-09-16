/*
 * Copyright (C) 2020 Red Hat, Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_MODULE_ENABLE dnf_command_module_enable_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandModuleEnable, dnf_command_module_enable, DNF, COMMAND_MODULE_ENABLE, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_module_enable_register_types (PeasObjectModule *module);

G_END_DECLS
