/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (Plugin COnfiguration)    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef GENS_MDP_HOST_GENS_CONFIG_HPP
#define GENS_MDP_HOST_GENS_CONFIG_HPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>

/* MDP Host Services. */
#include "mdp/mdp.h"
#include "mdp/mdp_fncall.h"
#include "mdp/mdp_host.h"

#ifdef __cplusplus
extern "C" {
#endif

int MDP_FNCALL mdp_host_config_get(mdp_t *plugin, const char* key, const char* def, char *out_buf, unsigned int size);
int MDP_FNCALL mdp_host_config_set(mdp_t *plugin, const char* key, const char* value);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MDP_HOST_GENS_CONFIG_HPP */
