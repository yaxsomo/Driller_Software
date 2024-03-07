/**
 * @file macAddr.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef MAC_ADDR_H
#define MAC_ADDR_H

#include <string>
#include <boost/log/trivial.hpp>

// This is Linux only code...
#ifdef __linux__

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <sstream>
#include <unistd.h>

#define DEFAULT_USED_INTERFACE "enp3s0"

/**
 * @brief Get the Mac Address object Function returning the specified interface's mac address
 * @param iface Interface to get the mac address from
 * @return String containing the mac address
 */
static std::string getMacAddress(const std::string& interface)
{
    int fd;
	struct ifreq ifr;
	char *iface;

	// Check environment variable validity
	if(interface != "")
	{
		iface = (char*)interface.c_str();
	} else {
		iface = (char*)DEFAULT_USED_INTERFACE;
	}
	
	unsigned char *mac;
    std::string macStr;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy((char *)ifr.ifr_name , (const char *)iface , IFNAMSIZ-1);

	if( ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
		throw std::invalid_argument("Specified network interface doesn't exist or is incorrect.");
	}	

	close(fd);
	
	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

	// Store mac address as a string without separator
    std::stringstream ss;
    for(int i=0; i<6; ++i)
    {
        ss << std::hex << (int)mac[i];
    }
    macStr = ss.str();

	return macStr;
}

#endif //linux

#endif //MAC_ADDR_H