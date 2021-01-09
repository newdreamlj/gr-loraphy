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

#ifndef LORAPHY_BUFFER_H_
#define LORAPHY_BUFFER_H_

#include <gnuradio/gr_complex.h>
#include <string.h>
#include <volk/volk.h>

namespace gr {
    namespace loraphy {

        class Buffer_t {
        private:
            gr_complex ** buf1;
            /* parameters related to samples in one block */
            int len_per_buffer, pos_in_current_buffer;
            /* parameters related to buffer blocks */
            int num_of_buffers, cnt_active_buffer;
            /* parameters related to popping */
            bool is_cnt_buffer_full, is_popping;
            int remaining_num_of_buffers_to_pop, pos_in_current_pop_buffer, cnt_pop_buffer;

        public:
            Buffer_t();
            ~Buffer_t();
            void init(int num_of_buffers, int len_per_buffer);
            /* push_back returns the number of samples successfully pushed */
            int push_back(int len, gr_complex * buf1);
            int pop_buffer(int len,gr_complex* buf1);
            bool is_buffer_ready();
            bool is_popping_enabled();
            bool fetch_cnt_buffer_and_proceed(gr_complex** buf1);
            void buf_by_offset(int offset,gr_complex** buf1);
            void enable_popping(int buffers_to_pop,int buffers_lead);

        }; //Buffer_t
    } /* namespace loraphy */
} /*namespace gr */

#endif /* LORAPHY_BUFFER_H_ */
