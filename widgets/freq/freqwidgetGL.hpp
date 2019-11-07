/*    This file is part of Tartini
      Copyright (C) 2016  Julien Thevenon ( julien_thevenon at yahoo.fr )
      Copyright (C) 2004 by Philip McLeod ( pmcleod@cs.otago.ac.nz )

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

//------------------------------------------------------------------------------
double FreqWidgetGL::leftTime() const
{
    return g_data->getView().viewLeft();
}

//------------------------------------------------------------------------------
double FreqWidgetGL::rightTime() const
{
    return g_data->getView().viewRight();
}

//------------------------------------------------------------------------------
double FreqWidgetGL::timeWidth() const
{
    return g_data->getView().viewTotalTime();
}

//------------------------------------------------------------------------------
QSize FreqWidgetGL::sizeHint() const
{
    return QSize(400, 350);
}
//EOF
