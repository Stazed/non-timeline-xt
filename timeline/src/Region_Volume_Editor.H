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
 * File:   Region_Volume_Editor.H
 * Author: sspresto
 *
 * Created on February 7, 2024, 7:43 PM
 */

#pragma once

#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Spinner.H>

#include "../../nonlib/debug.h"

class Region_Volume_Editor : public Fl_Menu_Window
{
public:
    Region_Volume_Editor(float &scale);

    /* not permitted */
    Region_Volume_Editor(const Region_Volume_Editor& orig);
    Region_Volume_Editor & operator = ( const Region_Volume_Editor &rhs );

    virtual ~Region_Volume_Editor();
private:
    
    float &_scale;
    Fl_Spinner *_si;

    static void
    enter_cb ( Fl_Widget *, void *v )
    {
        ((Region_Volume_Editor*)v)->enter_cb();
    }

    void
    enter_cb ( void )
    {
        _scale = _si->value();

        hide();
    }

protected:
    int handle ( int m );
};


