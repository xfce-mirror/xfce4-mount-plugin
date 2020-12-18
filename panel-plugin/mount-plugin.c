/* mount-plugin.c */

/*
Copyright (C) 2005 Jean-Baptiste <jb_dul@yahoo.com>
Copyright (C) 2005-2017 Fabian Nowak <timystery@arcor.de>.
Copyright (C) 2012 Landry Breuil <landry@xfce.org>

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

#include <stdlib.h>

#include "mount-plugin.h"
#include <libxfce4ui/libxfce4ui.h>

void format_LVM_name (const char *disk_device, gchar **formatted_diskname);

#define gtk_hbox_new(homogeneous, spacing) \
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing)

#define gtk_vbox_new(homogeneous, spacing) \
        gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing)


static void
on_activate_disk_display (GtkWidget *widget, t_disk * disk)
{
    t_mounter * mt;
    gboolean eject;

    TRACE ("enters on_activate_disk_display");

    mt = (t_mounter *) g_object_get_data (G_OBJECT(widget), "mounter");

    eject = mt->eject_drives && (disk->dc==CD_DVD);

    if (disk != NULL) {
        if (disk->mount_info != NULL) { /* disk is mounted */
            disk_umount (disk, mt->umount_command, mt->message_dialog, eject);
        }
        else { /* disk is not mounted */
            disk_mount (disk, mt->on_mount_cmd, mt->mount_command, eject);
        }
    }

    TRACE ("leaves on_activate_disk_display");
}


static void
mounter_set_size (XfcePanelPlugin *plugin, int size, t_mounter *mt)
{
   /* shrink the gtk button's image to new size -*/
   size /= xfce_panel_plugin_get_nrows (plugin);
   gtk_widget_set_size_request (GTK_WIDGET(mt->button), size, size);
}

/**
 * Format the long /dev/mapper/volume to something better:
 *  LVM x:y
 */
void
format_LVM_name (const char *disk_device, gchar **formatted_diskname)
{

    gint volume, logvol, i;

    i = strlen(disk_device) - 1;

    do
        i--;
    while (i>0 && g_ascii_isdigit(disk_device[i]) );
    logvol = atoi(disk_device+i+1);

    do
        i--;
    while (i>0 && g_ascii_isalpha (disk_device[i]));

    do
        i--;
    while (i>0 && g_ascii_isdigit(disk_device[i]));
    volume = atoi(disk_device+i+1);

    *formatted_diskname = g_strdup_printf("LVM  %d:%d", volume, logvol);
}

/**
 *  Set character sizes for all entries to maximum.
 */
static void
disk_display_set_sizes (GPtrArray *array)
{
    unsigned int i, max_width_label_disk=0, max_width_label_mount_point=0, max_width_label_mount_info=0, tmp;
    t_disk_display *disk_display;

    for (i=0; i<array->len; i++) {
        disk_display= g_ptr_array_index (array, i); /* get the disk */

        tmp = strlen(gtk_label_get_text(GTK_LABEL(disk_display->label_mount_info)));
        if (tmp>max_width_label_mount_info)
            max_width_label_mount_info = tmp;

        tmp = strlen(gtk_label_get_text(GTK_LABEL(disk_display->label_mount_point)));
        if (tmp>max_width_label_mount_point)
            max_width_label_mount_point = tmp;

        if (disk_display->label_disk != NULL)
        {
            tmp = strlen(gtk_label_get_text(GTK_LABEL(disk_display->label_disk)));
            if (tmp>max_width_label_disk)
                max_width_label_disk = tmp;
        }
    }

    for (i=0; i<array->len; i++) {
        disk_display = g_ptr_array_index (array, i); /* get the disk */

        gtk_label_set_width_chars(GTK_LABEL(disk_display->label_mount_info), max_width_label_mount_info);

        gtk_label_set_width_chars(GTK_LABEL(disk_display->label_mount_point), max_width_label_mount_point);

        if (disk_display->label_disk != NULL)
        {
            gtk_label_set_width_chars(GTK_LABEL(disk_display->label_disk), max_width_label_disk);
        }
    }

}

/**
 * Create a new t_disk_display from t_disk infos.
 */
static t_disk_display*
disk_display_new (t_disk *disk, t_mounter *mounter)
{
    t_disk_display * dd;
    char *formatted_diskname;

    TRACE ("enters disk_display_new");

    if (disk!=NULL)
    {
        dd = g_new0 (t_disk_display, 1) ;
        dd->menu_item = gtk_menu_item_new();
        g_signal_connect (G_OBJECT(dd->menu_item), "activate",
                          G_CALLBACK(on_activate_disk_display), disk);
        g_object_set_data (G_OBJECT(dd->menu_item), "mounter",
                           (gpointer)mounter);

        dd->hbox = gtk_hbox_new (FALSE, 10);
        gtk_container_add (GTK_CONTAINER(dd->menu_item), dd->hbox);

        if (mounter->trim_devicenames)
        {
            if (g_str_has_prefix(disk->device, "/dev/mapper/"))
                format_LVM_name (disk->device_short, &formatted_diskname);
            else
                formatted_diskname = g_strdup(disk->device_short);
        }
        else
        {
            if (g_str_has_prefix(disk->device, "/dev/mapper/"))
                format_LVM_name (disk->device, &formatted_diskname);
            else
                formatted_diskname = g_strdup(disk->device);
        }

        if (mounter->exclude_devicenames)
        {
            dd->label_disk = NULL;
            dd->label_mount_arrow = NULL;
        }
        else
        {
            dd->label_disk = gtk_label_new (formatted_diskname);
            dd->label_mount_arrow = gtk_label_new (_(" -> "));

            /*change to uniform label size*/
            /*gtk_label_set_width_chars(GTK_LABEL(dd->label_disk), 32); */
            gtk_label_set_xalign(GTK_LABEL(dd->label_disk), 1.0);
            gtk_widget_set_valign(dd->label_disk,GTK_ALIGN_CENTER);
            gtk_widget_set_valign(dd->label_mount_arrow,GTK_ALIGN_CENTER);

            gtk_box_pack_start(GTK_BOX(dd->hbox),dd->label_disk,FALSE,TRUE,0);
            gtk_box_pack_start(GTK_BOX(dd->hbox),dd->label_mount_arrow,FALSE,TRUE,0);
        }
        g_free (formatted_diskname);

        dd->label_mount_point = gtk_label_new (disk->mount_point);
        gtk_label_set_xalign(GTK_LABEL(dd->label_mount_point), 0.0);
        gtk_widget_set_valign(dd->label_mount_point,GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(dd->hbox),dd->label_mount_point,FALSE,TRUE,0);

        dd->label_mount_info = gtk_label_new("");
        /*change to uniform label size*/
        /* gtk_label_set_width_chars(GTK_LABEL(dd->label_mount_info),25); */
        gtk_label_set_use_markup(GTK_LABEL(dd->label_mount_info),TRUE);
        gtk_label_set_xalign (GTK_LABEL(dd->label_mount_info),
                               1.0);
        gtk_widget_set_valign((dd->label_mount_info),GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(dd->hbox),dd->label_mount_info,FALSE,TRUE,0);

        dd->progress_bar = gtk_progress_bar_new();
        gtk_box_pack_start(GTK_BOX(dd->hbox),dd->progress_bar,TRUE,TRUE,0);

        return dd ;
    }

    TRACE ("leaves disk_display_new");

    return NULL ;
}


static void
disk_display_refresh (t_disk_display * disk_display)
{
    t_mount_info * mount_info;
    char * text;
    char * used;
    char * size;
    char * avail;

    mount_info = disk_display->disk->mount_info;

    TRACE("enters disk_display_refresh");

    if (disk_display != NULL)
    {
        if (mount_info != NULL)
        {    /* device is mounted */
            used = get_size_human_readable (mount_info->used);
            size = get_size_human_readable (mount_info->size);
            avail = get_size_human_readable (mount_info->avail);
            text = g_strdup_printf (_("[%s/%s] %s free"), used, size, avail);

            g_free(used);
            g_free(size);
            g_free(avail);
            gtk_label_set_text(GTK_LABEL(disk_display->label_mount_info), text);

            gtk_progress_bar_set_fraction (
                     GTK_PROGRESS_BAR(disk_display->progress_bar),
                     ((gdouble)mount_info->percent / 100) );
            gtk_progress_bar_set_show_text (
                     GTK_PROGRESS_BAR(disk_display->progress_bar),
                     TRUE);

            gtk_progress_bar_set_text (
                     GTK_PROGRESS_BAR(disk_display->progress_bar),
                     g_strdup_printf ("%d%%",mount_info->percent));
            gtk_widget_show (GTK_WIDGET(disk_display->progress_bar));
        }
        else /* mount_info == NULL */
        {
            /*remove mount info */
            gtk_label_set_markup (GTK_LABEL(disk_display->label_mount_info),
                         _("<span foreground=\"#FF0000\">not mounted</span>"));
            gtk_widget_hide (GTK_WIDGET(disk_display->progress_bar));

        }
    }
    TRACE("leaves disk_display_refresh");
}


static void
mounter_data_free (t_mounter * mt)
{
    TRACE ("enters mounter_data_free");

    if (mt != NULL)
    {
        disks_free (&(mt->pdisks));
        gtk_widget_destroy (GTK_WIDGET(mt->menu));
        mt->menu = NULL;
    }

    TRACE ("leaves mounter_data_free");
}


static void
mounter_free (XfcePanelPlugin *plugin, t_mounter *mounter)
{
    TRACE ("enters mounter_free");

    mounter_data_free (mounter);

    g_free (mounter);

    TRACE ("leaves mounter_free");
}


static void
mounter_data_new (t_mounter *mt)
{
    unsigned int i;
    t_disk * disk;
    t_disk_display * disk_display;
    GtkWidget * title_menu_item, *title_label;
    GPtrArray *array =  NULL, *disk_displays = NULL;
    char *dev_mp; /* device or mountpoint */

    TRACE ("enters mounter_data_new");

    /* get static infos from /etc/fstab */
    mt->pdisks = disks_new (mt->include_NFSs, &(mt->showed_fstab_dialog), mt->trim_devicename_count);

    /* remove unwanted file systems from list */
    if (mt->exclude_FSs) {
        array = g_ptr_array_new();
        DBG("excluded_filesystems=%s", mt->excluded_filesystems);
        seperate_list(array, mt->excluded_filesystems);
        for (i=0; i<array->len; i++) {
            dev_mp = (char*) g_ptr_array_index(array, i);
            if ( strstr(dev_mp , "/dev") )
                disks_remove_device(mt->pdisks, dev_mp);
            else
                disks_remove_mountpoint (mt->pdisks, dev_mp);
        }
    }

    /* get dynamic infos on mounts */
    disks_refresh (mt->pdisks, array /* =GPtrArray *excluded_FSs */ , mt->trim_devicename_count);

    /* menu with menu_item */
    mt->menu = gtk_menu_new ();

    title_menu_item = gtk_menu_item_new();
    title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), _("<b><i><span font_size=\"large\">Xfce 4 Mount Plugin – Devices and Mount Points</span></i></b>"));
    gtk_container_add (GTK_CONTAINER(title_menu_item), title_label);
    gtk_menu_shell_append (GTK_MENU_SHELL(mt->menu),
                           title_menu_item);

    disk_displays =g_ptr_array_new();
    for (i=0; i < mt->pdisks->len; i++)
    {
        disk = g_ptr_array_index (mt->pdisks, i); /* get the disk */
        disk_display = disk_display_new (disk, mt); /* creates a disk_display */
        disk_display->disk = disk;
        g_ptr_array_add(disk_displays, disk_display);
        /* fill in mount infos */
        disk_display_refresh (disk_display);

        /* add the menu_item to the menu */
        gtk_menu_shell_append (GTK_MENU_SHELL(mt->menu),
                               disk_display->menu_item);
    }

    gtk_widget_show_all(mt->menu);

    disk_display_set_sizes(disk_displays);

    TRACE ("leaves mounter_data_new");

    return ;
}


static void
mounter_refresh (t_mounter * mt)
{
    TRACE ("enters mounter_refresh");

    mounter_data_free (mt);
    mounter_data_new (mt);

    TRACE ("leaves mounter_refresh");
}


static gboolean
on_button_press (GtkWidget *widget, GdkEventButton *eventButton, t_mounter *mounter)
{
    gboolean result = FALSE;
#if GTK_CHECK_VERSION (3,22,0)
    GdkEvent event;
#endif
    TRACE ("enters on_button_press");
    if (eventButton != NULL)
    {
        if (mounter != NULL && eventButton->button == 1) /* left click only */
        {
            mounter_refresh (mounter); /* refreshs infos regarding mounts data */
#if GTK_CHECK_VERSION (3,22,0)
            event.button = *eventButton;
            gtk_menu_popup_at_widget (GTK_MENU(mounter->menu), mounter->button, GDK_GRAVITY_CENTER, GDK_GRAVITY_CENTER, &event);
#else
            gtk_menu_popup (GTK_MENU(mounter->menu), NULL, NULL,
                            xfce_panel_plugin_position_menu, mounter->plugin,
                            0, eventButton->time);
#endif
            result = TRUE;
        }
    }
    TRACE ("leaves on_button_press");

    return result ;
}


static void
mounter_read_config (XfcePanelPlugin *plugin, t_mounter *mt)
{
    gchar *icon;
    char *file;
    XfceRc *rc;
    TRACE ("enter read_config");

    if ( !( file = xfce_panel_plugin_lookup_rc_file (plugin) ) )
        return;
    DBG("going to read config from %s", file);
    rc = xfce_rc_simple_open (file, TRUE);
    g_free (file);

    if (mt->icon != NULL) g_free(mt->icon);
    if (mt->on_mount_cmd != NULL) g_free(mt->on_mount_cmd);
    if (mt->mount_command != NULL) g_free(mt->mount_command);
    if (mt->umount_command != NULL) g_free(mt->umount_command);
    if (mt->excluded_filesystems != NULL) g_free(mt->excluded_filesystems);

    icon = g_strdup_printf ("%s/icons/hicolor/scalable/apps/xfce-mount.svg", PACKAGE_DATA_DIR );
    mt->icon = g_strdup(xfce_rc_read_entry(rc, "icon", icon));
    g_free(icon);

    mt->on_mount_cmd = g_strdup(xfce_rc_read_entry(rc, "on_mount_cmd", ""));
    mt->mount_command = g_strdup(xfce_rc_read_entry(rc, "mount_command", DEFAULT_MOUNT_COMMAND));
    mt->umount_command = g_strdup(xfce_rc_read_entry(rc, "umount_command", DEFAULT_UMOUNT_COMMAND));
    mt->excluded_filesystems = g_strdup(xfce_rc_read_entry(rc, "excluded_filesystems", ""));

    /* before 0.6.0 those booleans were stored as string "1".. handle/merge legacy configs */
    if (xfce_rc_has_entry(rc, "message_dialog"))
        mt->message_dialog = atoi(xfce_rc_read_entry(rc, "message_dialog", NULL));
    else
        mt->message_dialog = xfce_rc_read_bool_entry(rc, "show_message_dialog", FALSE);

    if (xfce_rc_has_entry(rc, "include_NFSs"))
        mt->include_NFSs = atoi(xfce_rc_read_entry(rc, "include_NFSs", NULL));
    else
        mt->include_NFSs = xfce_rc_read_bool_entry(rc, "include_networked_filesystems", FALSE);

    if (xfce_rc_has_entry(rc, "trim_devicenames"))
        mt->trim_devicenames = xfce_rc_read_bool_entry(rc, "trim_devicenames", FALSE);

    if (xfce_rc_has_entry(rc, "td_count"))
        mt->trim_devicename_count = atoi(xfce_rc_read_entry(rc, "td_count", NULL));


    if (xfce_rc_has_entry(rc, "exclude_FSs"))
        mt->exclude_FSs = atoi(xfce_rc_read_entry(rc, "exclude_FSs", NULL));
    else
        mt->exclude_FSs = xfce_rc_read_bool_entry(rc, "exclude_selected_filesystems", FALSE);

    if (xfce_rc_has_entry(rc, "exclude_devicenames"))
        mt->exclude_devicenames = atoi(xfce_rc_read_entry(rc, "exclude_devicenames", NULL));
    else
        mt->exclude_devicenames = xfce_rc_read_bool_entry(rc, "exclude_devicenames_in_menu", FALSE);

    if (xfce_rc_has_entry(rc, "eject_drives"))
        mt->eject_drives = atoi(xfce_rc_read_entry(rc, "eject_drives", NULL));
    else
        mt->eject_drives = xfce_rc_read_bool_entry(rc, "eject_cddrives", FALSE);

    xfce_rc_close (rc);

    TRACE ("leaves read_config");
}


static void
mounter_write_config (XfcePanelPlugin *plugin, t_mounter *mt)
{
    XfceRc *rc;
    char *file, tmp[4];
    TRACE ("enter write_config");

    if (!(file = xfce_panel_plugin_save_location (plugin, TRUE)))
        return;

    /* int res = */ unlink (file);

    DBG("going to write config to %s", file);
    rc = xfce_rc_simple_open (file, FALSE);
    g_free (file);

    if (!rc)
        return;

    xfce_rc_write_entry (rc, "on_mount_cmd", mt->on_mount_cmd);
    xfce_rc_write_entry (rc, "mount_command", mt->mount_command);
    xfce_rc_write_entry (rc, "umount_command", mt->umount_command);
    xfce_rc_write_entry (rc, "excluded_filesystems", mt->excluded_filesystems);
    xfce_rc_write_entry (rc, "icon", mt->icon);
    xfce_rc_write_bool_entry (rc, "show_message_dialog", mt->message_dialog);
    xfce_rc_write_bool_entry (rc, "include_networked_filesystems", mt->include_NFSs);
    xfce_rc_write_bool_entry (rc, "trim_devicenames", mt->trim_devicenames);
    snprintf(tmp, 4, "%d", mt->trim_devicename_count);
    xfce_rc_write_entry (rc, "td_count", tmp);
    xfce_rc_write_bool_entry (rc, "exclude_selected_filesystems", mt->exclude_FSs);
    xfce_rc_write_bool_entry (rc, "exclude_devicenames_in_menu", mt->exclude_devicenames);
    xfce_rc_write_bool_entry (rc, "eject_cddrives", mt->eject_drives);

    xfce_rc_close (rc);

    TRACE ("leaves write config");
}


static t_mounter *
create_mounter_control (XfcePanelPlugin *plugin)
{
    t_mounter *mounter;
    TRACE ("enters create_mounter_control");

    mounter = g_new0(t_mounter,1);

    /* default configuration values if no configuration is found */
    mounter->icon = g_strdup(PACKAGE_DATA_DIR"/icons/hicolor/scalable/apps/xfce-mount.svg");
    mounter->mount_command = g_strdup(DEFAULT_MOUNT_COMMAND);
    mounter->umount_command = g_strdup(DEFAULT_UMOUNT_COMMAND);
    mounter->on_mount_cmd = g_strdup("");
    mounter->excluded_filesystems = g_strdup("");
    mounter->message_dialog = FALSE;
    mounter->include_NFSs = FALSE;
    mounter->trim_devicenames = TRUE;
    mounter->trim_devicename_count = 14;
    mounter->exclude_FSs = FALSE;
    mounter->eject_drives = FALSE;
    mounter->exclude_devicenames = FALSE;

    mounter->plugin = plugin;

    /*plugin button */

    /*get the data*/
    mounter_read_config(plugin, mounter);
    mounter_data_new (mounter);

    g_assert (mounter->icon!=NULL);

    mounter->button = gtk_button_new ();
    mounter->image = xfce_panel_image_new_from_source (mounter->icon);
    gtk_widget_show(mounter->image);
    gtk_container_add (GTK_CONTAINER(mounter->button), mounter->image);
    gtk_button_set_relief (GTK_BUTTON(mounter->button), GTK_RELIEF_NONE);

    gtk_widget_set_tooltip_text( GTK_WIDGET(mounter->button), _("devices"));

    g_signal_connect (G_OBJECT(mounter->button), "button_press_event",
                      G_CALLBACK(on_button_press), mounter);
    gtk_widget_show(mounter->button);

    TRACE ("leaves create_mounter_control");
    return mounter;
}


/* static void
free_mounter_dialog(GtkWidget * widget, t_mounter_dialog * md)
{
    g_free(md);
} */


static void
mounter_apply_options (t_mounter_dialog *md)
{
    gboolean incl_NFSs, excl_FSs;
    t_mounter * mt = md->mt;
    TRACE ("enters mounter_apply_options");


    incl_NFSs = mt->include_NFSs;
    excl_FSs = mt->exclude_FSs;

    mt->on_mount_cmd = g_strdup ( gtk_entry_get_text
                          (GTK_ENTRY(md->string_cmd)) );

    if ( gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON(md->specify_commands)) ) {
        mt->mount_command = g_strdup ( gtk_entry_get_text
                        (GTK_ENTRY(md->string_mount_command)) );
        mt->umount_command = g_strdup ( gtk_entry_get_text
                        (GTK_ENTRY(md->string_umount_command)) );
    }
    else {
        mt->mount_command = g_strdup ( DEFAULT_MOUNT_COMMAND );
        mt->umount_command = g_strdup ( DEFAULT_UMOUNT_COMMAND );
    }

    mt->excluded_filesystems = g_strdup ( gtk_entry_get_text
                        (GTK_ENTRY(md->string_excluded_filesystems)) );

    mt->message_dialog = gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON(md->show_message_dialog));

    mt->include_NFSs = gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON(md->show_include_NFSs));

    mt->eject_drives = gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON(md->show_eject_drives));

    mt->exclude_FSs = gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON(md->show_exclude_FSs));

    mt->exclude_devicenames = gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON(md->show_exclude_devicenames));

    mt->trim_devicenames = gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON(md->show_trim_devicenames));

    mt->trim_devicename_count = gtk_spin_button_get_value_as_int
            (GTK_SPIN_BUTTON(md->spin_trim_devicename_count));

    if (mt->include_NFSs!=incl_NFSs || mt->exclude_FSs!=excl_FSs
        || strlen(mt->excluded_filesystems)!=0) {
            /* re-read disk information */
            mounter_refresh (mt);
    }

    if ( gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(md->string_icon))
            !=NULL )
        mt->icon = g_strdup( gtk_file_chooser_get_filename (
                                   GTK_FILE_CHOOSER(md->string_icon)) );
    else
        mt->icon = g_strdup_printf (
                   "%s/icons/hicolor/scalable/apps/xfce-mount.svg",
                   PACKAGE_DATA_DIR );

    gtk_container_remove(GTK_CONTAINER(mt->button), mt->image);
    mt->image = xfce_panel_image_new_from_source (mt->icon);
    gtk_widget_show(mt->image);
    gtk_container_add (GTK_CONTAINER(mt->button), mt->image);

    TRACE ("leaves mounter_apply_options");
}


static void
on_optionsDialog_response (GtkWidget *dlg, int response, t_mounter_dialog * md)
{
    TRACE ("enters on_optionsDialog_response");

    mounter_apply_options (md);

    gtk_widget_destroy (md->dialog);

    xfce_panel_plugin_unblock_menu (md->mt->plugin);

    mounter_write_config (md->mt->plugin, md->mt);

    TRACE ("leaves on_optionsDialog_response");
}


/**
 * This shows a way to update plugin settings when the user leaves a text
 * entry, by connecting to the "focus-out" event on the entry.
 */
/* static gboolean
entry_lost_focus(t_mounter_dialog * md)
{
    mounter_apply_options (md);

    // also write config?

    // NB: needed to let entry handle focus-out as well
    return FALSE;
} */


static gboolean
specify_command_toggled (GtkWidget *widget, t_mounter_dialog *md)
{
    gboolean is_active = gtk_toggle_button_get_active
                            (GTK_TOGGLE_BUTTON (widget));
    gtk_widget_set_sensitive ( md->box_mount_commands, is_active );

    return TRUE;
}


static gboolean
exlude_FSs_toggled (GtkWidget *widget, t_mounter_dialog *md)
{
    gboolean is_active = gtk_toggle_button_get_active
                            (GTK_TOGGLE_BUTTON (widget));
    gtk_widget_set_sensitive ( md->string_excluded_filesystems, is_active );

    /* md->mt->exclude_FSs = is_active;
    mounter_refresh(md->mt); */

    return TRUE;
}


static gboolean
exclude_devicenames_toggled (GtkWidget *widget, t_mounter_dialog *md)
{
    gboolean val;

    val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gtk_widget_set_sensitive(gtk_widget_get_parent(GTK_WIDGET(md->show_trim_devicenames)), !val);

    return TRUE;
}

static gboolean
trim_devicenames_toggled (GtkWidget *widget, t_mounter_dialog *md)
{
    gboolean val;

    val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gtk_widget_set_sensitive(GTK_WIDGET(md->spin_trim_devicename_count), val);

    return TRUE;
}

static void
mounter_create_options (XfcePanelPlugin *plugin, t_mounter *mt)
{

    GtkWidget *dlg;
    GtkWidget *vbox;
    GtkWidget *_eventbox;
    GtkWidget *_label;
    GtkWidget *_vbox, *_vbox2;
    GtkWidget *_hbox;
    GtkWidget *_notebook;
    t_mounter_dialog * md;
    gboolean set_active;

    TRACE ("enters mounter_create_options");

    xfce_panel_plugin_block_menu (plugin);

    dlg = xfce_titled_dialog_new_with_buttons(
                _("Mount Plugin"),
                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                "gtk-close", GTK_RESPONSE_OK, NULL);

    xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (dlg), _("Properties"));
    gtk_window_set_icon_name(GTK_WINDOW(dlg),"drive-harddisk");

    gtk_container_set_border_width (GTK_CONTAINER (dlg), 2);


    md = g_new0 (t_mounter_dialog, 1);

    md->mt = mt;

    md->dialog = dlg;

    vbox = gtk_dialog_get_content_area(GTK_DIALOG(dlg));

    _notebook = gtk_notebook_new ();
    gtk_widget_show (_notebook);
    gtk_container_set_border_width (GTK_CONTAINER(_notebook), BORDER);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(_notebook),
                        TRUE, TRUE, BORDER);

    /* --------------- General tab page ----------------------*/
    _vbox = gtk_vbox_new (FALSE, BORDER);
    gtk_container_set_border_width (GTK_CONTAINER(_vbox), BORDER);
    gtk_widget_show (_vbox);

       /* Show "unmounted" message */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_widget_set_tooltip_text(_eventbox,
        _("This is only useful and recommended if you specify \"sync\" as part "
        "of the \"unmount\" command string."));

    md->show_message_dialog = gtk_check_button_new_with_mnemonic (
                                _("Show _message after unmount") );
    gtk_widget_show (md->show_message_dialog);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(md->show_message_dialog),
                                mt->message_dialog);
    gtk_container_add (GTK_CONTAINER (_eventbox), md->show_message_dialog );

        /* Symbol chooser */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_widget_set_tooltip_text(_eventbox,
        _("You can specify a distinct icon to be displayed in the panel."));

    _hbox = gtk_hbox_new (FALSE, BORDER);
    gtk_widget_show (_hbox);
    gtk_container_add (GTK_CONTAINER(_eventbox), _hbox );

    _label = gtk_label_new_with_mnemonic (_("Icon:"));
    gtk_widget_show (_label);
    gtk_box_pack_start (GTK_BOX(_hbox), _label, FALSE, FALSE, 0);

    md->string_icon = gtk_file_chooser_button_new (_("Select an image"),
                                   GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(md->string_icon),
                                   mt->icon);
    gtk_widget_show (md->string_icon);
    gtk_box_pack_start (GTK_BOX(_hbox), md->string_icon, TRUE, TRUE, 0);

    _label = gtk_label_new_with_mnemonic (_("_General"));
    gtk_widget_show (_label);
    gtk_notebook_append_page (GTK_NOTEBOOK(_notebook), _vbox, _label);

    /* --------------- Commands tab page ----------------------*/
    _vbox = gtk_vbox_new (FALSE, BORDER);
    gtk_container_set_border_width (GTK_CONTAINER(_vbox), BORDER);
    gtk_widget_show (_vbox);

        /* After-mount command */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_widget_set_tooltip_text(_eventbox,
        _("This command will be executed after mounting the device with the "
        "mount point of the device as argument.\n"
        "If you are unsure what to insert, try \"exo-open %m\".\n"
        "'%d' can be used to specify the device, '%m' for the mountpoint."));

    _hbox = gtk_hbox_new (FALSE, BORDER);
    gtk_widget_show (_hbox);
    gtk_container_add (GTK_CONTAINER (_eventbox), _hbox);

    _label = gtk_label_new_with_mnemonic (_("_Execute after mounting:"));
    gtk_widget_show (_label);
    gtk_box_pack_start (GTK_BOX (_hbox), _label, FALSE, FALSE, 0);

    md->string_cmd = gtk_entry_new ();
    if (mt->on_mount_cmd != NULL)
    gtk_entry_set_text (GTK_ENTRY(md->string_cmd),
                        g_strdup(mt->on_mount_cmd));
    gtk_entry_set_width_chars (GTK_ENTRY(md->string_cmd), 15);
    gtk_widget_show (md->string_cmd);
    gtk_box_pack_start (GTK_BOX(_hbox), md->string_cmd, TRUE, TRUE, 0);

        /* Specify custom commands */
    _vbox2 = gtk_vbox_new (FALSE, BORDER);
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET (_vbox2), FALSE, FALSE,
                    0);
    gtk_widget_show (_vbox2);

    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox2), GTK_WIDGET (_eventbox), FALSE,
                    FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_widget_set_tooltip_text(_eventbox,
        _("WARNING: These options are for experts only! If you do not know "
        "what they may be good for, keep your hands off!"));

    md->specify_commands = gtk_check_button_new_with_mnemonic (
                                _("_Custom commands") );
    set_active =
        ( strcmp(mt->mount_command, DEFAULT_MOUNT_COMMAND)!=0 ||
        strcmp(mt->umount_command, DEFAULT_UMOUNT_COMMAND)!=0 ) ?
                TRUE : FALSE;

    gtk_widget_show (md->specify_commands);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(md->specify_commands),
                                set_active);
    g_signal_connect ( G_OBJECT(md->specify_commands), "toggled",
        G_CALLBACK(specify_command_toggled), md);
    gtk_container_add (GTK_CONTAINER (_eventbox), md->specify_commands );

        /* Custom commands */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox2), GTK_WIDGET (_eventbox), FALSE,
                    FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_widget_set_tooltip_text(_eventbox,
        _("Most users will only want to prepend \"sudo\" to both "
        "commands or prepend \"sync %d &&\" to the \"unmount %d\" command.\n"
        "'%d' is used to specify the device, '%m' for the mountpoint."));

    md->box_mount_commands = gtk_grid_new ();
    gtk_container_add (GTK_CONTAINER (_eventbox), md->box_mount_commands);
    gtk_widget_show (md->box_mount_commands);

    _label = gtk_label_new_with_mnemonic (_("_Mount command:"));
    gtk_widget_set_valign(_label,GTK_ALIGN_CENTER);
    gtk_widget_show (_label);
    gtk_grid_attach (GTK_GRID(md->box_mount_commands), _label, 0, 0, 1, 1);

    _label = gtk_label_new_with_mnemonic (_("_Unmount command:"));
    gtk_widget_set_valign(_label,GTK_ALIGN_CENTER);
    gtk_widget_show (_label);
    gtk_grid_attach (GTK_GRID(md->box_mount_commands), _label, 0, 1, 1, 1);

    md->string_mount_command = gtk_entry_new ();
    DBG("mt->mount_command: %s", mt->mount_command);
    gtk_entry_set_text (GTK_ENTRY(md->string_mount_command ),
                    g_strdup(mt->mount_command ));
    gtk_widget_show (md->string_mount_command );
    gtk_grid_attach (GTK_GRID(md->box_mount_commands),
                    md->string_mount_command , 1, 0,
                    1, 1);

    md->string_umount_command = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY(md->string_umount_command ),
                    g_strdup(mt->umount_command ));
    gtk_widget_show (md->string_umount_command );
    gtk_grid_attach (GTK_GRID(md->box_mount_commands),
                    md->string_umount_command , 1, 1,
                    2, 1);

    if (!set_active) /* following command wasn't executed by signal handler! */
        gtk_widget_set_sensitive ( md->box_mount_commands, FALSE );

    _label = gtk_label_new_with_mnemonic (_("_Commands"));
    gtk_widget_show (_label);
    gtk_notebook_append_page (GTK_NOTEBOOK(_notebook), _vbox, _label);

    /* File systems tab page */
    _vbox = gtk_vbox_new (FALSE, BORDER);
    gtk_container_set_border_width (GTK_CONTAINER(_vbox), BORDER);
    gtk_widget_show (_vbox);

        /* show include_NFSs */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_widget_set_tooltip_text(_eventbox,
        _("Activate this option to also display network file systems like "
        "NFS, SMBFS, SHFS and SSHFS."));

    md->show_include_NFSs = gtk_check_button_new_with_mnemonic (
                                _("Display _network file systems") );

    gtk_widget_show (md->show_include_NFSs);
    gtk_container_add (GTK_CONTAINER (_eventbox), md->show_include_NFSs );
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(md->show_include_NFSs),
                                mt->include_NFSs);

        /* eject CD-drives */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_widget_set_tooltip_text(_eventbox,
        _("Activate this option to also eject a CD-drive after unmounting"
        " and to insert before mounting."));

    md->show_eject_drives = gtk_check_button_new_with_mnemonic (
                                _("_Eject CD-drives") );

    gtk_widget_show (md->show_eject_drives);
    gtk_container_add (GTK_CONTAINER (_eventbox), md->show_eject_drives );
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(md->show_eject_drives),
                                mt->eject_drives);

    /* exclude device names */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_widget_set_tooltip_text(_eventbox,
        _("Activate this option to only have the mount points be displayed."));

    md->show_exclude_devicenames = gtk_check_button_new_with_mnemonic (
                                _("Display _mount points only") );

    gtk_widget_show (md->show_exclude_devicenames);
    gtk_container_add (GTK_CONTAINER (_eventbox), md->show_exclude_devicenames );
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(md->show_exclude_devicenames),
                                mt->exclude_devicenames);
    g_signal_connect ( G_OBJECT(md->show_exclude_devicenames), "toggled",
        G_CALLBACK(exclude_devicenames_toggled), md);

    /* Trim device names */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_widget_set_tooltip_text(_eventbox,
        _("Trim the device names to the number of characters specified in the spin button."));
    _hbox = gtk_hbox_new (FALSE, BORDER);
    gtk_widget_show (_hbox);
    gtk_container_add (GTK_CONTAINER (_eventbox), _hbox );
    gtk_widget_set_sensitive(GTK_WIDGET(_hbox), !mt->exclude_devicenames);
    md->show_trim_devicenames = gtk_check_button_new_with_mnemonic (
                                _("Trim device names: ") );
    gtk_widget_show (md->show_trim_devicenames);
    gtk_box_pack_start (GTK_BOX (_hbox), GTK_WIDGET(md->show_trim_devicenames),
                        FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(md->show_trim_devicenames),
                                mt->trim_devicenames);
    g_signal_connect ( G_OBJECT(md->show_trim_devicenames), "toggled",
        G_CALLBACK(trim_devicenames_toggled), md);

    _label = gtk_label_new(_(" characters"));
    gtk_widget_show (_label);
    gtk_box_pack_end (GTK_BOX (_hbox), GTK_WIDGET(_label),
                        FALSE, FALSE, 0);
    md->spin_trim_devicename_count = gtk_spin_button_new_with_range (9.0, 99.0, 1.0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(md->spin_trim_devicename_count), (double) mt->trim_devicename_count);
    gtk_widget_show (md->spin_trim_devicename_count);
    gtk_box_pack_end (GTK_BOX (_hbox), GTK_WIDGET(md->spin_trim_devicename_count),
                        FALSE, FALSE, 0);


        /* Exclude file systems  */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_widget_set_tooltip_text(_eventbox,
        _("Exclude the following file systems from the menu.\n"
        "The list is separated by simple spaces.\n"
        "It is up to you to specify correct devices or mount points.\n"
        "An asterisk (*) can be used as a placeholder at the end of\n"
        "a path, e.g., \"/mnt/*\" to exclude any mountpoints below \"/mnt\".\n"));

    _vbox2 = gtk_vbox_new (FALSE, BORDER);
    gtk_widget_show (_vbox2);
    gtk_container_add (GTK_CONTAINER (_eventbox), _vbox2 );

    md->show_exclude_FSs = gtk_check_button_new_with_mnemonic (
                                _("E_xclude specified file systems") );
    gtk_widget_show (md->show_exclude_FSs);
    gtk_box_pack_start (GTK_BOX(_vbox2), md->show_exclude_FSs, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(md->show_exclude_FSs),
                                mt->exclude_FSs);
    g_signal_connect ( G_OBJECT(md->show_exclude_FSs), "toggled",
        G_CALLBACK(exlude_FSs_toggled), md);

    md->string_excluded_filesystems = gtk_entry_new ();
    if (!mt->exclude_FSs)
        gtk_widget_set_sensitive (md->string_excluded_filesystems, FALSE);
    gtk_entry_set_text (GTK_ENTRY(md->string_excluded_filesystems), mt->excluded_filesystems);
    gtk_widget_show(md->string_excluded_filesystems);
    gtk_box_pack_start (GTK_BOX(_vbox2), md->string_excluded_filesystems, TRUE, TRUE, 0);

    _label = gtk_label_new_with_mnemonic (_("_File systems"));
    gtk_widget_show(_label);
    gtk_notebook_append_page (GTK_NOTEBOOK(_notebook), _vbox, _label);

    g_signal_connect (dlg, "response",
                      G_CALLBACK(on_optionsDialog_response), md);

    gtk_widget_show (dlg);

    TRACE ("leaves mounter_create_options");
}

static void
mounter_show_about(XfcePanelPlugin *plugin, t_mounter *mt)
{
   GdkPixbuf *icon;
   const gchar *auth[] = { "Jean-Baptiste Dulong",
                           "Fabian Nowak <timystery@arcor.de>",
                           "Landry Breuil <landry@xfce.org>", NULL };
   icon = xfce_panel_pixbuf_from_source("drive-harddisk", NULL, 32);
   gtk_show_about_dialog(NULL,
      "logo", icon,
      "license", xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
      "version", PACKAGE_VERSION,
      "program-name", PACKAGE_NAME,
      "comments", _("Show partitions/devices and allow to mount/unmount them"),
      "website", "https://docs.xfce.org/panel-plugins/xfce4-mount-plugin",
      "copyright", _("Copyright (c) 2005-2018\n"),
      "authors", auth, NULL);
  // TODO: add translators.

   if(icon)
      g_object_unref(G_OBJECT(icon));
}

static void
mount_construct (XfcePanelPlugin *plugin)
{
    t_mounter *mounter;

    xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    mounter = create_mounter_control (plugin);

    if (xfce_panel_plugin_get_mode(plugin) == XFCE_PANEL_PLUGIN_MODE_DESKBAR)
        xfce_panel_plugin_set_small(plugin, FALSE);
    else
        xfce_panel_plugin_set_small (plugin, TRUE);

    g_signal_connect (plugin, "free-data", G_CALLBACK (mounter_free), mounter);

/* superfluous, as potential race condition with too fast shutdown of the panel exists;
 * and unnecessary because stuff is saved when closing the settings dialog */
/*    g_signal_connect (plugin, "save", G_CALLBACK (mounter_write_config),
                      mounter);*/

    xfce_panel_plugin_menu_show_configure (plugin);
    g_signal_connect (plugin, "configure-plugin",
                      G_CALLBACK (mounter_create_options), mounter);

    xfce_panel_plugin_menu_show_about(plugin);
    g_signal_connect (plugin, "about", G_CALLBACK (mounter_show_about), mounter);

    g_signal_connect (plugin, "size-changed", G_CALLBACK (mounter_set_size),
                         mounter);

    gtk_container_add (GTK_CONTAINER(plugin), mounter->button);

    xfce_panel_plugin_add_action_widget (plugin, mounter->button);

}

XFCE_PANEL_PLUGIN_REGISTER (mount_construct);
