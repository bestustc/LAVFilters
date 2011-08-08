/*
 *      Copyright (C) 2011 Hendrik Leppkes
 *      http://www.1f0.de
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 */

#include "stdafx.h"
#include "LAVPixFmtConverter.h"

#include <MMReg.h>
#include "moreuuids.h"

extern "C" {
#include "libavutil/intreadwrite.h"
};

typedef struct {
  enum PixelFormat ff_pix_fmt; // ffmpeg pixel format
  int num_pix_fmt;
  enum LAVVideoPixFmts lav_pix_fmts[LAVPixFmt_NB];
} FF_LAV_PIXFMT_MAP;

static FF_LAV_PIXFMT_MAP lav_pixfmt_map[] = {
  // Default
  { PIX_FMT_NONE, 5, { LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },

  // 4:2:0
  { PIX_FMT_YUV420P,  5, { LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUVJ420P, 5, { LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_NV12,     5, { LAVPixFmt_NV12, LAVPixFmt_YV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_NV21,     5, { LAVPixFmt_NV12, LAVPixFmt_YV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },

  { PIX_FMT_YUV420P9BE,  6, { LAVPixFmt_P010, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUV420P9LE,  6, { LAVPixFmt_P010, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUV420P10BE, 6, { LAVPixFmt_P010, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUV420P10LE, 6, { LAVPixFmt_P010, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUV420P16BE, 7, { LAVPixFmt_P016, LAVPixFmt_P010, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUV420P16LE, 7, { LAVPixFmt_P016, LAVPixFmt_P010, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_YUY2, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },

  // 4:2:2
  { PIX_FMT_YUV422P,  5, { LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUVJ422P, 5, { LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUYV422,  5, { LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_UYVY422,  5, { LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },

  { PIX_FMT_YUV422P10BE, 6, { LAVPixFmt_P210, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUV422P10LE, 6, { LAVPixFmt_P210, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUV422P16BE, 7, { LAVPixFmt_P216, LAVPixFmt_P210, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  { PIX_FMT_YUV422P16LE, 7, { LAVPixFmt_P216, LAVPixFmt_P210, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12, LAVPixFmt_RGB32, LAVPixFmt_RGB24 } },
  
  // 4:4:4
  { PIX_FMT_YUV444P,  6, { LAVPixFmt_AYUV, LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_YUVJ444P, 6, { LAVPixFmt_AYUV, LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },

  { PIX_FMT_YUV444P9BE,  7, { LAVPixFmt_Y410, LAVPixFmt_AYUV, LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_YUV444P9LE,  7, { LAVPixFmt_Y410, LAVPixFmt_AYUV, LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_YUV444P10BE, 7, { LAVPixFmt_Y410, LAVPixFmt_AYUV, LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_YUV444P10LE, 7, { LAVPixFmt_Y410, LAVPixFmt_AYUV, LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_YUV444P16BE, 8, { LAVPixFmt_Y416, LAVPixFmt_Y410, LAVPixFmt_AYUV, LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_YUV444P16LE, 8, { LAVPixFmt_Y416, LAVPixFmt_Y410, LAVPixFmt_AYUV, LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },

  // RGB
  { PIX_FMT_RGB24,    5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_BGR24,    5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_RGBA,     5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_ARGB,     5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_BGRA,     5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_ABGR,     5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_RGB565BE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_RGB565LE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_RGB555BE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_RGB555LE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_BGR565BE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_BGR565LE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_BGR555BE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_BGR555LE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_RGB444LE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_RGB444BE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_BGR444LE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
  { PIX_FMT_BGR444BE, 5, { LAVPixFmt_RGB32, LAVPixFmt_RGB24, LAVPixFmt_YUY2, LAVPixFmt_YV12, LAVPixFmt_NV12 } },
};

static LAVPixFmtDesc lav_pixfmt_desc[] = {
  { MEDIASUBTYPE_YV12,  12, 3, { 1, 2, 2 }, { 1, 2, 2 } },        // YV12
  { MEDIASUBTYPE_NV12,  12, 2, { 1, 2 }, { 1, 1 } },              // NV12
  { MEDIASUBTYPE_YUY2,  16, 0 },                                  // YUY2 (packed)
  { MEDIASUBTYPE_AYUV,  32, 0 },                                  // AYUV (packed)
  { MEDIASUBTYPE_P010,  15, 2, { 1, 2 }, { 1, 1 } },              // P010
  { MEDIASUBTYPE_P210,  20, 2, { 1, 1 }, { 1, 1 } },              // P210
  { FOURCCMap('014Y'),  32, 0 },                                  // Y410 (packed)
  { MEDIASUBTYPE_P016,  24, 2, { 1, 2 }, { 1, 1 } },              // P016
  { MEDIASUBTYPE_P216,  32, 2, { 1, 1 }, { 1, 1 } },              // P216
  { FOURCCMap('614Y'),  64, 0 },                                  // Y416 (packed)
  { MEDIASUBTYPE_RGB32, 32, 0 },                                  // RGB32
  { MEDIASUBTYPE_RGB24, 24, 0 },                                  // RGB24
};

CLAVPixFmtConverter::CLAVPixFmtConverter()
  : m_pSettings(NULL)
  , m_InputPixFmt(PIX_FMT_NONE)
  , m_OutputPixFmt(LAVPixFmt_YV12)
  , m_pSwsContext(NULL)
  , swsWidth(0), swsHeight(0)
{
}

CLAVPixFmtConverter::~CLAVPixFmtConverter()
{
  DestroySWScale();
}

LAVVideoPixFmts CLAVPixFmtConverter::GetOutputBySubtype(const GUID *guid)
{
  DWORD FourCC = guid->Data1;
  for (int i = 0; i < countof(lav_pixfmt_desc); ++i) {
    if (lav_pixfmt_desc[i].subtype == *guid) {
      return (LAVVideoPixFmts)i;
    }
  }
  return LAVPixFmt_None;
}

LAVVideoPixFmts CLAVPixFmtConverter::GetPreferredOutput()
{
  int i = 0;
  for (i = 0; i < countof(lav_pixfmt_map); ++i) {
    if (lav_pixfmt_map[i].ff_pix_fmt == m_InputPixFmt)
      return lav_pixfmt_map[i].lav_pix_fmts[0];
  }
  return lav_pixfmt_map[0].lav_pix_fmts[0];
}

int CLAVPixFmtConverter::GetNumMediaTypes()
{
  int i = 0;
  for (i = 0; i < countof(lav_pixfmt_map); ++i) {
    if (lav_pixfmt_map[i].ff_pix_fmt == m_InputPixFmt)
      return lav_pixfmt_map[i].num_pix_fmt;
  }
  
  return lav_pixfmt_map[0].num_pix_fmt;
}

CMediaType CLAVPixFmtConverter::GetMediaType(int index, LONG biWidth, LONG biHeight, DWORD dwAspectX, DWORD dwAspectY, REFERENCE_TIME rtAvgTime)
{
  FF_LAV_PIXFMT_MAP *pixFmtMap = NULL;
  for (int i = 0; i < countof(lav_pixfmt_map); ++i) {
    if (lav_pixfmt_map[i].ff_pix_fmt == m_InputPixFmt) {
      pixFmtMap = &lav_pixfmt_map[i];
      break;
    }
  }
  if (!pixFmtMap)
    pixFmtMap = &lav_pixfmt_map[0];

  if (index >= pixFmtMap->num_pix_fmt)
    index = 0;

  CMediaType mt;
  GUID guid = lav_pixfmt_desc[pixFmtMap->lav_pix_fmts[index]].subtype;

  mt.SetType(&MEDIATYPE_Video);
  mt.SetSubtype(&guid);
  mt.SetFormatType(&FORMAT_VideoInfo2);

  VIDEOINFOHEADER2 *vih2 = (VIDEOINFOHEADER2 *)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
  memset(vih2, 0, sizeof(VIDEOINFOHEADER2));


  // Validate the Aspect Ratio - an AR of 0 crashes VMR-9
  if (dwAspectX == 0 || dwAspectY == 0) {
    int dwX = 0;
    int dwY = 0;
    av_reduce(&dwX, &dwY, biWidth, biHeight, max(biWidth, biHeight));

    dwAspectX = dwX;
    dwAspectY = dwY;
  }

  vih2->rcSource.right = vih2->rcTarget.right = biWidth;
  vih2->rcSource.bottom = vih2->rcTarget.bottom = biHeight;
  vih2->AvgTimePerFrame = rtAvgTime;
  vih2->dwPictAspectRatioX = dwAspectX;
  vih2->dwPictAspectRatioY = dwAspectY;
  vih2->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  vih2->bmiHeader.biWidth = biWidth;
  vih2->bmiHeader.biHeight = biHeight;
  vih2->bmiHeader.biBitCount = lav_pixfmt_desc[pixFmtMap->lav_pix_fmts[index]].bpp;
  vih2->bmiHeader.biPlanes = lav_pixfmt_desc[pixFmtMap->lav_pix_fmts[index]].planes;
  vih2->bmiHeader.biSizeImage = (biWidth * biHeight * vih2->bmiHeader.biBitCount) >> 3;
  vih2->bmiHeader.biCompression = guid.Data1;

  if (!vih2->bmiHeader.biPlanes) {
    vih2->bmiHeader.biPlanes = 1;
  }

  if (guid == MEDIASUBTYPE_RGB32 || guid == MEDIASUBTYPE_RGB24) {
    vih2->bmiHeader.biCompression = BI_RGB;
    vih2->bmiHeader.biHeight = -vih2->bmiHeader.biHeight;
  }

  // Always set interlace flags, the samples will be flagged appropriately then.
  if (m_pSettings->GetReportInterlacedFlags())
    vih2->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOrWeave;

  mt.SetSampleSize(vih2->bmiHeader.biSizeImage);
  mt.SetTemporalCompression(0);

  return mt;
}


BOOL CLAVPixFmtConverter::IsAllowedSubtype(const GUID *guid)
{
  FF_LAV_PIXFMT_MAP *pixFmtMap = NULL;
  for (int i = 0; i < countof(lav_pixfmt_map); ++i) {
    if (lav_pixfmt_map[i].ff_pix_fmt == m_InputPixFmt) {
      pixFmtMap = &lav_pixfmt_map[i];
      break;
    }
  }
  if (!pixFmtMap)
    pixFmtMap = &lav_pixfmt_map[0];

  for (int i = 0; i < pixFmtMap->num_pix_fmt; ++i) {
    if (lav_pixfmt_desc[pixFmtMap->lav_pix_fmts[i]].subtype == *guid)
      return TRUE;
  }

  return FALSE;
}

inline SwsContext *CLAVPixFmtConverter::GetSWSContext(int width, int height, enum PixelFormat srcPix, enum PixelFormat dstPix, int flags)
{
  if (!m_pSwsContext || swsWidth != width || swsHeight != height) {
    // Map full-range formats to their limited-range variants
    // All target formats we have are limited range and we don't want compression
    if (dstPix != PIX_FMT_BGRA && dstPix != PIX_FMT_BGR24) {
      if (srcPix == PIX_FMT_YUVJ420P)
        srcPix = PIX_FMT_YUV420P;
      else if (srcPix == PIX_FMT_YUVJ422P)
        srcPix = PIX_FMT_YUV422P;
      else if (srcPix == PIX_FMT_YUVJ440P)
        srcPix = PIX_FMT_YUV440P;
      else if (srcPix == PIX_FMT_YUVJ444P)
        srcPix = PIX_FMT_YUV444P;
    }

    // Get context
    m_pSwsContext = sws_getCachedContext(m_pSwsContext,
                                 width, height, srcPix,
                                 width, height, dstPix,
                                 flags|SWS_PRINT_INFO, NULL, NULL, NULL);
    swsWidth = width;
    swsHeight = height;
  }
  return m_pSwsContext;
}

HRESULT CLAVPixFmtConverter::swscale_scale(enum PixelFormat srcPix, enum PixelFormat dstPix, AVFrame *pFrame, BYTE *pOut, int width, int height, int stride, LAVPixFmtDesc pixFmtDesc, bool swapPlanes12)
{
  uint8_t *dst[4];
  int     dstStride[4];
  int     i, ret;

  SwsContext *ctx = GetSWSContext(width, height, srcPix, dstPix, SWS_BICUBIC);
  CheckPointer(m_pSwsContext, E_POINTER);

  memset(dst, 0, sizeof(dst));
  memset(dstStride, 0, sizeof(dstStride));

  dst[0] = pOut;
  dstStride[0] = stride;
  for (i = 1; i < pixFmtDesc.planes; ++i) {
    dst[i] = dst[i-1] + (stride / pixFmtDesc.planeWidth[i-1]) * (height / pixFmtDesc.planeWidth[i-1]);
    dstStride[i] = stride / pixFmtDesc.planeWidth[i];
  }
  
  if (swapPlanes12) {
    BYTE *tmp = dst[1];
    dst[1] = dst[2];
    dst[2] = tmp;
  }
  ret = sws_scale(ctx, pFrame->data, pFrame->linesize, 0, height, dst, dstStride);

  return S_OK;
}

HRESULT CLAVPixFmtConverter::Convert(AVFrame *pFrame, BYTE *pOut, int width, int height, int dstStride)
{
  HRESULT hr = S_OK;
  switch (m_OutputPixFmt) {
  case LAVPixFmt_YV12:
    hr = swscale_scale(m_InputPixFmt, PIX_FMT_YUV420P, pFrame, pOut, width, height, dstStride, lav_pixfmt_desc[m_OutputPixFmt], true);
    break;
  case LAVPixFmt_NV12:
    hr = swscale_scale(m_InputPixFmt, PIX_FMT_NV12, pFrame, pOut, width, height, dstStride, lav_pixfmt_desc[m_OutputPixFmt]);
    break;
  case LAVPixFmt_YUY2:
    hr = swscale_scale(m_InputPixFmt, PIX_FMT_YUYV422, pFrame, pOut, width, height, dstStride * 2, lav_pixfmt_desc[m_OutputPixFmt]);
    break;
  case LAVPixFmt_AYUV:
    hr = ConvertToAYUV(pFrame, pOut, width, height, dstStride);
    break;
  case LAVPixFmt_P010:
    hr = ConvertToPX1X(pFrame, pOut, width, height, dstStride, 2);
    break;
  case LAVPixFmt_P016:
    hr = ConvertToPX1X(pFrame, pOut, width, height, dstStride, 2);
    break;
  case LAVPixFmt_P210:
    hr = ConvertToPX1X(pFrame, pOut, width, height, dstStride, 1);
    break;
  case LAVPixFmt_P216:
    hr = ConvertToPX1X(pFrame, pOut, width, height, dstStride, 1);
    break;
  case LAVPixFmt_Y410:
    hr = ConvertToY410(pFrame, pOut, width, height, dstStride);
    break;
  case LAVPixFmt_Y416:
    hr = ConvertToY416(pFrame, pOut, width, height, dstStride);
    break;
  case LAVPixFmt_RGB32:
    hr = swscale_scale(m_InputPixFmt, PIX_FMT_BGRA, pFrame, pOut, width, height, dstStride * 4, lav_pixfmt_desc[m_OutputPixFmt]);
    break;
  case LAVPixFmt_RGB24:
    hr = swscale_scale(m_InputPixFmt, PIX_FMT_BGR24, pFrame, pOut, width, height, dstStride * 3, lav_pixfmt_desc[m_OutputPixFmt]);
    break;
  default:
    ASSERT(0);
    hr = E_FAIL;
    break;
  }
  return hr;
}

HRESULT CLAVPixFmtConverter::ConvertToAYUV(AVFrame *pFrame, BYTE *pOut, int width, int height, int stride)
{
  const BYTE *y = NULL;
  const BYTE *u = NULL;
  const BYTE *v = NULL;
  int line, i = 0;
  int srcStride = 0;
  BYTE *pTmpBuffer = NULL;

  if (m_InputPixFmt != PIX_FMT_YUV444P) {
    uint8_t *dst[4] = {NULL};
    int     dstStride[4] = {0};

    pTmpBuffer = (BYTE *)av_malloc(height * stride * 3);

    dst[0] = pTmpBuffer;
    dst[1] = dst[0] + (height * stride);
    dst[2] = dst[1] + (height * stride);
    dst[3] = NULL;
    dstStride[0] = stride;
    dstStride[1] = stride;
    dstStride[2] = stride;
    dstStride[3] = 0;

    SwsContext *ctx = GetSWSContext(width, height, m_InputPixFmt, PIX_FMT_YUV444P, SWS_POINT);
    sws_scale(ctx, pFrame->data, pFrame->linesize, 0, height, dst, dstStride);

    y = dst[0];
    u = dst[1];
    v = dst[2];
    srcStride = stride;
  } else {
    y = pFrame->data[0];
    u = pFrame->data[1];
    v = pFrame->data[2];
    srcStride = pFrame->linesize[0];
  }

  for (line = 0; line < height; ++line) {
    BYTE *pLine = pOut + line * stride * 4;
    int32_t *idst = (int32_t *)pLine;
    for (i = 0; i < width; ++i) {
      *idst++ = v[i] + (u[i] << 8) + (y[i] << 16) + (0xff << 24);
    }
    y += srcStride;
    u += srcStride;
    v += srcStride;
  }

  av_freep(&pTmpBuffer);

  return S_OK;
}

HRESULT CLAVPixFmtConverter::ConvertToPX1X(AVFrame *pFrame, BYTE *pOut, int width, int height, int stride, int chromaVertical)
{
  const BYTE *y = NULL;
  const BYTE *u = NULL;
  const BYTE *v = NULL;
  int line, i = 0;
  int srcStride = 0;

  // Stride needs to be doubled for 16-bit per pixel
  stride *= 2;

  BYTE *pTmpBuffer = NULL;

  PixelFormat pixFmtRequired = (chromaVertical == 2) ? PIX_FMT_YUV420P16LE : PIX_FMT_YUV422P16LE;

  if (m_InputPixFmt != pixFmtRequired) {
    uint8_t *dst[4] = {NULL};
    int     dstStride[4] = {0};

    pTmpBuffer = (BYTE *)av_malloc(height * stride * 2);

    dst[0] = pTmpBuffer;
    dst[1] = dst[0] + (height * stride);
    dst[2] = dst[1] + ((height / chromaVertical) * (stride / 2));
    dst[3] = NULL;
    dstStride[0] = stride;
    dstStride[1] = stride / 2;
    dstStride[2] = stride / 2;
    dstStride[3] = 0;

    SwsContext *ctx = GetSWSContext(width, height, m_InputPixFmt, pixFmtRequired, SWS_POINT);
    sws_scale(ctx, pFrame->data, pFrame->linesize, 0, height, dst, dstStride);

    y = dst[0];
    u = dst[1];
    v = dst[2];
    srcStride = stride;
  } else {
    y = pFrame->data[0];
    u = pFrame->data[1];
    v = pFrame->data[2];
    srcStride = pFrame->linesize[0];
  }

  // copy Y
  BYTE *pLineOut = pOut;
  const BYTE *pLineIn = y;
  for (line = 0; line < height; ++line) {
    memcpy(pLineOut, pLineIn, width * 2);
    pLineOut += stride;
    pLineIn += srcStride;
  }

  // Merge U/V
  BYTE *dstUV = pLineOut;
  const int16_t *uc = (int16_t *)u;
  const int16_t *vc = (int16_t *)v;
  for (line = 0; line < height/chromaVertical; ++line) {
    int32_t *idst = (int32_t *)(dstUV + line * stride);
    for (i = 0; i < width/2; ++i) {
      *idst++ = uc[i] + (vc[i] << 16);
    }
    uc += srcStride/4;
    vc += srcStride/4;
  }

  av_freep(&pTmpBuffer);

  return S_OK;
}

HRESULT CLAVPixFmtConverter::ConvertToY410(AVFrame *pFrame, BYTE *pOut, int width, int height, int stride)
{
  const BYTE *y = NULL;
  const BYTE *u = NULL;
  const BYTE *v = NULL;
  int line, i = 0;
  int srcStride = 0;
  bool bBigEndian = false, b9Bit = false;

  BYTE *pTmpBuffer = NULL;

  if (m_InputPixFmt != PIX_FMT_YUV444P10BE && m_InputPixFmt != PIX_FMT_YUV444P10LE && m_InputPixFmt != PIX_FMT_YUV444P9BE && m_InputPixFmt != PIX_FMT_YUV444P9LE) {
    uint8_t *dst[4] = {NULL};
    int     dstStride[4] = {0};

    pTmpBuffer = (BYTE *)av_malloc(height * stride * 6);

    dst[0] = pTmpBuffer;
    dst[1] = dst[0] + (height * stride * 2);
    dst[2] = dst[1] + (height * stride * 2);
    dst[3] = NULL;
    dstStride[0] = stride * 2;
    dstStride[1] = stride * 2;
    dstStride[2] = stride * 2;
    dstStride[3] = 0;

    SwsContext *ctx = GetSWSContext(width, height, m_InputPixFmt, PIX_FMT_YUV444P10LE, SWS_POINT);
    sws_scale(ctx, pFrame->data, pFrame->linesize, 0, height, dst, dstStride);

    y = dst[0];
    u = dst[1];
    v = dst[2];
    srcStride = stride * 2;
  } else {
    y = pFrame->data[0];
    u = pFrame->data[1];
    v = pFrame->data[2];
    srcStride = pFrame->linesize[0];

    bBigEndian = (m_InputPixFmt == PIX_FMT_YUV444P10BE || m_InputPixFmt == PIX_FMT_YUV444P9BE);
    b9Bit = (m_InputPixFmt == PIX_FMT_YUV444P9BE || m_InputPixFmt == PIX_FMT_YUV444P9LE);
  }

  // 32-bit per pixel
  stride *= 4;

  for (line = 0; line < height; ++line) {
    const int16_t *yc = (int16_t *)(y + line * srcStride);
    const int16_t *uc = (int16_t *)(u + line * srcStride);
    const int16_t *vc = (int16_t *)(v + line * srcStride);
    int32_t *idst = (int32_t *)(pOut + (line * stride));
    for (i = 0; i < width; ++i) {
      int16_t yv = bBigEndian ? AV_RB16(yc+i) : AV_RL16(yc+i);
      int16_t uv = bBigEndian ? AV_RB16(uc+i) : AV_RL16(uc+i);
      int16_t vv = bBigEndian ? AV_RB16(vc+i) : AV_RL16(vc+i);
      if (b9Bit) {
        yv <<= 1;
        uv <<= 1;
        vv <<= 1;
      }
      *idst++ = (uv & 0x3FF) + ((yv & 0x3FF) << 10) + ((vv & 0x3FF) << 20) + (3 << 30);
    }
  }

  av_freep(&pTmpBuffer);

  return S_OK;
}

HRESULT CLAVPixFmtConverter::ConvertToY416(AVFrame *pFrame, BYTE *pOut, int width, int height, int stride)
{
  const BYTE *y = NULL;
  const BYTE *u = NULL;
  const BYTE *v = NULL;
  int line, i = 0;
  int srcStride = 0;
  bool bBigEndian = false;

  BYTE *pTmpBuffer = NULL;

  if (m_InputPixFmt != PIX_FMT_YUV444P16BE && m_InputPixFmt != PIX_FMT_YUV444P16LE) {
    uint8_t *dst[4] = {NULL};
    int     dstStride[4] = {0};

    pTmpBuffer = (BYTE *)av_malloc(height * stride * 6);

    dst[0] = pTmpBuffer;
    dst[1] = dst[0] + (height * stride * 2);
    dst[2] = dst[1] + (height * stride * 2);
    dst[3] = NULL;
    dstStride[0] = stride * 2;
    dstStride[1] = stride * 2;
    dstStride[2] = stride * 2;
    dstStride[3] = 0;

    SwsContext *ctx = GetSWSContext(width, height, m_InputPixFmt, PIX_FMT_YUV444P16LE, SWS_POINT);
    sws_scale(ctx, pFrame->data, pFrame->linesize, 0, height, dst, dstStride);

    y = dst[0];
    u = dst[1];
    v = dst[2];
    srcStride = stride * 2;
  } else {
    y = pFrame->data[0];
    u = pFrame->data[1];
    v = pFrame->data[2];
    srcStride = pFrame->linesize[0];

    bBigEndian = (m_InputPixFmt == PIX_FMT_YUV444P16BE);
  }

  // 64-bit per pixel
  stride *= 8;

  for (line = 0; line < height; ++line) {
    const int16_t *yc = (int16_t *)(y + line * srcStride);
    const int16_t *uc = (int16_t *)(u + line * srcStride);
    const int16_t *vc = (int16_t *)(v + line * srcStride);
    int32_t *idst = (int32_t *)(pOut + (line * stride));
    for (i = 0; i < width; ++i) {
      int16_t yv = bBigEndian ? AV_RB16(yc+i) : AV_RL16(yc+i);
      int16_t uv = bBigEndian ? AV_RB16(uc+i) : AV_RL16(uc+i);
      int16_t vv = bBigEndian ? AV_RB16(vc+i) : AV_RL16(vc+i);
      *idst++ = 0xFFFF + (vv << 16);
      *idst++ = yv + (uv << 16);
    }
  }

  av_freep(&pTmpBuffer);

  return S_OK;
}