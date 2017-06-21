deps_config := \
	/home/Admin/esp-idf/components/aws_iot/Kconfig \
	/home/Admin/esp-idf/components/bt/Kconfig \
	/home/Admin/esp-idf/components/esp32/Kconfig \
	/home/Admin/esp-idf/components/ethernet/Kconfig \
	/home/Admin/esp-idf/components/fatfs/Kconfig \
	/home/Admin/esp-idf/components/freertos/Kconfig \
	/home/Admin/esp-idf/components/log/Kconfig \
	/home/Admin/esp-idf/components/lwip/Kconfig \
	/home/Admin/esp-idf/components/mbedtls/Kconfig \
	/home/Admin/esp-idf/components/openssl/Kconfig \
	/home/Admin/esp-idf/components/spi_flash/Kconfig \
	/home/Admin/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/Admin/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/Admin/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/Admin/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
