
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

#include <FL/fl_draw.H>
#include "../../FL/test_press.H"

#include "Control_Point.H"
#include "Track.H"

Control_Point::Control_Point ( Sequence *t, nframes_t when, float y )
{
    _sequence = NULL;
    _y = y;
    _r->start = when;
    _box_color = FL_WHITE;

    t->add( this );

    log_create();
}

Control_Point::Control_Point ( const Control_Point &rhs ) : Sequence_Point( rhs )
{
    _y = rhs._y;

    log_create();
}

void
Control_Point::get ( Log_Entry &e ) const
{
    Sequence_Point::get( e );

    e.add( ":y", _y );
}

void
Control_Point::set ( Log_Entry &e )
{
    for ( int i = 0; i < e.size(); ++i )
    {
        const char *s, *v;

        e.get( i, &s, &v );

        if ( ! strcmp( s, ":y" ) )
            _y = atof( v );

        redraw();

        //          _make_label();
    }

    Sequence_Point::set( e );
}

void
Control_Point::draw_box ( void )
{
    if ( selected() )
        fl_color( selection_color() );
    else
        fl_color( box_color() );

    int Y = y();

    // To adjust the control point size based on the track size
    Track *t = timeline->track_under( Y );

    int size = 0;
    if(t)
    {
        size = t->size();
    }

    int W = abs_w() + size;
    int H = h() + size;

    fl_pie( x() - ( W / 2 ), Y - ( H / 2 ), W, H, 0, 360 );

    if ( this == Sequence_Widget::belowmouse() ||
        this == Sequence_Widget::pushed() )
    {
        char val[10];
        snprintf( val, sizeof( val ), "%+.2f", 1.0 - _y * 2 );

        Fl_Align a = 0;

        if ( x() < _sequence->x() + ( _sequence->w() / 2 ) )
            a |= FL_ALIGN_RIGHT;
        else
            a |= FL_ALIGN_LEFT;

        if ( y() < _sequence->y() + ( _sequence->h() / 2 ) )
            a |= FL_ALIGN_BOTTOM;
        else
            a |= FL_ALIGN_TOP;

        draw_label( val, a, FL_FOREGROUND_COLOR );
    }
}

int
Control_Point::handle ( int m )
{
    int r = Sequence_Widget::handle( m );

    switch ( m )
    {
        case FL_RELEASE:
            redraw();
            break;
        case FL_DRAG:
        {
            if ( nselected() > 1 )
            {
                // If multiple items then only allow horizontal movement with FL_BUTTON1.
                // If FL_BUTTON1 + FL_ALT, then selected control points will all align vertically
                // while fixed on the horizontal axis, and can be dragged vertically.
                if(!test_press( FL_BUTTON1 + FL_ALT ))
                    break;  // horizontal only
            }

            int Y = Fl::event_y() - parent()->y();

            if ( Y >= 0 && Y < parent()->h() )
            {
                _y = (float)Y / parent()->h();
                redraw();
            }

            break;
        }
    }

    return r;
}
