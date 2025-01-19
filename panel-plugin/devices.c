/* devices.c */

/*
Copyright (C) 2005 Jean-Baptiste jb_dul@yahoo.com
Copyright (C) 2005-2008 Fabian Nowak timystery@arcor.de.
Copyright (C) 2009,2012 Landry Breuil <landry@xfce.org>

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstab.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* WEXITSTATUS */
#include <sys/types.h>
#ifdef HAVE_GETMNTENT
#include <mntent.h>
#include <sys/vfs.h>
#elif defined (HAVE_GETMNTINFO)
#include <sys/param.h>
#include <sys/mount.h>
#else
#error no getmntent/getmntinfo ? send patches !
#endif

#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>

#include "devices.h"

/* declare that one here to appease -Werror */
t_mount_info * mount_info_new_from_stat (struct statfs * pstatfs, char * mnt_type, char * mnt_dir);

#define KB 1024
#define MB 1048576
#define GB 1073741824

#define MTAB "/etc/mtab"


/**
 * Return a string containing a size expressed in KB, MB or GB and the unit
 * it is expressed in.
 *
 * @param  size: Size in bytes
 * @return  Formatted output, e.g. "1.5 GB"
 */
char *
get_size_human_readable (float size)
{
    if (size < KB) return g_strdup_printf (_("%.1f B"), size);
    if (size < MB) return g_strdup_printf (_("%.1f KB"), size/KB);
    if (size < GB) return g_strdup_printf (_("%.1f MB"), size/MB);
    return g_strdup_printf (_("%.1f GB"), size/GB);
}


void
mount_info_print(t_mount_info * mount_info)
{
    if (mount_info != NULL)
    {
        printf (_("size:                %g\n"), mount_info->size);
        printf (_("used size:           %g\n") ,mount_info->used);
        printf (_("available size:       %g\n"), mount_info->avail);
        printf (_("percentage used:     %d\n"), mount_info->percent);
        printf (_("file system type:    %s\n"), mount_info->type);
        printf (_("actual mount point:  %s\n"), mount_info->mounted_on);
    }
    return ;
}


t_mount_info *
mount_info_new (float size, float used, float avail,
                    unsigned int percent, char * type, char * mounted_on)
{
    t_mount_info * mount_info;

    /* check superfluous, as already done in mount_info_new_from_stat */
    /* if ( type != NULL && mounted_on != NULL ) // && size != 0 )
        { */
        mount_info = g_new0(t_mount_info,1);
        mount_info->size = size;
        mount_info->used = used;
        mount_info->avail = avail;
        mount_info->percent = percent;
        mount_info->type = g_strdup(type);
        mount_info->mounted_on = g_strdup(mounted_on);
        return mount_info;
    /*  }
    return NULL; */
}


/**
 * Creates a new struct t_mount_info from a struct statfs data.
 */
t_mount_info *
mount_info_new_from_stat (struct statfs * pstatfs,
                                         char * mnt_type,
                                         char * mnt_dir)
{
    t_mount_info * mount_info ;
    float size;  /* total size of device */
    float used;  /* used size of device */
    float avail; /* Available size of device */
    unsigned int percent ; /* percentage used */

    TRACE("Entering mount_info_new_from_stat");

    if (pstatfs!=NULL && mnt_type!=NULL && mnt_dir!=NULL)
    {
        /* compute sizes in bytes */
        size = (float) pstatfs->f_bsize * (float) pstatfs->f_blocks;
        used = (float) pstatfs->f_bsize
                * ( (float) pstatfs->f_blocks - (float) pstatfs->f_bfree );
        avail = (float) pstatfs->f_bsize * (float) pstatfs->f_bavail;
        percent = ( (float) pstatfs->f_blocks - (float) pstatfs->f_bavail ) * 100
                / (float) pstatfs->f_blocks;

        /* create new t_mount_info */
        mount_info = mount_info_new (size, used, avail, percent, mnt_type,
                                    mnt_dir);
        return mount_info;
    }
    TRACE("Leaving mount_info_new_from_stat with NULL");
    return NULL;

}


void
mount_info_free(t_mount_info * * mount_info)
{
    if (*mount_info != NULL)
    {
        g_free ((*mount_info)->mounted_on);
        g_free ((*mount_info)->type);
        g_free ((*mount_info));
        *mount_info = NULL ;
    }
    return;
}


void
disk_print (t_disk * pdisk)
{
    if (pdisk != NULL)
    {
        printf (_("disk: %s\n"),pdisk->device);
        printf (_("mount point: %s\n"), pdisk->mount_point);

        if (pdisk->mount_info != NULL)
            mount_info_print (pdisk->mount_info);
        else printf (_("not mounted\n"));
    }
    return;
}

char *
shorten_disk_name (const char *dev, guint len)
{
    char *r, *lastchars, *firstchars;
    if (strncmp(dev, "LABEL=", 6)==0)
    {
      r = g_strdup(dev+6*sizeof(char));
    }
    else if (strlen(dev)>len) // len cannot be set lower than 9
    {
        // we want at least 5 characters at the end so that trimmed UUIDs are still readable
        lastchars = (char *) (dev + strlen(dev) - 5);
        firstchars = g_strndup(dev, len-5-3); // 3 additional ones for the three dots
        r = g_strdup_printf ("%s...%s", firstchars, lastchars);
        g_free (firstchars);
    }
    else
        r = g_strdup (dev);

    return r;
}

t_disk *
disk_new (const char * dev, const char * mountpoint, gint len)
{
    t_disk  * pdisk;

    TRACE("Entering disk_new");

    if ( dev != NULL && mountpoint != NULL)
    {
        pdisk = g_new0 (t_disk,1);
        pdisk->device_short = shorten_disk_name (dev, len);
        pdisk->device = g_strdup(dev);
        pdisk->mount_point = g_strdup (mountpoint);
        pdisk->mount_info = NULL;

        return pdisk ;
    }

    TRACE("Leaving disk_new with NULL");
    return NULL;
}


void
disk_free(t_disk **pdisk)
{
    if (*pdisk !=NULL)
    {
        g_free ((*pdisk)->device);
        g_free ((*pdisk)->device_short);
        g_free ((*pdisk)->mount_point);
        mount_info_free (&((*pdisk)->mount_info));
        g_free(*pdisk);
        *pdisk = NULL;
    }
    return;
}


/**
 * Return exit status of the mount command
 */
void
disk_mount (t_disk *pdisk, char *on_mount_cmd, char* mount_command, gboolean eject)
{
    gchar *tmp = NULL, *cmd = NULL;
    gchar *output = NULL, *erroutput = NULL;
    int exit_status = 0;
    GError *error = NULL;
    gboolean val;

    if (pdisk != NULL)
    {
        DBG("disk_mount: dev=%s, mountpoint=%s, mount_command=%s, on_mount_cmd=%s, eject=%d", pdisk->device, pdisk->mount_point, mount_command, on_mount_cmd, eject);
        if (eject) {
#ifdef __OpenBSD__
/* hack: on OpenBSD, eject(1) -t expects cd0/cd1 (or rcd0c/rcd1c), if passed /dev/cdXa it will spit 'No medium found' */
            tmp = g_strstr_len(pdisk->device, strlen(pdisk->device), "/dev/cd");
            if (tmp) {
                cmd = g_strconcat ("eject -t cd", tmp + 7, NULL);
                /* remove chars after cdX */
                cmd[12] = '\0';
                tmp = NULL;
            }
            else
                cmd = g_strconcat ("eject -t ", pdisk->device, NULL);
#else
            cmd = g_strconcat ("eject -t ", pdisk->device, NULL);
#endif
            val = g_spawn_command_line_sync (cmd, &output, &erroutput, &exit_status, &error);
            DBG("cmd: '%s', returned %d, exit_status=%d", cmd, val, exit_status);
            if (val == FALSE || exit_status != 0)
               goto out;
            g_free(cmd);
            cmd = NULL;
        }
        deviceprintf (&tmp, mount_command, pdisk->device);
        mountpointprintf (&cmd, tmp, pdisk->mount_point);
        /* cmd contains mount_command device mount_point */
        val = g_spawn_command_line_sync (cmd, &output, &erroutput, &exit_status, &error);
        DBG("cmd: '%s', returned %d, exit_status=%d", cmd, val, exit_status);
        if (val == FALSE || exit_status != 0)
        {
            /* show error message if smth failed */
            //xfce_dialog_show_error (NULL, error, "%s %s %d, %s %s", _("Mount Plugin:\n\nError executing command."),
                //_("Return value:"), WEXITSTATUS(exit_status), _("\nError was:"), erroutput);
            //xfce_dialog_show_error (NULL, error, _("Failed to mount device \"%s\"."), pdisk->device);
            xfce_message_dialog (NULL,
                               _("Xfce 4 Mount Plugin"),
                               "dialog-error",
                               _("Failed to mount device:"),
                               pdisk->device,
                               "gtk-ok",
                               GTK_RESPONSE_OK,
                               NULL);
            goto out;
        }

        if (on_mount_cmd != NULL && strlen(on_mount_cmd)!=0) {
            g_free(tmp);
            tmp = NULL;
            g_free(cmd);
            cmd = NULL;
            deviceprintf(&tmp, on_mount_cmd, pdisk->device);
            mountpointprintf(&cmd, tmp, pdisk->mount_point);
            val = g_spawn_command_line_async (cmd, &error);
            DBG("cmd: '%s', returned %d", cmd, val);
            /* show error message if smth failed */
            if (val == FALSE)
                //xfce_dialog_show_error (NULL, error, _("Error executing on-mount command \"%s\"."), on_mount_cmd);
            xfce_message_dialog (NULL,
                               _("Xfce 4 Mount Plugin"),
                               "dialog-error",
                               _("Error executing on-mount command:"),
                               on_mount_cmd,
                               "gtk-ok",
                               GTK_RESPONSE_OK,
                               NULL);

        }
out:
        g_free(cmd);
        if (tmp)
            g_free(tmp);
    }
}


/**
 * Return exit status of the umount command.
 */
void
disk_umount (t_disk *pdisk, char* umount_command, gboolean show_message_dialog, gboolean eject)
{
    gchar *tmp = NULL, *cmd = NULL;
    gchar *output = NULL, *erroutput = NULL;
    int exit_status = 0;
    GError *error = NULL;
    gboolean val;

    if (pdisk != NULL)
    {

        DBG("disk_umount: dev=%s, mountpoint=%s, umount_command=%s, show_message_dialog=%d, eject=%d, type=%s", pdisk->device, pdisk->mount_point, umount_command, show_message_dialog, eject, pdisk->mount_info->type);
        if (strstr(pdisk->mount_info->type, "fuse."))
          deviceprintf(&tmp, "fusermount -u %m", pdisk->device);
        else
          deviceprintf(&tmp, umount_command, pdisk->device);

        mountpointprintf(&cmd, tmp, pdisk->mount_point);
        /* cmd contains umount_command device mount_point */
        val = g_spawn_command_line_sync (cmd, &output, &erroutput, &exit_status, &error);
        DBG("cmd: '%s', returned %d, exit_status=%d", cmd, val, exit_status);
        if (val == FALSE || exit_status != 0)
           goto out;

        if (eject) {
            g_free(cmd);
            cmd = NULL;
            cmd = g_strconcat ("eject ", pdisk->device, NULL);
            val = g_spawn_command_line_sync (cmd, &output, &erroutput, &exit_status, &error);
            DBG("cmd: '%s', returned %d, exit_status=%d", cmd, val, exit_status);
        }

out:
        g_free(cmd);
        if (tmp)
            g_free(tmp);
        /* show error message if smth failed */
        if (val == FALSE || exit_status != 0)
            //xfce_dialog_show_error (NULL, error, "%s %s %d, %s %s", _("Mount Plugin: Error executing command."),
                //_("Returned"), WEXITSTATUS(exit_status), _("error was"), erroutput);
            //xfce_dialog_show_error (NULL, error, _("Failed to umount device \"%s\"."), pdisk->device);
            xfce_message_dialog (NULL,
                               _("Xfce 4 Mount Plugin"),
                               "dialog-error",
                               _("Failed to umount device:"),
                               pdisk->device,
                               "gtk-ok",
                               GTK_RESPONSE_OK,
                               NULL);

        if (show_message_dialog && !eject && val == TRUE && exit_status == 0)
            //xfce_dialog_show_info (NULL, NULL, _("The device \"%s\" should be removable safely now."), pdisk->device);
            xfce_message_dialog (NULL,
                               _("Xfce 4 Mount Plugin"),
                               "dialog-information",
                               _("The device should be removable safely now:"),
                               pdisk->device,
                               "gtk-ok",
                               GTK_RESPONSE_OK,
                               NULL);
        if (show_message_dialog && disk_check_mounted(pdisk->device))
            //xfce_dialog_show_error (NULL, NULL, _("An error occurred. The device \"%s\" should not be removed!"), pdisk->device);
            xfce_message_dialog (NULL,
                               _("Xfce 4 Mount Plugin"),
                               "dialog-error",
                               _("An error occurred. The device should not be removed:"),
                               pdisk->device,
                               "gtk-ok",
                               GTK_RESPONSE_OK,
                               NULL);
    }
}

/**
 * Checks whether the pdisk is already inserted with a similar path with more
 * or less slashes.
 * @param pdisks  Pointer to disks array
 * @param pdisk   Pointer to new disk to lateron insert into pdisks
 * @return true if device or mount point does already exist, false otherwise
 */
gboolean
device_or_mountpoint_exists (GPtrArray * pdisks, t_disk * pdisk)
{
  gboolean returnValue = FALSE;
  guint i;
  t_disk *disk;
  int stringlength1, stringlength2, stringlength3, stringlength4;

  stringlength1 = strlen(pdisk->device);
  stringlength3 = strlen(pdisk->mount_point);

  for (i=0; i < pdisks->len ; i++)
  {
    disk = (t_disk *) (g_ptr_array_index(pdisks,i));
    stringlength2 = strlen(disk->device);
    stringlength4 = strlen(disk->mount_point);

    if (stringlength1==stringlength2+1 && pdisk->device[stringlength1-1]=='/' && strncmp(pdisk->device, disk->device, stringlength2)==0 )
    {
      returnValue = TRUE;
      break;
    }
    else if (stringlength2==stringlength1+1 && disk->device[stringlength2-1]=='/' && strncmp(pdisk->device, disk->device, stringlength1)==0 )
    {
      returnValue = TRUE;
      break;
    }
    else if (stringlength3==stringlength4+1 && pdisk->mount_point[stringlength3-1]=='/' && strncmp(pdisk->mount_point, disk->mount_point, stringlength4)==0 )
    {
      returnValue = TRUE;
      break;
    }
    else if (stringlength4==stringlength3+1 && disk->mount_point[stringlength4-1]=='/' && strncmp(pdisk->mount_point, disk->mount_point, stringlength3)==0 )
    {
      returnValue = TRUE;
      break;
    }
  }

  return returnValue;
}

/**
 * Fill a GPtrArray with pointers on struct t_disk containing infos on devices
 * and theoretical mount point. used setfsent() and getfsent(),
 * now uses setmntent() and getmntent() and enmntent().
 * @param include_NFSs    whether to include network file systems
 * @return                GPtrArray *pdisks containing elements to be displayed
 */
GPtrArray *
disks_new (gboolean include_NFSs, gboolean *showed_fstab_dialog, gint length)
{
    GPtrArray * pdisks; /* to be returned */
    t_disk * pdisk;
    struct fstab *pfstab;
    gboolean has_valid_mount_device;

    pdisks = g_ptr_array_new();

    /* open fstab */
    if (setfsent()!=1)
    {
        /* popup notification dialog */
        if (! (*showed_fstab_dialog) ) {
            //dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                                //GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
    //_("Your /etc/fstab could not be read. This will severely degrade the plugin's abilities."));

            xfce_message_dialog (NULL,
                               _("Xfce 4 Mount Plugin"),
                               "dialog-info",
                               _("Your /etc/fstab could not be read. This will severely degrade the plugin's abilities."),
                               NULL,
                               "gtk-ok",
                               GTK_RESPONSE_OK,
                               NULL);
            /* gtk_dialog_run (GTK_DIALOG (dialog)); */
            //g_signal_connect (dialog, "response",
                    //G_CALLBACK (gtk_widget_destroy), dialog);
             //gtk_widget_show (dialog);
             *showed_fstab_dialog = TRUE;
         }

        return pdisks; /* on error return empty pdisks */
    }


    for (pfstab = getfsent(); pfstab!=NULL; pfstab = getfsent())
    {
        has_valid_mount_device =
                        g_str_has_prefix(pfstab->fs_spec, "/dev/") ||  g_str_has_prefix(pfstab->fs_spec, "UUID=") ||  g_str_has_prefix(pfstab->fs_spec, "LABEL=");

        if (include_NFSs)
            has_valid_mount_device = has_valid_mount_device |
                g_str_has_prefix(pfstab->fs_vfstype, "fuse") |
                g_str_has_prefix(pfstab->fs_vfstype, "shfs") |
                g_str_has_prefix(pfstab->fs_vfstype, "nfs") |
                g_str_has_prefix(pfstab->fs_vfstype, "cifs") |
                g_str_has_prefix(pfstab->fs_vfstype, "smbfs");

        if ( has_valid_mount_device &&
                g_str_has_prefix(pfstab->fs_file, "/") ) {
            pdisk = disk_new (pfstab->fs_spec, pfstab->fs_file, length);
            if (pdisk != NULL) {
                pdisk->dc = disk_classify (pfstab->fs_spec, pfstab->fs_file);
                if (!device_or_mountpoint_exists(pdisks, pdisk))
                  g_ptr_array_add (pdisks , pdisk);
                else
                  disk_free (&pdisk);
            }

        }

    } /* end for */

    endfsent(); /* close file */

    return pdisks;
}


/**
 * Free a GPtrArray containing pointer on struct t_disk elements
 */
void
disks_free (GPtrArray ** pdisks)
{
    unsigned int i ;
    t_disk * pdisk ;

    if (pdisks != NULL && *pdisks != NULL)
    {
        for (i=0; i < (*pdisks)->len ; i++)
        {
            pdisk = (t_disk *) (g_ptr_array_index((*pdisks),i));
            disk_free (&pdisk) ;
        }
        g_ptr_array_free ((*pdisks), TRUE);
        (*pdisks) = NULL ;
    }
}


/**
 * Print a GPtrArray containing pointer on struct t_disk elements
 */
void
disks_print (GPtrArray * pdisks)
{
    unsigned int i ;
    for (i=0; i < pdisks->len ; i++)
    {
        disk_print (g_ptr_array_index(pdisks, i));
    }
    return ;

}


/**
 * Removes specfied device from array.
 * @return true on success, else false.
 */
gboolean
disks_remove_device (GPtrArray * pdisks, char *device)
{
    unsigned int i;
    gpointer p=NULL;
    gboolean removedEntries = FALSE;
    char *exclude_device;
    size_t device_len;

    for (i=0; i < pdisks->len ; i++)
    {
        exclude_device = ((t_disk *) g_ptr_array_index(pdisks, i))->device;
        if ( strcmp ( exclude_device,
            device )==0 )
        {
            p = g_ptr_array_remove_index(pdisks, i);
              if (p!=NULL)
                removedEntries = TRUE;
        }

        device_len = strlen(device);

        if ( device[device_len-1]=='*' &&
        strncmp ( exclude_device,
            device, device_len-1 )==0 )
        {
            p = g_ptr_array_remove_index(pdisks, i);
              if (p!=NULL)
                removedEntries = TRUE;
        }
    }

    return removedEntries;
}


/**
 * Removes specfied mount point from array.
 * @return true on success, else false.
 */
gboolean
disks_remove_mountpoint (GPtrArray * pdisks, char *mountp)
{
    gboolean removedEntries = FALSE;
    unsigned int i;
    gpointer p=NULL;
    char *exclude_mp;
    size_t mountp_len;

    for (i=0; i < pdisks->len ; i++)
    {
        exclude_mp = ((t_disk *) g_ptr_array_index(pdisks, i))->mount_point;

        if (strcmp ( exclude_mp,
            mountp)==0)
        {
          DBG("REMOVED %s @ %i %s", mountp, i, exclude_mp);
            p = g_ptr_array_remove_index(pdisks, i);
            if (p!=NULL)
                removedEntries = TRUE;
        }

        mountp_len = strlen(mountp);

        if ( mountp[mountp_len-1]=='*' &&
        strncmp ( exclude_mp,
            mountp, mountp_len-1 )==0 )
        {
          DBG("removed %s @ %i %s", mountp, i, exclude_mp);
            p = g_ptr_array_remove_index(pdisks, i);
            if (p!=NULL)
                removedEntries = TRUE;
        }
    }

    return removedEntries;
}


/**
 * @return a pointer on FIRST struct t_disk containing char * device as
 * device field; if not found return NULL.
 */
t_disk *
disks_search (GPtrArray * pdisks, char * mount_point)
{
    unsigned int i ;

    for (i=0; i < pdisks->len ; i++)
    {
        if (g_ascii_strcasecmp (
                ((t_disk *) g_ptr_array_index(pdisks, i))->mount_point,
                mount_point
           )==0 ) return g_ptr_array_index (pdisks, i);
    }
    return NULL;
}


/**
 * Remove struct t_mount_info from a GPtrArray containing
 * struct t_disk * elements.
 * @param pdisks    Array of t_mount_info
 */
void
disks_free_mount_info(GPtrArray * pdisks)
{
    unsigned int i ;

    for (i=0; i < pdisks->len ; i++)
    {
        mount_info_free( &(((t_disk*)g_ptr_array_index(pdisks,i))->mount_info)) ;
    }
    return ;

}


/**
 * Searches array for device and mountpoint. Returns TRUE if found.
 */
gboolean
exclude_filesystem (GPtrArray *excluded_FSs, gchar *mountpoint, gchar *device)
{
    unsigned int i;
    gchar *excluded_FS_i;
    size_t excluded_FS_i_len = 0;

    TRACE("Entering exclude_filesystems");

    g_assert(excluded_FSs != NULL);

    for (i=0; i < excluded_FSs->len; i++)
    {
        excluded_FS_i = (gchar *) g_ptr_array_index(excluded_FSs, i);
        DBG("Comparing %s and %s to %s", mountpoint, device, excluded_FS_i);
        if (g_ascii_strcasecmp (
                excluded_FS_i, mountpoint)==0
            ||
            g_ascii_strcasecmp (
                excluded_FS_i, device)==0
            )
            return TRUE;

        excluded_FS_i_len = strlen(excluded_FS_i);

        if ((excluded_FS_i[excluded_FS_i_len-1]=='*') &&
            (g_ascii_strncasecmp (
                excluded_FS_i, mountpoint, excluded_FS_i_len-1)==0
            ||
            g_ascii_strncasecmp (
                excluded_FS_i, device, excluded_FS_i_len-1)==0
            ))
            return TRUE;
    }

    TRACE("Leaving exclude_filesystems with FALSE");

    return FALSE;
}


/**
 * Refreshes t_mount_info infos in a GPtrArray containing
 * struct t_disk * elements.
 */
void
disks_refresh(GPtrArray * pdisks, GPtrArray *excluded_FSs, gint length)
{
    /* using getmntent/getmntinfo to get filesystems mount information */

#ifdef HAVE_GETMNTENT
    FILE * fmtab = NULL; /* file /etc/mtab */
    struct mntent * pmntent = NULL; /* struct for mnt info */
#elif defined (HAVE_GETMNTINFO)
    int i, nb_mounted_fs = 0;
#endif
    struct statfs * pstatfs = NULL;
    gboolean exclude =  FALSE;

    t_mount_info * mount_info;
    t_disk * pdisk ;

    TRACE("Entering disks_refresh");

    /* remove t_mount_info for all devices */
    disks_free_mount_info (pdisks);

#ifdef HAVE_GETMNTENT
    /* allocate new struct statfs */
    pstatfs = g_new0 (struct statfs, 1);

    /* open file */
    fmtab = setmntent (MTAB, "r"); /* mtab file */
#elif defined (HAVE_GETMNTINFO)
    /* get mounted fs */
    nb_mounted_fs = getmntinfo(&pstatfs,MNT_WAIT);
#endif

    /* start looking for mounted devices */
#ifdef HAVE_GETMNTENT
    for (pmntent=getmntent(fmtab); pmntent!=NULL; pmntent=getmntent(fmtab)) {

        DBG (" have entry: %s on %s", pmntent->mnt_fsname, pmntent->mnt_dir );

        statfs (pmntent->mnt_dir, pstatfs);
#elif defined (HAVE_GETMNTINFO)
    for (i = 0; i < nb_mounted_fs ; i++) {
        DBG (" have entry: %s on %s : type %s", pstatfs[i].f_mntfromname, pstatfs[i].f_mntonname, pstatfs[i].f_fstypename );
#endif

        /* if we got the stat and the block number is non-zero */

        /* get pointer on disk from pdisks */
        /* CHANGED to reflect change in disk_search */
#ifdef HAVE_GETMNTENT
        pdisk = disks_search (pdisks, pmntent->mnt_dir);
#elif defined (HAVE_GETMNTINFO)
        pdisk = disks_search (pdisks, pstatfs[i].f_mntonname);
#endif
        if (excluded_FSs!=NULL)
#ifdef HAVE_GETMNTENT
            exclude = exclude_filesystem (excluded_FSs, pmntent->mnt_dir, pmntent->mnt_fsname);
#elif defined (HAVE_GETMNTINFO)
            exclude = exclude_filesystem (excluded_FSs, pstatfs[i].f_mntonname, pstatfs[i].f_mntfromname);
#endif

        if (pdisk == NULL) { /* if disk is not found in pdisks */

            /* create a new struct t_disk and add it to pdisks */
            /* test for mnt_dir==none or neither block device nor NFS or system device */
            if ( exclude ||
#ifdef HAVE_GETMNTENT
              g_ascii_strcasecmp(pmntent->mnt_dir, "none") == 0 ||
              g_str_has_prefix(pmntent->mnt_fsname, "gvfs-fuse-daemon") ||
              !(g_str_has_prefix(pmntent->mnt_fsname, "/dev/") ||
                g_str_has_prefix(pmntent->mnt_type, "fuse") ||
                g_str_has_prefix(pmntent->mnt_type, "nfs") ||
                g_str_has_prefix(pmntent->mnt_type, "smbfs") ||
                g_str_has_prefix(pmntent->mnt_type, "cifs") ||
                g_str_has_prefix(pmntent->mnt_type, "shfs")
              ) ||
              g_str_has_prefix(pmntent->mnt_dir, "/sys/")
#elif defined (HAVE_GETMNTINFO)
              /* TODO: add support for more fs types on BSD */
              g_ascii_strcasecmp(pstatfs[i].f_mntonname, "none") == 0 ||
              !(g_str_has_prefix(pstatfs[i].f_mntfromname, "/dev/") ||
                g_str_has_prefix(pstatfs[i].f_fstypename, "nfs") ||
                g_str_has_prefix(pstatfs[i].f_fstypename, "mfs")
              )
#endif
            ) continue;

            /* else have valid entry reflecting block device or NFS */
#ifdef HAVE_GETMNTENT
            pdisk = disk_new (pmntent->mnt_fsname, pmntent->mnt_dir, length);
            if (pdisk != NULL)
                pdisk->dc = disk_classify (pmntent->mnt_fsname, pmntent->mnt_dir);
#elif defined (HAVE_GETMNTINFO)
            pdisk = disk_new (pstatfs[i].f_mntfromname, pstatfs[i].f_mntonname, length);
            if (pdisk != NULL)
                pdisk->dc = disk_classify (pstatfs[i].f_mntfromname, pstatfs[i].f_mntonname);
#endif
            g_ptr_array_add (pdisks, pdisk);
        }

        if (pdisk != NULL) {
            /* create new t_mount_info */
#ifdef HAVE_GETMNTENT
            mount_info = mount_info_new_from_stat (pstatfs, pmntent->mnt_type,
                                                   pmntent->mnt_dir);
#elif defined (HAVE_GETMNTINFO)
            mount_info = mount_info_new_from_stat (&pstatfs[i], pstatfs[i].f_fstypename,
                                                   pstatfs[i].f_mntonname);
#endif
            /* add it to pdisk */
            pdisk->mount_info = mount_info;
        }

    } /* end for */

#ifdef HAVE_GETMNTENT
    g_free (pstatfs);
    endmntent (fmtab); /* close file */
#endif

    return;
}


t_deviceclass
disk_classify (char *device, char *mountpoint)
{
    t_deviceclass dc = UNKNOWN;

    if (device == NULL || mountpoint == NULL)
        return dc;

    /* Note: Since linux-2.6.19, you cannot distinguish between scsi/removable
     * drives by sdX and hard disks by hdY, since hdY is replaced by sdY.
     */
    if (strstr(device, "/dev")==NULL) { /* remote or unknown */
        if (strstr(device, "nfs") || strstr(device, "smbfs")
            || strstr(device, "shfs") || strstr(device, "cifs") || strstr(device, "fuse")) {
            dc = REMOTE;
        }
    }
    /* it needs to be said _here_ that BSDs use cd0, cd1 etc., so we hope cd* works for cdrw, cdrom and cd0,1,... nd no other devices */
    else if ( strstr(device, "cd")
                || strstr(device, "dvd") || strstr(mountpoint, "cd") || strstr(mountpoint, "dvd")) {
        dc = CD_DVD;
    }
    else if ( strstr(mountpoint, "usr") || strstr(mountpoint, "var")
                || strstr(mountpoint, "home") || strcmp(mountpoint, "/")==0 ) {
        dc = HARDDISK;
    }

    else if ( strstr(mountpoint, "media") || strstr(mountpoint, "usb") ) {
        dc = REMOVABLE;
    }

    return dc;
}


gboolean
disk_check_mounted (const char *disk)
{
#ifdef HAVE_GETMNTENT
    FILE *fmtab = NULL; /* file /etc/mtab */
    struct mntent *pmntent = NULL; /* struct for mnt info */
#elif defined (HAVE_GETMNTINFO)
    struct statfs * pstatfs = NULL;
    int i, nb_mounted_fs = 0;
#endif
    gboolean retval = FALSE;

#ifdef HAVE_GETMNTENT
    /* open file */
    fmtab = setmntent (MTAB, "r"); /* mtab file */
#elif defined (HAVE_GETMNTINFO)
    /* get mounted fs */
    nb_mounted_fs = getmntinfo(&pstatfs,MNT_WAIT);
#endif

    /* start looking for mounted devices */
#ifdef HAVE_GETMNTENT
    for (pmntent=getmntent(fmtab); pmntent!=NULL; pmntent=getmntent(fmtab))
#elif defined (HAVE_GETMNTINFO)
    for (i = 0; i < nb_mounted_fs ; i++)
#endif
    {
#ifdef HAVE_GETMNTENT
        if (strcmp(pmntent->mnt_dir, disk)==0 ||
            strcmp(pmntent->mnt_fsname, disk)==0 )
#elif defined (HAVE_GETMNTINFO)
        if (strcmp(pstatfs[i].f_mntonname, disk)==0 ||
            strcmp(pstatfs[i].f_mntfromname, disk)==0 )
#endif
        {
            retval = TRUE;
            break;
        }
    }

#ifdef HAVE_GETMNTENT
    endmntent (fmtab); /* close file */
#endif

    return retval;
}
