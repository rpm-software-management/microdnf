/* dnf-command-download.h
 *
 * Copyright © 2020-2021 Daniel Hams <daniel.hams@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "dnf-command.h"
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define DNF_TYPE_COMMAND_DOWNLOAD dnf_command_download_get_type ()
G_DECLARE_FINAL_TYPE (DnfCommandDownload, dnf_command_download, DNF, COMMAND_DOWNLOAD, PeasExtensionBase)

G_MODULE_EXPORT void dnf_command_download_register_types (PeasObjectModule *module);

G_END_DECLS
