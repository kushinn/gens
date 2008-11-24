/***************************************************************************
 * Gens: [MDP] 25% Scanline renderer. (Plugin Data File)                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include <stdint.h>
#include <string.h>

#include "plugins/mdp.h"
#include "plugins/mdp_cpuflags.h"

#include "mdp_render_scanline_25.hpp"

static MDP_Desc_t MDP_Desc =
{
	.name = "25% Scanline Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "25% scanline renderer.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
};

static MDP_Render_t MDP_Render =
{
	.interfaceVersion = MDP_RENDER_INTERFACE_VERSION,
	.blit = mdp_render_scanline_25_cpp,
	.scale = 2,
	.flags = 0,
	.tag = "25% Scanline"
};

MDP_t mdp_render_scanline_25 =
{
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 1, 0),
	.type = MDPT_RENDER,
	
	// UUID: dacc2ec9-c442-4071-a4bd-dd6298875e42
	.uuid = {0xDA, 0xCC, 0x2E, 0xC9,
		 0xC4, 0x42,
		 0x40, 0x71,
		 0xA4, 0xBD,
		 0xDD, 0x62, 0x98, 0x87, 0x5E, 0x42},
	
	// CPU flags
	.cpuFlagsSupported = MDP_CPUFLAG_MMX,
	.cpuFlagsRequired = 0,
	
	// Description
	.desc = &MDP_Desc,
	
	// Init/Shutdown functions
	.init = NULL,
	.end = NULL,
	
	.plugin_t = (void*)&MDP_Render
};
