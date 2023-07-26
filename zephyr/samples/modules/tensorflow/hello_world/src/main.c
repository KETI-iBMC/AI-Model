/*
 * Copyright 2020 The TensorFlow Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "main_functions.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>


#define SHM_SIZE 1024

#define NUM_LOOPS 50

/* This is the default main used on systems that have the standard C entry
 * point. Other devices (for example FreeRTOS or ESP32) that have different
 * requirements for entry code (like an app_main function) should specialize
 * this main.cc file in a target-specific subfolder.
 */
int main(int argc, char *argv[])
{

	
// 	printf("\n");
// 	printf(".. zephyr-app-commands::\n");
// 	printf("   :zephyr-app: home/keti/BMC_SDK/source/AST2600_BMC/zephyr\n");
// 	printf("   :host-os: unix\n");
// 	printf("   :board: ast2600A3\n");
// 	printf("   :goals: run\n");
// 	printf("   :compact:\n");
// 	printf("\n");
// 	printf("\nInit Secondary Service Processor... \n\n");
// /**
//  * @brief hangyeol
//  * 
//  */
// 	uintptr_t physicalAddress = 0x01000000;

  
    



    /**
     * @brief han gyeol
     * 
     */
    // setup();

	/* Note: Modified from original while(true) to accommodate CI */
	// hello world example//
	// for (int i = 0; i < NUM_LOOPS; i++) {
	// 	loop();
		
	// }
	
    printf("--------------------------------- \n");
   


    volatile int* address = (int*)0x70000000; // Create a pointer to the memory address

    *address = 14;  // Write the value 1 to the memory address
    // *address2 = 20;

    printf("!Value at memory address 0x01000000: %x\n", &address);
    printf("Value at memory address 0x01000000: %x\n", *address);
    printf("Value at memory address 0x01000000: %x\n", address);

    // Define the address where you want to save the file
    uint32_t* file_address = (uint32_t*)0x70000000;

    // The text to be saved in the file
    const char* text = "Cortec M3 Sending Sample FIle\n";

    // Open the file using direct memory access
    FILE* file = fmemopen(file_address, strlen(text), "wb");
    if (file == NULL) {
        perror("Error opening the file");
        return 1;
    }

    // Write the text to the file using fwrite
    if (fwrite(text, sizeof(char), strlen(text), file) != strlen(text)) {
        perror("Error writing to the file");
        fclose(file);
        return 1;
    }

    // Close the file
    fclose(file);

    printf("Text written successfully at address 0x70000000.\n");


    return 0;

}
