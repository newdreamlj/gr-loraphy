/* -*- c++ -*- */

#define LORAPHY_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "loraphy_swig_doc.i"

%{
#include "loraphy/lora_phy_recorder.h"
%}


%include "loraphy/lora_phy_recorder.h"
GR_SWIG_BLOCK_MAGIC2(loraphy, lora_phy_recorder);
