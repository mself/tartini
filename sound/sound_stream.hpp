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

#include "sound_stream.h"

//------------------------------------------------------------------------------
SoundStream::SoundStream(void):
  mode(F_NONE)
{
}

//------------------------------------------------------------------------------
SoundStream::~SoundStream(void)
{
}

//------------------------------------------------------------------------------
int SoundStream::sample_size(void) const
{
 return (bits + 7) / 8;
}

//------------------------------------------------------------------------------
int SoundStream::frame_size(void) const
{
  return sample_size() * channels;
}

//------------------------------------------------------------------------------
long SoundStream::wait_bytes(long /*length*/)
{
  return 0;
}

//------------------------------------------------------------------------------
long SoundStream::wait_frames(long /*length*/)
{
  return 0;
}

//------------------------------------------------------------------------------
void
SoundStream::set_frequency(int p_frequency)
{
    freq = p_frequency;
}

//------------------------------------------------------------------------------
int
SoundStream::get_frequency() const
{
    return freq;
}

//------------------------------------------------------------------------------
void
SoundStream::set_channels(int p_channels)
{
    channels = p_channels;
}

//------------------------------------------------------------------------------
int
SoundStream::get_channels() const
{
    return channels;
}
//EOF
