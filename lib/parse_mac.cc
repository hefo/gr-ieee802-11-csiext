/*
 * Copyright (C) 2013, 2016 Bastian Bloessl <bloessl@ccs-labs.org>
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
#include <ieee802-11-csiext/parse_mac.h>
#include "utils.h"

#include <gnuradio/io_signature.h>
#include <gnuradio/block_detail.h>
#include <string>
#include <iostream>
#include <sys/time.h>
#include <time.h>



using namespace gr::ieee802_11_csiext;

class parse_mac_impl : public parse_mac {

public:

std::string SSID = "No SSID";
parse_mac_impl(bool log, bool debug, bool logcsi,std::string filename,bool filter_udp,int dest_filter) :
		block("parse_mac",
				gr::io_signature::make(0, 0, 0),
				gr::io_signature::make(0, 0, 0)),
		d_log(log), d_last_seq_no(-1),
		d_snr(0), //HF
		d_cfo(0), //HF
		d_enc(0),
		d_debug(debug),
		d_logcsi(logcsi),
		d_filter_udp(filter_udp),
		d_filename(filename),
		d_dest_filter(dest_filter) {

	message_port_register_in(pmt::mp("in"));
	set_msg_handler(pmt::mp("in"), boost::bind(&parse_mac_impl::parse, this, _1));

	message_port_register_out(pmt::mp("fer"));
	
	// Mod
	message_port_register_out(pmt::mp("csi_data"));
	message_port_register_out(pmt::mp("udp_payload"));
	
}

~parse_mac_impl() {

}

void parse(pmt::pmt_t msg) {
	
	pmt::pmt_t dict = pmt::car(msg);
	
	if(pmt::is_eof_object(msg)) {
		detail().get()->set_done(true);
		return;
	} else if(pmt::is_symbol(msg)) {
		return;
	}
	
	msg = pmt::cdr(msg);

	int data_len = pmt::blob_length(msg);
	mac_header *h = (mac_header*)pmt::blob_data(msg);

	mylog(boost::format("length: %1%") % data_len );
	
	
	// mod hefo
	char *frame = (char*)pmt::blob_data(msg);
	/*if ((((h->frame_control >> 2) & 3) != 2) || (*(frame + 32) = 0x08) || (*(frame + 33) != 0x00) || (*(frame + 43) != 0x11 ) ){
			return;
	}*/
	
	if(d_filter_udp == true){
	
		bool udp_detected = false;
		if(((h->frame_control >> 2) & 3) == 2){ //if DATA
			if((*(frame + 32) == 0x08) & (*(frame + 33) == 0x00)){//if IPv4
				if(*(frame + 43) == 0x11 ){
					dout << std::endl << "----- UDP packet detected -----" << std::endl;
					udp_detected = true;
				}
			}
		}
	
		if(udp_detected == false){return;}	
	}

	dout << std::endl << "new mac frame (length " << data_len << ")" << std::endl;
	dout << "=========================================" << std::endl; 
	
	if(data_len < 20) {
		dout << "frame too short to parse (<20)" << std::endl;
		return;
	}
	#define HEX(a) std::hex << std::setfill('0') << std::setw(2) << int(a) << std::dec
	dout << "duration: " << HEX(h->duration >> 8) << " " << HEX(h->duration  & 0xff) << std::endl;
	dout << "frame control: " << HEX(h->frame_control >> 8) << " " << HEX(h->frame_control & 0xff);

        switch((h->frame_control >> 2) & 3) {

		case 0:
			dout << " (MANAGEMENT)" << std::endl;
			parse_management((char*)h, data_len);
			break;
		case 1:
			dout << " (CONTROL)" << std::endl;
			parse_control((char*)h, data_len);
			break;

		case 2:
			dout << " (DATA)" << std::endl;
			parse_data((char*)h, data_len);
			break;

		default:
			dout << " (unknown)" << std::endl;
			break;
	}

	//char *frame = (char*)pmt::blob_data(msg);
	
	/*std::cout << "Entire frame:";
	print_hex(frame, data_len);
	std::cout << "\n";*/
	
	// DATA
	std::ostringstream udp_payload;
	
	if(d_filter_udp == true){
		int dest_port = ((unsigned char)frame[56]<<8)+(unsigned char)frame[57];
		std::cout << "Destination port:" << dest_port << "\n";	
		std::cout << "UDP Payload:";
		print_ascii(frame + 62, data_len - 62);
		std::cout << "\n";
			
		udp_payload << "DEST_"<< dest_port << "_";
		
		char* buf = frame + 62;
		for(int i = 0; i < data_len - 62; i++) {
			if((buf[i] > 31) && (buf[i] < 127)) {
				udp_payload << buf[i];
			} else {
				udp_payload << ".";
			}
		}
		udp_payload << "\n";
		
	} else {	
		if((((h->frame_control) >> 2) & 63) == 2) {
			print_ascii(frame + 24, data_len - 24);
			//std::cout << "MAC header stripped:";
			//print_hex(frame + 24, data_len - 24);
			//std::cout << "\n";	
		// QoS Data
		} else if((((h->frame_control) >> 2) & 63) == 34) {
			print_ascii(frame + 26, data_len - 26);
			//std::cout << "MAC header stripped:";
			//print_hex(frame + 26, data_len - 26);
			//std::cout << "\n";
		}
			
	}
	
	
	// ---------------------- mod (hefo@kth.se) ---------------------- ----------------------	

	std::ostringstream outfile;
	
	// Write microsecond timestamp
	struct timeval tv;
	struct tm* ptm;
	char time_string[40];
	long milliseconds;
	gettimeofday (&tv, NULL);
	ptm = localtime (&tv.tv_sec);
	strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
	milliseconds = tv.tv_usec / 1000;
	outfile << time_string << "." << milliseconds << ",";
	
	
	// Write SSID
	outfile << SSID + ",";
	
	// Extract and write source MAC adress
	uint8_t *mac_addr = h->addr1;

	outfile << std::setfill('0') << std::hex << std::setw(2);
	for(int i = 0; i < 6; i++) {
		outfile << (int)mac_addr[i];
		if(i != 5) {
			outfile << ":";
		}
	}
	outfile << std::dec << ",";
	
	// Extract and write destination MAC adress
	mac_addr = (h->addr2);

	outfile << std::setfill('0') << std::hex << std::setw(2);
	for(int i = 0; i < 6; i++) {
		outfile << (int)mac_addr[i];
		if(i != 5) {
			outfile << ":";
		}
	}
	outfile << std::dec << ",";
	
	// Extract and write BSS MAC adress
	mac_addr = (h->addr3);

	outfile << std::setfill('0') << std::hex << std::setw(2);
	for(int i = 0; i < 6; i++) {
		outfile << (int)mac_addr[i];
		if(i != 5) {
			outfile << ":";
		}
	}
	outfile << std::dec << ",";
	
	
	// Extract SNR
	if(pmt::dict_has_key(dict, pmt::mp("snr"))) {
		pmt::pmt_t s = pmt::dict_ref(dict, pmt::mp("snr"), pmt::PMT_NIL);
		if(pmt::is_number(s)) {
			d_snr = std::round(pmt::to_double(s));
		}
	}
	
	// Extract CFO
	if(pmt::dict_has_key(dict, pmt::mp("freqofs"))) {
		pmt::pmt_t s = pmt::dict_ref(dict, pmt::mp("freqofs"), pmt::PMT_NIL);
		if(pmt::is_number(s)) {
			d_cfo = std::round(pmt::to_double(s));
		}
	}
	
	outfile << int(h->seq_nr >> 4) << ","; // Write frame sequence number...
	outfile << double(d_snr) << ","; // signal-to-noise ratio...
	outfile << double(d_cfo) << ","; // carrier frequency offset...
	outfile << pmt::dict_ref(dict, pmt::mp("csi"), pmt::PMT_NIL) << "\n"; // and channel state information.
	
	
	// New Mod: Create string for writing to PAYLOAD output

	std::string csi_string = outfile.str(); 	
	pmt::pmt_t p = pmt::make_u8vector(csi_string.length(),0);				
	for(int i = 0; i < csi_string.length(); i++) {
		
		pmt::u8vector_set(p,i,csi_string[i]);
	
	}		
	message_port_pub(pmt::mp("csi_data"),pmt::cons(pmt::PMT_NIL, p));
	
	
	if(d_filter_udp == true){
		
		int dest_port = ((unsigned char)frame[56]<<8)+(unsigned char)frame[57];
		if (dest_port == d_dest_filter){
		
			std::string udp_string = udp_payload.str();	
			pmt::pmt_t p_udp = pmt::make_u8vector(udp_string.length(),0);				
			for(int i = 0; i < udp_string.length(); i++) {
				pmt::u8vector_set(p_udp,i,udp_string[i]);
	
			}
			//std::cout << "DEBUG:" << d_dest_filter << "\n";
			//std::cout << "DEBUG:" << dest_port << "\n";
			message_port_pub(pmt::mp("udp_payload"),pmt::cons(pmt::PMT_NIL, p_udp));
		}
	}
	
	
	
	if(!d_logcsi) {
		return;
	}
	std::ofstream outfile_csv;
	outfile_csv.open(d_filename, std::ios_base::app); // Create csv file
	outfile_csv << csi_string;
	outfile_csv.close();
	
	// ---------------------- ---------------------- ---------------------- ----------------------		
}

void parse_management(char *buf, int length) {
	mac_header* h = (mac_header*)buf;

	if(length < 24) {
		dout << "too short for a management frame" << std::endl;
		return;
	}

	dout << "Subtype: ";
	switch(((h->frame_control) >> 4) & 0xf) {
		case 0:
			dout << "Association Request";
			break;
		case 1:
			dout << "Association Response";
			break;
		case 2:
			dout << "Reassociation Request";
			break;
		case 3:
			dout << "Reassociation Response";
			break;
		case 4:
			dout << "Probe Request";
			break;
		case 5:
			dout << "Probe Response";
			break;
		case 6:
			dout << "Timing Advertisement";
			break;
		case 7:
			dout << "Reserved";
			break;
		case 8:
			dout << "Beacon" << std::endl;
			if(length < 38) {
				return;
			}
			{
			uint8_t* len = (uint8_t*) (buf + 24 + 13);
			if(length < 38 + *len) {
				return;
			}
			std::string s(buf + 24 + 14, *len);
			dout << "SSID: " << s;
			SSID = s;
			}
			break;
		case 9:
			dout << "ATIM";
			break;
		case 10:
			dout << "Disassociation";
			break;
		case 11:
			dout << "Authentication";
			break;
		case 12:
			dout << "Deauthentication";
			break;
		case 13:
			dout << "Action";
			break;
		case 14:
			dout << "Action No ACK";
			break;
		case 15:
			dout << "Reserved";
			break;
		default:
			break;
	}
	dout << std::endl;

	dout << "seq nr: " << int(h->seq_nr >> 4) << std::endl;
	dout << "mac 1: ";
	print_mac_address(h->addr1, true);
	dout << "mac 2: ";
	print_mac_address(h->addr2, true);
	dout << "mac 3: ";
	print_mac_address(h->addr3, true);

}


void parse_data(char *buf, int length) {
	mac_header* h = (mac_header*)buf;
	if(length < 24) {
		dout << "too short for a data frame" << std::endl;
		return;
	}

	dout << "Subtype: ";
	switch(((h->frame_control) >> 4) & 0xf) {
		case 0:
			dout << "Data";
			break;
		case 1:
			dout << "Data + CF-ACK";
			break;
		case 2:
			dout << "Data + CR-Poll";
			break;
		case 3:
			dout << "Data + CF-ACK + CF-Poll";
			break;
		case 4:
			dout << "Null";
			break;
		case 5:
			dout << "CF-ACK";
			break;
		case 6:
			dout << "CF-Poll";
			break;
		case 7:
			dout << "CF-ACK + CF-Poll";
			break;
		case 8:
			dout << "QoS Data";
			break;
		case 9:
			dout << "QoS Data + CF-ACK";
			break;
		case 10:
			dout << "QoS Data + CF-Poll";
			break;
		case 11:
			dout << "QoS Data + CF-ACK + CF-Poll";
			break;
		case 12:
			dout << "QoS Null";
			break;
		case 13:
			dout << "Reserved";
			break;
		case 14:
			dout << "QoS CF-Poll";
			break;
		case 15:
			dout << "QoS CF-ACK + CF-Poll";
			break;
		default:
			break;
	}
	dout << std::endl;

	int seq_no = int(h->seq_nr >> 4);
	dout << "seq nr: " << seq_no << std::endl;
	dout << "mac 1: ";
	print_mac_address(h->addr1, true);
	dout << "mac 2: ";
	print_mac_address(h->addr2, true);
	dout << "mac 3: ";
	print_mac_address(h->addr3, true);

	float lost_frames = seq_no - d_last_seq_no - 1;
	if(lost_frames  < 0)
		lost_frames += 1 << 12;

	// calculate frame error rate
	float fer = lost_frames / (lost_frames + 1);
	dout << "instantaneous fer: " << fer << std::endl;

	// keep track of values
	d_last_seq_no = seq_no;

	// publish FER estimate
	pmt::pmt_t pdu = pmt::make_f32vector(lost_frames + 1, fer * 100);
	message_port_pub(pmt::mp("fer"), pmt::cons( pmt::PMT_NIL, pdu ));
	

}

void parse_control(char *buf, int length) {
	mac_header* h = (mac_header*)buf;

	dout << "Subtype: ";
	switch(((h->frame_control) >> 4) & 0xf) {
		case 7:
			dout << "Control Wrapper";
			break;
		case 8:
			dout << "Block ACK Requrest";
			break;
		case 9:
			dout << "Block ACK";
			break;
		case 10:
			dout << "PS Poll";
			break;
		case 11:
			dout << "RTS";
			break;
		case 12:
			dout << "CTS";
			break;
		case 13:
			dout << "ACK";
			break;
		case 14:
			dout << "CF-End";
			break;
		case 15:
			dout << "CF-End + CF-ACK";
			break;
		default:
			dout << "Reserved";
			break;
	}
	dout << std::endl;

	dout << "RA: ";
	print_mac_address(h->addr1, true);
	dout << "TA: ";
	print_mac_address(h->addr2, true);

}

void print_mac_address(uint8_t *addr, bool new_line = false) {
	if(!d_debug) {
		return;
	}

	std::cout << std::setfill('0') << std::hex << std::setw(2);

	for(int i = 0; i < 6; i++) {
		std::cout << (int)addr[i];
		if(i != 5) {
			std::cout << ":";
		}
	}

	std::cout << std::dec;

	if(new_line) {
		std::cout << std::endl;
	}
}

void print_ascii(char* buf, int length) {

	for(int i = 0; i < length; i++) {
		if((buf[i] > 31) && (buf[i] < 127)) {
			dout << buf[i];
		} else {
			dout << ".";
		}
	}
	dout << std::endl;
}

// Mod HF 
void print_hex(char* buf, int length) {
	std::cout << std::setfill('0') << std::hex << std::setw(2);
	for(int i = 0; i < length; i++) {
		std::cout << (unsigned int)(unsigned char)buf[i];
		std::cout << std::dec;
		std::cout << " ";
		std::cout << std::setfill('0') << std::hex << std::setw(2);
	}
	std::cout << std::dec;
	std::cout << std::endl;
}

private:
	bool d_log;
	bool d_debug;
	bool d_logcsi;
	bool d_filter_udp;
	int d_dest_filter;
	int dest_port = 5010;
	std::string d_filename;
	int d_last_seq_no;
	uint64_t d_enc;
	double d_snr; //HF
	double d_cfo; //HF
	std::string d_csi;
};

parse_mac::sptr
parse_mac::make(bool log, bool debug, bool logcsi,std::string filename,bool filter_udp,int dest_filter) {
	return gnuradio::get_initial_sptr(new parse_mac_impl(log, debug, logcsi, filename,filter_udp,dest_filter));
}


