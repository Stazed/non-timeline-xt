
/*******************************************************************************/
/* Copyright (C) 2012 Jonathan Moore Liles                                     */
/*                                                                             */
/* This program is free software; you can redistribute it and/or modify it     */
/* under the terms of the GNU General Public License as published by the       */
/* Free Software Foundation; either version 2 of the License, or (at your      */
/* option) any later version.                                                  */
/*                                                                             */
/* This program is distributed in the hope that it will be useful, but WITHOUT */
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       */
/* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   */
/* more details.                                                               */
/*                                                                             */
/* You should have received a copy of the GNU General Public License along     */
/* with This program; see the file COPYING.  If not,write to the Free Software */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */
/*******************************************************************************/

#include "const.h"
#include "debug.h"
#include "Timeline.H"
#include "TLE.H"
#include "NSM.H"
#include "Project.H"

#define OSC_INTERVAL 0.2f

extern char *instance_name;
extern Timeline *timeline;

extern NSM_Client *nsm;

NSM_Client::NSM_Client ( )
{
}

int command_open ( const char *name, const char *display_name, const char *client_id, char **out_msg );
int command_save ( char **out_msg );

int
NSM_Client::command_save ( char **out_msg )
{
   if ( timeline->command_save() )
       return ERR_OK;
   else
   {
       *out_msg = strdup( "Failed to save for unknown reason");
       return ERR_GENERAL;
   }
}

int 
NSM_Client::command_open ( const char *name, const char *display_name, const char *client_id, char **out_msg )
{
    if ( instance_name )
        free( instance_name );
    
    instance_name = strdup( client_id );

    if ( Project::validate( name ) )
    {
        if ( timeline->command_load( name, display_name ) )
            return ERR_OK;
        else
        {
            *out_msg = strdup( "Failed to load for unknown reason" );
            return ERR_GENERAL;
        }
    }
    else
    {
        if ( timeline->command_new( name, display_name ) )
            return ERR_OK;
        else
        {
            *out_msg = strdup( "Failed to load for unknown reason" );
            return ERR_GENERAL;
        }
    }

    return 0;
}