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

#ifndef INCLUDED_LORAPHY_LORA_PHY_RECORDER_H
#define INCLUDED_LORAPHY_LORA_PHY_RECORDER_H

#include <loraphy/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace loraphy {

    /*!
     * \brief detect LoRa packet based on up-chirp active, and record the following several ms
     * \ingroup loraphy
     *
     */
    class LORAPHY_API lora_phy_recorder : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<lora_phy_recorder> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of loraphy::lora_phy_recorder.
       *
       * To avoid accidental use of raw pointers, loraphy::lora_phy_recorder's
       * constructor is in a private implementation
       * class. loraphy::lora_phy_recorder::make is the public interface for
       * creating new instances.
       */
      static sptr make(int sf, int bw, int fs, int recording_time_ms, uint64_t freq_center, uint64_t freq_lora);
    };

  } // namespace loraphy
} // namespace gr

#endif /* INCLUDED_LORAPHY_LORA_PHY_RECORDER_H */

