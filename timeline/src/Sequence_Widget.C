
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

#include "Sequence_Widget.H"
#include "Track.H"

#include "const.h"
#include "../../nonlib/debug.h"

using namespace std;

list <Sequence_Widget *> Sequence_Widget::_selection;
Sequence_Widget * Sequence_Widget::_current = NULL;
Sequence_Widget * Sequence_Widget::_pushed = NULL;
Sequence_Widget * Sequence_Widget::_belowmouse = NULL;
Fl_Color Sequence_Widget::_selection_color = FL_MAGENTA;

const double C_NUDGE_RATIO = 0.0213333333333;

Sequence_Widget::Sequence_Widget ( ) :
    _nudge_dirty(false),
    _label(0),
    _can_resize_label(true),
    _sequence(NULL),
    _r(&_range),
    _color(FL_FOREGROUND_COLOR),
    _box_color(FL_BACKGROUND_COLOR),
    _drag(NULL)
{
    _r->start = _r->offset = _r->length = 0;
}

/* careful with this, it doesn't journal */
Sequence_Widget::Sequence_Widget ( const Sequence_Widget &rhs ) : Loggable( rhs )
{
    _drag = NULL;
    _nudge_dirty = false;

    if ( rhs._label )
        _label = strdup( rhs._label );
    else
        _label = 0;

    _range = rhs._range;
    _dragging_range = rhs._dragging_range;
    _r = &_range;

    _color = rhs._color;
    _box_color = rhs._box_color;

    _sequence = NULL;

    if ( rhs._sequence )
        rhs._sequence->add( this );
};

const Sequence_Widget &
Sequence_Widget::operator= ( const Sequence_Widget &rhs )
{
    if ( this == &rhs )
        return *this;

    _r         = &_range;
    _range     = rhs._range;
    _dragging_range = rhs._dragging_range;
    _sequence     = rhs._sequence;
    _box_color = rhs._box_color;
    _color     = rhs._color;
    _drag = NULL;
    _nudge_dirty = false;

    if ( rhs._label )
        _label = strdup( rhs._label );

    return *this;
}

Sequence_Widget::~Sequence_Widget ( )
{
    Sequence_Widget::redraw();

    if ( this == _pushed )
        _pushed = NULL;

    if ( this == _belowmouse )
        _belowmouse = NULL;

    Sequence_Widget::label( NULL );

    _sequence->remove( this );

    _selection.remove( this );
}



void
Sequence_Widget::get ( Log_Entry &e ) const
{
    e.add( ":start", _r->start );
    //    e.add( ":offset", _r->offset );
    //    e.add( ":length", _r->length );
    e.add( ":sequence", _sequence );
    e.add( ":selected", selected() );
}

void
Sequence_Widget::set ( Log_Entry &e )
{
    for ( int i = 0; i < e.size(); ++i )
    {
        const char *s, *v;

        e.get( i, &s, &v );

        if ( ! strcmp( s, ":start" ) )
            _r->start = atoll( v );
        //        else if ( ! strcmp( s, ":offset" ) )
        //            _r->offset = atoll( v );
        //        else if ( ! strcmp( s, ":length" ) )
        //            _r->length = atoll( v );
        else if ( ! strcmp( s, ":selected" ) )
        {
            if ( atoi( v ) )
                select();
            else
                deselect();
        }
        else if ( ! strcmp( s, ":sequence" ) )
        {
            unsigned int ii;
            sscanf( v, "%X", &ii );
            Sequence *t = static_cast<Sequence*>( Loggable::find( ii ) );

            ASSERT( t, "No such object ID (%s)", v );

            t->add( this );
        }
        //                else
        //                    e.erase( i );
    }

    if ( _sequence )
    {
        _sequence->handle_widget_change( _r->start, _r->length );
        _sequence->damage( FL_DAMAGE_USER1 );
    }
}

void
Sequence_Widget::begin_drag ( const Drag &d )
{
    _drag = new Drag( d );

    timeline->sequence_lock.wrlock();

    /* copy current values */

    _dragging_range = _range;

    /* tell display to use temporary */
    _r = &_dragging_range;

    timeline->sequence_lock.unlock();
}

void
Sequence_Widget::end_drag ( void )
{
    /* swap in the new value */
    /* go back to playback and display using same values */

    timeline->sequence_lock.wrlock();

    _range = _dragging_range;
    _r = &_range;

    delete _drag;
    _drag = NULL;

    /* this will result in a sort */
    sequence()->handle_widget_change( _r->start, _r->length );

    timeline->sequence_lock.unlock();
}

void
Sequence_Widget::nudge_some(bool left)
{
    timeline->sequence_lock.wrlock();

    start_log_nudge();

    int X = _r->start;
    if(left)
    {
        X -= (int) double(timeline->sample_rate() * C_NUDGE_RATIO) + 0.5 ;
    }
    else
    {
        X += (int) double(timeline->sample_rate() * C_NUDGE_RATIO) + 0.5 ;
    }

    if(X <= 0)
        X = 0;

    _r->start = X;

    if ( _sequence )
    {
        _sequence->handle_widget_change( _r->start, _r->length );
        _sequence->damage( FL_DAMAGE_USER1 );
    }

    timeline->sequence_lock.unlock();
}

void
Sequence_Widget::pan_some(bool left)
{
    timeline->sequence_lock.wrlock();

    start_log_nudge();

    int Of = _r->offset;
    if(left)
    {
        Of += (int) double(timeline->sample_rate() * C_NUDGE_RATIO) + 0.5 ;
    }
    else
    {
        Of -= (int) double(timeline->sample_rate() * C_NUDGE_RATIO) + 0.5 ;
    }

    if(Of <= 0)
        Of = 0;

    _r->offset = Of;

    if ( _sequence )
    {
        _sequence->handle_widget_change( _r->start, _r->length );
        _sequence->damage( FL_DAMAGE_USER1 );
    }

    timeline->sequence_lock.unlock();
}

void
Sequence_Widget::start_log_nudge (void )
{
    if(!nudge_dirty())
    {
        set_nudge();
        log_start();
    }
}

void
Sequence_Widget::end_log_nudge( void )
{
    if ( nudge_dirty() )
    {
        log_end();
        clear_nudge();
    }
}

/** set position of widget on the timeline. */
void
Sequence_Widget::start ( nframes_t where )
{
    /* this is pretty complicated because of selection and snapping */

    if  ( ! selected() )
    {
        redraw();
        _r->start = where;
    }
    else
    {
        if ( this != Sequence_Widget::_current )
            return;

        /* difference between where we are current and desired position */

        if ( where < _r->start )
        {
            nframes_t d = _r->start - where;

            /* first, make sure we stop at 0 */
            nframes_t m = JACK_MAX_FRAMES;

            /* find the earliest region start point */
            for ( list <Sequence_Widget *>::const_iterator i = _selection.begin(); i != _selection.end(); ++i )
                m = min( m, (*i)->_r->start );

            if ( d > m )
                d = 0;

            for ( list <Sequence_Widget *>::iterator i = _selection.begin(); i != _selection.end(); ++i )
            {
                (*i)->redraw();
                (*i)->_r->start -= d;
            }
        }
        else
        {
            nframes_t d = where - _r->start;

            /* TODO: do like the above and disallow wrapping */
            for ( list <Sequence_Widget *>::iterator i = _selection.begin(); i != _selection.end(); ++i )
            {
                (*i)->redraw();
                (*i)->_r->start += d;
            }
        }
    }
}

void
Sequence_Widget::draw_label ( void )
{
}

void
Sequence_Widget::draw_label ( const char *label, Fl_Align align, Fl_Color color, int xo, int yo )
{
    int X = x();
    int Y = y();
    int W = w();
    int H = h();

    // To adjust the label size based on the track size
    Track *t = timeline->track_under( Y );

    if ( align & FL_ALIGN_CLIP ) fl_push_clip( X, Y, W, H );

    X += xo;
    Y += yo;

    Fl_Label lab;

    lab.color = color;
    lab.type = FL_NORMAL_LABEL;
    lab.value = label;
    lab.font = FL_HELVETICA_ITALIC;

    if (_can_resize_label)
    {
        if(t)
            lab.size = 9 + t->size();
        else
            lab.size = 9;
    }
    else    // Tempo and Time point labels are fixed size
    {
        lab.size = 11;
    }

    int lw = 0, lh = 0;

    fl_font( lab.font, lab.size );
    fl_measure( lab.value, lw, lh );

    int dx = 0;

    /* adjust for scrolling */
    if ( abs_x() < scroll_x() )
        dx = min( 32767, scroll_x() - abs_x() );

    const Fl_Boxtype b = FL_BORDER_BOX;
    const int bx = Fl::box_dx( b );
    const int bw = Fl::box_dw( b );
    const int by = Fl::box_dy( b );
    const int bh = Fl::box_dh( b );

    /* FIXME: why do we have to do this here? why doesn't Fl_Label::draw take care of this stuff? */
    if ( align & FL_ALIGN_INSIDE )
    {
        if ( align & FL_ALIGN_BOTTOM  )
            Y += h() - ( lh + bh );
        else if ( align & FL_ALIGN_TOP )
            Y += by;
        else
            Y += ( h() / 2 ) - ( lh + bh );

        if ( align & FL_ALIGN_RIGHT )
            X += abs_w() - ( lw + bw );
        else if ( align & FL_ALIGN_LEFT )
            X += bx;
        else
            X += ( abs_w() / 2 ) - ( ( lw + bw ) / 2 );

    }
    else
    {
        if ( align & FL_ALIGN_RIGHT )
            X += abs_w();
        else if ( align & FL_ALIGN_LEFT )
            X -= lw + bw;
        else
            X += ( abs_w() / 2 ) - ( ( lw + bw ) / 2 );

        if ( align & FL_ALIGN_BOTTOM  )
            Y += h();
        else if ( align & FL_ALIGN_TOP )
            Y -= lh + bh;
        else
            Y += ( h() / 2 ) - ( ( lh + bh ) / 2 );
    }
#if defined(FLTK_SUPPORT) || defined (FLTK14_SUPPORT)
    fl_draw_box( b, ( X - dx ), Y - by, lw + bw, lh + bh, FL_DARK1  );
#else
    fl_draw_box( b, ( X - dx ), Y - by, lw + bw, lh + bh, fl_color_add_alpha( FL_DARK1, 150 )  );
#endif

    fl_color( color );

    fl_draw( label, ( X - dx ), Y, lw + bw, lh, (Fl_Align)(FL_ALIGN_CENTER) );

    if ( align & FL_ALIGN_CLIP ) fl_pop_clip();
}

int
Sequence_Widget::dispatch ( int m )
{
    Sequence_Widget::_current = this;

    if ( selected() )
    {
        Loggable::block_start();

        int r = 0;

        for ( list <Sequence_Widget *>::iterator i = _selection.begin(); i != _selection.end(); ++i )
            if ( *i != this )
                r |= (*i)->handle( m );

        r |= handle( m );

        Loggable::block_end();

        return r;
    }
    else
        return handle( m );
}

void
Sequence_Widget::draw ( void )
{
    draw_box();
}

void
Sequence_Widget::draw_box ( void )
{
    fl_draw_box( box(), x(), y(), w(), h(), selected() ? FL_MAGENTA : _box_color );
}

#include "../../FL/test_press.H"

/* base hanlde just does basic dragging */
int
Sequence_Widget::handle ( int m )
{
    int X = Fl::event_x();
    int Y = Fl::event_y();

    if ( !active_r() )
        /* don't mess with anything while recording... */
        return 0;

    Logger _log( this );

    switch ( m )
    {
        case FL_ENTER:
            fl_cursor( FL_CURSOR_HAND );
            return 1;
        case FL_LEAVE:
            //            DMESSAGE( "leave" );
            fl_cursor( sequence()->cursor() );
            return 1;
        case FL_PUSH:
        {
            /* deletion */
            if ( test_press( FL_BUTTON3 + FL_CTRL ) )
            {
                remove();

                return 1;
            }
            else if ( test_press( FL_BUTTON1 ) || test_press( FL_BUTTON1 + FL_CTRL ) || test_press( FL_BUTTON1 + FL_ALT ) )
            {
                /* traditional selection model */
                if ( Fl::event_ctrl() )
                    select();

                fl_cursor( FL_CURSOR_MOVE );

                /* movement drag */
                return 1;
            }

            return 0;
        }
        case FL_RELEASE:

            if ( _drag )
            {
                end_drag();
                _log.release();
            }

            fl_cursor( FL_CURSOR_HAND );

            return 1;
        case FL_DRAG:
        {
            Fl::event_key( 0 );

            if ( ! _drag )
            {
                begin_drag ( Drag( X, Y, x_to_offset( X ), start() ) );
                _log.hold();
            }

            if ( test_press( FL_BUTTON1 + FL_CTRL ) && ! _drag->state )
            {
                /* duplication */
                timeline->sequence_lock.wrlock();
                DMESSAGE("SW original = %p", this);
                this->clone();  // This will add
                timeline->sequence_lock.unlock();

                _drag->state = 1;
                return 1;
            }
            else if ( test_press( FL_BUTTON1 ) || test_press( FL_BUTTON1 + FL_CTRL ) )
            {
                redraw();

                const nframes_t of = timeline->x_to_offset( X );

                int64_t s = (int64_t)of - _drag->offset;

                if ( s < 0 )
                    s = 0;

                start(s);

                if ( Sequence_Widget::_current == this )
                    sequence()->snap( this );

                if ( X >= sequence()->x() + sequence()->w() ||
                    X <= sequence()->drawable_x() )
                {
                    /* this drag needs to scroll */

                    int64_t pos = s - ( _drag->mouse_offset - _drag->offset );

                    if ( X > sequence()->x() + sequence()->w() )
                        pos -= timeline->x_to_ts( sequence()->drawable_w() );

                    if ( s == 0 )
                        pos = 0;

                    if ( pos < 0 )
                        pos = 0;

                    timeline->xposition(timeline->ts_to_x(pos));

                    /* timeline->redraw();  */
                    sequence()->damage( FL_DAMAGE_USER1 );
                }

                if ( ! selected() || _selection.size() == 1 )
                {
                    /* track jumping */
                    if ( Y > _sequence->y() + _sequence->h() || Y < _sequence->y() )
                    {
                        Track *t = timeline->track_under( Y );

                        fl_cursor( FL_CURSOR_HAND );

                        if ( t )
                            t->handle( FL_ENTER );

                        return 0;
                    }
                }

                return 1;
            }
            else if(test_press( FL_BUTTON1 + FL_ALT ))
            {
                // Control point fixed horizontal axis & aligned vertical with vertical dragging
                return 1;
            }
            else
            {
                DMESSAGE( "unknown" );
                return 0;
            }
        }
        default:
            return 0;
    }
}

/**********/
/* Public */
/**********/

/** add this widget to the selection */
void
Sequence_Widget::select ( void )
{
    if ( selected() )
        return;

    _selection.push_back( this );
    _selection.sort( sort_func );

    redraw();
}

/** remove this widget from the selection */
void
Sequence_Widget::deselect ( void )
{
    _selection.remove( this );
    redraw();
}

bool
Sequence_Widget::selected ( void ) const
{
    return std::find( _selection.begin(), _selection.end(), this ) != _selection.end();
}

/** remove this widget from its sequence */
void
Sequence_Widget::remove ( void )
{
    redraw();
    sequence()->queue_delete( this );
}

void
Sequence_Widget::delete_selected ( void )
{
    Loggable::block_start();

    while ( _selection.size() )
        delete _selection.front();

    Loggable::block_end();
}

void
Sequence_Widget::select_none ( void )
{
    Loggable::block_start();

    while ( _selection.size() )
    {
        Sequence_Widget *w = _selection.front();

        w->log_start();

        _selection.front()->redraw();
        _selection.pop_front();

        w->log_end();
    }

    Loggable::block_end();
}
