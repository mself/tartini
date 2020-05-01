/***************************************************************************
                          hstackwidget.h  -  description
                             -------------------
    begin                : Mon Jan 10 2005
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
#ifndef HSTACKWIDGET_H
#define HSTACKWIDGET_H

#include "drawwidget.h"
#include <QPixmap>
#include <QPaintEvent>

// Forward declarations of classes the h file doesn't need to know specifics about
class QPixmap;

class HStackWidget : public DrawWidget
{
  Q_OBJECT

  public:
    HStackWidget(QWidget * p_parent);
    virtual ~HStackWidget();

    void paintEvent(QPaintEvent *);

  public slots:
    void setWindowSize(double);
    void setDBRange(double);

 signals:
    void windowSizeChanged(double);
    void dBOffsetChanged(double);
    void dBRangeChanged(double);

 private:
    std::string format_label(float p_label);

    QPixmap * m_buffer;
    int m_window_size;
    float m_view_height;
    float m_top;
};
#endif // HSTACKWIDGET_H
// EOF

