git clone https://github.com/plutinosoft/Platinum.git
Targets/x86_64-unknown-linux/Release/TextToHeader Source/Devices/MediaRenderer/AVTransportSCPD.{xml,cpp} -v RDR_AVTransportSCPD -h AVTransport
scons Platinum Neptune axTLS PltMediaRenderer build_config=Release -j5
