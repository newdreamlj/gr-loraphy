/* -*- c++ -*- */
/*
 * Copyright 2003,2008,2011,2012 Free Software Foundation, Inc.
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

#include <fft_fftw.h>
#include <gnuradio/sys_paths.h>
#include <gnuradio/gr_complex.h>
#include <volk/volk.h>

#ifdef _WIN32 //http://www.fftw.org/install/windows.html#DLLwisdom
static void my_fftwf_write_char(char c, void *f) { fputc(c, (FILE *) f); }
#define fftwf_export_wisdom_to_file(f) fftwf_export_wisdom(my_fftwf_write_char, (void*) (f))
#define fftwf_export_wisdom_to_file(f) fftwf_export_wisdom(my_fftwf_write_char, (void*) (f))
#define fftwl_export_wisdom_to_file(f) fftwl_export_wisdom(my_fftwf_write_char, (void*) (f))

static int my_fftwf_read_char(void *f) { return fgetc((FILE *) f); }
#define fftwf_import_wisdom_from_file(f) fftwf_import_wisdom(my_fftwf_read_char, (void*) (f))
#define fftwf_import_wisdom_from_file(f) fftwf_import_wisdom(my_fftwf_read_char, (void*) (f))
#define fftwl_import_wisdom_from_file(f) fftwl_import_wisdom(my_fftwf_read_char, (void*) (f))
#include <fcntl.h> 
#include <io.h>
#define O_NOCTTY 0
#define O_NONBLOCK 0
#endif //_WIN32

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fftw3.h>

#define FFTW3F_THREADS
//#include <cassert>
//#include <stdexcept>

//#include <boost/filesystem/operations.hpp>
//#include <boost/filesystem/path.hpp>
//#include <boost/interprocess/sync/file_lock.hpp>
//namespace fs = boost::filesystem;

namespace gr {
  namespace loraphy {

  	fft_complex_t::fft_complex_t(int fft_size, bool forward)
  	{
        d_fft_size = fft_size;
        d_inbuf = (fftwf_complex *) volk_malloc (sizeof (gr_complex) * fft_size, volk_get_alignment ());
        d_outbuf = (fftwf_complex *) volk_malloc (sizeof (gr_complex) * fft_size, volk_get_alignment ());
//        d_inbuf = (fftwf_complex *) fftwf_malloc (sizeof (fftwf_complex) * d_fft_size);
//        d_outbuf = (fftwf_complex *) fftwf_malloc (sizeof (fftwf_complex) * d_fft_size);

		#ifdef FFTW3F_THREADS
			  printf("fft multi thread\n");
			  fftwf_init_threads();
			  fftwf_plan_with_nthreads(4);
		#endif

      d_plan = fftwf_plan_dft_1d (fft_size,
                  d_inbuf,
                  d_outbuf,
                  forward ? FFTW_FORWARD : FFTW_BACKWARD,
                  FFTW_ESTIMATE);

      if (d_plan == NULL) {
        fprintf(stderr, "gr::fft: error creating plan\n");
      }
    }

    fft_complex_t::~fft_complex_t()
    {
      // fftwf_destroy_plan ((fftwf_plan) d_plan);
      // fftw_cleanup_threads(); this function cause segmentation fault, no idea why
      volk_free(d_inbuf);
      volk_free(d_outbuf);
    }


    void fft_complex_t::execute()
    {
        fftwf_execute((fftwf_plan) d_plan);
    }

//    void fft_complex_t::execute_dft(fftw_complex *in, fftw_complex *out)
//    {
//    	//void fftw_execute_dft( const fftw_plan p, fftw_complex *in, fftw_complex *out);
//    	fftwf_execute_dft((fftwf_plan) d_plan, in, out);
//        //fftwf_execute((fftwf_plan) d_plan);
//    }
  } /* namespace fft */
} /* namespace gr */
