/***************************************************************************
 * Gens: (Win32) Main Window. (Menu Handling Code)                         *
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

#ifndef GENS_WIN32_MENU_HPP
#define GENS_WIN32_MENU_HPP

#include "ui/common/gens/gens_menu.h"
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

void		gens_menu_parse(const GensMenuItem_t* menu, HMENU container);

// New menu handler.
HMENU		gens_menu_find_item(uint16_t id);

extern HMENU	MainMenu;
extern HACCEL	hAccelTable_Menu;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// Hash table containing all the menu items.
// Key is the menu ID.
#include "macros/hashtable.hpp"
#include <utility>
typedef GENS_HASHTABLE<uint16_t, HMENU> gensMenuMap_t;
typedef std::pair<uint16_t, HMENU> gensMenuMapItem_t;

extern gensMenuMap_t gens_menu_map;

#endif /* __cplusplus */

#endif /* GENS_WIN32_MENU_HPP */