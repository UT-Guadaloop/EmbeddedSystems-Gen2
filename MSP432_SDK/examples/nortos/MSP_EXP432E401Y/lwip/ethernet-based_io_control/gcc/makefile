SIMPLELINK_MSP432E4_SDK_INSTALL_DIR ?= $(abspath ../../../../../..)

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

OBJECTS = lwip_ports_msp432e4_netif_msp432e4if.obj lwip_ports_msp432e4_perf.obj lwip_ports_msp432e4_sys_arch.obj lwip_ports_msp432e4_utils_locator.obj lwip_ports_msp432e4_utils_lwiplib.obj lwip_ports_msp432e4_utils_swupdate.obj lwip_src_api_api_lib.obj lwip_src_api_api_msg.obj lwip_src_api_err.obj lwip_src_api_netbuf.obj lwip_src_api_netdb.obj lwip_src_api_netifapi.obj lwip_src_api_sockets.obj lwip_src_api_tcpip.obj lwip_src_apps_httpd_httpd.obj lwip_src_apps_lwiperf_lwiperf.obj lwip_src_apps_mdns_mdns.obj lwip_src_apps_mqtt_mqtt.obj lwip_src_apps_netbiosns_netbiosns.obj lwip_src_apps_snmp_snmp_asn1.obj lwip_src_apps_snmp_snmp_core.obj lwip_src_apps_snmp_snmp_mib2.obj lwip_src_apps_snmp_snmp_mib2_icmp.obj lwip_src_apps_snmp_snmp_mib2_interfaces.obj lwip_src_apps_snmp_snmp_mib2_ip.obj lwip_src_apps_snmp_snmp_mib2_snmp.obj lwip_src_apps_snmp_snmp_mib2_system.obj lwip_src_apps_snmp_snmp_mib2_tcp.obj lwip_src_apps_snmp_snmp_mib2_udp.obj lwip_src_apps_snmp_snmp_msg.obj lwip_src_apps_snmp_snmp_netconn.obj lwip_src_apps_snmp_snmp_pbuf_stream.obj lwip_src_apps_snmp_snmp_raw.obj lwip_src_apps_snmp_snmp_scalar.obj lwip_src_apps_snmp_snmp_table.obj lwip_src_apps_snmp_snmp_threadsync.obj lwip_src_apps_snmp_snmp_traps.obj lwip_src_apps_snmp_snmpv3.obj lwip_src_apps_snmp_snmpv3_dummy.obj lwip_src_apps_snmp_snmpv3_mbedtls.obj lwip_src_apps_sntp_sntp.obj lwip_src_apps_tftp_tftp_server.obj lwip_src_core_def.obj lwip_src_core_dns.obj lwip_src_core_inet_chksum.obj lwip_src_core_init.obj lwip_src_core_ip.obj lwip_src_core_ipv4_autoip.obj lwip_src_core_ipv4_dhcp.obj lwip_src_core_ipv4_etharp.obj lwip_src_core_ipv4_icmp.obj lwip_src_core_ipv4_igmp.obj lwip_src_core_ipv4_ip4.obj lwip_src_core_ipv4_ip4_addr.obj lwip_src_core_ipv4_ip4_frag.obj lwip_src_core_ipv6_dhcp6.obj lwip_src_core_ipv6_ethip6.obj lwip_src_core_ipv6_icmp6.obj lwip_src_core_ipv6_inet6.obj lwip_src_core_ipv6_ip6.obj lwip_src_core_ipv6_ip6_addr.obj lwip_src_core_ipv6_ip6_frag.obj lwip_src_core_ipv6_mld6.obj lwip_src_core_ipv6_nd6.obj lwip_src_core_mem.obj lwip_src_core_memp.obj lwip_src_core_netif.obj lwip_src_core_pbuf.obj lwip_src_core_raw.obj lwip_src_core_stats.obj lwip_src_core_sys.obj lwip_src_core_tcp.obj lwip_src_core_tcp_in.obj lwip_src_core_tcp_out.obj lwip_src_core_timeouts.obj lwip_src_core_udp.obj lwip_src_netif_ethernet.obj lwip_src_netif_ethernetif.obj lwip_src_netif_lowpan6.obj lwip_src_netif_ppp_auth.obj lwip_src_netif_ppp_ccp.obj lwip_src_netif_ppp_chap-md5.obj lwip_src_netif_ppp_chap-new.obj lwip_src_netif_ppp_chap_ms.obj lwip_src_netif_ppp_demand.obj lwip_src_netif_ppp_eap.obj lwip_src_netif_ppp_ecp.obj lwip_src_netif_ppp_eui64.obj lwip_src_netif_ppp_fsm.obj lwip_src_netif_ppp_ipcp.obj lwip_src_netif_ppp_ipv6cp.obj lwip_src_netif_ppp_lcp.obj lwip_src_netif_ppp_magic.obj lwip_src_netif_ppp_mppe.obj lwip_src_netif_ppp_multilink.obj lwip_src_netif_ppp_polarssl_arc4.obj lwip_src_netif_ppp_polarssl_des.obj lwip_src_netif_ppp_polarssl_md4.obj lwip_src_netif_ppp_polarssl_md5.obj lwip_src_netif_ppp_polarssl_sha1.obj lwip_src_netif_ppp_ppp.obj lwip_src_netif_ppp_pppapi.obj lwip_src_netif_ppp_pppcrypt.obj lwip_src_netif_ppp_pppoe.obj lwip_src_netif_ppp_pppol2tp.obj lwip_src_netif_ppp_pppos.obj lwip_src_netif_ppp_upap.obj lwip_src_netif_ppp_utils.obj lwip_src_netif_ppp_vj.obj lwip_src_netif_slipif.obj pinout.obj uartstdio.obj ustdlib.obj cgifuncs.obj enet_io.obj io.obj io_fs.obj system_msp432e401y.obj gcc_startup_msp432e401y_gcc.obj

NAME = ethernet-based_io_control

CFLAGS = -I.. \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/ports/msp432e4/include" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/ports/msp432e4/include/utils" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/include" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/bsd" \
    -DDeviceFamily_MSP432E4 \
    -D__MSP432E401Y__ \
    -mcpu=cortex-m4 \
    -march=armv7e-m \
    -mthumb \
    -std=c99 \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -ffunction-sections \
    -fdata-sections \
    -g \
    -gstrict-dwarf \
    -Wall \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include/newlib-nano" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS = -Wl,-T,../gcc/msp432e401y.lds \
    "-Wl,-Map,$(NAME).map" \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    -l:ti/display/lib/display.am4fg \
    -l:ti/grlib/lib/gcc/m4f/grlib.a \
    -l:third_party/spiffs/lib/gcc/m4f/spiffs.a \
    -l:ti/drivers/lib/drivers_msp432e4.am4fg \
    -l:third_party/fatfs/lib/gcc/m4f/fatfs.a \
    -l:ti/devices/msp432e4/driverlib/lib/gcc/m4f/msp432e4_driverlib.a \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -static \
    -Wl,--gc-sections \
    "-L$(GCC_ARMCOMPILER)/arm-none-eabi/lib/thumb/v7e-m/hard" \
    -lgcc \
    -lc \
    -lm \
    -lnosys \
    --specs=nano.specs

all: $(NAME).out

lwip_ports_msp432e4_netif_msp432e4if.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/ports/msp432e4/netif/msp432e4if.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_ports_msp432e4_perf.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/ports/msp432e4/perf.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_ports_msp432e4_sys_arch.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/ports/msp432e4/sys_arch.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_ports_msp432e4_utils_locator.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/ports/msp432e4/utils/locator.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_ports_msp432e4_utils_lwiplib.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/ports/msp432e4/utils/lwiplib.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_ports_msp432e4_utils_swupdate.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/ports/msp432e4/utils/swupdate.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_api_api_lib.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/api/api_lib.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_api_api_msg.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/api/api_msg.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_api_err.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/api/err.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_api_netbuf.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/api/netbuf.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_api_netdb.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/api/netdb.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_api_netifapi.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/api/netifapi.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_api_sockets.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/api/sockets.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_api_tcpip.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/api/tcpip.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_httpd_httpd.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/httpd/httpd.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_lwiperf_lwiperf.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/lwiperf/lwiperf.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_mdns_mdns.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/mdns/mdns.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_mqtt_mqtt.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/mqtt/mqtt.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_netbiosns_netbiosns.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/netbiosns/netbiosns.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_asn1.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_asn1.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_core.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_core.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_mib2.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_mib2.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_mib2_icmp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_mib2_icmp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_mib2_interfaces.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_mib2_interfaces.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_mib2_ip.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_mib2_ip.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_mib2_snmp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_mib2_snmp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_mib2_system.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_mib2_system.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_mib2_tcp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_mib2_tcp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_mib2_udp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_mib2_udp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_msg.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_msg.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_netconn.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_netconn.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_pbuf_stream.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_pbuf_stream.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_raw.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_raw.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_scalar.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_scalar.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_table.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_table.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_threadsync.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_threadsync.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmp_traps.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmp_traps.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmpv3.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmpv3.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmpv3_dummy.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmpv3_dummy.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_snmp_snmpv3_mbedtls.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/snmp/snmpv3_mbedtls.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_sntp_sntp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/sntp/sntp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_apps_tftp_tftp_server.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/apps/tftp/tftp_server.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_def.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/def.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_dns.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/dns.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_inet_chksum.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/inet_chksum.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_init.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/init.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ip.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ip.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv4_autoip.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv4/autoip.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv4_dhcp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv4/dhcp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv4_etharp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv4/etharp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv4_icmp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv4/icmp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv4_igmp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv4/igmp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv4_ip4.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv4/ip4.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv4_ip4_addr.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv4/ip4_addr.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv4_ip4_frag.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv4/ip4_frag.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv6_dhcp6.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv6/dhcp6.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv6_ethip6.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv6/ethip6.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv6_icmp6.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv6/icmp6.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv6_inet6.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv6/inet6.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv6_ip6.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv6/ip6.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv6_ip6_addr.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv6/ip6_addr.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv6_ip6_frag.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv6/ip6_frag.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv6_mld6.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv6/mld6.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_ipv6_nd6.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/ipv6/nd6.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_mem.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/mem.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_memp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/memp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_netif.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/netif.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_pbuf.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/pbuf.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_raw.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/raw.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_stats.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/stats.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_sys.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/sys.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_tcp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/tcp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_tcp_in.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/tcp_in.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_tcp_out.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/tcp_out.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_timeouts.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/timeouts.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_core_udp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/core/udp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ethernet.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ethernet.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ethernetif.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ethernetif.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_lowpan6.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/lowpan6.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_auth.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/auth.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_ccp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/ccp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_chap-md5.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/chap-md5.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_chap-new.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/chap-new.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_chap_ms.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/chap_ms.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_demand.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/demand.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_eap.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/eap.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_ecp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/ecp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_eui64.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/eui64.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_fsm.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/fsm.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_ipcp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/ipcp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_ipv6cp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/ipv6cp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_lcp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/lcp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_magic.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/magic.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_mppe.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/mppe.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_multilink.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/multilink.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_polarssl_arc4.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/polarssl/arc4.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_polarssl_des.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/polarssl/des.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_polarssl_md4.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/polarssl/md4.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_polarssl_md5.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/polarssl/md5.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_polarssl_sha1.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/polarssl/sha1.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_ppp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/ppp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_pppapi.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/pppapi.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_pppcrypt.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/pppcrypt.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_pppoe.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/pppoe.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_pppol2tp.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/pppol2tp.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_pppos.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/pppos.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_upap.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/upap.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_utils.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/utils.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_ppp_vj.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/ppp/vj.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

lwip_src_netif_slipif.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/lwip/src/netif/slipif.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

pinout.obj: ../pinout.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

uartstdio.obj: ../uartstdio.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

ustdlib.obj: ../ustdlib.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

cgifuncs.obj: ../cgifuncs.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

enet_io.obj: ../enet_io.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

io.obj: ../io.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

io_fs.obj: ../io_fs.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

system_msp432e401y.obj: ../system_msp432e401y.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

gcc_startup_msp432e401y_gcc.obj: ../gcc/startup_msp432e401y_gcc.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

$(NAME).out: $(OBJECTS)
	@ echo linking $@
	@ $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
