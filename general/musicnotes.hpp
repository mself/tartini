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
double freq2pitch(const double & p_freq)
{
#ifdef log2
    //From log rules  log(x/y) = log(x) - log(y)
    //return 69 + 12*(log2(p_freq) - log2(440));
    return -36.3763165622959152488 + 12.0*log2(p_freq);
#else
    //From log rules  log_b(x) = log_a(x) / log_a(b)
    //return 69 + 39.8631371386483481 * log10(p_freq / 440);
    return -36.3763165622959152488 + 39.8631371386483481 * log10(p_freq);
#endif
}

//------------------------------------------------------------------------------
double pitch2freq(const double & p_note)
{
    double l_result = pow10((p_note + 36.3763165622959152488) / 39.8631371386483481);
    return l_result;
}

//------------------------------------------------------------------------------
const char * noteName(const double & p_pitch)
{
    return noteName(toInt(p_pitch));
}

//------------------------------------------------------------------------------
int noteOctave(const double & p_pitch)
{
    return noteOctave(toInt(p_pitch));
}

//------------------------------------------------------------------------------
int noteValue(const double & p_pitch)
{
    return noteValue(toInt(p_pitch));
}

//------------------------------------------------------------------------------
bool isBlackNote(const double & p_pitch)
{
    return isBlackNote(toInt(p_pitch));
}

//------------------------------------------------------------------------------
MusicScale::MusicScale()
: m_p_name(NULL)
, m_semitone_offset(0)
{
}

//------------------------------------------------------------------------------
int MusicScale::size()const
{
    return m_p_notes.size();
}

//------------------------------------------------------------------------------
int MusicScale::note(int j)const
{
#ifdef MYDEBUG
    return m_p_notes.at(j);
#else // MYDEBUG
    return m_p_notes[j];
#endif // MYDEBUG
}

//------------------------------------------------------------------------------
bool MusicScale::hasSemitone(int p_j)const
{
#ifdef MYDEBUG
    return m_p_semitone_lookup.at(p_j);
#else // MYDEBUG
    return m_p_semitone_lookup[p_j];
#endif // MYDEBUG
}

//------------------------------------------------------------------------------
const char * MusicScale::name()const
{
    return m_p_name;
}

//------------------------------------------------------------------------------
int MusicScale::semitoneOffset()const
{
    return m_semitone_offset;
}

//------------------------------------------------------------------------------
MusicKey::MusicKey()
: m_name(NULL)
{
}

//------------------------------------------------------------------------------
const char * MusicKey::name()const
{
    return m_name;
}

//------------------------------------------------------------------------------
int MusicKey::size() const
{
    return m_note_offsets.size();
}

//------------------------------------------------------------------------------
double MusicKey::noteOffset(int p_j) const
{
#ifdef MYDEBUG
    return m_note_offsets.at(p_j);
#else // MYDEBUG
    return m_note_offsets[p_j];
#endif // MYDEBUG
}

//------------------------------------------------------------------------------
int MusicKey::noteType(int p_j) const
{
#ifdef MYDEBUG
    return m_note_types.at(p_j);
#else // MYDEBUG
    return m_note_types[p_j];
#endif // MYDEBUG
}
//EOF
