/* mount-plugin.c */

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
#include "mount-plugin.h"

static void
on_message_dialog_response (GtkWidget *widget, gpointer *data)
{
    gtk_widget_destroy (widget);
}


static void
on_activate_disk_display (GtkWidget *widget, t_disk * disk)
{
    t_mounter * mt;
    gboolean eject;
    gchar *msg;
    GtkWidget *my_dlg;

    TRACE ("enters on_activate_disk_display");

    mt = (t_mounter *) g_object_get_data (G_OBJECT(widget), "mounter");

    eject = mt->eject_drives && (disk->dc==CD_DVD);

    if (disk != NULL) {
        if (disk->mount_info != NULL) { /* disk is mounted */
            int result = disk_umount (disk, mt->umount_command,
                                      mt->message_dialog, eject);

            if (mt->message_dialog && (!eject || result!=NONE) ) { /* popup dialog */

                    msg = (gchar *) g_malloc (1024*sizeof(gchar));
                    if (result==NONE)
                        msg = _("The device \"%s\" should be removable safely "
                                    "now.");
                    else
                        msg = _("An error occurred. The device should not be "
                                    "removed!");

                my_dlg = gtk_message_dialog_new (NULL,
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_INFO,
                        GTK_BUTTONS_OK,
                        msg,
                        disk->mount_point);

                 g_signal_connect (my_dlg, "response",
                        G_CALLBACK (on_message_dialog_response), my_dlg);
                 gtk_widget_show (my_dlg);
                    /* gtk_dialog_run (GTK_DIALOG (my_dlg)); */
            }
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
   gtk_widget_set_size_request (GTK_WIDGET(mt->button), size - 4, size - 4);

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
    int i, max_width_label_disk=0, max_width_label_mount_info=0, tmp;
    t_disk_display *disk_display;

    for (i=0; i<array->len; i++) {
        disk_display= g_ptr_array_index (array, i); /* get the disk */
        tmp = strlen(gtk_label_get_text(GTK_LABEL(disk_display->label_mount_info)));
        if (tmp>max_width_label_mount_info)
            max_width_label_mount_info = tmp;

        tmp = strlen(gtk_label_get_text(GTK_LABEL(disk_display->label_disk)));
        if (tmp>max_width_label_disk)
            max_width_label_disk = tmp;
    }

    for (i=0; i<array->len; i++) {
        disk_display = g_ptr_array_index (array, i); /* get the disk */
        gtk_label_set_width_chars(GTK_LABEL(disk_display->label_disk), max_width_label_disk);
        gtk_label_set_width_chars(GTK_LABEL(disk_display->label_mount_info), max_width_label_mount_info);
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

        if (g_str_has_prefix(disk->device, "/dev/mapper/"))
            format_LVM_name (disk->device, &formatted_diskname);
        else
            formatted_diskname = g_strdup(disk->device);

        if (mounter->exclude_devicenames)
            dd->label_disk = gtk_label_new (disk->mount_point);
        else
            dd->label_disk = gtk_label_new (g_strconcat(formatted_diskname, " -> ",
                                        disk->mount_point, NULL));

        g_free (formatted_diskname);

        /*change to uniform label size*/
        /*gtk_label_set_width_chars(GTK_LABEL(dd->label_disk), 32); */
        /* gtk_label_set_justify(GTK_LABEL(dd->label_disk),GTK_JUSTIFY_LEFT); */
        gtk_misc_set_alignment(GTK_MISC(dd->label_disk),0.0, 0.5);
        gtk_box_pack_start(GTK_BOX(dd->hbox),dd->label_disk,FALSE,TRUE,0);

        dd->label_mount_info = gtk_label_new("");
        /*change to uniform label size*/
        /* gtk_label_set_width_chars(GTK_LABEL(dd->label_mount_info),25); */
        gtk_label_set_use_markup(GTK_LABEL(dd->label_mount_info),TRUE);
        /* gtk_label_set_justify (GTK_LABEL(dd->label_mount_info),
                               GTK_JUSTIFY_RIGHT); */
        gtk_misc_set_alignment(GTK_MISC(dd->label_mount_info),1.0, 0.5);
        gtk_box_pack_start(GTK_BOX(dd->hbox),dd->label_mount_info,TRUE,TRUE,0);

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
            text = g_strdup_printf ("[%s/%s] %s free", used, size, avail);

            g_free(used);
            g_free(size);
            g_free(avail);
            gtk_label_set_text(GTK_LABEL(disk_display->label_mount_info), text);

            gtk_progress_bar_set_fraction (
                     GTK_PROGRESS_BAR(disk_display->progress_bar),
                     ((gdouble)mount_info->percent / 100) );
            gtk_progress_bar_set_text (
                     GTK_PROGRESS_BAR(disk_display->progress_bar),
                     g_strdup_printf ("%d",mount_info->percent));
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

    disks_free (&(mt->pdisks));
    gtk_widget_destroy (GTK_WIDGET(mt->menu));
    mt->menu = NULL;

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
    TRACE ("enters mounter_data_new");

    int i, res;
    t_disk * disk;
    t_disk_display * disk_display;
    GPtrArray *array =  NULL, *disk_displays = NULL;
    char *dev_mp; /* device or mountpoint */
    gboolean removed_device;

    /*get static infos from /etc/fstab */
    mt->pdisks = disks_new (mt->include_NFSs);

    /* remove unwanted file systems from list */
    if (mt->exclude_FSs) {
        array = g_ptr_array_new();
        DBG("excluded_filesystems=%s\n", mt->excluded_filesystems);
        res = seperate_list(array, mt->excluded_filesystems);
        for (i=0; i<array->len; i++) {
            dev_mp = (char*) g_ptr_array_index(array, i);
            if ( strstr(dev_mp , "/dev") )
                removed_device = disks_remove_device(mt->pdisks, dev_mp);
            else
                removed_device = disks_remove_mountpoint (mt->pdisks, dev_mp);
        }
    }

    /* get dynamic infos on mounts */
    disks_refresh (mt->pdisks, array /* =GPtrArray *excluded_FSs */ );

    /* menu with menu_item */
    mt->menu = gtk_menu_new ();
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

    mt->icon = PACKAGE_DATA_DIR"/icons/hicolor/scalable/apps/xfce-mount.svg";
    mt->mount_command = DEFAULT_MOUNT_COMMAND;
    mt->umount_command = DEFAULT_UMOUNT_COMMAND;

    mt->excluded_filesystems = "";

    mt->message_dialog = FALSE;

    mt->include_NFSs = FALSE;

    mt->exclude_FSs = FALSE;

    mt->eject_drives = FALSE;

    mt->exclude_devicenames = FALSE;

    TRACE ("leaves mounter_data_new");

    return ;
}


static void
mounter_refresh (t_mounter * mt)
{
    TRACE ("enters mounter_refresh");

    mounter_data_free (mt);
    gchar *mount = g_strdup (mt->mount_command);
    gchar *umount = g_strdup (mt->umount_command);
    gchar *icon = g_strdup (mt->icon);
    gchar *excl_filesystems = g_strdup (mt->excluded_filesystems);
    DBG ("Changed icon value from '%s' to '%s'.\n", mt->icon, icon);
    gboolean msg_dlg = mt->message_dialog;
    gboolean incl_NFSs = mt->include_NFSs;
    gboolean excl_FSs = mt->exclude_FSs;
    gboolean excl_DNs = mt->exclude_devicenames;
    gboolean eject = mt->eject_drives;

    mounter_data_new (mt);
    mt->icon = g_strdup (icon);
    DBG ("Changed icon value from '%s' to '%s'.\n", icon, mt->icon);
    mt->mount_command = g_strdup (mount);
    mt->umount_command = g_strdup (umount);
    mt->excluded_filesystems = g_strdup (excl_filesystems);
    mt->message_dialog = msg_dlg;
    mt->include_NFSs = incl_NFSs;
    mt->exclude_FSs = excl_FSs;
    mt->exclude_devicenames = excl_DNs;
    mt->eject_drives = eject;

    TRACE ("leaves mounter_refresh");

}


static gboolean
on_button_press (GtkWidget *widget, GdkEventButton *event, t_mounter *mounter)
{
    TRACE ("enters on_button_press");
    if (mounter != NULL && event->button == 1) /* left click only */
    {

        mounter_refresh (mounter); /* refreshs infos regarding mounts data */
        gtk_menu_popup (GTK_MENU(mounter->menu), NULL, NULL, NULL, NULL, 0,
                        event->time);
        return TRUE;
    }
    TRACE ("leaves on_button_press");
    return FALSE ;
}


static void
mounter_read_config (XfcePanelPlugin *plugin, t_mounter *mt)
{
    TRACE ("enter read_config");

    const char *value;
    char *file;
    XfceRc *rc;

    if ( !( file = xfce_panel_plugin_lookup_rc_file (plugin) ) )
        return;

    rc = xfce_rc_simple_open (file, TRUE);
    g_free (file);

    if ( (value = xfce_rc_read_entry(rc, "on_mount_cmd", NULL)) )
      mt->on_mount_cmd = g_strdup (value);

    if ( (value = xfce_rc_read_entry(rc, "icon", NULL)) )
        mt->icon = g_strdup (value);
    else
        mt->icon = g_strdup_printf (
                "%s/icons/hicolor/scalable/apps/xfce-mount.svg",
                PACKAGE_DATA_DIR );

    if ( (value = xfce_rc_read_entry (rc, "mount_command", NULL)) )
        mt->mount_command = g_strdup (value);
    else
        mt->mount_command = g_strdup (DEFAULT_MOUNT_COMMAND);

    if ( (value = xfce_rc_read_entry (rc, "umount_command", NULL)) )
        mt->umount_command = g_strdup (value);
    else
        mt->umount_command = g_strdup (DEFAULT_UMOUNT_COMMAND);

    if ( (value = xfce_rc_read_entry (rc, "excluded_filesystems", NULL)) )
        mt->excluded_filesystems = g_strdup (value);
    else
        mt->excluded_filesystems = g_strdup ("");

    if ( (value = xfce_rc_read_entry(rc, "message_dialog", NULL)) )
        mt->message_dialog = atoi (value);

    if ( (value = xfce_rc_read_entry(rc, "include_NFSs", NULL)) )
        mt->include_NFSs= atoi (value);

    if ( (value = xfce_rc_read_entry(rc, "exclude_FSs", NULL)) )
        mt->exclude_FSs= atoi (value);

    if ( (value = xfce_rc_read_entry(rc, "exclude_devicenames", NULL)) )
        mt->exclude_devicenames= atoi (value);

    if ( (value = xfce_rc_read_entry(rc, "eject_drives", NULL)) )
        mt->eject_drives= atoi (value);

    xfce_rc_close (rc);

    TRACE ("leaves read_config");
}


static void
mounter_write_config (XfcePanelPlugin *plugin, t_mounter *mt)
{
    TRACE ("enter write_config");

     XfceRc *rc;
    char *file;

    if (!(file = xfce_panel_plugin_save_location (plugin, TRUE)))
        return;

    /* int res = */ unlink (file);

    rc = xfce_rc_simple_open (file, FALSE);
    g_free (file);

    if (!rc)
        return;

    xfce_rc_write_entry (rc, "on_mount_cmd",
         mt->on_mount_cmd ? mt->on_mount_cmd : "");

    if ( strcmp(mt->mount_command, DEFAULT_MOUNT_COMMAND)!=0 )
        xfce_rc_write_entry (rc, "mount_command", mt->mount_command);

    if ( strcmp(mt->umount_command, DEFAULT_UMOUNT_COMMAND)!=0 )
        xfce_rc_write_entry (rc, "umount_command", mt->umount_command);

    if ( strlen(mt->excluded_filesystems)!=0 )
        xfce_rc_write_entry (rc, "excluded_filesystems", mt->excluded_filesystems);

    if ( mt->message_dialog==1 )
        xfce_rc_write_entry (rc, "message_dialog", "1");

    if ( mt->include_NFSs==1 )
        xfce_rc_write_entry (rc, "include_NFSs", "1");

    if ( mt->exclude_FSs==1 )
        xfce_rc_write_entry (rc, "exclude_FSs", "1");

    if ( mt->exclude_devicenames==1 )
        xfce_rc_write_entry (rc, "exclude_devicenames", "1");

    if ( mt->eject_drives==1 )
        xfce_rc_write_entry (rc, "eject_drives", "1");

    xfce_rc_write_entry (rc, "icon", mt->icon);

    xfce_rc_close (rc);

    TRACE ("leaves write config");
}


static t_mounter *
create_mounter_control (XfcePanelPlugin *plugin)
{
    TRACE ("enters create_mounter_control");

    t_mounter *mounter;

    mounter = g_new0(t_mounter,1);

    /* default mount command */
    mounter->on_mount_cmd = NULL;

    mounter->icon = NULL;

    mounter->plugin = plugin;

    if (!tooltips)
    {
        tooltips = gtk_tooltips_new();
    }

    /*plugin button */

    /*get the data*/
    mounter_read_config(plugin, mounter);
    mounter_data_new (mounter);

    g_assert (mounter->icon!=NULL);

    mounter->button_pb = gdk_pixbuf_new_from_file (mounter->icon, NULL);
    mounter->button = xfce_iconbutton_new_from_pixbuf (mounter->button_pb);
    gtk_button_set_relief (GTK_BUTTON(mounter->button), GTK_RELIEF_NONE);

    gtk_tooltips_set_tip (tooltips, GTK_WIDGET(mounter->button), _("devices"),
                         NULL);

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
    TRACE ("enters mounter_apply_options");

    t_mounter * mt = md->mt;

    const char * tmp;
    tmp = gtk_entry_get_text (GTK_ENTRY(md->string_cmd));

    gboolean incl_NFSs = mt->include_NFSs;
    gboolean excl_FSs = mt->exclude_FSs;

    g_free (mt->on_mount_cmd);
    if (tmp && *tmp)
        mt->on_mount_cmd = g_strdup (tmp);
    else
        mt->on_mount_cmd = NULL;

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

       mt->button_pb = gdk_pixbuf_new_from_file (mt->icon, NULL);
       xfce_iconbutton_set_pixbuf (XFCE_ICONBUTTON(mt->button), mt->button_pb);

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


    return TRUE;
}

static void
mounter_create_options (XfcePanelPlugin *plugin, t_mounter *mt)
{

    TRACE ("enters mounter_create_options");

    xfce_panel_plugin_block_menu (plugin);

    GtkWidget *dlg, *header;
    //dlg = gtk_dialog_new_with_buttons (_("Edit Properties"),
    dlg = xfce_titled_dialog_new_with_buttons(
                _("Xfce 4 Mount Plugin"),
                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
                GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);

    gtk_window_set_icon_name(GTK_WINDOW(dlg),"xfce-mount");

    gtk_container_set_border_width (GTK_CONTAINER (dlg), 2);

    /* header = xfce_create_header (NULL, _("Mount devices"));
    gtk_widget_set_size_request (GTK_BIN (header)->child, -1, 32);
    gtk_container_set_border_width (GTK_CONTAINER (header), BORDER - 2);
    gtk_widget_show (header);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), header, FALSE,
                        TRUE, 0); */

    GtkWidget *vbox;
    t_mounter_dialog * md;

    md = g_new0 (t_mounter_dialog, 1);

    md->mt = mt;

    md->dialog = dlg;

    vbox = GTK_DIALOG (dlg)->vbox;

    /* "local" variables */
    GtkWidget *_eventbox;
    GtkWidget *_label;
    GtkWidget *_vbox, *_vbox2;
    GtkWidget *_hbox;
    GtkTooltips *tip;
    GtkWidget *_notebook;


    _notebook = gtk_notebook_new ();
    gtk_widget_show (_notebook);
    gtk_container_border_width (GTK_CONTAINER(_notebook), BORDER);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(_notebook),
                        TRUE, TRUE, BORDER);

    tip = gtk_tooltips_new ();
    gtk_tooltips_enable (tip);

    /* --------------- General tab page ----------------------*/
    _vbox = gtk_vbox_new (FALSE, BORDER);
    gtk_container_border_width (GTK_CONTAINER(_vbox), BORDER);
    gtk_widget_show (_vbox);

       /* Show "unmounted" message */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), _eventbox,
        _("This is only useful and recommended if you specify \"sync\" as part "
        "of the \"unmount\" command string."),
        NULL );

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
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), _eventbox,
        _("You can specify a distinct icon to be displayed in the panel."),
        NULL );

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
    gtk_container_border_width (GTK_CONTAINER(_vbox), BORDER);
    gtk_widget_show (_vbox);

        /* After-mount command */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), _eventbox,
        _("This command will be executed after mounting the device with the "
        "mount point of the device as argument.\n"
        "If you are unsure what to insert, try \"thunar %m\".\n"
        "'%d' can be used to specify the device, '%m' for the mountpoint."),
        NULL);

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
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), _eventbox,
        _("WARNING: These options are for experts only! If you do not know "
        "what they may be good for, keep your hands off!"),
        NULL );

    md->specify_commands = gtk_check_button_new_with_mnemonic (
                                _("_Custom commands") );
    gboolean set_active =
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
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), _eventbox,
        _("Most users will only want to prepend \"sudo\" to both "
        "commands or prepend \"sync %d &&\" to the \"unmount %d\" command.\n"
        "'%d' is used to specify the device, '%m' for the mountpoint."),
        NULL );

    md->box_mount_commands = gtk_table_new (2, 2, FALSE);
    gtk_container_add (GTK_CONTAINER (_eventbox), md->box_mount_commands);
    gtk_widget_show (md->box_mount_commands);

    _label = gtk_label_new_with_mnemonic (_("_Mount command:"));
    gtk_misc_set_alignment (GTK_MISC(_label), 0.0, 0.5);
    gtk_widget_show (_label);
    gtk_table_attach (GTK_TABLE(md->box_mount_commands), _label, 0, 1, 0, 1,
                    GTK_FILL, GTK_SHRINK, 0, 0);

    _label = gtk_label_new_with_mnemonic (_("_Unmount command:"));
    gtk_misc_set_alignment (GTK_MISC(_label), 0.0, 0.5);
    gtk_widget_show (_label);
    gtk_table_attach (GTK_TABLE(md->box_mount_commands), _label, 0, 1, 1, 2,
                    GTK_FILL, GTK_SHRINK, 0, 0);

    md->string_mount_command = gtk_entry_new ();
    DBG("mt->mount_command: %s\n", mt->mount_command);
    gtk_entry_set_text (GTK_ENTRY(md->string_mount_command ),
                    g_strdup(mt->mount_command ));
    gtk_widget_show (md->string_mount_command );
    gtk_table_attach (GTK_TABLE(md->box_mount_commands),
                    md->string_mount_command , 1, 2,
                    0, 1, GTK_EXPAND|GTK_FILL, GTK_SHRINK, BORDER, 0);

    md->string_umount_command = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY(md->string_umount_command ),
                    g_strdup(mt->umount_command ));
    gtk_widget_show (md->string_umount_command );
    gtk_table_attach (GTK_TABLE(md->box_mount_commands),
                    md->string_umount_command , 1, 2,
                    1, 2, GTK_EXPAND|GTK_FILL, GTK_SHRINK, BORDER, 0);

    if (!set_active) /* following command wasn't executed by signal handler! */
        gtk_widget_set_sensitive ( md->box_mount_commands, FALSE );

    _label = gtk_label_new_with_mnemonic (_("_Commands"));
    gtk_widget_show (_label);
    gtk_notebook_append_page (GTK_NOTEBOOK(_notebook), _vbox, _label);

    /* File systems tab page */
    _vbox = gtk_vbox_new (FALSE, BORDER);
    gtk_container_border_width (GTK_CONTAINER(_vbox), BORDER);
    gtk_widget_show (_vbox);

        /* show include_NFSs */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), _eventbox,
        _("Activate this option to also display network file systems like "
        "NFS, SMBFS, SHFS and SSHFS."),
        NULL );

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
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), _eventbox,
        _("Activate this option to also eject a CD-drive after unmounting"
        " and to insert before mounting."),
        NULL );

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
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), _eventbox,
        _("Activate this option to only have the mount points be displayed."),
        NULL );

    md->show_exclude_devicenames = gtk_check_button_new_with_mnemonic (
                                _("Display _mount points only") );

    gtk_widget_show (md->show_exclude_devicenames);
    gtk_container_add (GTK_CONTAINER (_eventbox), md->show_exclude_devicenames );
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(md->show_exclude_devicenames),
                                mt->exclude_devicenames);



        /* Exclude file systems  */
    _eventbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (_vbox), GTK_WIDGET(_eventbox),
                        FALSE, FALSE, 0);
    gtk_widget_show (_eventbox);
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), _eventbox,
        _("Exclude the following file systems from the menu.\n"
        "The list is separated by simple spaces.\n"
        "It is up to you to specify correct devices or mount points."),
        NULL );

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
mount_construct (XfcePanelPlugin *plugin)
{
    xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    t_mounter *mounter;

    mounter = create_mounter_control (plugin);

    mounter_read_config (plugin, mounter);

    mounter->button_pb = gdk_pixbuf_new_from_file (mounter->icon, NULL);
       xfce_iconbutton_set_pixbuf (XFCE_ICONBUTTON(mounter->button),
                                   mounter->button_pb);

    g_signal_connect (plugin, "free-data", G_CALLBACK (mounter_free), mounter);

    g_signal_connect (plugin, "save", G_CALLBACK (mounter_write_config),
                      mounter);

    xfce_panel_plugin_menu_show_configure (plugin);
    g_signal_connect (plugin, "configure-plugin",
                      G_CALLBACK (mounter_create_options), mounter);

    g_signal_connect (plugin, "size-changed", G_CALLBACK (mounter_set_size),
                         mounter);

    gtk_container_add (GTK_CONTAINER(plugin), mounter->button);

    xfce_panel_plugin_add_action_widget (plugin, mounter->button);

}

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (mount_construct);
