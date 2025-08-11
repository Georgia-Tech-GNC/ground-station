#pragma once

#define MG_ARCH MG_ARCH_ARMGCC     // For all ARM GCC- and newlib-based environments 
#define MG_ENABLE_TCPIP 1          // Enables built-in TCP/IP stack
#define MG_ENABLE_CUSTOM_MILLIS 1  // We must implement mg_millis()
#define MG_ENABLE_TCPIP_PRINT_DEBUG_STATS 1  // Enable debug stats log
