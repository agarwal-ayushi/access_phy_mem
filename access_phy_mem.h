#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

/* Function to read 32-bit registers */

uint32_t access_phy_mem_read(uint64_t mem_address) {

	int mem_dev = open("/dev/mem", O_RDONLY | O_SYNC);
	
	if (mem_dev == -1 ){
		printf("Unable to access physical memory through /dev/mem \n"); 
	} 
	
	const uint64_t total_mem = 4; //4B block
	const uint64_t mem_size = 0x2000; //Size of the block of memory that you want to access
	
	uint64_t alloc_mem_size, page_mask, page_size, page_base, page_offset;
	unsigned char *mem_pointer;

	page_size = sysconf(_SC_PAGESIZE); //0x1000 Get the page size of your system
	alloc_mem_size = (((mem_size / page_size) + 1) * page_size); //align the memory pointer to page size
	page_mask = (page_size - 1); //0xFFF
	
	/* Align the physical address to page size 
	 */
	page_base = (mem_address / page_size) * page_size;
	page_offset = mem_address - page_base;

	//printf("Reading from addr %lX, page_offset %X\n", mem_address, page_offset);

	mem_pointer = (unsigned char*)mmap64(NULL,
                   alloc_mem_size,
                   PROT_READ,
                   MAP_SHARED,
                   mem_dev,
                   page_base 
                   );
	if(mem_pointer == MAP_FAILED)
	{  
      		printf("Failed to map memory \n");
		return -1;
	}
	unsigned char *virt_addr;
	virt_addr = mem_pointer + (mem_address & page_mask);

	uint32_t val = *((volatile uint32_t*)virt_addr);
	munmap(mem_pointer, alloc_mem_size);

	int close_file = close(mem_dev);                                                      
	return val;
}

void access_phy_mem_write(uint64_t mem_address, uint32_t val) {
	int mem_dev = open("/dev/mem", O_RDWR | O_SYNC);
	
	if (mem_dev == -1 ){
		printf("Unable to access physical memory through /dev/mem \n"); 
	} 
	
	const uint64_t total_mem = 4; //4B block
	const uint64_t mem_size = 0x2000; //Size of the block of memory that you want to access
	
	uint64_t alloc_mem_size, page_mask, page_size, page_base, page_offset;
	unsigned char *mem_pointer;

	page_size = sysconf(_SC_PAGESIZE); //0x1000 Get the page size of your system
	alloc_mem_size = (((mem_size / page_size) + 1) * page_size); //align the memory pointer to page size
	page_mask = (page_size - 1); //0xFFF
	
	/* Align the physical address to page size 
	 */
	page_base = (mem_address / page_size) * page_size;
	page_offset = mem_address - page_base;
	//printf("Writing to addr %lX, page_offset %X\n", mem_address, page_offset);

	mem_pointer = (unsigned char*)mmap64(NULL,
                   alloc_mem_size,
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED,
                   mem_dev,
                   page_base 
                   );
	if(mem_pointer == MAP_FAILED)
	{  
      		printf("Failed to map memory \n");
	}
	unsigned char *virt_addr;
	virt_addr = mem_pointer + (mem_address & page_mask);
	
	uint32_t value = *((volatile uint32_t*)virt_addr);
	//printf("The original value at the physical address %lX is %08x \n ", mem_address, value);

	*((volatile uint32_t*)(virt_addr)) = val;
	//printf("Writing to memory location done\n");	
	val = *((unsigned int*)(virt_addr+4));
	//printf("The new value at the physical address %lX is %08x \n ", mem_address+4, val);
	int close_file = close(mem_dev);                                                      
}
