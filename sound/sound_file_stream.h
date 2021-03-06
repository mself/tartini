/***************************************************************************
                          sound_file_stream.h  -  description
                             -------------------
    begin                : Sat Jul 10 2004
    copyright            : (C) 2004-2005 by Philip McLeod
    email                : pmcleod@pmcleod.otago.ac.nz
    copyright            : (C) 2016 by Julien Thevenon
    email                : julien_thevenon at yahoo.fr
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#ifndef SOUND_FILE_STREAM_H
#define SOUND_FILE_STREAM_H

#include "sound_stream.h"
#include <stdio.h>

class SoundFileStream : public SoundStream
{
  public:

    inline SoundFileStream();
    inline ~SoundFileStream() override;

    inline int data_length() const;
    inline int totalFrames() const;
    inline int pos() const;
  
    virtual int open_read(const std::string & /*filename*/) = 0;

    virtual int open_write( const std::string & /*filename*/
                          , int /*freq_*/ = 44100
                          , int /*channels_*/ = 2
                          , int /*bits_*/ = 16
                           ) = 0;

    virtual void close() = 0;

    virtual void jump_to_frame(int /*frame*/) = 0;
    virtual void jump_back(int /*frames*/) = 0;
    virtual void jump_forward(int /*frames*/) = 0;

  protected:

    inline void setPos(int p_pos);
    inline void set_total_frames(int p_total_frames);
    inline int get_pos() const;

  private:

    int m_total_frames;
    int m_pos; //in frames
};

#include "sound_file_stream.hpp"

#endif
