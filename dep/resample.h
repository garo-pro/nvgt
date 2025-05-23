/*
    WDL - resample.h
    Copyright (C) 2010 and later Cockos Incorporated

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

    You may also distribute this software under the LGPL v2 or later.

*/

#ifndef _WDL_RESAMPLE_H_
#define _WDL_RESAMPLE_H_
// Local note: temporary modification to this header until I figure out why SCons is going to war with me over adding defines.
#define WDL_RESAMPLE_TYPE float
#include <stdlib.h>
#include <string.h>
#include "wdltypes.h"
#include "heapbuf.h"

// default to floats for sinc filter ceofficients
#ifdef WDL_RESAMPLE_FULL_SINC_PRECISION
typedef double WDL_SincFilterSample;
#else
typedef float WDL_SincFilterSample;
#endif

// default to doubles for audio samples
#ifdef WDL_RESAMPLE_TYPE
typedef WDL_RESAMPLE_TYPE WDL_ResampleSample;
#else
typedef double WDL_ResampleSample;
#endif

class WDL_Resampler
{
public:
  WDL_Resampler();
  ~WDL_Resampler();
  // if sinc set, it overrides interp or filtercnt
  void SetMode(bool interp, int filtercnt, bool sinc, int sinc_size = 64, int sinc_interpsize = 32);

  void SetFilterParms(float filterpos = 0.693, float filterq = 0.707)
  {
    m_filterpos = filterpos;
    m_filterq = filterq;
  } // used for filtercnt>0 but not sinc
  void SetFeedMode(bool wantInputDriven) { m_feedmode = wantInputDriven; } // if true, that means the first parameter to ResamplePrepare will specify however much input you have, not how much you want

  void Reset(double fracpos = 0.0);
  void SetRates(double rate_in, double rate_out);

  double GetCurrentLatency(); // amount of input that has been received but not yet converted to output, in seconds

  // req_samples is output samples desired if !wantInputDriven, or if wantInputDriven is input samples that we have
  // returns number of samples desired (put these into *inbuffer)
  // note that it is safe to call ResamplePrepare without calling ResampleOut (the next call of ResamplePrepare will function as normal)
  int ResamplePrepare(int req_samples, int nch, WDL_ResampleSample **inbuffer);

  // if numsamples_in < the value return by ResamplePrepare(), then it will be flushed to produce all remaining valid samples
  // do NOT call with nsamples_in greater than the value returned from resamplerprpare()! the extra samples will be ignored.
  // returns number of samples successfully outputted to out
  int ResampleOut(WDL_ResampleSample *out, int nsamples_in, int nsamples_out, int nch);

  void Prealloc(int nch, int inputsize, int outputsize);

private:
  const WDL_SincFilterSample *BuildLowPass(bool *isIdeal);

  double m_sratein WDL_FIXALIGN;
  double m_srateout;
  double m_fracpos;
  double m_ratio;
  double m_filter_ratio;
  float m_filterq, m_filterpos;
  WDL_TypedBuf<WDL_ResampleSample> m_rsinbuf;
  WDL_TypedBuf<WDL_SincFilterSample> m_filter_coeffs;

  class WDL_Resampler_Filter;
  WDL_Resampler_Filter *m_pre_filter, *m_post_filter;
  int m_prepost_filtercnt;

  int m_filter_coeffs_size;
  int m_last_requested;
  int m_filtlatency;
  int m_samples_in_rsinbuf;
  int m_rsinbuf_nch;
  int m_lp_oversize;
  int m_sinc_ideal_calced; // -1=not yet calced

  int m_sincsize;
  int m_sincoversize;
  bool m_interp;
  bool m_feedmode;
};

#endif
