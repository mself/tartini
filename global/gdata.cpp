/***************************************************************************
                          gdata.cpp  -  
                             -------------------
    begin                : 2003
    copyright            : (C) 2003-2005 by Philip McLeod
    email                : pmcleod@cs.otago.ac.nz
    copyright            : (C) 2016 by Julien Thevenon
    email                : julien_thevenon at yahoo.fr
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   Please read LICENSE.txt for details.
 ***************************************************************************/
#include <q3filedialog.h>
#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
#include <unistd.h>
#endif

#include <qpixmap.h>
#include <QMessageBox>
#include <qfile.h>
#include <stdio.h>

#include "gdata.h"
#include "wave_stream.h"
#ifdef USE_SOX
#include "sox_stream.h"
#else
#if USE_OGG
#include "ogg_stream.h"
#endif
#endif
#include "audio_stream.h"
#include "Filter.h"
#include "mystring.h"
#include "q3listview.h"
#include "soundfile.h"
#include "channel.h"
#include "tartinisettingsdialog.h"
#include "savedialog.h"
#include "conversions.h"
#include "musicnotes.h"

#ifndef WINDOWS
//for multi-threaded profiling
struct itimerval g_profiler_value;
struct itimerval g_profiler_ovalue;
#endif

int g_frame_window_sizes[NUM_WIN_SIZES] =
  { 512
  , 1024
  , 2048
  , 4096
  , 8192
  };

const char * g_frame_window_strings[NUM_WIN_SIZES] =
  { "512"
  , "1024"
  , "2048"
  , "4096"
  , "8192"
  };

float g_step_sizes[NUM_STEP_SIZES] =
  { 1.0f
  , 0.5f
  , 0.25f
  , 0.2f
  , 0.1f
  , 0.05f
  };

const char * g_step_size_strings[NUM_STEP_SIZES] =
  { "100%"
  , "50%"
  , "25%"
  , "20%"
  , "10%"
  , "5%"
  };

const char * g_pitch_method_strings[NUM_PITCH_METHODS] =
  { "FFT interpolation"
  , "Fast-correlation"
  , "Correlation (squared error) 1"
  , "Correlation (squared error) 2"
  , "Correlation (abs error) 1"
  , "Correlation (abs error) 2"
  , "Correlation (multiplied) 1"
  , "Correlation (multiplied) 2"
  };

GData * gdata = NULL;

//Define the Phase function. This one is applicable to 
//accelerating sources since the phase goes as x^2.
//------------------------------------------------------------------------------
float phase_function(float x)
{
    float phase;
    //phase = x*x;
    phase = x;
    return(phase);
}

//------------------------------------------------------------------------------
GData::GData(void)
: m_settings(NULL)
, m_sound_mode(SOUND_PLAY)
, m_audio_stream(NULL)
, m_need_update(false)
, m_running(STREAM_STOP)
, m_next_color_index(0)
, m_view(NULL)
, m_active_channel(NULL)
, m_doing_harmonic_analysis(false)
, m_doing_freq_analysis(false)
, m_doing_equal_loudness(false)
, m_doing_auto_noise_floor(false)
, m_doing_active_analysis(false)
, m_doing_active_FFT(false)
, m_doing_active_cepstrum(false)
, m_fast_update_speed(0)
, m_slow_update_speed(0)
, m_polish(true)
, m_show_mean_variance_bars(false)
, m_saving_mode(0)
, m_vibrato_sine_style(false)
, m_music_key_type(0) //ALL_NOTES
, m_tempered_type(0) //EVEN_TEMPERED
, m_mouse_wheel_zooms(false)
, m_freq_A(440)
, m_semitone_offset(0.0)
, m_amplitude_mode(0)
, m_pitch_contour_mode(0)
, m_analysis_type(0)
, m_dB_floor(-150.0)
, m_drawing_buffer(new QPixmap(1, 1))
, m_left_time(1.0) // Put a dummy value so that setLeftTime will be completely executed
, m_right_time(0.0) // Put a dummy value so that setRightTime will be completely executed
, m_top_pitch(128.0)
{
    setLeftTime(0.0);
    setRightTime(5.0);

    m_amp_thresholds[AMPLITUDE_RMS][0]           = -85.0;
    m_amp_thresholds[AMPLITUDE_RMS][1]           = -0.0;

    m_amp_thresholds[AMPLITUDE_MAX_INTENSITY][0] = -30.0;
    m_amp_thresholds[AMPLITUDE_MAX_INTENSITY][1] = -20.0;

    m_amp_thresholds[AMPLITUDE_CORRELATION][0]   =  0.40;
    m_amp_thresholds[AMPLITUDE_CORRELATION][1]   =  1.00;

    m_amp_thresholds[FREQ_CHANGENESS][0]         =  0.50;
    m_amp_thresholds[FREQ_CHANGENESS][1]         =  0.02;

    m_amp_thresholds[DELTA_FREQ_CENTROID][0]     =  0.00;
    m_amp_thresholds[DELTA_FREQ_CENTROID][1]     =  0.10;

    m_amp_thresholds[NOTE_SCORE][0]              =  0.03;
    m_amp_thresholds[NOTE_SCORE][1]              =  0.20;

    m_amp_thresholds[NOTE_CHANGE_SCORE][0]       =  0.12;
    m_amp_thresholds[NOTE_CHANGE_SCORE][1]       =  0.30;

    m_amp_weights[0] = 0.2;
    m_amp_weights[1] = 0.2;
    m_amp_weights[2] = 0.2;
    m_amp_weights[3] = 0.2;
    m_amp_weights[4] = 0.2;

    m_settings = new QSettings("cs.otago.ac.nz", TARTINI_NAME_STR);
    TartiniSettingsDialog::setUnknownsToDefault(*this);

    updateQuickRefSettings();

    m_line_color.push_back(Qt::darkRed);
    m_line_color.push_back(Qt::darkGreen);
    m_line_color.push_back(Qt::darkBlue);
    m_line_color.push_back(Qt::darkCyan);
    m_line_color.push_back(Qt::darkMagenta);
    m_line_color.push_back(Qt::darkYellow);
    m_line_color.push_back(Qt::darkGray);

    initMusicStuff();
}

//------------------------------------------------------------------------------
GData::~GData(void)
{
    m_audio_thread.stopAndWait();

    m_settings->sync();

    //Note: The m_sound_files is responsible for cleaning up the data the channels point to
    m_channels.clear();
    for(uint j = 0; j < m_sound_files.size(); j++)
    {
        delete m_sound_files[j];
    }
    m_sound_files.clear();
      
    std::vector<Filter *>::iterator fi;
    for(fi = m_filter_hp.begin(); fi != m_filter_hp.end(); ++fi)
    {
        delete (*fi);
    }
    m_filter_hp.clear();
    for(fi = m_filter_lp.begin(); fi != m_filter_lp.end(); ++fi)
    {
        delete (*fi);
    }

    m_filter_lp.clear();

    delete m_settings;

    delete m_drawing_buffer;
}

//------------------------------------------------------------------------------
SoundFile * GData::getActiveSoundFile(void)
{
    return (m_active_channel) ? m_active_channel->getParent() : NULL;
}

//------------------------------------------------------------------------------
void GData::pauseSound(void)
{
    if(m_running == STREAM_FORWARD)
    {
        m_running = STREAM_PAUSE;
    }
    else if(m_running == STREAM_PAUSE)
    {
        m_running = STREAM_FORWARD;
    }
}

/**
   Opens the soundcard for recording.
   @param p_sound_file_rec A new created blank SoundFile to recort to
   @param p_soundfile_play A SoundFile to play when recording or NULL for record only
   @return true if success
*/
bool GData::openPlayRecord( SoundFile * p_sound_file_rec
                          , SoundFile * p_soundfile_play
                          )
{
    int theOpenMode;

    stop();

    int rate = p_sound_file_rec->rate();
    int numChannels = p_sound_file_rec->numChannels();
    int bits = p_sound_file_rec->bits();
    int theBufferSize = p_sound_file_rec->framesPerChunk();

    if(p_soundfile_play)
    {
        m_sound_mode = SOUND_PLAY_REC;
        theOpenMode = F_RDWR;
        p_soundfile_play->jumpToChunk(0);
        gdata->m_view->setCurrentTime(0);
    }
    else
    {
        m_sound_mode = SOUND_REC;
        theOpenMode = F_READ;
    }

    //open the audio input
    m_audio_stream = new AudioStream;
    if(m_audio_stream->open(theOpenMode, rate, numChannels, bits, theBufferSize))
    {
        fprintf(stderr, "Error initilizing sound\n");
        delete m_audio_stream;
        m_audio_stream = NULL;
        return false;
    }
    else
    {
        if((theOpenMode & F_WRITE))
        {
            m_audio_thread.start(p_soundfile_play, p_sound_file_rec);
        }
        else
        {
            m_audio_thread.start(NULL, p_sound_file_rec);
        }
    }
    return true;
}

/**
 * Starts playing the SoundFile s
 * @return true if the sound started playing (or is already playing)
*/
bool GData::playSound(SoundFile * p_sound_file)
{
    if(p_sound_file == NULL)
    {
        return false;
    }
    if(m_running == STREAM_PAUSE)
    {
        m_running = STREAM_FORWARD;
        return true;
    }
    if(m_running != STREAM_STOP)
    {
        return true;
    }

    stop();
  
    int chunk = p_sound_file->currentChunk();
    if(chunk < 0 || chunk+1 >= p_sound_file->totalChunks())
    {
        p_sound_file->jumpToChunk(0); /*< If at the end of the file reset to the start */
    }

    m_sound_mode = SOUND_PLAY;

    if(!m_audio_stream)
    {
        m_audio_stream = new AudioStream;
        if(m_audio_stream->open(F_WRITE, p_sound_file->rate(), p_sound_file->numChannels(), p_sound_file->bits(), p_sound_file->bufferSize() / 2))
        {
            fprintf(stderr, "Error initialising sound\n");
            delete m_audio_stream;
            m_audio_stream = NULL;
            g_main_window->message("Error opening sound device. Another program might be using it", 2000);
        }
        m_audio_thread.start(p_sound_file, NULL);
    }
    return true;
}

//------------------------------------------------------------------------------
void GData::updateViewLeftRightTimes(void)
{
    double left = 0.0; //in seconds
    double right = 0.0; //in seconds
    Channel *ch;
    for(uint j = 0; j < m_channels.size(); j++)
    {
        ch = m_channels.at(j);
        if(ch->startTime() < left)
        {
            left = ch->startTime();
        }
        if(ch->finishTime() > right)
        {
            right = ch->finishTime();
        }
    }
    setLeftTime(left); //in seconds
    setRightTime(right); //in seconds
}

//------------------------------------------------------------------------------
void GData::setActiveChannel(Channel * p_to_active)
{
    m_active_channel = p_to_active;
    if(m_active_channel)
    {
        updateActiveChunkTime(m_view->currentTime());
    }
    emit activeChannelChanged(m_active_channel);
    emit activeIntThresholdChanged(getActiveIntThreshold());
    m_view->doUpdate();
}

//------------------------------------------------------------------------------
void GData::updateActiveChunkTime(double p_time)
{
    if((m_running != STREAM_STOP) && (m_sound_mode & SOUND_REC))
    {
        return;
    }

    Channel *active = getActiveChannel();
    p_time = bound(p_time, leftTime(), rightTime());
    if(active)
    {
        active->jumpToTime(p_time);
        if(gdata->doingActive())
        {
            active->lock();
            active->processChunk(active->currentChunk());
            active->unlock();
        }
    }
    m_view->setCurrentTime(p_time);
    doChunkUpdate();
}

//------------------------------------------------------------------------------
void GData::setLeftTime(double p_x)
{
    if(p_x != m_left_time)
    {
        m_left_time = p_x;
        emit timeRangeChanged(leftTime(), rightTime());
        emit leftTimeChanged(leftTime());
    }
}

//------------------------------------------------------------------------------
void GData::setRightTime(double p_x)
{
    if(p_x != m_right_time)
    {
        m_right_time = p_x;
        emit timeRangeChanged(leftTime(), rightTime());
        emit rightTimeChanged(rightTime());
    }
}

//------------------------------------------------------------------------------
void GData::setTopPitch(double p_y)
{
    if(p_y != m_top_pitch)
    {
        m_top_pitch = p_y;
    }
}

//------------------------------------------------------------------------------
void GData::beginning(void)
{
    updateActiveChunkTime(leftTime());
    m_view->doSlowUpdate();
}


//------------------------------------------------------------------------------
void GData::rewind(void)
{
    double speed = 16;
    double diffTime = m_view->zoomX() * speed;
    if(getActiveChannel())
    {
        diffTime = MAX(diffTime, getActiveChannel()->timePerChunk());
    }
    updateActiveChunkTime(m_view->currentTime() - diffTime);
    m_view->doSlowUpdate();
}

//------------------------------------------------------------------------------
bool GData::play(void)
{
    return playSound(getActiveSoundFile());
}

//------------------------------------------------------------------------------
void GData::stop(void)
{
    m_audio_thread.stopAndWait();
}

//------------------------------------------------------------------------------
void GData::fastforward(void)
{
    double speed = 16;
    double diffTime = m_view->zoomX() * speed;
    if(getActiveChannel())
    {
        diffTime = MAX(diffTime, getActiveChannel()->timePerChunk());
    }
    updateActiveChunkTime(m_view->currentTime() + diffTime);
    m_view->doSlowUpdate();
}
  
//------------------------------------------------------------------------------
void GData::end(void)
{
    updateActiveChunkTime(rightTime());
    m_view->doSlowUpdate();
}

//------------------------------------------------------------------------------
int GData::getAnalysisBufferSize(int p_rate)
{  
    int windowSize = m_settings->value("Analysis/bufferSizeValue", 48).toInt();
    QString windowSizeUnit = m_settings->value("Analysis/bufferSizeUnit", "milli-seconds").toString();
    if(windowSizeUnit.lower() == "milli-seconds")
    {
        //convert to samples
        windowSize = int(double(windowSize) * double(p_rate) / 1000.0);
    }
    if(m_settings->value("Analysis/bufferSizeRound", true).toBool())
    {
        windowSize = toInt(nearestPowerOf2(windowSize));
    }
    return windowSize;
}

//------------------------------------------------------------------------------
int GData::getAnalysisStepSize(int p_rate)
{  
    int stepSize = m_settings->value("Analysis/stepSizeValue", 24).toInt();
    QString stepSizeUnit = m_settings->value("Analysis/stepSizeUnit", "milli-seconds").toString();
    if(stepSizeUnit.lower() == "milli-seconds")
    {
        //convert to samples
        stepSize = int(double(stepSize) * double(p_rate) / 1000.0);
    }
    if(m_settings->value("Analysis/stepSizeRound", true).toBool())
    {
        stepSize = toInt(nearestPowerOf2(stepSize));
    }
    return stepSize;
}

//------------------------------------------------------------------------------
void GData::updateQuickRefSettings(void)
{
    m_background_color.setNamedColor(m_settings->value("Display/theBackgroundColor", "#BBCDE2").toString());
    m_shading_1_color.setNamedColor(m_settings->value("Display/shading1Color", "#BBCDEF").toString());
    m_shading_2_Color.setNamedColor(m_settings->value("Display/shading2Color", "#CBCDE2").toString());
    m_doing_harmonic_analysis = m_settings->value("Analysis/doingHarmonicAnalysis", true).toBool();
    m_doing_freq_analysis = m_settings->value("Analysis/doingFreqAnalysis", true).toBool();
    m_doing_equal_loudness = m_settings->value("Analysis/doingEqualLoudness", true).toBool();
    m_doing_auto_noise_floor = m_settings->value("Analysis/doingAutoNoiseFloor", true).toBool();
    m_doing_detailed_pitch = m_settings->value("Analysis/doingDetailedPitch", false).toBool();
    m_fast_update_speed = m_settings->value("Display/fastUpdateSpeed", 75).toInt();
    m_slow_update_speed = m_settings->value("Display/slowUpdateSpeed", 150).toInt();
    m_vibrato_sine_style = m_settings->value("Advanced/vibratoSineStyle", false).toBool();
    QString analysisString = m_settings->value("Analysis/analysisType", "MPM").toString();
    if(analysisString == QString("MPM"))
    {
        m_analysis_type = MPM;
    }
    else if(analysisString == QString("AUTOCORRELATION"))
    {
        m_analysis_type = AUTOCORRELATION;
    }
    else
    {
        m_analysis_type = MPM_MODIFIED_CEPSTRUM;
    }
    m_show_mean_variance_bars = m_settings->value("Advanced/showMeanVarianceBars", false).toBool();
    QString s = m_settings->value("Advanced/savingMode", "Ask when closing unsaved files (normal)").toString();
    if(s == "Ask when closing unsaved files (normal)")
    {
        m_saving_mode = 0;
    }
    else if(s == "Don't ask when closing unsaved files (use with care)")
    {
        m_saving_mode = 1;
    }
    else
    {
        m_saving_mode = 2;
    }
    m_mouse_wheel_zooms = m_settings->value("Advanced/mouseWheelZooms", false).toBool();
    setFreqA(m_settings->value("View/freqA", 440).toInt());
}

//------------------------------------------------------------------------------
QString GData::getFilenameString(void)
{
    QString fileGeneratingString = m_settings->value("General/filenameGeneratingString", "Untitled").toString();
    QString filename;
    int fileGeneratingNumber = m_settings->value("General/fileGeneratingNumber", 1).toInt();
    int digits = m_settings->value("General/fileNumberOfDigits", 2).toInt();
    if(digits == 0)
    {
        filename.sprintf("%s.wav", fileGeneratingString.latin1());
    }
    else
    {
        filename.sprintf("%s%0*d.wav", fileGeneratingString.latin1(), digits, fileGeneratingNumber);
    }
    return filename;
}

//------------------------------------------------------------------------------
QColor GData::getNextColor(void)
{
    return m_line_color.at((m_next_color_index++) % m_line_color.size());
}

//------------------------------------------------------------------------------
void GData::addFileToList(SoundFile * p_sound_file)
{
    int c;
    m_sound_files.push_back(p_sound_file);
    for(c = 0; c < p_sound_file->numChannels(); c++)
    {
      m_channels.push_back(&(p_sound_file->getChannel(c)));
    }
    emit channelsChanged();
}

//------------------------------------------------------------------------------
void GData::removeFileFromList(SoundFile * p_sound_file)
{
    int j;
    int curPos;
    int prevPos;
    //remove all the channels in p_sound_file from the channels list
    for(j = 0; j < p_sound_file->numChannels(); j++)
    {
        Channel * c = &(p_sound_file->getChannel(j));
        curPos = prevPos = 0;
        for(std::vector<Channel*>::iterator it1=m_channels.begin(); it1 != m_channels.end(); it1++, curPos++)
        {
            if((*it1) == c)
            {
                if(c == getActiveChannel())
                {
                    prevPos = curPos;
                }
                it1 = m_channels.erase(it1) - 1;
            }
        }
        if(m_channels.empty())
        {
            setActiveChannel(NULL);
        }
        else
        {
            setActiveChannel(m_channels.at(bound(prevPos, 0, int(m_channels.size()-1))));
        }
    }
    //remove the soundFile from the m_sound_files list
    for(std::vector<SoundFile*>::iterator it2 = m_sound_files.begin(); it2 != m_sound_files.end(); it2++)
    {
        if((*it2) == p_sound_file)
        {
            it2 = m_sound_files.erase(it2) - 1;
        }
    }
    emit channelsChanged();
}

//------------------------------------------------------------------------------
void GData::saveActiveFile(void)
{
    SoundFile * s = getActiveSoundFile();
    if(!s)
    {
        return;
    }
    if(s->saved())
    {
        return;
    }
    if(m_audio_thread.playSoundFile() == s || m_audio_thread.recSoundFile() == s)
    {
        stop();
    }
    int val = saveFile(s, saveFileAsk(s->getFileName()));
    if(val == 0)
    {
        //success
        emit channelsChanged();
    }
    else if(val == -1)
    {
        QMessageBox::warning(g_main_window, "Error", QString("Error saving file '") + QString(s->getFileName()) + QString("'"), QMessageBox::Ok, Qt::NoButton);
    }
}

//------------------------------------------------------------------------------
QString GData::saveFileAsk(QString p_old_filename)
{
    QString newFilename = SaveDialog::getSaveWavFileName(g_main_window);
    if(newFilename.isNull())
    {
        return QString();
    }
    newFilename = QDir::convertSeparators(newFilename);
    if(newFilename != p_old_filename && QFile::exists(newFilename))
    {
        if(QMessageBox::warning(g_main_window, tr("Overwrite File?"),
			      QString("A file called '") + newFilename + QString("' already exists.\n Do you want to overwrite it?"),
			      tr("&Yes"), tr("&No"), QString::null, 0, 1 )
		  )
        {
            return QString(); //user canceled overwrite
        }
    }
    return newFilename;
}

/**
  @return 0 if the file was saved, 1 if cancled, or -1 if an error occured
*/
int GData::saveFile( SoundFile * p_sound_file
                   , QString p_new_filename
                   )
{
    if(p_new_filename.isNull())
    {
        return 1;
    }
    QString oldFilename(p_sound_file->getFileName());
    oldFilename = QDir::convertSeparators(oldFilename);
    int pos = p_sound_file->getStream().pos();
    p_sound_file->getStream().close();
  
    int ret = (moveFile(oldFilename.latin1(), p_new_filename.latin1())) ? 0 : -1;
    if(ret == 0)
    {
        p_sound_file->getStream().open_read(p_new_filename.latin1());
        p_sound_file->getStream().jump_to_frame(pos);
        p_sound_file->setSaved(true);
        p_sound_file->setFilename(p_new_filename.latin1());
    }
    else
    {
        p_sound_file->getStream().open_read(oldFilename.latin1());
        p_sound_file->getStream().jump_to_frame(pos);
    }
    return ret;
}

/**
  @return true if all the files were closed. false if cancelled
*/
bool GData::closeAllFiles(void)
{
    while(!m_sound_files.empty())
    {
        if(closeFile(m_sound_files.at(0), gdata->savingMode()) == 1)
        {
            return false; //cancelled
        }
    }
    return true;
}

//------------------------------------------------------------------------------
void GData::closeActiveFile(void)
{
    SoundFile * s = getActiveSoundFile();
    if(!s)
    {
        return;
    }
    closeFile(s, gdata->savingMode());
    m_view->doUpdate();
    doChunkUpdate();
}

/**
  @param p_sound_file The sound file to be closed (This pointer will become invalid after returning)
  //@param ask If true (default), the user will be asked to save the file if it's been modified.
  @return 0 if the file was closed, 1 if canclled, -1 if error
*/
int GData::closeFile( SoundFile * p_sound_file
                    , int p_saving_mode
                    )
{
    if(p_sound_file == NULL)
    {
        return -1;
    }
    QString newFilename;
    QString oldFilename(p_sound_file->getFileName());
    oldFilename = QDir::convertSeparators(oldFilename);

    if(gdata->m_audio_thread.playSoundFile() == p_sound_file || gdata->m_audio_thread.recSoundFile() == p_sound_file)
    {
        gdata->stop();
    }

    if(p_sound_file->saved())
    {
        //file is already saved
        removeFileFromList(p_sound_file);
        delete p_sound_file;
        return 0;
    }

    if(p_saving_mode == ALWAYS_ASK)
    {
        QString filename = QString(getFilenamePart(oldFilename.latin1()));
        int option = QMessageBox::question(NULL
                                          , QString("Save changes to file '") + filename + "' ?"
                                          , QString("Save changes to the file '") + filename + QString("' ?\n")
                                          , "&Yes"
                                          , "&No"
                                          , "&Cancel"
                                          , 0
                                          , 2
					                      );

        switch(option)
        {
            case 0: //Yes
                newFilename = saveFileAsk(oldFilename);
                if(newFilename.isNull())
                {
                    return 1;
                }
                removeFileFromList(p_sound_file);
                delete p_sound_file;
                break;
            case 1: //No
                removeFileFromList(p_sound_file);
                delete p_sound_file;
                break;
            default: //Cancelled
                return 1;
        }
    }
    else if(p_saving_mode == NEVER_SAVE)
    {
        removeFileFromList(p_sound_file);
        delete p_sound_file;
    }
    else if(p_saving_mode == ALWAYS_SAVE)
    {
        removeFileFromList(p_sound_file);
        delete p_sound_file;
    }
    return 0;
}

//------------------------------------------------------------------------------
void GData::clearFreqLookup(void)
{
    for(std::vector<Channel *>::iterator it1=m_channels.begin(); it1 != m_channels.end(); it1++)
    {
        (*it1)->clearFreqLookup();
    }
}

//------------------------------------------------------------------------------
void GData::setAmplitudeMode(int p_amplitude_mode)
{
    if(p_amplitude_mode != m_amplitude_mode)
    {
        m_amplitude_mode = p_amplitude_mode;
        clearAmplitudeLookup();
    }
}

//------------------------------------------------------------------------------
void GData::setPitchContourMode(int p_pitch_contour_mode)
{
    if(p_pitch_contour_mode != m_pitch_contour_mode)
    {
        m_pitch_contour_mode = p_pitch_contour_mode;
        clearFreqLookup();
    }
}

//------------------------------------------------------------------------------
void GData::clearAmplitudeLookup(void)
{
    for(std::vector<Channel* >::iterator it1=m_channels.begin(); it1 != m_channels.end(); it1++)
    {
        (*it1)->clearAmplitudeLookup();
    }
}
  
//------------------------------------------------------------------------------
int GData::getActiveIntThreshold(void)
{
    Channel * active = getActiveChannel();
    if(active)
    {
        return toInt(active->threshold() * 100.0f);
    }
    else
    {
        return m_settings->value("Analysis/thresholdValue", 93).toInt();
    }
}

//------------------------------------------------------------------------------
void GData::resetActiveIntThreshold(int p_threshold_percentage)
{
    Channel* active = getActiveChannel();
    if(active)
    {
        active->resetIntThreshold(p_threshold_percentage);
    }
}

//------------------------------------------------------------------------------
void GData::setAmpThreshold( int p_mode
                           , int p_index
                           , double p_value
                           )
{
    m_amp_thresholds[p_mode][p_index] = p_value;
    clearFreqLookup();
    recalcScoreThresholds();
}

//------------------------------------------------------------------------------
double GData::ampThreshold( int p_mode
                          , int p_index
                          )
{
    return m_amp_thresholds[p_mode][p_index];
}

//------------------------------------------------------------------------------
void GData::setAmpWeight( int p_mode
                        , double p_value
                        )
{
    m_amp_weights[p_mode] = p_value;
    clearFreqLookup();
    recalcScoreThresholds();
}

//------------------------------------------------------------------------------
double GData::ampWeight(int p_mode)
{
    return m_amp_weights[p_mode];
}

//------------------------------------------------------------------------------
void GData::recalcScoreThresholds(void)
{
    for(std::vector<Channel *>::iterator it1=m_channels.begin(); it1 != m_channels.end(); it1++)
    {
        (*it1)->recalcScoreThresholds();
    }
}

//------------------------------------------------------------------------------
void GData::doChunkUpdate(void)
{
    emit onChunkUpdate();
}

//------------------------------------------------------------------------------
void GData::setTemperedType(int p_type)
{
    if(m_tempered_type != p_type)
    {
        if(m_tempered_type == 0 && p_type > 0)
        {
            //remove out the minors
            if(m_music_key_type >= 2)
            {
                setMusicKeyType(0);
            }
            for(int j = gMusicScales.size()-1; j >= 2; j--)
            {
                g_main_window->m_key_type_combo_box->removeItem(j);
            }
        }
        else if(m_tempered_type > 0 && p_type == 0)
        {
            QStringList s;
            for(unsigned int j = 2; j < gMusicScales.size(); j++)
            {
                s << gMusicScales[j].name();
            }
            g_main_window->m_key_type_combo_box->addItems(s);
        }
        m_tempered_type = p_type; emit temperedTypeChanged(p_type);
    }
}

//------------------------------------------------------------------------------
void GData::setFreqA(double p_x)
{
    m_freq_A = p_x;
    m_semitone_offset = freq2pitch(p_x) - freq2pitch(440.0);
    m_settings->setValue("View/freqA", p_x);
}

//EOF
