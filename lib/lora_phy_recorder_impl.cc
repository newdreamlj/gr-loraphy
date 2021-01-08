/* -*- c++ -*- */
/*
 * Copyright 2020 Jun Liu.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "lora_phy_recorder_impl.h"
#include <gnuradio/filter/firdes.h>
#include <volk/volk.h>

namespace gr {
  namespace loraphy {

    lora_phy_recorder::sptr
    lora_phy_recorder::make(int sf, int bw, int fs, int recording_time_ms, uint64_t freq_center, uint64_t freq_lora)
    {
      return gnuradio::get_initial_sptr
        (new lora_phy_recorder_impl(sf, bw, fs, recording_time_ms, freq_center, freq_lora));
    }

    /*
     * The private constructor
     */
    lora_phy_recorder_impl::lora_phy_recorder_impl(int sf, int bw, int fs, int recording_time_ms, uint64_t freq_center, uint64_t freq_lora)
      : gr::block("lora_phy_recorder",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
			  sf(sf),
			  bw(sf),
			  fs(fs),
			  recording_time_ms(recording_time_ms),
			  freq_center(freq_center),
			  freq_lora(freq_lora)
    {
    	sps = (int) (fs/bw*pow(2,sf));
    	log_symbols = (int)round(recording_time_ms/(1000*pow(2,sf)/bw));
    	nbins = pow(2,sf);
    	deci = (int)fs/bw;
    	ring_buffer.init(4096, sps);
    	filter_taps_dot_freq_cov = (gr_complex *) volk_malloc (sizeof(gr_complex) * ntaps,  volk_get_alignment ());
    	freq_cov_phase_calibration = (gr_complex *) volk_malloc (sizeof(gr_complex) * nbins,  volk_get_alignment ());
    	downchirp = (gr_complex*) volk_malloc (sizeof(gr_complex) * nbins,  volk_get_alignment ());
    	downchirp_dot_phase_calibration = (gr_complex*) volk_malloc (sizeof(gr_complex) * nbins,  volk_get_alignment ());
		baseband = (gr_complex*) volk_malloc (sizeof(gr_complex) * nbins,  volk_get_alignment ());
		mag = (float*) volk_malloc (sizeof(float) * nbins,  volk_get_alignment ());
		buf1 = NULL;//(gr_complex*) volk_malloc (sizeof(gr_complex) * sps,  volk_get_alignment ());
		d_fft = new fft_complex_t(nbins, true);

        const int alignment = volk_get_alignment() / sizeof(gr_complex);
        set_alignment(std::max(1, alignment));

        std::vector<float> taps = gr::filter::firdes::low_pass_2(1.0, (double)fs, (double)bw*0.55, (double)bw*1.1, 40.0);
    	ntaps = taps.size();

		// Create reference filter taps, merged with frequency conversion

		double freq_offset = freq_center - freq_lora; // eg. 924.8MHz - 925.5MHz
		double delta_phi_freqcov = 2 * M_PI * freq_offset / fs;
    	double phase=0;

    	for (int p=0; p<ntaps ; p++){
    		filter_taps_dot_freq_cov[p] = taps[p] * gr_complex(cosf(phase), sinf(phase));
    		phase = phase + delta_phi_freqcov;
		}

		// Create reference phase calibration caused by frequency conversion

		double phase_delta=delta_phi_freqcov*deci;
    	phase = 0;
    	for (int p=0; p<nbins ; p++){
    		freq_cov_phase_calibration[p] =  gr_complex(cosf(phase), sinf(phase));
    		phase = phase + phase_delta;
		}

    	// Create reference down-chirp
    	double t, k=(double)bw*bw/pow(2,sf);
    	phase=0;
    	for (int p=0; p<nbins ; p++){
    		t = (double)p/bw;
    		phase = 2*M_PI*(0.5*bw*t-0.5*k*t*t);
    		downchirp[p] =  gr_complex(cosf(phase), sinf(phase));
		}

		// Create reference down-chirp merged with phase calibration
    	volk_32fc_x2_multiply_32fc(downchirp_dot_phase_calibration, downchirp, freq_cov_phase_calibration, nbins);
    	printf("Init done with filter taps=%d\n", ntaps);
    }

    /*
     * Our virtual destructor.
     */
    lora_phy_recorder_impl::~lora_phy_recorder_impl()
    {
		volk_free(downchirp_dot_phase_calibration);
		volk_free(downchirp);
		volk_free(freq_cov_phase_calibration);
		volk_free(filter_taps_dot_freq_cov);
    	volk_free(mag);
    	volk_free(baseband);
		delete d_fft;
    }

    void
    lora_phy_recorder_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
    	ninput_items_required[0] = (int)(noutput_items/4);
    }

    int lora_phy_recorder_impl::lora_detect(gr_complex* buf){
		/* 1. filter and multiply with down-chirp*/
		for (int i=0; i<nbins; i++) {
			if ((i+1)*deci-ntaps<0)
				baseband[i] = 0;
			else
				volk_32fc_x2_dot_prod_32fc(baseband+i, buf+(i+1)*deci-ntaps, filter_taps_dot_freq_cov, ntaps);
		}

		volk_32fc_x2_multiply_32fc((gr_complex *)d_fft->d_inbuf, baseband, downchirp_dot_phase_calibration, nbins);
		/* 2. FFT */
		d_fft->execute();

		/* 3. Peak above threshold */
		uint16_t target;
		float sum;
		volk_32fc_magnitude_32f(mag, (gr_complex*)d_fft->get_outbuf(), nbins);
		volk_32f_index_max_16u(&target, mag, nbins);
		volk_32f_accumulator_s32f(&sum, mag, nbins);
		if (mag[target]>7*(sum-mag[target])/(nbins-1)){
			return target;
		}
		else
			return -1;
	}

    int
    lora_phy_recorder_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const gr_complex **in = (const gr_complex **) &input_items[0];
        gr_complex **out = (gr_complex **) &output_items[0];
        int cnt = 0, ninput;
        int nout=0;

		// Do <+signal processing+>
		// Tell runtime system how many input items we consumed on
		// each input stream.
		/*
		* 1. buffering
		* 2. detection
		* 3. popping
		* */

        ninput = ninput_items[0];

    	if (nout<noutput_items && ring_buffer.is_popping_enabled()){
    		nout += ring_buffer.pop_buffer(noutput_items-nout, (gr_complex *)out[0]+nout);
    		if (!ring_buffer.is_popping_enabled()) {
    			timeval tv;
    		    gettimeofday(&tv,NULL);
				printf("[%ld.%06ld] Data logging finished (elapsed %d ms)...\n", tv.tv_sec,tv.tv_usec,recording_time_ms);
    		}
    	}

        while (cnt < ninput) {
        	cnt += ring_buffer.push_back(ninput-cnt, (gr_complex *)in[0]+cnt);
        	if (ring_buffer.is_buffer_ready()){
        		ring_buffer.fetch_cnt_buffer_and_proceed(&buf1);
            	if (!ring_buffer.is_popping_enabled()){
            		// Here, buffer is ready for detection
            		// If buffer is popping, no need to do any detection.
            		// We assume buffered samples are not too long, so we have enough time waiting for popping to finished before the next detection
            		if (lora_detect(buf1)>-1) {
            			// lora detected
            			ring_buffer.enable_popping(log_symbols,12);
            			timeval tv;
            		    gettimeofday(&tv,NULL);
            		    pmt::pmt_t sec = pmt::from_long(tv.tv_sec);
            		    pmt::pmt_t usec = pmt::from_long(tv.tv_usec);
            		    pmt::pmt_t len = pmt::from_long(log_symbols*sps);
            		    pmt::pmt_t pop_key = pmt::string_to_symbol("popping");
            		    //pmt::pmt_t offset = pmt::from_long(d_next_tag_pos);
            		    // printf("nitems_written(0) = %ld  nout = %d\n",nitems_written(0), nout);
            		    add_item_tag(0, nitems_written(0), pop_key, pmt::make_tuple(sec, usec, len));
						printf("[%ld.%06ld] Channel Activity Detected...\n", tv.tv_sec,tv.tv_usec);
            		}
            	}
        	}

        	if (nout<noutput_items && ring_buffer.is_popping_enabled()){
        		nout += ring_buffer.pop_buffer(noutput_items-nout, (gr_complex *)out[0]+nout);
        		if (!ring_buffer.is_popping_enabled()) {
        			timeval tv;
        		    gettimeofday(&tv,NULL);
					printf("[%ld.%06ld] Data logging finished (elapsed %d ms)...\n", tv.tv_sec,tv.tv_usec,recording_time_ms);
        		}
        	}
        }
      consume_each (ninput);

      // Tell runtime system how many output items we produced.
      return nout;
    }

  } /* namespace loraphy */
} /* namespace gr */

