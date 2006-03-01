/* mount-plugin.c */

/*
Copyright (C) 2005 Jean-Baptiste jb_dul@yahoo.com
Copyright (C) 2005, 2006 Fabian Nowak timystery@arcor.de.

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

/* #define DEBUG 1
#define DEBUG_TRACE 1 */

#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include "devices.h"
#include "icons.h"

#define APP_NAME N_("Mount Plugin")

#define BORDER 6

#define DEFAULT_MOUNT_COMMAND "mount"
#define DEFAULT_UMOUNT_COMMAND "umount"

static GtkTooltips *tooltips = NULL;

/*--------- graphical interface ----------*/
typedef struct 
{
   XfcePanelPlugin *plugin;
	char *on_mount_cmd;
	gchar *mount_command;
	gchar *umount_command;
	gboolean message_dialog;
	GtkWidget * button;
	GtkWidget * menu;
	GPtrArray * pdisks; /* contains pointers to struct t_disk */
} t_mounter;
/*---------------------------------*/

/*--------- disk button data ------------------*/
typedef struct
{
	GtkWidget * menu_item;
	GtkWidget * hbox;
	GtkWidget * label_disk;
	GtkWidget * label_mount_info;
	GtkWidget * progress_bar;
} t_disk_display;
/*------------------------------------------------*/

/*------------- settings dialog --------------------------*/
typedef struct
{
	t_mounter *mt;
	GtkWidget *dialog;
	/* options */
	GtkWidget *string_cmd;
	GtkWidget *specify_commands;
	GtkWidget *box_mount_commands;
	GtkWidget *string_mount_command;
	GtkWidget *string_umount_command;
	GtkWidget *show_message_dialog;
}
t_mounter_dialog;
/*------------------------------------------------------*/


static void
on_message_dialog_response (GtkWidget *widget, gpointer *data) {
	gtk_widget_destroy(widget);
}

/*---------------- on_activate_disk_display ---------------*/
static void 
on_activate_disk_display (GtkWidget *widget, t_disk * disk)
{
    TRACE ("enters on_activate_disk_display");
	
    t_mounter * mt;
    mt = (t_mounter *) g_object_get_data (G_OBJECT(widget), "mounter");
	
    DBG ("message dialog to be shown? %d", mt->message_dialog);
	
    if (disk != NULL) {
        if (disk->mount_info != NULL) { /* disk is mounted */
            int result = disk_umount (disk, mt->umount_command, mt->message_dialog);
			
            if (mt->message_dialog) { /* popup dialog */
            
            		gchar *msg = (gchar *) g_malloc (1024*sizeof(gchar));
            	    if (result==NONE)
            	    		msg = _("The device \"%s\" should be removable safely now.");
            	    	else
            	    		msg = _("An error occured. The device should not be removed!");
            
                GtkWidget *my_dlg;
                my_dlg = gtk_message_dialog_new (NULL, 
                        GTK_DIALOG_DESTROY_WITH_PARENT, 
                        GTK_MESSAGE_INFO,
                        GTK_BUTTONS_OK,
                        msg,
                        disk->mount_point);
                 
                 g_signal_connect (my_dlg, "response",
                        G_CALLBACK(on_message_dialog_response), my_dlg);
                 gtk_widget_show (my_dlg);
            		/* gtk_dialog_run (GTK_DIALOG (my_dlg)); */
            }
        }
        else { /* disk is not mounted */
            disk_mount (disk, mt->on_mount_cmd, mt->mount_command);
        }
    }

    TRACE ("leaves on_activate_disk_display");
}
/*---------------------------------------------------------*/

/*---------------------- mounter_set_size ----------------------*/
static void
mounter_set_size (XfcePanelPlugin *plugin, int size, t_mounter *mt)
{
   /* schrink the gtk button's image to new size - */
   gtk_widget_set_size_request (GTK_WIDGET(mt->button), size - 4, size - 4);
 
}
/*---------------------------------------------------------*/

/*-------------------- disk_display_new -----------------*/
/*create a new t_disk_display from t_disk infos */
static t_disk_display* 
disk_display_new (t_disk *disk, t_mounter *mounter)
{
	TRACE ("enters disk_display_new");

	if (disk != NULL) 
	{
		t_disk_display * dd ;
		dd = g_new0 (t_disk_display, 1) ;
		dd->menu_item = gtk_menu_item_new();
		g_signal_connect (G_OBJECT(dd->menu_item), "activate",
		                  G_CALLBACK(on_activate_disk_display), disk);
		g_object_set_data (G_OBJECT(dd->menu_item), "mounter", (gpointer)mounter);
		
		dd->hbox = gtk_hbox_new (FALSE, 10);
		gtk_container_add (GTK_CONTAINER(dd->menu_item), dd->hbox);
		
		dd->label_disk = gtk_label_new (g_strconcat(disk->device, " -> ",
		                                disk->mount_point, NULL));
		/*change to uniform label size*/
		gtk_label_set_width_chars(GTK_LABEL(dd->label_disk),28);
		gtk_label_set_justify(GTK_LABEL(dd->label_disk),GTK_JUSTIFY_LEFT);
		gtk_box_pack_start(GTK_BOX(dd->hbox),dd->label_disk,FALSE,TRUE,0);
		
		dd->label_mount_info = gtk_label_new("");
		/*change to uniform label size*/
		gtk_label_set_width_chars(GTK_LABEL(dd->label_mount_info),25);
		gtk_label_set_use_markup(GTK_LABEL(dd->label_mount_info),TRUE);
		gtk_label_set_justify(GTK_LABEL(dd->label_mount_info),GTK_JUSTIFY_RIGHT);
		gtk_box_pack_start(GTK_BOX(dd->hbox),dd->label_mount_info,TRUE,TRUE,0);
		
		dd->progress_bar = gtk_progress_bar_new();
		gtk_box_pack_start(GTK_BOX(dd->hbox),dd->progress_bar,TRUE,TRUE,0);
		return dd ;
	}
	TRACE ("leaves disk_display_new");
	
	return NULL ;
}
/*-----------------------------------------------------------*/


/*-------------------- disk_display_refresh -----------------*/
static void 
disk_display_refresh(t_disk_display * disk_display, 
                                 t_mount_info * mount_info)
{
	TRACE("enters disk_display_refresh");
	if (disk_display != NULL)
	{
		if (mount_info != NULL)
		{	/* device is mounted */
			char * text ;
			char * used = get_size_human_readable(mount_info->used);
			//DBG("used is now : %s",used);
			char * size = get_size_human_readable(mount_info->size);
			//DBG("size is now : %s",size);
			char * avail = get_size_human_readable(mount_info->avail);
			//DBG("avail is now : %s",size);
			text = g_strdup_printf("[%s/%s] %s free", used ,size,avail );
			//DBG("text is now : %s",text);
			
			g_free(used);
			g_free(size);
			g_free(avail);
			//text = g_strdup_printf("mounted on : %s\t[%g Mb/%g Mb] %g Mb free",mount_info->mounted_on, mount_info->used, mount_info->size, mount_info->avail);
			gtk_label_set_text(GTK_LABEL(disk_display->label_mount_info),text);
			
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(disk_display->progress_bar),((gdouble)mount_info->percent / 100) );
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(disk_display->progress_bar),g_strdup_printf("%d",mount_info->percent));
			gtk_widget_show(GTK_WIDGET(disk_display->progress_bar));
		}
		else /* mount_info == NULL */
		{
			/*remove mount info */
			gtk_label_set_markup(GTK_LABEL(disk_display->label_mount_info),_("<span foreground=\"#FF0000\">not mounted</span>"));
			gtk_widget_hide(GTK_WIDGET(disk_display->progress_bar));

		}
	}
	TRACE("leaves disk_display_refresh");
}
/*----------------------------------------------*/

/*--------------- mounter_data_free -----------------*/
static void 
mounter_data_free (t_mounter * mt)
{
	TRACE ("enters mounter_data_free");
	
	disks_free (&(mt->pdisks));
	gtk_widget_destroy (GTK_WIDGET(mt->menu));
	mt->menu = NULL;
	
	TRACE ("leaves mounter_data_free");
}
/*----------------------------------------------*/

/*--------------- mounter_free -----------------*/
static void 
mounter_free (XfcePanelPlugin *plugin, t_mounter *mounter)
{
	TRACE ("enters mounter_free");
	
	mounter_data_free (mounter);
	
	g_free (mounter);
	
	TRACE ("leaves mounter_free");
}
/*----------------------------------------------*/
/*---------------- mounter_data_new --------------------------*/
static void 
mounter_data_new (t_mounter *mt)
{
	TRACE ("enters mounter_data_new");

	int i ;
	t_disk * disk ;
	t_disk_display * disk_display ;
	
	/*get static infos from /etc/fstab */
	mt->pdisks = disks_new();
	
	/* get dynamic infos on mounts */
	disks_refresh (mt->pdisks);
	
	/* menu with menu_item */
	mt->menu = gtk_menu_new ();
	
	for(i=0;i < mt->pdisks->len;i++)
	{
		disk = g_ptr_array_index(mt->pdisks,i); /* get the disk */
		disk_display = disk_display_new(disk,mt); /* creates a disk_display */
		
		/* fill in mount infos */
		disk_display_refresh(disk_display,disk->mount_info); 
		
		/* add the menu_item to the menu */
		gtk_menu_shell_append(GTK_MENU_SHELL(mt->menu),disk_display->menu_item); 
	}
	gtk_widget_show_all(mt->menu);
	
	mt->mount_command = DEFAULT_MOUNT_COMMAND;
	mt->umount_command = DEFAULT_UMOUNT_COMMAND;
	
	mt->message_dialog = FALSE;
	
	TRACE ("leaves mounter_data_new");
	
	return ;
}
/*------------------------------------------------------*/

/*---------------------- mounter_refresh ---------------*/
static void 
mounter_refresh (t_mounter * mt)
{
	TRACE ("enters mounter_refresh");
	
	mounter_data_free (mt);
	gchar *mount = g_strdup(mt->mount_command);
	gchar *umount = g_strdup(mt->umount_command);
	gboolean msg_dlg = mt->message_dialog;
	
	mounter_data_new (mt);
	mt->mount_command = g_strdup(mount);
	mt->umount_command = g_strdup(umount);
	mt->message_dialog = msg_dlg;

	TRACE ("leaves mounter_refresh");
	
}
/*---------------------------------------------------------*/

/* --------------plugin event --------------------------------*/
static gboolean 
on_button_press (GtkWidget *widget, GdkEventButton *event, t_mounter *mounter)
{
	TRACE ("enters on_button_pressed");
	if (mounter != NULL && event->button == 1)
	{
		
		mounter_refresh (mounter); /* refreshs infos regarding mounts data */
		gtk_menu_popup (GTK_MENU(mounter->menu), NULL, NULL, NULL, NULL, 0,
		                event->time);
		return TRUE;
	}
	TRACE ("leaves on_button_pressed");
	return FALSE ;
}
/*------------------------------------------------------*/

/*---------------------- mounter_read_config --------------------*/
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
    
    if ( value = xfce_rc_read_entry(rc, "on_mount_cmd", NULL) )
      mt->on_mount_cmd = g_strdup (value);
    
    
    if ( value = xfce_rc_read_entry (rc, "mount_command", NULL) )
        mt->mount_command = g_strdup (value);
    else
        mt->mount_command = g_strdup (DEFAULT_MOUNT_COMMAND);
    
    if ( value = xfce_rc_read_entry (rc, "umount_command", NULL) )
        mt->umount_command = g_strdup (value);
    else
        mt->umount_command = g_strdup (DEFAULT_UMOUNT_COMMAND);
        
    if ( value = xfce_rc_read_entry(rc, "message_dialog", NULL) )
      mt->message_dialog = atoi (value);
    
    xfce_rc_close (rc);

    TRACE ("leaves read_config");
}
/*-------------------------------------------------------*/

/*------------------- mounter_write_config -----------------------*/
static void
mounter_write_config (XfcePanelPlugin *plugin, t_mounter *mt)
{
	TRACE ("enter write_config");
	
	 XfceRc *rc;
    char *file;
    char value[20];

    if (!(file = xfce_panel_plugin_save_location (plugin, TRUE)))
        return;
        
    /* int res = */ unlink (file);
    
    rc = xfce_rc_simple_open (file, FALSE);
    g_free (file);

    if (!rc)
        return;
	
	DBG ("save on_mount_cmd : %s", mt->on_mount_cmd);
	DBG ("save message_dialog : %d", mt->message_dialog);

    xfce_rc_write_entry (rc, "on_mount_cmd", 
         mt->on_mount_cmd ? mt->on_mount_cmd : "");
         
    if ( strcmp(mt->mount_command, DEFAULT_MOUNT_COMMAND)!=0 )
        xfce_rc_write_entry (rc, "mount_command", mt->mount_command);
    
    if ( strcmp(mt->umount_command, DEFAULT_UMOUNT_COMMAND)!=0 ) 
        xfce_rc_write_entry (rc, "umount_command", mt->umount_command); 
        
    if ( mt->message_dialog==1 ) 
        xfce_rc_write_entry (rc, "message_dialog", "1"); 
    
         
    xfce_rc_close (rc);

	TRACE ("leaves write config");
}
/*--------------------------------------------*/

/*------------------- create_mounter -------------------*/
static t_mounter *
create_mounter_control (XfcePanelPlugin *plugin)
{
	TRACE ("enters create_mounter_control");

	t_mounter *mounter;
	
	mounter = g_new0(t_mounter,1);

	/* default mount command */
	mounter->on_mount_cmd = NULL;	
	
	mounter->plugin = plugin;
	
	if (!tooltips) 
    {
        tooltips = gtk_tooltips_new();
    }

	/*plugin button */
	
	GdkPixbuf * pb ;
	pb = gdk_pixbuf_new_from_inline (sizeof(icon_plugin), icon_plugin, FALSE, 
	                                 NULL);
	mounter->button = xfce_iconbutton_new_from_pixbuf (pb);
	gtk_button_set_relief (GTK_BUTTON(mounter->button), GTK_RELIEF_NONE);

	/* add_tooltip (GTK_WIDGET(mounter->button), _("devices")); */

    gtk_tooltips_set_tip (tooltips, GTK_WIDGET(mounter->button), _("devices"), 
                         NULL);

	/*-------------------------------------------------------------*/
	g_signal_connect (G_OBJECT(mounter->button), "button_press_event",
	                  G_CALLBACK(on_button_press), mounter);
	gtk_widget_show(mounter->button);
	
	/*get the data*/	
	mounter_data_new (mounter);
	
	TRACE ("leaves create_mounter_control");
	return mounter;
}


/*---------- free_mounter_dialog ---------------------*/
static void 
free_mounter_dialog(GtkWidget * widget, t_mounter_dialog * md)
{
	g_free(md);
}
/*----------------------------------------------------*/

/*---------------- mounter_apply_options ---------------*/
static void
mounter_apply_options (t_mounter_dialog *md) 
{
	TRACE ("enters mounter_apply_options");

	t_mounter * mt = md->mt;

    const char * tmp;
	tmp = gtk_entry_get_text (GTK_ENTRY(md->string_cmd));
	
	g_free(mt->on_mount_cmd);
	if (tmp && *tmp)
		mt->on_mount_cmd = g_strdup(tmp);
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
    
    mt->message_dialog = gtk_toggle_button_get_active 
            (GTK_TOGGLE_BUTTON(md->show_message_dialog));
    DBG ("message dialog checkbox activated? %d", mt->message_dialog);
    
    TRACE ("leaves mounter_apply_options");
		
}

static void 
on_optionsDialog_response (GtkWidget *dlg, int response, t_mounter_dialog * md)
{
	mounter_apply_options (md);

    gtk_widget_destroy (md->dialog);
    xfce_panel_plugin_unblock_menu (md->mt->plugin);
    mounter_write_config (md->mt->plugin, md->mt);
}
/*------------------------------------------------------*/

/*-------------- entry_lost_focus -----------------------*/
/* This shows a way to update plugin settings when the user leaves a text entry,
by connecting to the "focus-out" event on the entry.*/

static gboolean
entry_lost_focus(t_mounter_dialog * md)
{
	mounter_apply_options (md);

	/* NB: needed to let entry handle focus-out as well */
	return FALSE;
}
/*----------------------------------------------------*/


static gboolean
specify_command_toggled (GtkWidget *widget, t_mounter_dialog *md) {

    gboolean is_active = gtk_toggle_button_get_active 
                            (GTK_TOGGLE_BUTTON (widget));
    gtk_widget_set_sensitive ( md->box_mount_commands, is_active );

    return TRUE;
}


/*----------------- mounter_create_options -------------*/
static void 
mounter_create_options (XfcePanelPlugin *plugin, t_mounter *mt)
{

	TRACE ("enters mounter_create_options");

    xfce_panel_plugin_block_menu (plugin);

    GtkWidget *dlg, *header;
    dlg = gtk_dialog_new_with_buttons (_("Edit Properties"), 
                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
                GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);
                
    gtk_container_set_border_width (GTK_CONTAINER (dlg), 2);

    header = xfce_create_header (NULL, _("Mount devices"));
    gtk_widget_set_size_request (GTK_BIN (header)->child, -1, 32);
    gtk_container_set_border_width (GTK_CONTAINER (header), BORDER - 2);
    gtk_widget_show (header);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), header, FALSE, 
                        TRUE, 0);

	GtkWidget *vbox, *label;
	t_mounter_dialog * md;
	
	md = g_new0 (t_mounter_dialog, 1);
	
	md->mt = mt;
	
	md->dialog = dlg;
	
	/* don't set a border width, the dialog will take care of that */
	
	vbox =  GTK_DIALOG (dlg)->vbox; /* gtk_vbox_new (FALSE, BORDER); */
	/* gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), GTK_WIDGET (vbox), 
	                    TRUE, TRUE, 0); */
	
	/* entries */	
	GtkWidget *eventbox = gtk_event_box_new ();
	gtk_widget_show (eventbox);
	gtk_container_set_border_width (GTK_CONTAINER (eventbox), BORDER);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (eventbox), FALSE, FALSE, 
	                   0);
	
	GtkWidget *hbox = gtk_hbox_new (FALSE, BORDER);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (eventbox), hbox);
	
	label = gtk_label_new (_("Execute after mounting:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	GtkTooltips *tip = gtk_tooltips_new ();
	gtk_tooltips_enable (tip);
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), eventbox, 
        _("This command will be executed after mounting the device with the "
          "mount point of the device as argument.\n"
          "If you are unsure what to insert, try \"xffm\" or \"rox\" or "
          "\"thunar\"."), 
        NULL );
	
	md->string_cmd = gtk_entry_new ();
	if (mt->on_mount_cmd != NULL)
		gtk_entry_set_text (GTK_ENTRY(md->string_cmd), 
		                    g_strdup(mt->on_mount_cmd));
    gtk_entry_set_width_chars (GTK_ENTRY(md->string_cmd), 21);
	gtk_widget_show (md->string_cmd);
	gtk_box_pack_start (GTK_BOX(hbox), md->string_cmd, FALSE, FALSE, 0);

    GtkWidget *innervbox = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (innervbox), BORDER);
    gtk_widget_show (innervbox);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (innervbox), FALSE, FALSE, 
	                    0);

	eventbox = gtk_event_box_new ();
	gtk_widget_show (eventbox);
	gtk_box_pack_start (GTK_BOX (innervbox), GTK_WIDGET (eventbox), FALSE, FALSE, 
	                    0);
	
	md->specify_commands = gtk_check_button_new_with_label ( 
	                               _("Specify own commands") );
            
    gboolean set_active = 
        ( strcmp(mt->mount_command, DEFAULT_MOUNT_COMMAND)!=0 || 
          strcmp(mt->umount_command, DEFAULT_UMOUNT_COMMAND)!=0 ) ? TRUE : FALSE;
                                    
    gtk_widget_show (md->specify_commands);
    gtk_container_add (GTK_CONTAINER (eventbox), md->specify_commands );
    
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), eventbox, 
        _("WARNING: These options are for experts only! If you do not know "
          "what they may be good for, keep your hands off!"), 
        NULL );
        
    eventbox = gtk_event_box_new ();
	gtk_widget_show (eventbox);
	gtk_box_pack_start (GTK_BOX (innervbox), GTK_WIDGET (eventbox), FALSE, FALSE,
	                    0);
	
    md->box_mount_commands = gtk_table_new (2, 2, FALSE);
    gtk_widget_show (md->box_mount_commands);
    gtk_container_add (GTK_CONTAINER (eventbox), md->box_mount_commands);
	                  
    /* FIXME: labels are centered.
        gtk_label_set_justify does not work,
        adding alignment containers does not do s, either.
        so it must be something with the table: GTK_FILL doesn't do it. */
	label = gtk_label_new (_("Mount command:"));
	gtk_misc_set_alignment (GTK_MISC(label), 0.0, 0.5);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE(md->box_mount_commands), label, 0, 1, 0, 1, 
	                  GTK_SHRINK, GTK_SHRINK, BORDER, 0);
	
	                  
	label = gtk_label_new (_("Unmount command:"));
	gtk_misc_set_alignment (GTK_MISC(label), 0.0, 0.5);
	gtk_widget_show (label);
	gtk_table_attach (GTK_TABLE(md->box_mount_commands), label, 0, 1, 1, 2, 
	                  GTK_SHRINK, GTK_SHRINK, BORDER, 0);
	                  
	md->string_mount_command = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY(md->string_mount_command ), 
	                    g_strdup(mt->mount_command ));
	gtk_widget_show (md->string_mount_command );
	gtk_table_attach (GTK_TABLE(md->box_mount_commands),
	                  md->string_mount_command , 1, 2,
	                  0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	
	md->string_umount_command = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY(md->string_umount_command ), 
	                    g_strdup(mt->umount_command ));
	gtk_widget_show (md->string_umount_command );
	gtk_table_attach (GTK_TABLE(md->box_mount_commands), 
	                  md->string_umount_command , 1, 2,
	                  1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	
	gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), eventbox, 
        _("Most users will only want to prepend \"sudo\" to both "
          "commands or prepend \"sync &&\" to the \"unmount\" command."), 
        NULL );
	
	/* g_signal_connect_swapped (md->string_cmd, "focus-out-event",
			G_CALLBACK(entry_lost_focus), md); */
			
	g_signal_connect ( G_OBJECT(md->specify_commands), "toggled",
			G_CALLBACK(specify_command_toggled), md);
			
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(md->specify_commands),
                                  set_active);
    if (!set_active) /* following command wasn't executed by signal handler! */
        gtk_widget_set_sensitive ( md->box_mount_commands, FALSE );
        
    // show message dialog stuff
    eventbox = gtk_event_box_new ();
	gtk_widget_show (eventbox);
	gtk_box_pack_start (GTK_BOX (innervbox), GTK_WIDGET (eventbox), FALSE, FALSE,
	                    0);
    md->show_message_dialog = gtk_check_button_new_with_label ( 
	                               _("Show message after unmount") );
	gtk_widget_show (md->show_message_dialog);
    gtk_container_add (GTK_CONTAINER (eventbox), md->show_message_dialog );

    gtk_widget_show (md->show_message_dialog);
    gtk_tooltips_set_tip ( GTK_TOOLTIPS(tip), eventbox, 
        _("This is only useful and recommended if you specify \"sync\" as part "
        "of the \"unmount\" command string."), 
        NULL );
        
    DBG ("message dialog checkbox to be activated? %d", mt->message_dialog);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(md->show_message_dialog),
                                  mt->message_dialog);
				
	g_signal_connect (dlg, "response",
            G_CALLBACK(on_optionsDialog_response), md);
	
	gtk_widget_show (dlg);
	
	TRACE ("enters mounter_create_options");
}
/*----------------------------------------------------*/


/* extensions for panel 4.4 */

static void 
mount_construct (XfcePanelPlugin *plugin)
{   
    xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    t_mounter *mounter;

    mounter = create_mounter_control (plugin);

    mounter_read_config (plugin, mounter);
    
    g_signal_connect (plugin, "free-data", G_CALLBACK (mounter_free), mounter);
    
    g_signal_connect (plugin, "save", G_CALLBACK (mounter_write_config), mounter);
    
    xfce_panel_plugin_menu_show_configure (plugin);
    g_signal_connect (plugin, "configure-plugin", 
                      G_CALLBACK (mounter_create_options), mounter);
    
    g_signal_connect (plugin, "size-changed", G_CALLBACK (mounter_set_size), 
                         mounter);
    
    /* g_signal_connect (plugin, "orientation-changed", 
                      G_CALLBACK (monitor_set_orientation), mounter); */
    
    gtk_container_add (GTK_CONTAINER(plugin), mounter->button);

    xfce_panel_plugin_add_action_widget (plugin, mounter->button);
	
}

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (mount_construct);


/* end extensions for panel 4.4 */

