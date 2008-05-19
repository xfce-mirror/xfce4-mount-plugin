/* devices.h */
/*
Copyright (C) 2005 Jean-Baptiste jb_dul@yahoo.com
Copyright (C) 2007 , 2008Fabian Nowak <timystery@arcor.de>

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
# define _DEVICES_H_
#include <glib.h>

#include "helpers.h"
/* #include "mount-plugin.h" */
/* extern t_disk_display ; */
/* People, learn to program in an object-oriented way! */

#ifdef DEBUG
#undef DEBUG
#endif
#ifdef DEBUG_TRACE
#undef DEBUG_TRACE
#endif

/**
 * An enum.
 * NONE and ERROR as aliases.
 */
enum {
    NONE = 0,
    ERROR
};


/**
 * Device class.
 * Used for classification of devices discovered in the system into CDs,
 * removable media etc.
 */
typedef enum {
    HARDDISK = 0,
    REMOTE,
    CD_DVD,
    REMOVABLE,
    UNKNOWN
} t_deviceclass;


/**
 * Mount information.
 * Additional info when a t_disk is mounted.
 */
typedef struct s_mount_info {
    float size;                /**< Total size of device */
    float used;                /**< Used size of device */
    float avail;            /**< Available size of device */
    unsigned int percent;     /**< Percentage used */
    char *type;                /**< Unused? Distinguish remote file systems? */
    char *mounted_on;        /**< Current mount point */
} t_mount_info;


/**
 * Disk information.
 */
typedef struct s_disk {
    char *device;                 /**< Device name, e.g. /dev/cdrom */
    char *mount_point;             /**< Device mount point, e.g. /mnt/cdrom */
    t_mount_info *  mount_info; /**< NULL if not mounted */
    /*`t_disk_display *disk_display; */ /* People, learn to program in an object-oriented way! */
    t_deviceclass dc;             /**< Device classification */
} t_disk;


/**
 *  Return a string containing a size expressed in KB, MB or GB and the unit it
 * is expressed in.
 * @param size    Disk size in Bytes
 * @return        Disk size in MBytes or GBytes
 */
char * get_size_human_readable(float size);


/**
 * Mount a t_disk.
 * @param pdisk            Disk to mount
 * @param on_mount_cmd    Command to execute after successfully mounting the device
 * @param mount_command    Command to use for mounting the device, still containing placeholders like \%d and \%m.
 * @param eject            Whether to inject the device before mounting.
 * @return                Exit status of the mount command
 */
void disk_mount (t_disk *pdisk, char *on_mount_cmd, char* mount_command, gboolean eject);


/**
 * Unmount a t_disk.
 * @param pdisk            Disk to mount
 * @param umount_command    Command to use for unmounting the device, still containing placeholders like \%d and \%m.
 * @param synchronous    Whether to execute the command synchronously to the program itself thus waiting for the output
 * @param eject            Whether to eject the device after unmounting.
 * @return                 Exit status of the mount command
 */
int disk_umount (t_disk *pdisk, char* umount_command, gboolean synchronous, gboolean eject);


/**
 * Fill a GPtrArray with pointers on struct t_disk containing infos on devices
 * and theoretical mount point. Uses setfsent() and getfsent().
 * @param include_NFSs    TRUE if remote file systems should also be displayed.
 * @return                GPtrArray with t_disks
 */
GPtrArray * disks_new (gboolean include_NFSs) ;


/**
 * Free a GPtrArray containing pointer on struct t_disk elements.
 * @param pdisks    Pointer array of t_disk
 */
void disks_free (GPtrArray * * pdisks);


/**
 * Print a GPtrArray containing pointer on struct t_disk elements.
 * @param pdisks    Pointer array of t_disk
 */
void disks_print (GPtrArray * pdisks);


/**
 * Removes specfied device from array.
 * @param pdisks    Pointer array of t_disk
 * @param device    Device to remove from list
 * @return            TRUE on success.
 */
gboolean disks_remove_device (GPtrArray * pdisks, char *device);


/**
 * Removes specfied mountpoint from array.
 * @param pdisks    Pointer array of t_disk
 * @param device    Mountpoint to remove from list
 * @return            TRUE on success.
 */
gboolean disks_remove_mountpoint (GPtrArray * pdisks, char *mountpoint);


/**
 * Search for device in pointer array of t_disks.
 * @param pdisks    Pointer array of t_disk
 * @param device    Device to search for
 * @return            pointer to t_disk if found, NULL else.
 */
t_disk * disks_search (GPtrArray * pdisks, char * device);

/**
 * Lookup given mountpoint and device for exclusion
 * @param excluded_FSs  Pointer to array of excluded filesystems
 * @param mountpoint    Mountpoint of device to search for
 * @param device        Original device path to search for
 * @return              TRUE, if device is to be excluded
 */
gboolean exclude_filesystem (GPtrArray *excluded_FSs, gchar *mountpoint, gchar *device);

/**
 * Refresh t_mount_info infos in a GPtrArray containing struct t_disk *
 * elements.
 * @param pdisks    Pointer array of t_disk
 */
void disks_refresh (GPtrArray * pdisks, GPtrArray *excluded_FSs);


/**
 * Returns classification for given information
 * @param device        Device to get class for
 * @param mountpoint    Mountpoint used as additional information for classfication
 * @return                Device class
 */
t_deviceclass disk_classify (char* device, char *mountpoint);


/**
 * Checks, if disk is still mounted
 * @param disk          device name or mountpoint to check
 * @return              true, if mounted, else false.
 */
gboolean disk_check_mounted (const char *disk);

#endif /* _DEVICES_H_ */


