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

#include <glib.h>
#include <glib/gprintf.h>

/**
 * seperate list by spaces and store information in @array.
 * @param array  pointer array where list entries are stored in
 * @param list   list string with entres seperated by space
 * @return       0 on failure, number of entries on success.
 */
int
seperate_list (GPtrArray *array, char *list)
{
    int retval = 0;
    char *p, *q; /* , *r;
    r = p; */

	if (list==NULL)
		return retval;

    p = strdup(list);

    if (array==NULL)
        array = g_ptr_array_new ();

    q = strchr (p, ' ');
    while ( q < p+strlen(p) && q!=NULL)
    {
        q[0] = '\0';
        g_ptr_array_add (array, (gpointer) p);
        g_printf("p=%s\n", p);
        p = q+2;

        q = strchr (p, ' ');
        retval++;
    }
    g_printf("p=%s\n", p);
    g_ptr_array_add (array, (gpointer) p);
    retval++;

    /* free (r); */

    return retval;
}

/**
 * Print device to the format string where there is \%d. The new string dest
 * is allocated with malloc and must therefore be freed.
 * @param dest        double pointer to store location of newly created string
 * @param format     format string containing '\%d' to be replaced by @device
 * @param device    argument to replace '\%d' in @format with
 * @return             0 on failure or no replacement, >0 on success.
 */
int
deviceprintf (char **dest, char *format, char *device)
{
    int retval=0;
    char *p = strdup(format), *q, *r;

    r = p;

    if (*dest==NULL)
        *dest = "";

    for (q = strstr(p, "\%d"); q!=NULL; q = strstr(p, "\%d")) {
        q[0] = '\0';
        *dest = g_strconcat (*dest, p, device, " ", NULL);
        p = q+2;
        retval++;
    }
    *dest = g_strconcat (*dest, p, NULL);

    g_free(r);

    return retval;
}

/**
 * Print mount point to the format string where there is \%m. The new string
 * dest is allocated with malloc and must therefore be freed.
 * @param dest            double pointer to store location of newly created string
 * @param format        format string containing '%m' to be replaced by @mountpoint
 * @param mountpoint    argument to replace '%m' in @format with
 * @return                0 on failure or no replacement, >0 on success.
 */
int
mountpointprintf (char **dest, char *format, char *mountpoint)
{
    int retval=0;
    char *p = strdup(format), *q, *r;

    r = p;

    if (*dest==NULL)
        *dest = "";

    for (q = strstr(p, "\%m"); q!=NULL; q = strstr(p, "\%m")) {
        q[0] = '\0';
        *dest = g_strconcat (*dest, p, mountpoint, " ", NULL);
        p = q+2;
        retval++;
    }
    *dest = g_strconcat (*dest, p, NULL);

    g_free(r);

    return retval;
}
