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

#include "buffer.h"

namespace gr {
  namespace loraphy {

	Buffer_t::Buffer_t():
		buf1(NULL),
		len_per_buffer(0),pos_in_current_buffer(0),
		num_of_buffers(0),cnt_active_buffer(0),
		is_cnt_buffer_full(false),is_popping(false),
		remaining_num_of_buffers_to_pop(0),pos_in_current_pop_buffer(0),cnt_pop_buffer(0){
		printf("Buffer_t constructed\n");
	}

	Buffer_t::~Buffer_t(){
		if (buf1) {
			for (int i=0; i<num_of_buffers; i++)
				volk_free(buf1[i]);
			delete buf1;
		}
	}

	void Buffer_t::init(int num_of_buffers,int len_per_buffer) {
		printf("Buffer_t initialized\n");
		this->num_of_buffers = num_of_buffers;
		this->cnt_active_buffer = 0;
		this->len_per_buffer = len_per_buffer;
		this->pos_in_current_buffer = 0;
		this->is_cnt_buffer_full = false;
		this->is_popping = false;
//		this->buf1 = (gr_complex**) volk_malloc (sizeof(gr_complex*) * num_of_buffers,  volk_get_alignment ());
		this->buf1 = new gr_complex* [num_of_buffers];
		for (int i=0; i<num_of_buffers; i++) {
			this->buf1[i] = (gr_complex*) volk_malloc (sizeof(gr_complex) * len_per_buffer,  volk_get_alignment ());
		}
	}

	/* push_back returns the number of samples successfully pushed */
	int Buffer_t::push_back(int len, gr_complex * buf){
		if (this->is_cnt_buffer_full) return 0;
		int safe_len = std::min<int>(this->len_per_buffer - this->pos_in_current_buffer, len);
		memcpy(this->buf1[this->cnt_active_buffer]+this->pos_in_current_buffer, buf, sizeof(gr_complex)*safe_len);
		this->pos_in_current_buffer += safe_len;
		if (this->pos_in_current_buffer > this->len_per_buffer) {
			printf("pos_in_current_buffer > len_per_buffer\n");
			return 0;
		}
		if (this->pos_in_current_buffer == this->len_per_buffer) {
			this->is_cnt_buffer_full = true;
		}
		return safe_len;
	}

	bool Buffer_t::is_buffer_ready(){
		return this->is_cnt_buffer_full;
	}

	bool Buffer_t::is_popping_enabled(){
		return this->is_popping;
	}

	void Buffer_t::buf_by_offset(int offset, gr_complex** buf){
		int target = (this->cnt_active_buffer+offset) % this->num_of_buffers;
		target = (target+this->num_of_buffers) % this->num_of_buffers;
		*buf = this->buf1[target];
	}

	bool Buffer_t::fetch_cnt_buffer_and_proceed(gr_complex** buf){
		if (!this->is_cnt_buffer_full) return false;
		this->is_cnt_buffer_full = false;
		*buf = this->buf1[this->cnt_active_buffer];
		this->pos_in_current_buffer = 0;
		this->cnt_active_buffer = (this->cnt_active_buffer + 1) % this->num_of_buffers;
		return true;
	}

	void Buffer_t::enable_popping(int buffers_to_pop,int buffers_lead){
		this->is_popping = true;
		this->remaining_num_of_buffers_to_pop = buffers_to_pop;
		this->pos_in_current_pop_buffer = 0;
		this->cnt_pop_buffer = (this->cnt_active_buffer-buffers_lead+this->num_of_buffers) % this->num_of_buffers;
	}

	int Buffer_t::pop_buffer(int len,gr_complex* buf){
		if (this->remaining_num_of_buffers_to_pop == 0) this->is_popping = false;
		if (!this->is_popping) return 0;
		if (this->cnt_pop_buffer == this->cnt_active_buffer) return 0;

		int npop = 0;
		int safe_len = std::min<int>(this->len_per_buffer - this->pos_in_current_pop_buffer, len);
		while (safe_len>0) {
			memcpy(buf, this->buf1[this->cnt_pop_buffer]+this->pos_in_current_pop_buffer, sizeof(gr_complex)*safe_len);
			buf += safe_len;
			npop += safe_len;
			len -= safe_len;
			this->pos_in_current_pop_buffer += safe_len;

			if (this->pos_in_current_pop_buffer == this->len_per_buffer) {
				this->cnt_pop_buffer = (this->cnt_pop_buffer+1)% this->num_of_buffers;
				this->pos_in_current_pop_buffer = 0;
				this->remaining_num_of_buffers_to_pop --;
			}
			if (this->remaining_num_of_buffers_to_pop == 0) break;
			if (this->cnt_pop_buffer == this->cnt_active_buffer) break;

			safe_len = std::min<int>(this->len_per_buffer - this->pos_in_current_pop_buffer, len);
		}
		if (this->remaining_num_of_buffers_to_pop == 0) this->is_popping = false;
		return npop;
	}
  } /* namespace loraphy */
} /* namespace gr */
