/***************************************************************************
                          samplewidget.cpp  -  description
                             -------------------
    begin                : May 18 2005
    copyright            : (C) 2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#include <QPixmap>
#include <QPainter>

#include "samplewidget.h"
#include "gdata.h"
#include "channel.h"
#include "analysisdata.h"
#include "useful.h"

//------------------------------------------------------------------------------
SampleWidget::SampleWidget(QWidget * p_parent)
: DrawWidget(p_parent, "SampleWidget")
{
}

//------------------------------------------------------------------------------
SampleWidget::~SampleWidget()
{
}

//------------------------------------------------------------------------------
void SampleWidget::paintEvent( QPaintEvent * )
{
    // Channel * l_active_channel = GData::getUniqueInstance().getActiveChannel();

    beginDrawing();

    // Drawing code goes here
    get_painter().drawLine(0, 0, width(), height());

    endDrawing();
}

//------------------------------------------------------------------------------
QSize SampleWidget::sizeHint() const
{
    return QSize(300, 200);
}

// EOF
