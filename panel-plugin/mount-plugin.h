/* mount-plugin.h */

/*
Copyright (C) 2005 Jean-Baptiste jb_dul@yahoo.com
Copyright (C) 2005-2008 Fabian Nowak timystery@arcor.de.

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

#ifndef __MOUNT_PLUGIN_H
# define __MOUNT_PLUGIN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DEBUG
#undef DEBUG
#endif
#ifdef DEBUG_TRACE
#undef DEBUG_TRACE
#endif

#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include <string.h>

#include "devices.h"
#include "helpers.h"
#include "icons.h"

#define APP_NAME N_("Mount Plugin")

#define BORDER 6

#define DEFAULT_MOUNT_COMMAND "mount %d"
#define DEFAULT_UMOUNT_COMMAND "umount %d"

#define DEFAULT_ICON PACKAGE_DATA_DIR "/icons/hicolor/scalable/apps/xfce-mount.svg"

static GtkTooltips *tooltips = NULL;

/**
 * structure t_mounter.
 * Contains values and strings for functionality and GtkWidgets for the GUI.
 */
typedef struct
{
    XfcePanelPlugin *plugin;
    char  *on_mount_cmd;
    gchar *mount_command;
    gchar *umount_command;
    gchar *icon;
    gchar *excluded_filesystems;
    gboolean message_dialog; /**< whether to show (un)success after umount */
    gboolean include_NFSs; /**< whether to also display network file systems */
    gboolean exclude_FSs;
    gboolean exclude_devicenames;
    gboolean eject_drives;
    gboolean showed_fstab_dialog;
    GtkWidget *button;
    GdkPixbuf *button_pb;
    GtkWidget *menu;
    GPtrArray *pdisks; /* contains pointers to struct t_disk */
} t_mounter;


/**
 * Disk button data.
 * Contains labels, progress bars, ... for one line in the menu.
 */
typedef struct
{
    GtkWidget * menu_item;
    GtkWidget * hbox;
    GtkWidget * label_disk;
    GtkWidget * label_mount_info;
    GtkWidget * progress_bar;
    t_disk *disk; /* People, learn to program in an object-oriented way! */
} t_disk_display;


/**
 * Settings dialog.
 * Contains widgets needed for determining the user's preferences.
 */
typedef struct
{
    t_mounter *mt;
    GtkWidget *dialog;
    /* options */
    GtkWidget *string_cmd;
    GtkWidget *string_icon;
    GtkWidget *specify_commands;
    GtkWidget *box_mount_commands;
    GtkWidget *string_mount_command;
    GtkWidget *string_umount_command;
    GtkWidget *show_message_dialog;
    GtkWidget *show_include_NFSs;
    GtkWidget *show_exclude_FSs;
    GtkWidget *show_eject_drives;
    GtkWidget *show_exclude_devicenames;
    GtkWidget *string_excluded_filesystems;
}
t_mounter_dialog;

#endif /* __MOUNT_PLUGIN_H */
