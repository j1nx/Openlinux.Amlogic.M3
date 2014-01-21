#
# aml_nftl
#

ifneq ($(KBUILD_SRC),)
TOP_KBUILD_SRC := $(KBUILD_SRC)/
endif

$(obj)/aml_nftl_new.o:
	@cp $(TOP_KBUILD_SRC)drivers/amlogic/nand/aml_nftl_new/aml_nftl_new.z \
        drivers/amlogic/nand/aml_nftl_new/aml_nftl_new.o

obj-$(CONFIG_AML_NFTL_NEW)	+= aml_nftl_new.o
