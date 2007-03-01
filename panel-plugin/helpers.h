/* helpers.c */

/*
Copyright (C) 2007 Fabian Nowak timystery@arcor.de.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef HELPERS_H
#define HELPERS_H

#include <glib.h>
#include <glib/gprintf.h>

#include <string.h>

/**
 * seperate list by spaces and store information in @array.
 * @param array  pointer array where list entries are stored in
 * @param list   list string with entres seperated by space
 * @return       0 on failure, number of entries on success.
 */
int
seperate_list (GPtrArray *array, char *list);

/**
 * Print device to the format string where there is \%d. The new string dest
 * is allocated with malloc and must therefore be freed.
 * @param dest        double pointer to store location of newly created string
 * @param format     format string containing '\%d' to be replaced by @device
 * @param device    argument to replace '\%d' in @format with
 * @return             0 on failure or no replacement, >0 on success.
 */
int
deviceprintf (char **dest, char *format, char *device);

/**
 * Print mount point to the format string where there is \%m. The new string
 * dest is allocated with malloc and must therefore be freed.
 * @param dest            double pointer to store location of newly created string
 * @param format        format string containing '%m' to be replaced by @mountpoint
 * @param mountpoint    argument to replace '%m' in @format with
 * @return                0 on failure or no replacement, >0 on success.
 */
int
mountpointprintf (char **dest, char *format, char *mountpoint);

#endif /* HELPER_H */

