/*
 * Copyright (C) 2019 Red Hat, Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_REINSTALL dnf_command_reinstall_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandReinstall, dnf_command_reinstall, DNF, COMMAND_REINSTALL, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_reinstall_register_types (PeasObjectModule *module);

G_END_DECLS
