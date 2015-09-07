#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
    binaryname = "ns_output";

    // LAB 6: Your code here:
    // 	- read a packet from the network server
    //	- send the packet to the device driver
    int r = 0;
    while (r >= 0 && ipc_recv(NULL, &nsipcbuf, NULL) >= 0) {
	// cprintf("the address is %x, size is %d\n", 
	//	nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len);
	r = sys_transmit_packet(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len);
	// missing definitions are in inc/syscall.c
    }
}
