
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

#include "Sequence.H"
#include "Timeline.H"

#include <FL/fl_draw.H>

#include "Track.H"

#include "../../FL/event_name.H"

#include "Transport.H" // for locate()

#include "const.h"
#include "../../nonlib/debug.h"
#include "Control_Point.H"

using namespace std;

class Control_Point;

extern bool g_snapshot;

queue <Sequence_Widget *> Sequence::_delete_queue;

Sequence::Sequence ( Track *track, const char *name ) : Fl_Group( 0, 0, 0, 0 ), Loggable( true  )
{
    init();

    _track = track;

    if ( name )
        _name = strdup( name );

    //    log_create();
}

Sequence::Sequence ( int X, int Y, int W, int H ) : Fl_Group( X, Y, W, H ), Loggable( false )
{
    init();
}

void
Sequence::init ( void )
{
    _track = NULL;

    _name = NULL;

    box( FL_FLAT_BOX );
    color(  FL_BACKGROUND_COLOR );
    align( FL_ALIGN_LEFT );

    end();
    //    clear_visible_focus();
}

Sequence::~Sequence (  )
{
    DMESSAGE( "destroying sequence" );

    if ( _widgets.size() )
        FATAL( "programming error: leaf destructor must call Sequence::clear()!" );

    if ( parent() )
        parent()->remove( this );

    if ( _name )
        free( _name );
}



int
Sequence::drawable_w ( void ) const
{
    return w() - Track::width();
}

int
Sequence::drawable_x ( void ) const
{
    return x() + Track::width();
}

void
Sequence::log_children ( void ) const
{
    if ( id() > 0 )
        log_create();

    for ( std::list <Sequence_Widget * >::const_iterator i = _widgets.begin();
        i != _widgets.end(); ++i )
        (*i)->log_create();
}

/** remove all widgets from this sequence */
void
Sequence::clear ( void )
{
    Loggable::block_start();

    while ( _widgets.size() )
        delete _widgets.front();

    Loggable::block_end();
}

/** given screen pixel coordinate X, return an absolute frame offset into this sequence */
nframes_t
Sequence::x_to_offset ( int X )
{
    return timeline->xoffset + timeline->x_to_ts( X - x() );
}

/** sort the widgets in this sequence by position */
void
Sequence::sort ( void )
{
    _widgets.sort( Sequence_Widget::sort_func );
}

/** return a pointer to the widget that /r/ overlaps, or NULL if none. */
Sequence_Widget *
Sequence::overlaps ( Sequence_Widget *r )
{
    for ( list <Sequence_Widget *>::const_iterator i = _widgets.begin(); i != _widgets.end(); ++i )
    {
        if ( *i == r ) continue;
        if ( (*i)->overlaps( r ) )
            return *i;
    }

    return NULL;
}

void
Sequence::handle_widget_change ( nframes_t /* start */, nframes_t /* length */ )
{
    /* this might be invoked from Capture or GUI thread */
    //    Fl::lock();
    sort();
    timeline->damage_sequence();
    //    Fl::unlock();

    //    timeline->update_length( start + length );
}

Sequence_Widget *
Sequence::widget_at ( nframes_t ts, int Y )
{
    for ( list <Sequence_Widget *>::const_reverse_iterator r = _widgets.rbegin(); r != _widgets.rend(); ++r )
        if ( ts >= (*r)->start() && ts <= (*r)->start() + (*r)->length()
            && Y >= (*r)->y() && Y <= (*r)->y() + (*r)->h() )
        {
            MESSAGE("Y = %d: (*r)->y() = %d", Y, (*r)->y());
            
            return (*r);
        }

    return NULL;
}

/** return a pointer to the widget under the current mouse event, or
 * NULL if no widget intersects the event coordinates */
Sequence_Widget *
Sequence::event_widget ( void )
{
    if ( Fl::event_x() < drawable_x() )
        return NULL;

    nframes_t ets = timeline->xoffset + timeline->x_to_ts( Fl::event_x() - drawable_x() );
    return widget_at( ets, Fl::event_y() );
}

void
Sequence::add ( Sequence_Widget *r )
{
    //    Logger _log( this );

    if ( r->sequence() == this )
    {
        WARNING( "Programming error: attempt to add sequence widget to the same sequence twice" );
        return;
    }

    if ( r->sequence() )
    {
        /* This method can be called from the Capture thread as well as the GUI thread, so we must lock FLTK before redraw */
        r->redraw();
        r->sequence()->remove( r );
    }

    r->sequence( this );

    _widgets.push_back( r );

    handle_widget_change( r->start(), r->length() );
}

void
Sequence::remove ( Sequence_Widget *r )
{
    _widgets.remove( r );

    handle_widget_change( r->start(), r->length() );
}

static nframes_t
abs_diff ( nframes_t n1, nframes_t n2 )
{
    return n1 > n2 ? n1 - n2 : n2 - n1;
}

/** snap widget /r/ to nearest edge */
void
Sequence::snap ( Sequence_Widget *r )
{
    if( Timeline::snap_toggle_bypass )
        return;

    const int snap_pixels = 10;
    const nframes_t snap_frames = timeline->x_to_ts( snap_pixels );

    /* snap to other widgets */
    if ( Timeline::snap_magnetic )
    {
        const int rx1 = r->start();
        const int rx2 = r->start() + r->length();

        for ( list <Sequence_Widget * >::const_iterator i = _widgets.begin(); i != _widgets.end(); ++i )
        {
            const Sequence_Widget *w = (*i);

            if ( w == r )
                continue;

            const int wx1 = w->start();
            const int wx2 = w->start() + w->length();

            if ( abs_diff( rx1, wx2 ) < snap_frames )
            {
                r->start( w->start() + w->length() + 1 );

                return;
            }

            if ( abs_diff( rx2, wx1 ) < snap_frames )
            {
                r->start( ( w->start() - r->length() ) - 1 );

                return;
            }
        }
    }

    nframes_t f = r->start();

    /* snap to beat/bar lines */
    if ( timeline->nearest_line( &f ) )
        r->start( f );
}

void
Sequence::draw_box ( void )
{
    /* draw the box with the ends cut off. */
    Fl_Group::draw_box( box(), x() - Fl::box_dx( box() )  - 1, y(), w() + Fl::box_dw( box() ) + 2, h(), color() );
}

void
Sequence::draw ( void )
{
    if ( damage() & ~FL_DAMAGE_USER1 )
    {
        Fl_Boxtype b = box();
        box( FL_NO_BOX );

        Fl_Group::draw();

        box( b );
    }

    fl_push_clip( drawable_x(), y(), drawable_w(), h() );

    draw_box();

    for ( list <Sequence_Widget *>::const_iterator r = _widgets.begin();  r != _widgets.end(); ++r )
        (*r)->draw_box();

    int X, Y, W, H;

    fl_clip_box( x(), y(), w(), h(), X, Y, W, H );

    timeline->draw_measure_lines( X, Y, W, H );

    // This will draw tempo, annotation labels, so for better visibility, draw them after the measure lines
    for ( list <Sequence_Widget *>::const_iterator r = _widgets.begin();  r != _widgets.end(); ++r )
        (*r)->draw();

    for ( list <Sequence_Widget *>::const_iterator r = _widgets.begin();  r != _widgets.end(); ++r )
        (*r)->draw_label();

    fl_pop_clip();
}

#include "../../FL/test_press.H"

int
Sequence::handle ( int m )
{

    /*     if ( m != FL_NO_EVENT ) */
    /*         DMESSAGE( "%s", event_name( m ) ); */

    switch ( m )
    {
        case FL_KEYBOARD:
        case FL_SHORTCUT:
            if ( Fl::test_shortcut( FL_CTRL + FL_Right ) )
            {
                const Sequence_Widget *w = next( transport->frame );

                if ( w )
                    transport->locate( w->start() );

                return 1;
            }
            else if ( Fl::test_shortcut( FL_CTRL + FL_Left ) )
            {
                const Sequence_Widget *w = prev( transport->frame );

                if ( w )
                    transport->locate( w->start() );

                return 1;
            }
            else if ( Fl::test_shortcut( FL_CTRL + ' ' ) )
            {
                Sequence_Widget *r = widget_at( transport->frame, y() );

                if ( r )
                {
                    if ( r->selected() )
                        r->deselect();
                    else
                        r->select();
                }
            }
            else
            {
                switch ( Fl::event_key() )
                {
                    case FL_Left:
                    case FL_Right:
                    case FL_Up:
                    case FL_Down:
                        /* this is a hack to override FLTK's use of arrow keys for
                         * focus navigation */
                        return timeline->handle_scroll( m );
                    default:
                        break;
                }
            }

            if ( Sequence_Widget::belowmouse() )
                return Sequence_Widget::belowmouse()->dispatch( m );

            break;
        case FL_KEYUP:
        {
            if(Fl::event_key() == FL_Alt_L || Fl::event_key() == FL_Alt_R)    // nudge left/right, up/down
            {
                if(g_snapshot)
                {
                    g_snapshot = false;
                    timeline->nudge_snapshot();
                    return 1;
                }
            }

            return 0;
        }
        case FL_NO_EVENT:
            /* garbage from overlay window */
            return 0;
        case FL_FOCUS:
        case FL_UNFOCUS:
            Fl_Group::handle( m );
            redraw();
            return 1;
        case FL_LEAVE:
            //            DMESSAGE( "leave" );
            fl_cursor( FL_CURSOR_DEFAULT );
            Fl_Group::handle( m );
            return 1;
        case FL_ENTER:
            //            DMESSAGE( "enter" );
            if ( Fl::event_x() >= drawable_x() )
            {
                if ( Sequence_Widget::pushed() )
                {
                    if ( Sequence_Widget::pushed()->sequence()->class_name() == class_name() )
                    {
                        /* accept objects dragged from other sequences of this type */
                        if ( Sequence_Widget::pushed()->sequence() != this )
                        {
                            timeline->sequence_lock.wrlock();
                            add( Sequence_Widget::pushed() );
                            timeline->sequence_lock.unlock();

                            damage( FL_DAMAGE_USER1 );

                            fl_cursor( FL_CURSOR_MOVE );
                        }
                    }
                    else
                        fl_cursor( FL_CURSOR_DEFAULT );
                }
                else if ( ! event_widget() )
                    fl_cursor( cursor() );

                Fl_Group::handle( m );

                return 1;
            }
            else
            {
                return Fl_Group::handle(m);
            }
        case FL_DND_DRAG:
        case FL_DND_ENTER:
        case FL_DND_LEAVE:
        case FL_DND_RELEASE:
            if ( Fl::event_x() >= drawable_x() )
                return 1;
            else
                return 0;
        case FL_MOVE:
        {
            if ( Fl::event_x() >= drawable_x() )
            {
                Sequence_Widget *r = event_widget();

                if ( r != Sequence_Widget::belowmouse() )
                {
                    if ( Sequence_Widget::belowmouse() )
                        Sequence_Widget::belowmouse()->handle( FL_LEAVE );

                    Sequence_Widget::belowmouse( r );

                    if ( r )
                        r->handle( FL_ENTER );
                }
            }

            return 1;
        }
        default:
        {
            Sequence_Widget *r = Sequence_Widget::pushed() ? Sequence_Widget::pushed() : event_widget();

            /*             if ( this == Fl::focus() ) */
            /*                 DMESSAGE( "Sequence widget = %p", r ); */

            if ( m == FL_RELEASE )
            {
                // in the case of track jumping, the sequence widget may not get the FL_RELEASE less we send it here:
                if ( Sequence_Widget::pushed() )
                    Sequence_Widget::pushed()->handle(FL_RELEASE);

                Sequence_Widget::pushed( NULL );
            }

            if ( r )
            {
                int retval = r->dispatch( m );

                /*                 DMESSAGE( "retval = %d", retval ); */

                if ( m == FL_PUSH )
                    take_focus();

                if ( retval )
                {
                    if ( m == FL_PUSH )
                    {
                        if ( Sequence_Widget::pushed() )
                            Sequence_Widget::pushed()->handle( FL_UNFOCUS );

                        Sequence_Widget::pushed( r );

                        r->handle( FL_FOCUS );
                    }
                }

                if ( _delete_queue.size() )
                {
                    Loggable::block_start();

                    while ( _delete_queue.size() )
                    {
                        Sequence_Widget *t = _delete_queue.front();
                        _delete_queue.pop();

                        if ( Sequence_Widget::pushed() == t )
                            Sequence_Widget::pushed( NULL );
                        if ( Sequence_Widget::belowmouse() == t )
                        {
                            Sequence_Widget::belowmouse()->handle( FL_LEAVE );
                            Sequence_Widget::belowmouse( NULL );
                        }

                        timeline->sequence_lock.wrlock();
                        delete t;
                        timeline->sequence_lock.unlock();
                    }

                    Loggable::block_end();
                }

                if ( m == FL_PUSH )
                    return 1;
                else
                    return retval;
            }
            else
            {
                if ( test_press( FL_BUTTON1 ) )
                {
                    /* traditional selection model */
                    Sequence_Widget::select_none();
                }

                return Fl_Group::handle( m );
            }
        }
    }

    return 0;
}

/**********/
/* Public */
/**********/

/** calculate the length of this sequence by looking at the end of the
 * least widget it contains */

/** return the length in frames of this sequence calculated from the
 * right edge of the rightmost widget */
nframes_t
Sequence::length ( void ) const
{
    nframes_t l = 0;

    for ( list <Sequence_Widget *>::const_iterator r = _widgets.begin(); r != _widgets.end(); ++r )
        l = max( l, (*r)->start() + (*r)->length() );

    return l;
}

/** return the location of the next widget from frame /from/ */
const Sequence_Widget *
Sequence::next ( nframes_t from ) const
{
    for ( list <Sequence_Widget * >::const_iterator i = _widgets.begin(); i != _widgets.end(); ++i )
        //            if ( (*i)->start() >= from )
        if ( (*i)->start() > from )
            return *i;

    if ( _widgets.size() )
        return _widgets.back();
    else
        return 0;
}

/** return the location of the next widget from frame /from/ */
const Sequence_Widget *
Sequence::prev ( nframes_t from ) const
{
    for ( list <Sequence_Widget * >::const_reverse_iterator i = _widgets.rbegin(); i != _widgets.rend(); ++i )
        if ( (*i)->start() < from )
            return *i;

    if ( _widgets.size() )
        return _widgets.front();
    else
        return 0;
}

/** delete all selected widgets in this sequence */
void
Sequence::remove_selected ( void )
{
    Loggable::block_start();

    for ( list <Sequence_Widget *>::iterator r = _widgets.begin(); r != _widgets.end(); )
        if ( (*r)->selected() )
        {
            Sequence_Widget *t = *r;
            _widgets.erase( r++ );
            delete t;
        }
        else
            ++r;

    Loggable::block_end();
}

/** select all widgets intersecting with the range defined by the
 * pixel coordinates X through W, and for control widgets Y through H */
void
Sequence::select_range ( int X, int W, int Y, int H )
{
    nframes_t sts = x_to_offset( X );
    nframes_t ets = sts + timeline->x_to_ts( W );

    for ( list <Sequence_Widget *>::const_reverse_iterator r = _widgets.rbegin();  r != _widgets.rend(); ++r )
    {
        if ( ! ( (*r)->start() > ets || (*r)->start() + (*r)->length() < sts ) )
        {
            bool is_control = false;
            if ((Y != -1) && (H != -1)) // Control is only one to send Y & H
                is_control = true;

            if( is_control )
            {
                int y_bottom = Y + H;
                int r_bottom = (*r)->start_y() + (*r)->height();
                int r_y = (*r)->start_y();

                //    DMESSAGE("YYY = %d: Start Y = %d: y_bottom = %d: r_bottom = %d", Y, r_y, y_bottom, r_bottom);

                if( (Y <= r_y &&  (y_bottom >= r_y || y_bottom >= r_bottom ) ) )
                    (*r)->select();
            }
            else
                (*r)->select(); // Audio regions
        }
    }
}

void
Sequence::log_seq_nudges()
{
    for ( list <Sequence_Widget *>::const_reverse_iterator i = _widgets.rbegin();  i != _widgets.rend(); ++i )
    {
        Sequence_Widget *w = (*i);
        w->end_log_nudge();
    }
}

void
Sequence::nudge_selected(bool left)
{
    for ( list <Sequence_Widget *>::const_reverse_iterator i = _widgets.rbegin();  i != _widgets.rend(); ++i )
    {
        Sequence_Widget *w = (*i);
        if (w->selected() )
        {
            w->nudge_some(left);
        }
    }
}

void
Sequence::pan_selected(bool left)
{
    for ( list <Sequence_Widget *>::const_reverse_iterator i = _widgets.rbegin();  i != _widgets.rend(); ++i )
    {
        Sequence_Widget *w = (*i);
        if (w->selected() )
        {
            w->pan_some(left);
        }
    }
}

void
Sequence::log_control_nudges()
{
    for ( list <Sequence_Widget *>::const_reverse_iterator i = _widgets.rbegin();  i != _widgets.rend(); ++i )
    {
        Control_Point *w = (Control_Point*)(*i);
        w->end_log_nudge();
    }
}

void
Sequence::nudge_control_selected_X(bool left)
{
    for ( list <Sequence_Widget *>::const_reverse_iterator i = _widgets.rbegin();  i != _widgets.rend(); ++i )
    {
        Sequence_Widget *w = (*i);
        if (w->selected() )
        {
            w->nudge_some(left);
        }
    }
}

void
Sequence::nudge_control_selected_Y(bool up)
{
    for ( list <Sequence_Widget *>::const_reverse_iterator i = _widgets.rbegin();  i != _widgets.rend(); ++i )
    {
        Control_Point *r = (Control_Point*)(*i);
        if (r->selected() )
        {
            timeline->sequence_lock.wrlock();

            r->start_log_nudge();

            float Y = r->control();

            if(up)
                Y -= 0.01;
            else
                Y += 0.01;

            //    DMESSAGE("Y = %f", Y);
            /* This will allow the full range, but by doing so, you cannot enter
               the control widget with the mouse and grab it if at max on the bottom.
               Was originally limited to 0.99, but we are going with the full range
               with nudging since you cannot set it to max bottom from dragging,
               So you must use nudging to move the control point up from max bottom if
               you do max it.*/
            if ( Y >= 1.00 )
                Y = 1.00;

            if( Y <= 0.00 )
                Y = 0.00;

            r->control(Y);
            r->redraw();

            timeline->sequence_lock.unlock();
        }
    }
}
