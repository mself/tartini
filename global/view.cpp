/***************************************************************************
                          view.cpp  -  description
                             -------------------
    begin                : Mon Jul 26 2004
    copyright            : (C) 2004 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
    copyright            : (C) 2016 by Julien Thevenon
    email                : julien_thevenon at yahoo.fr
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
***************************************************************************/

#include "view.h"

#include "useful.h"
#include "gdata.h"
#include "channel.h"
#include "conversions.h"

//------------------------------------------------------------------------------
View::View(void)
: m_current_time(1.0) //to force a change in the setCurrentTime call
, m_view_bottom(0.0)
, m_view_offset(0.0) //to force a change in the setViewOffset call
, m_log_zoom_X(0.0)
, m_log_zoom_Y(0.0)
, m_pixel_height(0) //to force a change in the setPixelHeight call
, m_pixel_width(0) //to force a change in the setPixelWidth call
, m_zoom_X(0.0)
, m_zoom_Y(0.0)
, m_auto_follow(gdata->getSettingsValue("View/autoFollow", true))
, m_background_shading(gdata->getSettingsValue("View/backgroundShading", true))
, m_fast_update_timer(new QTimer(this))
, m_slow_update_timer(new QTimer(this))
, m_need_slow_update(false)
, m_need_fast_update(false)
{
    setCurrentTime(0.0); //in seconds

    setLogZoomX(1.0);
    setLogZoomY(1.0);
    setPixelWidth(400);
    setPixelHeight(350);
    setZoomFactorY(3.2);
    setViewBottom(62.0); //the lowest note visible (in semitones) from C0

    connect(m_fast_update_timer, SIGNAL(timeout()), this, SLOT(nextFastUpdate()));
    connect(m_slow_update_timer, SIGNAL(timeout()), this, SLOT(nextSlowUpdate()));
    connect(this, SIGNAL(viewChanged()), this, SLOT(newUpdate()));
}

//------------------------------------------------------------------------------
View::~View(void)
{
    delete m_fast_update_timer;
    delete m_slow_update_timer;
}

// please call this after a window has been created
//------------------------------------------------------------------------------
void View::init(void)
{
    setViewOffset(viewWidth() / 2.0);
}

//------------------------------------------------------------------------------
void View::doUpdate(void)
{
    doSlowUpdate();
    doFastUpdate();
}

//------------------------------------------------------------------------------
void View::doSlowUpdate(void)
{
    m_need_slow_update = false;
    if(!m_slow_update_timer->isActive())
    {
        m_slow_update_timer->start(gdata->slowUpdateSpeed());
    }
    emit onSlowUpdate(m_current_time);
}

//------------------------------------------------------------------------------
void View::doFastUpdate(void)
{
    m_need_fast_update = false;
    if(!m_fast_update_timer->isActive())
    {
        m_fast_update_timer->start(gdata->fastUpdateSpeed());
    }
    emit onFastUpdate(m_current_time);
}

//------------------------------------------------------------------------------
void View::newUpdate(void)
{
    if(m_slow_update_timer->isActive())
    {
        m_need_slow_update = true;
    }
    else
    {
        m_need_slow_update = false;
        m_slow_update_timer->start(gdata->slowUpdateSpeed());
        emit onSlowUpdate(m_current_time);
    }
    if(m_fast_update_timer->isActive())
    {
        m_need_fast_update = true;
    }
    else
    {
        m_need_fast_update = false;
        m_fast_update_timer->start(gdata->fastUpdateSpeed());
        emit onFastUpdate(m_current_time);
    }
}

//------------------------------------------------------------------------------
void View::nextFastUpdate(void)
{
    if(m_need_fast_update)
    {
        m_need_fast_update = false;
        m_fast_update_timer->start(gdata->fastUpdateSpeed());
        emit onFastUpdate(m_current_time);
    }
}

//------------------------------------------------------------------------------
void View::nextSlowUpdate(void)
{
    if(m_need_slow_update)
    {
        m_need_slow_update = false;
        m_slow_update_timer->start(gdata->slowUpdateSpeed());
        emit onSlowUpdate(m_current_time);
    }
}

//------------------------------------------------------------------------------
void View::setCurrentTimeRaw(double p_x)
{
    if(p_x != m_current_time)
    {
        Channel * l_active_channel = gdata->getActiveChannel();
        if(l_active_channel)
        {
            p_x = l_active_channel->timeAtChunk(l_active_channel->chunkAtTime(p_x)); //align time to an integer sample step
        }
        m_current_time = p_x;
    }
}

//------------------------------------------------------------------------------
void View::setCurrentTime(double p_x)
{
    if(p_x != m_current_time)
    {
        Channel * l_active_channel = gdata->getActiveChannel();
        if(l_active_channel)
        {
            p_x = l_active_channel->timeAtChunk(l_active_channel->chunkAtTime(p_x)); //align time to an integer sample step
        }

        m_current_time = p_x;
        emit currentTimeChanged(p_x);
        emit timeViewRangeChanged(viewLeft(), viewRight());
        emit viewChanged();
    }
}

//------------------------------------------------------------------------------
void View::setViewOffset(double p_x)
{
    if(p_x < 0)
    {
        p_x = 0;
    }

    if(p_x > viewWidth())
    {
        p_x = viewWidth();
    }
    if(p_x != m_view_offset)
    {
        m_view_offset = p_x;
        emit timeViewRangeChanged(viewLeft(), viewRight());
        emit viewChanged();
    }
}

//------------------------------------------------------------------------------
void View::setViewBottomRaw(const double & p_y)
{
    if(p_y != m_view_bottom)
    {
        m_view_bottom = p_y;
        emit viewBottomChanged(gdata->topPitch() - viewHeight() - p_y);
    }
}

//------------------------------------------------------------------------------
void View::setViewBottom(const double & p_y)
{
    if(p_y != m_view_bottom)
    {
        m_view_bottom = p_y;
        emit viewBottomChanged(gdata->topPitch() - viewHeight() - p_y);
        emit viewChanged();
    }
}


// Changes the view without using a step value
//------------------------------------------------------------------------------
void View::changeViewX(const double & p_x)
{
    setCurrentTime(p_x);
    emit viewChanged();
}

//------------------------------------------------------------------------------
void View::changeViewY(const double & p_y)
{
    setViewBottom(gdata->topPitch() - viewHeight() - p_y);
    emit viewChanged();
}

//------------------------------------------------------------------------------
void View::setPixelHeight(int p_h)
{
    if(p_h != m_pixel_height)
    {
        m_pixel_height = p_h;
        if(viewHeight() > gdata->topPitch())
        {
            setLogZoomY(log(double(m_pixel_height) / gdata->topPitch()));
            emit logZoomYChanged(logZoomY());
        }
        emit scrollableYChanged(gdata->topPitch() - viewHeight());
        emit viewHeightChanged(viewHeight());
    }
}

//------------------------------------------------------------------------------
void View::setPixelWidth(int p_w)
{
    if(p_w != m_pixel_width)
    {
        m_pixel_width = p_w;
        if(viewWidth() > gdata->totalTime() * 2.0)
        {
            setLogZoomX(log(double(m_pixel_width) / (gdata->totalTime() * 2.0)));
            emit logZoomXChanged(logZoomX());
        }
        emit viewWidthChanged(viewWidth());
    }
}

//------------------------------------------------------------------------------
void View::setZoomFactorX(const double & p_x)
{
    if(p_x != logZoomX())
    {
        double l_old_view_width = viewWidth();
        setLogZoomX(p_x);
        emit logZoomXChanged(logZoomX());
        emit viewWidthChanged(viewWidth());
    
        setViewOffset(viewOffset() * (viewWidth() / l_old_view_width));
    }
}

//------------------------------------------------------------------------------
void View::setZoomFactorX( const double & p_x
                         , int p_fixed_x
                         )
{
    if(p_x != logZoomX())
    {
        double l_fixed_time = viewLeft() + zoomX() * double(p_fixed_x);
        double l_old_view_width = viewWidth();
        setLogZoomX(p_x);
        emit logZoomXChanged(logZoomX());
        emit viewWidthChanged(viewWidth());
    
        double l_ratio = viewWidth() / l_old_view_width;
        setViewOffset(viewOffset() * l_ratio);
    
        //shift the current time to keep the keep the time fixed at the mouse pointer
        double l_new_time = viewLeft() + zoomX() * double(p_fixed_x);
        gdata->updateActiveChunkTime(currentTime() - (l_new_time - l_fixed_time));
    }
}

//------------------------------------------------------------------------------
void View::setZoomFactorY(const double & p_y)
{
    if(p_y != logZoomY())
    {
        double l_prev_center_Y = viewBottom() + viewHeight() / 2.0;
        setLogZoomY(p_y);
        emit logZoomYChanged(logZoomY());
        emit scrollableYChanged(bound(gdata->topPitch() - viewHeight(), 0.0, gdata->topPitch()));
        emit viewHeightChanged(viewHeight());
    
        setViewBottom(l_prev_center_Y - viewHeight() / 2.0);
        emit viewBottomChanged(gdata->topPitch() - viewHeight() - viewBottom());
    }
}

//------------------------------------------------------------------------------
void View::setZoomFactorY( const double & p_y
                         , int p_fixed_y
                         )
{
    if(p_y != logZoomY())
    {
        double l_fixed_note = viewBottom() + zoomY() * p_fixed_y;
        setLogZoomY(p_y);
        emit logZoomYChanged(logZoomY());
        emit scrollableYChanged(bound(gdata->topPitch() - viewHeight(), 0.0, gdata->topPitch()));
        emit viewHeightChanged(viewHeight());
    
        double l_new_note = viewBottom() + zoomY() * p_fixed_y;
        setViewBottom(viewBottom() - (l_new_note - l_fixed_note));
        emit viewBottomChanged(gdata->topPitch() - viewHeight() - viewBottom());
    }
}

//------------------------------------------------------------------------------
void View::doAutoFollowing(void)
{
    if(!autoFollow())
    {
        return;
    }
    double l_time = currentTime();
    // We want the average note value for the time period currentTime to viewRight
 
    Channel * l_active_channel = gdata->getActiveChannel();

    if(l_active_channel == NULL)
    {
        return;
    }
  

    double l_start_time = l_time - (viewWidth() / 8.0);
    double l_stop_time = l_time + (viewWidth() / 8.0);
  
    int l_start_frame = bound(toInt(l_start_time / l_active_channel->timePerChunk()), 0, l_active_channel->totalChunks() - 1);
    int l_stop_frame = bound(toInt(l_stop_time / l_active_channel->timePerChunk()), 0, l_active_channel->totalChunks() - 1);

    float l_pitch = l_active_channel->averagePitch(l_start_frame, l_stop_frame);

    if(l_pitch < 0)
    {
        return; // There were no good notes detected
    }

    float l_new_bottom = l_pitch - (viewHeight() / 2.0);

    setViewBottom(l_new_bottom);
}

//------------------------------------------------------------------------------
void View::setAutoFollow(bool p_is_checked)
{
    m_auto_follow = p_is_checked;
    gdata->setSettingsValue("View/autoFollow", p_is_checked);
}

//------------------------------------------------------------------------------
void View::setBackgroundShading(bool p_is_checked)
{
    m_background_shading = p_is_checked;
    gdata->setSettingsValue("View/backgroundShading", p_is_checked);
    emit onSlowUpdate(m_current_time);
}
