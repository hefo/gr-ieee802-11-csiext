/*
 * Copyright (C) 2013 Bastian Bloessl <bloessl@ccs-labs.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define IEEE802_11_CSIEXT_API
#define DIGITAL_API

%include "gnuradio.i"

%include "ieee802_11_csiext_swig_doc.i"

%{
#include "ieee802-11-csiext/constellations.h"
#include "ieee802-11-csiext/decode_mac.h"
#include "ieee802-11-csiext/frame_equalizer.h"
#include "ieee802-11-csiext/parse_mac.h"
#include "ieee802-11-csiext/signal_field.h"
%}

%include "gnuradio/digital/packet_header_default.h"

%ignore gr::digital::constellation_bpsk;
%ignore gr::digital::constellation_qpsk;
%ignore gr::digital::constellation_16qam;
%include "gnuradio/digital/constellation.h"

%include "ieee802-11-csiext/constellations.h"
%include "ieee802-11-csiext/decode_mac.h"
%include "ieee802-11-csiext/frame_equalizer.h"
%include "ieee802-11-csiext/parse_mac.h"
%include "ieee802-11-csiext/signal_field.h"

GR_SWIG_BLOCK_MAGIC2(ieee802_11_csiext, decode_mac);
GR_SWIG_BLOCK_MAGIC2(ieee802_11_csiext, frame_equalizer);
GR_SWIG_BLOCK_MAGIC2(ieee802_11_csiext, parse_mac);

%template(signal_field_sptr) boost::shared_ptr<gr::ieee802_11_csiext::signal_field>;
%pythoncode %{
signal_field_sptr.__repr__ = lambda self: "<signal_field>"
signal_field = signal_field.make;
%}

%template(constellation_bpsk_sptr) boost::shared_ptr<gr::ieee802_11_csiext::constellation_bpsk>;
%pythoncode %{
constellation_bpsk_sptr.__repr__ = lambda self: "<constellation BPSK>"
constellation_bpsk = constellation_bpsk.make;
%}

%template(constellation_qpsk_sptr) boost::shared_ptr<gr::ieee802_11_csiext::constellation_qpsk>;
%pythoncode %{
constellation_qpsk_sptr.__repr__ = lambda self: "<constellation QPSK>"
constellation_qpsk = constellation_qpsk.make;
%}

%template(constellation_16qam_sptr) boost::shared_ptr<gr::ieee802_11_csiext::constellation_16qam>;
%pythoncode %{
constellation_16qam_sptr.__repr__ = lambda self: "<constellation 16QAM>"
constellation_16qam = constellation_16qam.make;
%}

%template(constellation_64qam_sptr) boost::shared_ptr<gr::ieee802_11_csiext::constellation_64qam>;
%pythoncode %{
constellation_64qam_sptr.__repr__ = lambda self: "<constellation 64QAM>"
constellation_64qam = constellation_64qam.make;
%}
