/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_libc.c: libc translation.                                          *
 *                                                                         *
 * Copyright (c) 2009 by David Korth.                                      *
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

#include "w32u_libc.h"
#include "W/w32u_libcW.h"
#include "A/w32u_libcA.h"

/** Unicode wrappers. **/
MAKE_FUNCPTR(access);
MAKE_FUNCPTR(fopen);
MAKE_FUNCPTR(stat);

/** Unicode functions that are accessed directly. **/
MAKE_FUNCPTR(_wcsicmp);


int WINAPI w32u_libc_init(void)
{
	if (w32u_is_unicode)
		return w32u_libcW_init();
	else
		return w32u_libcA_init();
}
