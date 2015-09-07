#ifndef JOS_KERN_PCI_H
#define JOS_KERN_PCI_H

#include <inc/types.h>

// Constants for network cards
#define REG_TCTL 0x400
#define REG_TIPG 0x410
#define REG_TDBAL 0x3800
#define REG_TDBAH 0x3804
#define REG_TDLEN 0x3808
#define REG_TDH 0x3810
#define REG_TDT 0x3818
#define PACKET_BUFFER_SIZE 1518
#define MASK_TCTL_EN 0x2
#define MASK_TCTL_PSP 0x8
#define MASK_TDESC_CMD_RS 0x8
#define MASK_TDESC_CMD_END 0x1
#define MASK_TDESC_STATUS_DD 0x1
#define E_PACKET_TOO_LARGE 0x1
#define E_FULL_RETRY 0x2

// Transmit descriptor format
struct tx_desc {
    uint32_t addr;
    uint32_t padding;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
};

// PCI subsystem interface
enum { pci_res_bus, pci_res_mem, pci_res_io, pci_res_max };

struct pci_bus;

struct pci_func {
    struct pci_bus *bus;	// Primary bus for bridges

    uint32_t dev;
    uint32_t func;

    uint32_t dev_id;
    uint32_t dev_class;

    uint32_t reg_base[6];
    uint32_t reg_size[6];
    uint8_t irq_line;
};

struct pci_bus {
    struct pci_func *parent_bridge;
    uint32_t busno;
};

int  pci_init(void);
void pci_func_enable(struct pci_func *f);
int  pci_transmit_packet(void* packet, uint32_t size);

#endif
