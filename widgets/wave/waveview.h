/***************************************************************************
                          waveview.h  -  description
                             -------------------
    begin                : Mon Mar 14 2005
    copyright            : (C) 2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
    copyright            : (C) 2016 by Julien Thevenon
    email                : julien_thevenon at yahoo.fr
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#ifndef WAVEVIEW_H
#define WAVEVIEW_H

#include "viewwidget.h"
#include <QResizeEvent>

class WaveWidget;

class WaveView: public ViewWidget
{
  Q_OBJECT

  public:
    WaveView(int p_view_ID
            ,QWidget * p_parent = nullptr
            );

    virtual ~WaveView();

    void resizeEvent(QResizeEvent *);

    QSize sizeHint() const;

  private:
    WaveWidget * m_wave_widget;
};
#endif // WAVEVIEW_H
// EOF
