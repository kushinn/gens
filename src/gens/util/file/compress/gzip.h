/***************************************************************************
 * Gens: GZip File Compression Handler.                                    *
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

#ifndef GENS_COMPRESS_GZIP_H
#define GENS_COMPRESS_GZIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

int GZip_Detect_Format(FILE *f);
int GZip_Get_Num_Files(const char *filename);
int GZip_Get_First_File_Info(const char *filename, struct COMPRESS_FileInfo_t *retFileInfo);
struct COMPRESS_FileInfo_t* GZip_Get_File_Info(const char *filename);
int GZip_Get_File(const char *filename, const struct COMPRESS_FileInfo_t *fileInfo, void *buf, int size);

#ifdef __cplusplus
}
#endif

#endif
