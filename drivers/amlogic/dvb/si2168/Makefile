obj-$(CONFIG_AM_SI2168) += si2168_fe.o

si2168_src = src
nmi120_src = nmi120
nmi120_src_driver = nmi120/nmi_driver

si2168_fe-objs +=  sifrontend.o	
si2168_fe-objs +=  $(si2168_src)/Si2168_Commands_Strings.o  $(si2168_src)/Si2168_L1_API.o	 $(si2168_src)/Si2168_L1_Commands.o	$(si2168_src)/Si2168_L1_Properties.o  $(si2168_src)/Si2168_L2_API.o	\
										$(si2168_src)/Si2168_Properties_Strings.o  $(si2168_src)/SiLabs_API_L3_Wrapper.o $(si2168_src)/Si2168_L0_API.o
										
si2168_fe-objs +=  $(nmi120_src)/nmi_tuner_os.o	$(nmi120_src)/nmituner_api.o	$(nmi120_src_driver)/nmi_tuner_interface.o	$(nmi120_src_driver)/nmicmn.o \
										$(nmi120_src_driver)/nmitnr.o										

EXTRA_CFLAGS += -Idrivers/media/dvb/dvb-core
EXTRA_CFLAGS += -Idrivers/amlogic/dvb/si2168/include
EXTRA_CFLAGS += -Idrivers/amlogic/dvb/si2168
EXTRA_CFLAGS += -Idrivers/amlogic/dvb/si2168/nmi120
EXTRA_CFLAGS += -Idrivers/amlogic/dvb/si2168/nmi120/nmi_driver

