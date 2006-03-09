/* devices.h */
/*
Copyright (C) 2005 Jean-Baptiste jb_dul@yahoo.com

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

#ifndef _DEVICES_H_
#define _DEVICES_H_
#include <glib.h>

enum {
	NONE = 0,
	ERROR
};


/* struct mount info, additional info when a t_disk is mounted */
typedef struct s_mount_info {
	float size; /* total size of device */
	float used; /* used size of device */
	float avail; /* Available size of device */
	unsigned int percent; /* percentage used */
	char *type;
	char *mounted_on; /* actual mount point - current??? */
} t_mount_info;
/*-------------------------------------------------*/

/* struct t_disk, describes a t_disk */
typedef struct s_disk {
	char *device; /* device name, e.g. /dev/cdrom */
	char *mount_point; /* device mount point, e.g. /mnt/cdrom */
	t_mount_info *  mount_info; /* NULL if not mounted */

} t_disk;
/*----------------------------------------------*/

/*-------------------- get_size_human_readable --------------------*/
/* return a string containing a size expressed in KB,MB or GB and the unit it is expressed in */
char * get_size_human_readable(float size);


/*------------ mount a t_disk ---------------*/
/* return exit status of the mount command*/
void disk_mount (t_disk *pdisk, char *on_mount_cmd, char* mount_command);

/* --------------unmount a t_disk ----------------*/
int disk_umount (t_disk *pdisk, char* umount_command, gboolean synchronous);

/*------------------------- disks_new ----------------*/
/* fill a GPtrArray with pointers on struct t_disk containing infos on devices and theoretical mount point. use setfsent() and getfsent(). */
GPtrArray * disks_new(gboolean include_NFSs) ;

/*--------------------- disks_free --------------------------*/
/* free a GPtrArray containing pointer on struct t_disk elements */
void disks_free (GPtrArray * * pdisks);

/*----------------------- disks_print----------------------*/
/* print a GPtrArray containing pointer on struct t_disk elements */
void disks_print (GPtrArray * pdisks);

/*-------------------- disks_search -------------------------*/
/*return a pointer on FIRST struct t_disk containing char * device as device field 
if not found return NULL */
t_disk * disks_search (GPtrArray * pdisks, char * device);

/* --------------- disks_refresh ----------------------*/
/* refresh t_mount_info infos in a GPtrArray containing struct t_disk * elements */
void disks_refresh (GPtrArray * pdisks);

#endif /* _DEVICES_H_ */


