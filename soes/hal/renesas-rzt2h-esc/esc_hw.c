/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * ESC hardware layer functions for ESC RZT2H.
 *
 * Function to read and write commands to the ESC. Used to read/write ESC
 * registers and memory.
 */
#include "esc.h"
#include "esc_hw.h"
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

#define ESC_MMAP_SIZE	0x10000 // size of memory to allocate
#define MMAP_OFFSET	0

static int fd_esc = -1;     // this is a file descriptor that describes an open ESC UIO device
static char *esc_ptr; // this is the virtual address of the ESC UIO device registers

/** ESC read function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to read
 * @param[out]  buf         = pointer to buffer to read in
 * @param[in]   len         = number of bytes to read
 */
void ESC_read(uint16_t address, void *buf, uint16_t len)
{
	int i = 0;

	for (i = 0; i < len; i++)
		memcpy(buf + i, esc_ptr + address + i, 1);
}

/** ESC write function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to write
 * @param[out]  buf         = pointer to buffer to write from
 * @param[in]   len         = number of bytes to write
 */
void ESC_write(uint16_t address, void *buf, uint16_t len)
{
	int i;

	for (i = 0; i < len; i++)
		memcpy(esc_ptr + address + i, buf + i, 1);
}

void ESC_init(const esc_cfg_t * config)
{
	const char * user_arg = (char *)config->user_arg;

	// open the device
	fd_esc = open(user_arg, O_RDWR);
	if (fd_esc == -1) {
		printf("ESC init error -- did you forget to sudo?\n");
		return;
	}

	// memory map the physical address of the hardware into virtual address space
	esc_ptr = mmap(NULL, ESC_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_esc, MMAP_OFFSET);
	if (esc_ptr == MAP_FAILED) {
		printf("ESC mmap failed\n");
		return;
	}

	/* put hardware setup here */
	return;
}

void ESC_interrupt_enable(uint32_t mask)
{
	ESC_ALeventmaskwrite(ESC_ALeventmaskread() | mask);
}

void ESC_interrupt_disable(uint32_t mask)
{
	ESC_ALeventmaskwrite(ESC_ALeventmaskread() & (~mask));
}
