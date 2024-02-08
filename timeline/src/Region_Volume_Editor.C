/*******************************************************************************/
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

#include "Region_Volume_Editor.H"


Region_Volume_Editor::Region_Volume_Editor(float &scale) : 
    Fl_Menu_Window (100, 50, "Edit Volume"),
    _scale(scale)
{
    set_modal();

    Fl_Spinner *si = _si = new Fl_Spinner( 10, 18, 80, 24, "Volume" );
    si->align( Fl_Align(FL_ALIGN_TOP) );
    si->when( FL_WHEN_ENTER_KEY );
    si->type(FL_FLOAT_INPUT);
    si->minimum(0);
    si->maximum(5);
    si->step(0.01);
    si->callback( &Region_Volume_Editor::enter_cb, (void*)this );

    si->value( scale );

    end();

    show();

    while ( shown() )
        Fl::wait();
}

Region_Volume_Editor::~Region_Volume_Editor()
{
}

int
Region_Volume_Editor::handle ( int m )
{
    DMESSAGE("GET HERE %d", m);
    return Fl_Menu_Window::handle(m);
}