/* include/config.h.  Generated from config.h.in by configure.  */
/* include/config.h.in.  Generated from configure.in by autoheader.  */

/* Set if CLOCK_T is adequate by itself for the "indefinite future" (>= 100
   years) */
/* #undef CLOCK_T_IS_LONG_ENOUGH */

/* Use libc malloc instead of Heartbeat's custom one. */
/* #undef CL_USE_LIBC_MALLOC */

/* big-endian */
/* #undef CONFIG_BIG_ENDIAN */

/* little-endian */
#define CONFIG_LITTLE_ENDIAN 1

/* disable times(2) kludge */
/* #undef DISABLE_TIMES_KLUDGE */

/* enable -dlpreopen flag */
/* #undef DLPREOPEN */

/* pid inconsistent */
/* #undef GETPID_INCONSISTENT */

/* Define to 1 if you have the `alphasort' function. */
#define HAVE_ALPHASORT 1

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the <asm/page.h> header file. */
#define HAVE_ASM_PAGE_H 1

/* Define to 1 if you have the <bzlib.h> header file. */
#define HAVE_BZLIB_H 1

/* Define to 1 if you have the <curl/curl.h> header file. */
#define HAVE_CURL_CURL_H 1

/* Define to 1 if you have the <curses/curses.h> header file. */
/* #undef HAVE_CURSES_CURSES_H */

/* Define to 1 if you have the <curses.h> header file. */
#define HAVE_CURSES_H 1

/* Define to 1 if you have the `daemon' function. */
#define HAVE_DAEMON 1

/* Define to 1 if you have the <db.h> header file. */
#define HAVE_DB_H 1

/* Have getopt function */
#define HAVE_DECL_GETOPT 1

/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't.
   */
/* #undef HAVE_DECL_TZNAME */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <evs.h> header file. */
/* #undef HAVE_EVS_H */

/* Define to 1 if you have the `fcntl' function. */
#define HAVE_FCNTL 1

/* Define to 1 if you have the `flock' function. */
#define HAVE_FLOCK 1

/* Define to 1 if you have the `getopt' function. */
#define HAVE_GETOPT 1

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H 1

/* Define to 1 if you have the `getpeereid' function. */
/* #undef HAVE_GETPEEREID */

/* Define to 1 if you have the `getpeerucred' function. */
/* #undef HAVE_GETPEERUCRED */

/* Define to 1 if you have the <gnutls/gnutls.h> header file. */
#define HAVE_GNUTLS_GNUTLS_H 1

/* Define to 1 if you have the <hbaapi.h> header file. */
/* #undef HAVE_HBAAPI_H */

/* Do we have incompatible printw() in curses library? */
/* #undef HAVE_INCOMPATIBLE_PRINTW */

/* Define to 1 if you have the `inet_aton' function. */
#define HAVE_INET_ATON 1

/* Define to 1 if you have the `inet_pton' function. */
#define HAVE_INET_PTON 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `bz2' library (-lbz2). */
#define HAVE_LIBBZ2 1

/* Define to 1 if you have the `c' library (-lc). */
#define HAVE_LIBC 1

/* have curses library */
/* #undef HAVE_LIBCURSES */

/* Define to 1 if you have the `dl' library (-ldl). */
#define HAVE_LIBDL 1

/* Define to 1 if you have the `evs' library (-levs). */
/* #undef HAVE_LIBEVS */

/* Define to 1 if you have the `gnugetopt' library (-lgnugetopt). */
/* #undef HAVE_LIBGNUGETOPT */

/* Define to 1 if you have the `gnutls' library (-lgnutls). */
/* #undef HAVE_LIBGNUTLS */

/* Define to 1 if you have the `HBAAPI' library (-lHBAAPI). */
/* #undef HAVE_LIBHBAAPI */

/* Define to 1 if you have the <libintl.h> header file. */
#define HAVE_LIBINTL_H 1

/* have ncurses library */
#define HAVE_LIBNCURSES 1

/* Define to 1 if you have the `net' library (-lnet). */
/* #undef HAVE_LIBNET */

/* Libnet 1.0 API */
/* #undef HAVE_LIBNET_1_0_API */

/* Libnet 1.1 API */
#define HAVE_LIBNET_1_1_API 1

/* Define to 1 if you have the <libnet.h> header file. */
#define HAVE_LIBNET_H 1

/* Define to 1 if you have the `nsl' library (-lnsl). */
/* #undef HAVE_LIBNSL */

/* Define to 1 if you have the `pam' library (-lpam). */
#define HAVE_LIBPAM 1

/* Define to 1 if you have the `posix4' library (-lposix4). */
/* #undef HAVE_LIBPOSIX4 */

/* Define to 1 if you have the `rt' library (-lrt). */
#define HAVE_LIBRT 1

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the <libutil.h> header file. */
/* #undef HAVE_LIBUTIL_H */

/* Define to 1 if you have the `uuid' library (-luuid). */
#define HAVE_LIBUUID 1

/* Define to 1 if you have the `xml2' library (-lxml2). */
#define HAVE_LIBXML2 1

/* Define to 1 if you have the <libxml/xpath.h> header file. */
#define HAVE_LIBXML_XPATH_H 1

/* Define to 1 if you have the `z' library (-lz). */
#define HAVE_LIBZ 1

/* Define to 1 if you have the <linux/icmpv6.h> header file. */
#define HAVE_LINUX_ICMPV6_H 1

/* Define to 1 if you have the <linux/watchdog.h> header file. */
#define HAVE_LINUX_WATCHDOG_H 1

/* Define to 1 if you have the `mallinfo' function. */
#define HAVE_MALLINFO 1

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the `mallopt' function. */
#define HAVE_MALLOPT 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <ncurses.h> header file. */
#define HAVE_NCURSES_H 1

/* Define to 1 if you have the <ncurses/ncurses.h> header file. */
#define HAVE_NCURSES_NCURSES_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <netinet/in_systm.h> header file. */
#define HAVE_NETINET_IN_SYSTM_H 1

/* Define to 1 if you have the <netinet/ip_compat.h> header file. */
/* #undef HAVE_NETINET_IP_COMPAT_H */

/* Define to 1 if you have the <netinet/ip_fw.h> header file. */
/* #undef HAVE_NETINET_IP_FW_H */

/* Define to 1 if you have the <netinet/ip.h> header file. */
#define HAVE_NETINET_IP_H 1

/* Define to 1 if you have the <netinet/ip_var.h> header file. */
/* #undef HAVE_NETINET_IP_VAR_H */

/* Define to 1 if you have the <net/ethernet.h> header file. */
#define HAVE_NET_ETHERNET_H 1

/* Define to 1 if you have the <net-snmp/net-snmp-config.h> header file. */
#define HAVE_NET_SNMP_NET_SNMP_CONFIG_H 1

/* Define to 1 if you have the <net/tipc/tipc.h> header file. */
/* #undef HAVE_NET_TIPC_TIPC_H */

/* have new heartbeat api */
#define HAVE_NEW_HB_API 1

/* Do we have nfds_t? */
#define HAVE_NFDS_T 1

/* Define to 1 if you have the `NoSuchFunctionName' function. */
/* #undef HAVE_NOSUCHFUNCTIONNAME */

/* Define to 1 if you have the <openhpi/SaHpi.h> header file. */
#define HAVE_OPENHPI_SAHPI_H 1

/* Define to 1 if you have the <pam/pam_appl.h> header file. */
/* #undef HAVE_PAM_PAM_APPL_H */

/* Define to 1 if /proc/{pid} is supported. */
#define HAVE_PROC_PID 1

/* Define to 1 if you have the `pstat' function. */
/* #undef HAVE_PSTAT */

/* Define to 1 if you have the `scandir' function. */
#define HAVE_SCANDIR 1

/* Define to 1 if you have the <security/pam_appl.h> header file. */
#define HAVE_SECURITY_PAM_APPL_H 1

/* Define to 1 if you have the `setegid' function. */
#define HAVE_SETEGID 1

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if you have the `seteuid' function. */
#define HAVE_SETEUID 1

/* */
/* #undef HAVE_SETPROCTITLE */

/* Define to 1 if you have the `sigignore' function. */
/* #undef HAVE_SIGIGNORE */

/* */
/* #undef HAVE_SOCKADDR_IN_SIN_LEN */

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if cpp supports the ANSI # stringizing operator. */
#define HAVE_STRINGIZE 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcat' function. */
/* #undef HAVE_STRLCAT */

/* Define to 1 if you have the `strlcpy' function. */
/* #undef HAVE_STRLCPY */

/* Define to 1 if you have the `strndup' function. */
#define HAVE_STRNDUP 1

/* Define to 1 if you have the `strnlen' function. */
#define HAVE_STRNLEN 1

/* Define to 1 if you have the <stropts.h> header file. */
#define HAVE_STROPTS_H 1

/* Do we have struct cmsgcred? */
/* #undef HAVE_STRUCT_CMSGCRED */

/* Do we have struct cred? */
/* #undef HAVE_STRUCT_CRED */

/* Do we have struct fcred? */
/* #undef HAVE_STRUCT_FCRED */

/* Do we have struct sockcred? */
/* #undef HAVE_STRUCT_SOCKCRED */

/* Define to 1 if `tm_zone' is member of `struct tm'. */
#define HAVE_STRUCT_TM_TM_ZONE 1

/* Do we have struct ucred? */
/* #undef HAVE_STRUCT_UCRED */

/* Do we have the Darwin version of struct ucred? */
/* #undef HAVE_STRUCT_UCRED_DARWIN */

/* */
#define HAVE_SYSLOG_FACILITYNAMES 1

/* Define to 1 if you have the <sys/cred.h> header file. */
/* #undef HAVE_SYS_CRED_H */

/* Define to 1 if you have the <sys/filio.h> header file. */
/* #undef HAVE_SYS_FILIO_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/prctl.h> header file. */
#define HAVE_SYS_PRCTL_H 1

/* Define to 1 if you have the <sys/pstat.h> header file. */
/* #undef HAVE_SYS_PSTAT_H */

/* Define to 1 if you have the <sys/reboot.h> header file. */
#define HAVE_SYS_REBOOT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/sockio.h> header file. */
/* #undef HAVE_SYS_SOCKIO_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/syslimits.h> header file. */
/* #undef HAVE_SYS_SYSLIMITS_H */

/* Define to 1 if you have the <sys/termios.h> header file. */
#define HAVE_SYS_TERMIOS_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/ucred.h> header file. */
/* #undef HAVE_SYS_UCRED_H */

/* Define to 1 if you have the <tcpd.h> header file. */
#define HAVE_TCPD_H 1

/* */
#define HAVE_TERMIOS_C_LINE 1

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* Define to 1 if you have the <termio.h> header file. */
#define HAVE_TERMIO_H 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Do we have structure member tm_gmtoff? */
#define HAVE_TM_GMTOFF 1

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#define HAVE_TM_ZONE 1

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
/* #undef HAVE_TZNAME */

/* Define to 1 if you have the <ucd-snmp/snmp.h> header file. */
/* #undef HAVE_UCD_SNMP_SNMP_H */

/* Define to 1 if you have the <ucred.h> header file. */
/* #undef HAVE_UCRED_H */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `unsetenv' function. */
#define HAVE_UNSETENV 1

/* Define to 1 if you have the <uuid.h> header file. */
/* #undef HAVE_UUID_H */

/* Define to 1 if you have the `uuid_parse' function. */
#define HAVE_UUID_PARSE 1

/* Define to 1 if you have the <uuid/uuid.h> header file. */
#define HAVE_UUID_UUID_H 1

/* Define to 1 if you have the <vacmclient_api.h> header file. */
/* #undef HAVE_VACMCLIENT_API_H */

/* Define to 1 if you have the <xti.h> header file. */
/* #undef HAVE_XTI_H */

/* Define to 1 if you have the <zlib.h> header file. */
#define HAVE_ZLIB_H 1

/* Define to 1 if you have the `__default_morecore' function. */
#define HAVE___DEFAULT_MORECORE 1

/* */
#define HAVE___PROGNAME 1

/* id for api group */
#define HA_APIGID 496

/* group for our programs */
#define HA_APIGROUP "haclient"

/* id for ccm user */
#define HA_CCMUID 498

/* user to run privileged non-root things as */
#define HA_CCMUSER "hacluster"

/* top directory of area to drop core files in */
#define HA_COREDIR "/var/lib/heartbeat/cores"

/* data (arch-independent) directory */
#define HA_DATADIR "/usr/share"

/* Heartbeat configuration directory */
#define HA_HBCONF_DIR "/etc/ha.d"

/* lib directory */
#define HA_LIBDIR "/usr/lib"

/* $HB_PKG lib directory */
#define HA_LIBHBDIR "/usr/lib/heartbeat"

/* Logging Daemon IPC socket name */
#define HA_LOGDAEMON_IPC "/var/lib/log_daemon"

/* Default logging facility */
#define HA_LOG_FACILITY LOG_DAEMON

/* $HB_PKG noarch data directory */
#define HA_NOARCHDATAHBDIR "/usr/share/heartbeat"

/* PILS plugin dir */
#define HA_PLUGIN_DIR "/usr/lib/heartbeat/plugins"

/* heartbeat v1 script directory */
#define HA_RC_DIR "/etc/ha.d/rc.d"

/* Location of system configuration files */
#define HA_SYSCONFDIR "/etc"

/* Web site base URL */
#define HA_URLBASE "http://linux-ha.org/"

/* var lib directory */
#define HA_VARLIBDIR "/var/lib"

/* var lib heartbeat directory */
#define HA_VARLIBHBDIR "/var/lib/heartbeat"

/* System lock directory */
#define HA_VARLOCKDIR "/var/lock"

/* var log directory */
#define HA_VARLOGDIR "/var/log"

/* var run directory */
#define HA_VARRUNDIR "/var/run"

/* var run heartbeat directory */
#define HA_VARRUNHBDIR "/var/run/heartbeat"

/* var run heartbeat rsctmp directory */
#define HA_VARRUNHBRSCDIR "/var/run/heartbeat/rsctmp"

/* init start priority */
#define HB_INITSTARTPRI "75"

/* init stop priority */
#define HB_INITSTOPPRI "05"

/* Location for v1 Heartbeat v1 RAs */
#define HB_RA_DIR "/etc/ha.d/resource.d/"

/* path to the ifconfig command */
#define IFCONFIG "/sbin/ifconfig"

/* option for ifconfig command */
#define IFCONFIG_A_OPT "-a"

/* LRM plugin dir */
#define LRM_PLUGIN_DIR "/usr/lib/heartbeat/plugins/RAExec"

/* Location for LSB RAs */
#define LSB_RA_DIR "/etc/init.d"

/* Define to 1 if the mgmt-daemon and friends are enabled. */
#define MGMT_ENABLED 1

/* path to the netstat command */
#define NETSTAT "/bin/netstat"

/* parameters to netstat to retrieve route information */
#define NETSTATPARM "-rn "

/* Location for OCF RAs */
#define OCF_RA_DIR "/usr/lib/ocf/resource.d/"

/* OCF root directory - specified by the OCF standard */
#define OCF_ROOT_DIR "/usr/lib/ocf"

/* Compiling for Darwin platform */
/* #undef ON_DARWIN */

/* Name of package */
#define PACKAGE "heartbeat"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* mechanism to pretty-print ps output: setproctitle-equivalent */
#define PF_ARGV_TYPE PF_ARGV_WRITEABLE

/* option for ping timeout */
#define PING_TIMEOUT_OPT "-w1"

/* path to the poweroff command */
#define POWEROFF_CMD "/usr/bin/poweroff"

/* poweroff options */
#define POWEROFF_OPTIONS "-nf"

/* path were to find route information in proc */
#define PROCROUTE "/proc/net/route"

/* path to the reboot command */
#define REBOOT "/usr/bin/reboot"

/* reboot options */
#define REBOOT_OPTIONS "-nf"

/* path to route command */
#define ROUTE "/sbin/route"

/* paramters for route to retrieve route information */
#define ROUTEPARM "-n get"

/* The size of `char', as computed by sizeof. */
#define SIZEOF_CHAR 1

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* directory for the external stonith plugins */
#define STONITH_EXT_PLUGINDIR "/usr/lib/stonith/plugins/external"

/* Location for STONITH RAs */
#define STONITH_RA_DIR "/usr/lib/heartbeat/stonith.d/"

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Name of UCD SNMP development package */
#define UCDSNMPDEVEL 

/* Valgrind command */
#define VALGRIND_BIN "/usr/bin/valgrind"

/* Valgrind logging options */
#define VALGRIND_LOG "--log-socket=127.0.0.1:1234"

/* Name of a suppression file to pass to Valgrind */
#define VALGRIND_SUPP "/dev/null"

/* Version number of package */
#define VERSION "2.1.3"

/* Use the new Cluster Resource Manager */
#define WITH_CRM 1

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
#define YYTEXT_POINTER 1

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */
