#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
using namespace std;

int main(int argc, char *argv[]) {
	if (argc < 4) {
		printf("Usage: %s <phy_addr> <RD/WR> <#4B blocks>/<Write value> \n", argv[0]);
		return 0;	
	} else if ((strcmp(argv[2],"RD")==0) && (argc < 4)) {
		printf("RD Usage: %s <phy_addr> <RD/WR> <#4B Blocks> \n", argv[0]);
		return 0;
	} else if ((strcmp(argv[2],"WR")==0) && (argc < 4)) {
		printf("WR Usage: %s <phy_addr> <RD/WR> <write value> \n", argv[0]);
		return 0;
	}

	int mem_dev = open("/dev/mem", O_RDWR | O_SYNC);
	
	if (mem_dev == -1 ){
		cout << "Unable to access physical memory through /dev/mem \n"; 
	} else {
		cout <<"Able to access memory in read-only mode; YAY!! \n\n\n";
	}
	
	/* Starting address to memory block you want to access - THIS IS PHYSICAL ADDRESS
 	 * BE CAREFUL WHEN YOU ACCESS ANY PHYSICAL MEMORY
 	 * IF YOU MODIFY A CRITICAL MEMORY REGION THEN YOU CAN BREAK THE BOARD
 	 * */
	
	uint64_t total_blocks = 0;
	const uint64_t mem_address = strtoul(argv[1], NULL, 0); 
	if (strcmp(argv[2],"RD")==0) total_blocks = strtoul(argv[3], NULL, 0);
	const uint64_t total_mem = 4*total_blocks; //4B blocks
	
	const uint64_t mem_size = 0x8000; //Size of the block of memory that you want to access
	
	uint64_t alloc_mem_size, page_mask, page_size, page_base, page_offset;
	unsigned char *mem_pointer;

	page_size = sysconf(_SC_PAGESIZE); //0x1000 Get the page size of your system
	alloc_mem_size = (((mem_size / page_size) + 1) * page_size); //align the memory pointer to page size
	page_mask = (page_size - 1); //0xFFF
	
	/* Align the physical address to page size 
	 */
	page_base = (mem_address / page_size) * page_size;
	page_offset = mem_address - page_base;

	printf("addr %lX, page_offset %X\n", mem_address, page_offset);

	mem_pointer = (unsigned char*)mmap64(NULL,
                   alloc_mem_size,
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED,
                   mem_dev,
                   page_base 
                   );
	if(mem_pointer == MAP_FAILED)
	{  
      		cout << "Failed to map memory \n";
		return -1;
	}
	if (strcmp(argv[2],"RD")==0) {	
		for (int i=0; i < total_mem; i++) {
			printf("%02x ", (int)mem_pointer[page_offset + i]);
		}
		cout << "\n";
	}

	if (strcmp(argv[2], "WR")==0) {	
		unsigned char *virt_addr;
		virt_addr = mem_pointer + (mem_address & page_mask); // Maps the region to the virtual address space inorder to write to the registers. 
	
		int val = *((unsigned int*)virt_addr);
		printf("The original value at the physical address %lX is %08x \n ", mem_address, val);
	
		*((volatile unsigned long *)(virt_addr)) = strtoul(argv[3], NULL, 0);
		printf("Writing to memory location done\n");	
		val = *((unsigned int*)(virt_addr+4));
		printf("The new value at the physical address %lX is %08x \n ", mem_address+4, val);
	}
	munmap(mem_pointer, alloc_mem_size);

	int close_file = close(mem_dev);                                                      
        if (close_file == 0){                                                     
                cout<< "Closing the file to access physical memory successful \n";                          
        }

	return 0;
}
