
/*******************************************************************************/
/* Copyright (C) 2008-2021 Jonathan Moore Liles (as "Non-Timeline")            */
/* Copyright (C) 2023- Stazed                                                  */
/*                                                                             */
/* This file is part of Non-Timeline-XT                                        */
/*                                                                             */
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

#include "Audio_File.H"
#include "Audio_File_SF.H"
#include "Audio_File_Dummy.H"

#include "const.h"
#include "../../../nonlib/debug.h"
#include "../../../nonlib/Block_Timer.H"

#include <string.h>

std::map <std::string, Audio_File*> Audio_File::_open_files;

Audio_File::~Audio_File ( )
{
    DMESSAGE( "Freeing Audio_File object for \"%s\"", _filename );

    _open_files[ std::string( _filename ) ] = NULL;

    if ( _filename )
        free( _filename );

    if ( _path )
        free( _path );
}

const Audio_File::format_desc *
Audio_File::find_format ( const format_desc *fd, const char *name )
{
    for ( ; fd->name; ++fd )
        if ( ! strcmp( fd->name, name ) )
            return fd;

    return NULL;
}

void
Audio_File::all_supported_formats ( std::list <const char *> &formats )
{
    const format_desc *fd;

    fd = Audio_File_SF::supported_formats;

    for ( ; fd->name; ++fd )
        formats.push_back( fd->name );
}

static bool
is_absolute ( const char *name )
{
    return *name == '/';
}

/** return pointer to /name/ corrected for relative path. */
char *
Audio_File::path ( const char *name )
{
    char *path = 0;

    if ( is_absolute( name ) )
        path = strdup( name );
    else
        asprintf( &path, "sources/%s", name );

    return path;
}

const char *
Audio_File::filename ( void ) const
{
    return _path;
}

static bool
is_poor_seeker ( const char * filename )
{
    if ( ( strlen(filename) > 4 &&
        ! strcasecmp( &filename[strlen(filename) - 4], ".ogg" ) )
        ||
        ( strlen(filename) > 5 &&
        ! strcasecmp( &filename[strlen(filename) - 5], ".flac" ) )
    )
    {
        return true;
    }

    return false;
}

/** attempt to open any supported filetype */
Audio_File *
Audio_File::from_file ( const char * filename )
{
    Block_Timer timer( "Opened audio file" );

    Audio_File *a;

    if ( is_poor_seeker(filename) )
    {
        /* OGG and FLAC have poor seek performance, so they require
         * separate file descriptors to be useful */
    }
    else
    {
        /* WAV are quick enough to seek that we can save
         * filedescriptors by sharing them between regions */
        if ( ( a = _open_files[ std::string( filename ) ] ) )
        {
            ++a->_refs;

            return a;
        }
    }

    if ( ( a = Audio_File_SF::from_file( filename ) ) )
        goto done;

    // TODO: other formats

    DWARNING( "creating dummy source for \"%s\"", filename );

    /* FIXME: wrong place for this? */
    if ( ( a = Audio_File_Dummy::from_file( filename ) ) )
        goto done;

    return NULL;

done:

    /* ASSERT( ! _open_files[ std::string( filename ) ], "Programming errror" ); */

    _open_files[ std::string( filename ) ] = a;

    return a;
}

Audio_File *
Audio_File::duplicate ( void )
{
    if ( is_poor_seeker( _filename ) )
    {
        return from_file(_filename);
    }
    else
    {
        ++_refs;
        return this;
    }
}

/** release the resources assoicated with this audio file if no other
 * references to it exist */
void
Audio_File::release ( void )
{
    if ( --_refs == 0 )
        delete this;
}

bool
Audio_File::read_peaks( float fpp, nframes_t start, nframes_t end, int *peaks, Peak **pbuf, int *channels )
{
    *peaks = 0;
    *channels = 0;
    *pbuf = NULL;

    if ( dummy() )
        return false;
    else
    {
        *peaks = _peaks.fill_buffer( fpp, start, end );

        *channels = this->channels();

        *pbuf = _peaks.peakbuf();

        return true;
    }
}
