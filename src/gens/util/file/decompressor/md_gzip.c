/***************************************************************************
 * Gens: GZip Decompressor.                                                *
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

#include "md_gzip.h"

// ZLib.
#include <zlib.h>

// C includes.
#include <stdlib.h>

// Win32 Unicode Translation Layer.
#ifdef _WIN32
#include "libgsft/w32u/w32u.h"
#include "libgsft/w32u/w32u_libc.h"
#endif

// MDP includes.
#include "mdp/mdp_error.h"

// libgsft includes.
#include "libgsft/gsft_strdup.h"
#include "libgsft/gsft_unused.h"


// GZip decompressor functions.
static int decompressor_gzip_detect_format(FILE *zF);
static int decompressor_gzip_get_file_info(FILE *zF, const char* filename,
						mdp_z_entry_t** z_entry_out);
static int decompressor_gzip_get_file(FILE *zF, const char* filename,
					mdp_z_entry_t *z_entry, void *buf,
					const size_t size, size_t *ret_size);

// GZip decompressor struct.
const decompressor_t decompressor_gzip =
{
	.detect_format	= decompressor_gzip_detect_format,
	.get_file_info	= decompressor_gzip_get_file_info,
	.get_file	= decompressor_gzip_get_file
};


/**
 * decompressor_gzip_detect_format(): Detect if this file can be handled by this decompressor.
 * @param zF Open file handle.
 * @return Non-zero if this file can be handled; 0 if it can't be.
 */
static int decompressor_gzip_detect_format(FILE *zF)
{
	unsigned char buf[2];
	fseek(zF, 0, SEEK_SET);
	fread(buf, sizeof(unsigned char), 2, zF);
	return (buf[0] == 0x1F && buf[1] == 0x8B);
}


/**
 * decompressor_gzip_get_file_info(): Get information about all files in the archive.
 * @param zF		[in] Open file handle.
 * @param filename	[in] Filename of the archive.
 * @param z_entry_out	[out] Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code.
 */
static int decompressor_gzip_get_file_info(FILE *zF, const char* filename, mdp_z_entry_t** z_entry_out)
{
	GSFT_UNUSED_PARAMETER(zF);
	
	// GZip-compressed files can only have one file.
	unsigned char buf[4096];
	
	FILE *f = fopen(filename, "rb");
	if (!f)
		return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	
	gzFile gzfd = gzdopen(fileno(f), "rb");
	if (!gzfd)
	{
		fclose(f);
		return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}
	
	// Read through the GZip file until we hit an EOF.
	int filesize = 0;
	while (!gzeof(gzfd))
	{
		filesize += gzread(gzfd, buf, sizeof(buf));
	}
	
	// Close the GZip fd.
	gzclose(gzfd);
	
	// Allocate memory for the mdp_z_entry_t.
	mdp_z_entry_t *z_entry = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
	
	// Set the elements of the list.
	z_entry->filename = (filename ? strdup(filename) : NULL);
	z_entry->filesize = filesize;
	z_entry->next = NULL;
	
	// Return the list.
	*z_entry_out = z_entry;
	return MDP_ERR_OK;
}


/**
 * decompressor_gzip_get_file(): Get a file from the archive.
 * @param zF		[in] Open file handle.
 * @param filename	[in] Filename of the archive.
 * @param file_list	[in] Pointer to decompressor_file_list_t element to get from the archive.
 * @param buf		[in] Buffer to read the file into.
 * @param size		[in] Size of buf (in bytes).
 * @param ret_size	[in] Pointer to size_t to store the number of bytes read.
 * @return MDP error code.
 */
static int decompressor_gzip_get_file(FILE *zF, const char *filename,
					mdp_z_entry_t *z_entry, void *buf,
					const size_t size, size_t *ret_size)
{
	// Unused parameters.
	GSFT_UNUSED_PARAMETER(zF);
	GSFT_UNUSED_PARAMETER(z_entry);
	
	// All parameters (except zF and z_entry) must be specified.
	if (!filename || !buf || !size || !ret_size)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	FILE *f = fopen(filename, "rb");
	if (!f)
		return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	
	gzFile gzfd = gzdopen(fileno(f), "rb");
	if (!gzfd)
	{
		fclose(f);
		return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}
	
	// Decompress the GZip file into memory.
	*ret_size = gzread(gzfd, buf, size);
	gzclose(gzfd);
	return MDP_ERR_OK;
}
