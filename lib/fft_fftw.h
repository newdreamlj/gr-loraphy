/* -*- c++ -*- */
/*
 * Copyright 2003,2008,2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _LORAPHY_FFTW_H_
#define _LORAPHY_FFTW_H_

/*
 * Wrappers for FFTW single precision 1d dft
 */

// #include <clora/api.h>
#include <gnuradio/gr_complex.h>
#include <boost/thread.hpp>
#include <fftw3.h>

namespace gr {
  namespace loraphy {

    /*!
     * \brief Export reference to planner mutex for those apps that
     * want to use FFTW w/o using the fft_impl_fftw* classes.
     */
//    class  planner {
//    public:
//      typedef boost::mutex::scoped_lock scoped_lock;
//      /*!
//       * Return reference to planner mutex
//       */
//      static boost::mutex &mutex();
//    };

    /*!
     * \brief FFT: complex in, complex out
     * \ingroup misc
     */
    class  fft_complex_t {
    public:
      int         d_fft_size;
      fftwf_complex *d_inbuf;
      fftwf_complex *d_outbuf;
      void       *d_plan;

      fft_complex_t(int fft_size, bool forward = true);
      ~fft_complex_t();

      /*
       * These return pointers to buffers owned by fft_impl_fft_complex
       * into which input and output take place. It's done this way in
       * order to ensure optimal alignment for SIMD instructions.
       */
      fftwf_complex *get_inbuf()  const { return d_inbuf; }
      fftwf_complex *get_outbuf() const { return d_outbuf; }

      int inbuf_length()  const { return d_fft_size; }
      int outbuf_length() const { return d_fft_size; }

      /*!
       * compute FFT. The input comes from inbuf, the output is placed in
       * outbuf.
       */
      void execute();
//      void execute_dft(fftw_complex *,fftw_complex *);
    };
  } /* namespace loraphy */
} /*namespace gr */

#endif /* _LORAPHY_FFTW_H_ */
