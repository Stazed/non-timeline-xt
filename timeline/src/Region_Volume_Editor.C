/*******************************************************************************/
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

/*
 * File:   Region_Volume_Editor.C
 * Author: sspresto
 *
 * Created on February 7, 2024, 7:43 PM
 */

#include <FL/Fl.H>
#include "Region_Volume_Editor.H"

Region_Volume_Editor::Region_Volume_Editor(float &scale) :
    Fl_Menu_Window (75, 58, "Edit Volume"),
    _scale(scale)
{
    set_modal();

    Fl_Float_Input *fi = _fi = new Fl_Float_Input( 12, 0 + 24, 50, 24, "Volume" );
    fi->align( FL_ALIGN_TOP );
    fi->when( FL_WHEN_NOT_CHANGED | FL_WHEN_ENTER_KEY );
    fi->callback( &Region_Volume_Editor::enter_cb, (void*)this );

    char pat[10];
    snprintf( pat, sizeof( pat ), "%.2f", scale );

    fi->value( pat );

    end();

    Region_Volume_Editor::show();

    while ( shown() )
        Fl::wait();
}

Region_Volume_Editor::~Region_Volume_Editor()
{
}
