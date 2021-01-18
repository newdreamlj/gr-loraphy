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


#ifndef INCLUDED_LORAPHY_LORA_PHY_RECORDER_IMPL_H
#define INCLUDED_LORAPHY_LORA_PHY_RECORDER_IMPL_H

#include <loraphy/lora_phy_recorder.h>
#include "fft_fftw.h"
#include "buffer.h"

namespace gr {
    namespace loraphy {

        class lora_phy_recorder_impl : public lora_phy_recorder
        {
          private:
            int sf, bw, fs, recording_time_ms;
            uint64_t freq_center, freq_lora;
            int sps, log_symbols, nbins, ntaps, deci;
            fft_complex_t *d_fft;
            int n_chan;
            // downchirp .* phase_caused_by_freq_cov:2^SF,  filter_taps .* freq_cov:ntap
            gr_complex *filter_taps_dot_freq_cov, * freq_cov_phase_calibration, *downchirp, *downchirp_dot_phase_calibration;

            Buffer_t * ring_buffers;
            float *mag;

            gr_complex * buf1, * baseband;

            int lora_detect(gr_complex*);

          public:
            lora_phy_recorder_impl(int sf, int bw, int fs, int recording_time_ms, uint64_t freq_center, uint64_t freq_lora,int n_chan);
            ~lora_phy_recorder_impl();

          // Where all the action really happens
            void forecast (int noutput_items, gr_vector_int &ninput_items_required);

            int general_work(int noutput_items,
               gr_vector_int &ninput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
        };
    } // namespace loraphy
} // namespace gr

#endif /* INCLUDED_LORAPHY_LORA_PHY_RECORDER_IMPL_H */

