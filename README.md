
This is a CSI extension to the IEEE 802.11 a/g/p transceiver for GNU Radio available at https://github.com/bastibl/gr-ieee802-11 [1].

The main purpose of the extension is to expose the channel state information (CSI) to the user interface. With this functionality, the WiFi tranceiver [1] and an Ettus N210 USRP can be used to collect CSI measurements that, for example, can be used for physical layer security schemes (e.g., key agreement and authentication).

This extension requires GNU Radio 3.7 (or later) and gr-ieee802-11 available from [1].

# Installation

1. Follow the installation instructions for [gr-ieee802-11](https://github.com/bastibl/gr-ieee802-11)
2. Install the CSIEXT modules in this repository
3. Replace the blocks 'WiFi Frame Equalizer' and 'WiFi Parse MAC' with the installed blocks (CSIEXT) 


# Example

Running the wifi_tranceiver_csiext flowgraph will allow Tx and Rx where the metadata for received frames is fed to a TCP port through a Socket PDU as CSV string. The format of the metadata is the following

'Timestamp','SSID','SRC MAC','DST MAC','BSS MAC','Sequence Nr','SNR','Carrier frequency offset','CSI'

Example of a CSV output:

2019-09-24 15:54:08.250,No SSID,30:14:4a:e6:46:e4,12:34:56:78:90:ab,42:42:42:42:42:42,1,5,-456,0.001132+0.000269i,0.000774-0.000111i,0.000836+0.000135i,0.001289-0.000152i,0.000800+0.000091i,0.001378+0.000117i,0.001120+0.000094i,0.001254+0.000384i,0.001534-0.000159i,0.001149+0.000196i,0.001389+0.000029i,0.000953+0.000277i,0.001247+0.000471i,0.001480+0.000064i,0.001156+0.000263i,0.001242+0.000642i,0.001178+0.000019i,0.001501-0.000187i,0.001118+0.000157i,0.001306+0.000537i,0.001152+0.000415i,0.000862+0.000236i,0.001676-0.000026i,0.001255+0.000409i,0.001437-0.000091i,0.002253+0.000411i,0.001401-0.000860i,0.001749+0.000259i,0.000771-0.000469i,0.001249-0.000771i,0.000770-0.000627i,0.000934-0.000310i,0.001034-0.000025i,0.000690-0.000088i,0.000887-0.000364i,0.001383-0.000804i,0.001042-0.000724i,0.001174-0.000103i,0.001314+0.000318i,0.001305-0.000555i,0.001081-0.000719i,0.000861-0.000534i,0.000921-0.000213i,0.000699-0.000821i,0.000553-0.000507i,0.001016-0.000452i,0.001274-0.000629i,0.000961-0.001028i,0.001242-0.000053i,0.000740-0.000687i,0.000631-0.000653i,0.000811-0.000396i

 
## Note for Mac OSX

On Mac OSX, I used the following arguments to cmake

CC=/usr/bin/llvm-gcc CXX=/usr/bin/llvm-g++ cmake -DPYTHON_EXECUTABLE=/opt/local/bin/python2.7 -DPYTHON_INCLUDE_DIR=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/Headers -DPYTHON_LIBRARY=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/Python -DSPHINX_EXECUTABLE=/opt/local/bin/rst2html-2.7.py -DGR_PYTHON_DIR=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages -DCMAKE_INSTALL_PREFIX=/opt/local ..

