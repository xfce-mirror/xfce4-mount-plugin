/* devices.c */

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

#include <glib.h>
#include <fstab.h>
#include <mntent.h>
#include <sys/vfs.h>
#include <stdio.h>
#include "devices.h"

/* for internationalization, by F. Nowak */
#include <libxfce4util/i18n.h>
/* end i18n extension */

#include <panel/xfce.h>

#define MTAB "/etc/mtab"
#define KB 1024
#define MB 1048576
#define GB 1073741824

/*-------------------- get_size_human_readable --------------------*/
/* return a string containing a size expressed in KB,MB or GB and the unit it is expressed in */
char * get_size_human_readable(float size)
{
	if (size < KB) return g_strdup_printf(_("%.1f B"),size);
	if (size < MB) return g_strdup_printf(_("%.1f KB"),size / KB);
	if (size < GB) return g_strdup_printf(_("%.1f MB"),size / MB);
	return g_strdup_printf(_("%.1f GB"), size / GB);
	
}
/*--------------------------------------------------------------------*/

/*-------------------------- mount_info_print --------------------------*/
void mount_info_print(t_mount_info * mount_info)
{
	if (mount_info != NULL)
	{
		printf(_("size : %g\n"),mount_info->size);
		printf(_("used size : %g\n"),mount_info->used);
		printf(_("available size : %g\n"),mount_info->avail);
		printf(_("percentage used: %d\n"),mount_info->percent);
		printf(_("file system type : %s\n"),mount_info->type);
		printf(_("actual mount point : %s\n"),mount_info->mounted_on);
	}
	return ;
}
/*-----------------------------------------------*/

/*------------------------- mount_info_new --------------------*/
t_mount_info * mount_info_new(float size,float used,float avail, unsigned int percent,char * type, char * mounted_on)
{
	if ( type != NULL && mounted_on != NULL && size != 0 )
	{
		t_mount_info * mount_info ;
		mount_info = g_new0(t_mount_info,1);
		mount_info->size = size;
		mount_info->used = used ;
		mount_info->avail = avail ;
		mount_info->percent = percent;
		mount_info->type = g_strdup(type);
		mount_info->mounted_on = g_strdup(mounted_on);
		return mount_info;
	}
	return NULL;
}
/*-----------------------------------------------------*/

/* ------------- mount_info_new_from_stat----------------*/
/* create a new struct t_mount_info from a struct statfs data */
t_mount_info * mount_info_new_from_stat(struct statfs * pstatfs, char * mnt_type,char * mnt_dir)
{
	if (pstatfs != NULL && mnt_type != NULL && mnt_dir !=NULL)
	{
		t_mount_info * mount_info ;
		float size ; // total size of device
		float used; // used size of device
		float avail; //Available size of device
		unsigned int percent ; //percentage used
		
		/*compute sizes in bytes */
		size = (float)pstatfs->f_bsize * (float)pstatfs->f_blocks ;
		used = (float)pstatfs->f_bsize * ((float)pstatfs->f_blocks - (float)pstatfs->f_bfree);
		avail = (float)pstatfs->f_bsize * (float)pstatfs->f_bavail;
		percent = ((float)pstatfs->f_blocks - (float)pstatfs->f_bavail) * 100 / (float)pstatfs->f_blocks;
		
		/*create new t_mount_info */
		mount_info = mount_info_new(size,used,avail,percent,mnt_type,mnt_dir) ;
		return mount_info ;
	}
	return NULL;
	
}
/*---------------------------------------------------------*/


/*-------- free a struct t_mount_info -------------------*/
void mount_info_free(t_mount_info * * mount_info)
{
	if (*mount_info != NULL)
	{
		g_free((*mount_info)->mounted_on);
		g_free((*mount_info)->type);
		g_free((*mount_info));
		*mount_info = NULL ;
	}
	return ;
}
/*------------------------------------------------------*/




/*------- print a t_disk struct using printf -------------*/
void disk_print(t_disk * pdisk)
{
	if (pdisk != NULL)
	{
		printf(_("disk : %s\n"),pdisk->device);
		printf(_("mount point : %s\n"), pdisk->mount_point);
		if (pdisk->mount_info != NULL)
			mount_info_print(pdisk->mount_info);	
		else printf(_("not mounted\n"));
	}
	return;
}
/*----------------------------------------*/

/* create a new t_disk struct */
t_disk * disk_new( const char * dev, const char * mp)
{
	if ( dev != NULL && mp != NULL)
	{
		t_disk  * pdisk ;
		pdisk = g_new0(t_disk,1);
		pdisk->device = g_strdup(dev);
		pdisk->mount_point = g_strdup(mp) ;
		pdisk->mount_info = NULL ;
		return pdisk ;
	}
	return NULL ;
}
/* -------------------------------------*/

/* free a t_disk struct */
void disk_free(t_disk * * pdisk)
{
	if (*pdisk !=NULL)
	{
		g_free((*pdisk)->device);
		g_free((*pdisk)->mount_point);
		mount_info_free(&((*pdisk)->mount_info));
		g_free(*pdisk);
		*pdisk = NULL ;
	}
	return;
}
/* ----------------------------------------*/

/*------------ mount a t_disk ---------------*/
/* return exit status of the mount command*/
void disk_mount(t_disk * pdisk, char * on_mount_cmd)
{
	if (pdisk != NULL)
	{
		char * cmd ;

		cmd = g_strconcat("bash -c 'mount ",pdisk->mount_point,NULL);
		if (on_mount_cmd != NULL)
			cmd = g_strconcat(cmd," && ",on_mount_cmd," ", pdisk->mount_point, " ' ", NULL);
		else
			cmd = g_strconcat(cmd," ' ",NULL);
		DBG("cmd :%s",cmd);
		exec_cmd_silent(cmd,FALSE,FALSE);
		g_free(cmd);
	}
}
/*-------------------------------------------*/

/* --------------unmount a t_disk ----------------*/
/* return exit status of the mount command*/
void disk_umount(t_disk * pdisk )
{
	if (pdisk != NULL)
	{
		char * cmd ;
		/*char * standard_output ;
		char * standard_error ;
		int exit_status ;
		GError * error ;
		gboolean bresult ;
		*/
		/* changed from pdisk->device to pdisk->mount_point */
		cmd = g_strconcat("umount ",pdisk->mount_point,NULL);
		
		/*bresult = g_spawn_command_line_sync(cmd,&standard_output,&standard_error,&exit_status,&error);
		if (bresult)
		{
			if (exit_status) 
			{
				mount_info_free(&(pdisk->mount_info));
			}
			
			return exit_status ;
		}
		*/
		exec_cmd_silent(cmd,FALSE,FALSE);
		g_free(cmd);
	}
	//return -1 ;
}
/*------------------------------------------------*/

/*------------------------- disks_new ----------------*/
/* fill a GPtrArray with pointers on struct t_disk containing infos on devices and theoretical mount point. use setfsent() and getfsent(). */
GPtrArray * disks_new()
{
	GPtrArray * pdisks ;
	t_disk * pdisk ;
	struct fstab * pfstab ;
	
	// open fstab
	if (setfsent() != 1)
		return NULL ; // on error return NULL
	
	pdisks = g_ptr_array_new();
	
	for(;;)
	{
		pfstab = getfsent() ; //read a line in fstab
		if (pfstab == NULL) break ; // on eof exit
		// if pfstab->fs_spec do not begins with /dev discard
		//if pfstab->fs_file == none discard
		if ( g_str_has_prefix(pfstab->fs_spec,"/dev/") && (g_str_has_prefix(pfstab->fs_file,"/") != 0) )
		{
			pdisk = disk_new(pfstab->fs_spec, pfstab->fs_file);
			g_ptr_array_add( pdisks , pdisk );

		}
	}
	endfsent(); //closes file
	return pdisks ;
}

/*--------------------- disks_free --------------------------*/
/* free a GPtrArray containing pointer on struct t_disk elements */
void disks_free(GPtrArray * * pdisks)
{
	if (*pdisks != NULL)
	{
		int i ;
		t_disk * pdisk ;
		for (i=0; i < (*pdisks)->len ; i++)
		{
			pdisk = (t_disk*)(g_ptr_array_index((*pdisks),i)) ;
			disk_free( &pdisk ) ;
		}
		g_ptr_array_free((*pdisks), TRUE);
		(*pdisks) = NULL ;
	}
}

/*----------------------- disks_print----------------------*/
/* print a GPtrArray containing pointer on struct t_disk elements */
void disks_print(GPtrArray * pdisks)
{
	int i ;
	for (i=0; i < pdisks->len ; i++)
	{
		disk_print( g_ptr_array_index(pdisks,i) ) ;
	}
	return ;

}

/*-------------------- disks_search -------------------------*/
/*CHANGE !!!!!!!!!!!!!!!!!! search on mount directory rather than device name */
/*return a pointer on FIRST struct t_disk containing char * device as device field 
if not found return NULL */
t_disk * disks_search(GPtrArray * pdisks, char * mount_point)
{
	int i ;
	for (i=0; i < pdisks->len ; i++)
	{
		if (g_ascii_strcasecmp(((t_disk*)g_ptr_array_index(pdisks,i))->mount_point,mount_point)==0)
			return g_ptr_array_index(pdisks,i);
	}
	return NULL;
}

/* -------- disks_free_mount_info ----------------*/
/* remove struct t_mount_info from a GPtrArray containing struct t_disk * elements */
void disks_free_mount_info(GPtrArray * pdisks)
{
	int i ;
	for (i=0; i < pdisks->len ; i++)
	{
		mount_info_free( &(((t_disk*)g_ptr_array_index(pdisks,i))->mount_info)) ;
	}
	return ;

}


/* --------------- disks_refresh ----------------------*/
/* refresh t_mount_info infos in a GPtrArray containing struct t_disk * elements */
void disks_refresh(GPtrArray * pdisks)
{
	/* using getmntent to get filesystems mount information */
	
	FILE * fmtab = NULL ; // file /etc/mtab
	struct mntent * pmntent = NULL; // struct for mnt info
	struct statfs * pstatfs = NULL ;
	
	t_mount_info * mount_info ;
	t_disk * pdisk ;
	
	/*remove t_mount_info for all devices */
	disks_free_mount_info(pdisks);
	
	/*allocate new struct statfs */
	pstatfs = g_new0(struct statfs,1);
	
	/*open file*/
	fmtab = setmntent(MTAB,"r"); 
	
	/* start looking for mounted devices */
	for(pmntent = getmntent(fmtab) ; pmntent!=NULL ; pmntent = getmntent(fmtab))
	{
		/*getstat on disk */
		if (statfs(pmntent->mnt_dir,pstatfs)==0 && (pstatfs->f_blocks != 0))
		{	/*if we got the stat and they block number is non zero */
			
			/* get pointer on disk from pdisks */
			/*CHANGED to reflect change in disk_search*/
			pdisk = disks_search(pdisks,pmntent->mnt_dir);
			if (pdisk == NULL) //if disk is not found in pdisks
			{
				/*create a new struct t_disk and add it to pdisks */
				/* test for "/dev/" and mnt_dir != none */
				if ( !g_str_has_prefix(pmntent->mnt_fsname,"/dev/") || (g_ascii_strcasecmp(pmntent->mnt_dir,"none") == 0) ) continue ;
		
				pdisk = disk_new(pmntent->mnt_fsname,pmntent->mnt_dir);
				g_ptr_array_add(pdisks,pdisk);
			}
			
			/* create new t_mount_info */	
			mount_info = mount_info_new_from_stat(pstatfs, pmntent->mnt_type, pmntent->mnt_dir) ;
			/* add it to pdisk */
			pdisk->mount_info = mount_info ;
		
		}
	}
	
	g_free(pstatfs);
	endmntent(fmtab); //close file
	return ;
}
