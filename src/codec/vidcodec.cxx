/*
 * vidcodec.cxx
 *
 * Uncompressed video handler
 *
 * Open Phone Abstraction Library
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Open Phone Abstraction Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>

#ifdef __GNUC__
#pragma implementation "vidcodec.h"
#endif

#include <opal_config.h>

#if OPAL_VIDEO

#include <codec/vidcodec.h>

#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>

#define FRAME_WIDTH  PVideoDevice::CIF16Width
#define FRAME_HEIGHT PVideoDevice::CIF16Height
#define FRAME_RATE   30 // NTSC

const OpalVideoFormat & GetOpalRGB24()
{
  static const OpalVideoFormat RGB24(
    OPAL_RGB24,
    RTP_DataFrame::MaxPayloadType,
    NULL,
    FRAME_WIDTH, FRAME_HEIGHT,
    FRAME_RATE,
    24*FRAME_WIDTH*FRAME_HEIGHT*FRAME_RATE  // Bandwidth
  );
  return RGB24;
}

const OpalVideoFormat & GetOpalRGB32()
{
  static const OpalVideoFormat RGB32(
    OPAL_RGB32,
    RTP_DataFrame::MaxPayloadType,
    NULL,
    FRAME_WIDTH, FRAME_HEIGHT,
    FRAME_RATE,
    32*FRAME_WIDTH*FRAME_HEIGHT*FRAME_RATE  // Bandwidth
  );
  return RGB32;
}

const OpalVideoFormat & GetOpalBGR24()
{
  static const OpalVideoFormat BGR24(
    OPAL_BGR24,
    RTP_DataFrame::MaxPayloadType,
    NULL,
    FRAME_WIDTH, FRAME_HEIGHT,
    FRAME_RATE,
    24*FRAME_WIDTH*FRAME_HEIGHT*FRAME_RATE  // Bandwidth
  );
  return BGR24;
}

const OpalVideoFormat & GetOpalBGR32()
{
  static const OpalVideoFormat BGR32(
    OPAL_BGR32,
    RTP_DataFrame::MaxPayloadType,
    NULL,
    FRAME_WIDTH, FRAME_HEIGHT,
    FRAME_RATE,
    32*FRAME_WIDTH*FRAME_HEIGHT*FRAME_RATE  // Bandwidth
  );
  return BGR32;
}

const OpalVideoFormat & GetOpalYUV420P()
{
  static const OpalVideoFormat YUV420P(
    OPAL_YUV420P,
    RTP_DataFrame::MaxPayloadType,
    NULL,
    FRAME_WIDTH, FRAME_HEIGHT,
    FRAME_RATE,
    12*FRAME_WIDTH*FRAME_HEIGHT*FRAME_RATE  // Bandwidth
  );
  return YUV420P;
}


#define new PNEW

/////////////////////////////////////////////////////////////////////////////

OpalVideoTranscoder::OpalVideoTranscoder(const OpalMediaFormat & inputMediaFormat,
                                         const OpalMediaFormat & outputMediaFormat)
  : OpalTranscoder(inputMediaFormat, outputMediaFormat)
  , m_errorConcealment(false)
  , m_inDataSize(10*1024)
  , m_outDataSize(10*1024)
  , m_forceIFrame(false)
  , m_lastFrameWasIFrame(false)
  , m_totalFrames(0)
  , m_keyFrames(0)
{
  acceptEmptyPayload = true;
}


static void SetFrameBytes(const OpalMediaFormat & fmt, const PString & widthOption, const PString & heightOption, PINDEX & size)
{
  int width  = fmt.GetOptionInteger(widthOption, PVideoFrameInfo::CIFWidth);
  int height = fmt.GetOptionInteger(heightOption, PVideoFrameInfo::CIFHeight);
  int newSize = PVideoDevice::CalculateFrameBytes(width, height, fmt);
  if (newSize > 0)
    size = sizeof(PluginCodec_Video_FrameHeader) + newSize;
}


bool OpalVideoTranscoder::UpdateMediaFormats(const OpalMediaFormat & input, const OpalMediaFormat & output)
{
  PWaitAndSignal mutex(updateMutex);

  if (!OpalTranscoder::UpdateMediaFormats(input, output))
    return false;

  SetFrameBytes(inputMediaFormat,  OpalVideoFormat::MaxRxFrameWidthOption(), OpalVideoFormat::MaxRxFrameHeightOption(), m_inDataSize);
  SetFrameBytes(outputMediaFormat, OpalVideoFormat::FrameWidthOption(),      OpalVideoFormat::FrameHeightOption(),      m_outDataSize);

  if (outputMediaFormat.GetOptionInteger(OpalMediaFormat::MaxTxPacketSizeOption()) > (int)maxOutputSize) {
    PTRACE(4, "Media\tReducing \"" << OpalMediaFormat::MaxTxPacketSizeOption() << "\" to " << maxOutputSize);
    outputMediaFormat.SetOptionInteger(OpalMediaFormat::MaxTxPacketSizeOption(), maxOutputSize);
  }

  return true;
}


PINDEX OpalVideoTranscoder::GetOptimalDataFrameSize(PBoolean input) const
{
  if (input)
    return m_inDataSize;

  if (m_outDataSize < maxOutputSize)
    return m_outDataSize;

  return maxOutputSize;
}


PBoolean OpalVideoTranscoder::ExecuteCommand(const OpalMediaCommand & command)
{
  if (outputMediaFormat != OpalYUV420P && PIsDescendant(&command, OpalVideoUpdatePicture)) {
    PTRACE_IF(3, !m_forceIFrame, "Media\tI-Frame forced in video stream");
    m_forceIFrame = true; // Reset when I-Frame is sent
    return true;
  }

  return OpalTranscoder::ExecuteCommand(command);
}


PBoolean OpalVideoTranscoder::Convert(const RTP_DataFrame & /*input*/,
                                  RTP_DataFrame & /*output*/)
{
  return false;
}


#if OPAL_STATISTICS
void OpalVideoTranscoder::GetStatistics(OpalMediaStatistics & statistics) const
{
  statistics.m_totalFrames = m_totalFrames;
  statistics.m_keyFrames   = m_keyFrames;
}
#endif


///////////////////////////////////////////////////////////////////////////////

PString OpalVideoUpdatePicture::GetName() const
{
  return "Update Picture";
}


OpalVideoPictureLoss::OpalVideoPictureLoss(unsigned sequenceNumber, unsigned timestamp)
  : m_sequenceNumber(sequenceNumber)
  , m_timestamp(timestamp)
{
}


PString OpalVideoPictureLoss::GetName() const
{
  return "Picture Loss";
}


PString OpalTemporalSpatialTradeOff::GetName() const
{
  return "Temporal Spatial Trade Off";
}


#endif // OPAL_VIDEO

/////////////////////////////////////////////////////////////////////////////
