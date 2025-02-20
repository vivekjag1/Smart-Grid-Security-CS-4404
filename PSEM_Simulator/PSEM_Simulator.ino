#include "math.h"

#include "psem_info.h"

// Switch to easily move between server and client mode before uploading code to arduino
#define PSEM_MODE SERVER

uint8_t psem_ctrl_byte = 0x00; // ctrl_byte starts as zero, but can be toggled accordingly
uint8_t recv_buf[RECV_BUFSIZE]; // Global buffer for receiving a PSEM packet
uint16_t recv_buf_sz; // Global index for above recv_buf buffer

void setup() {
    // Configure serial drivers
    Serial.begin(PSEM_DEFAULT_BAUD);
    Serial1.begin(PSEM_DEFAULT_BAUD);

    // Print newlines to clear the output
    Serial.print("\n\n\n");

    // Client mode loop
    if(PSEM_MODE == CLIENT) {
        Serial.println("-------------------- Starting in Client Mode --------------------");
        Serial.println("PSEM Client is Running:\n");
        while(1) {
            // Server loop awaits serial input for a command
            if(Serial.available() == 0)
                continue;

            // Command is simply the first byte read in from serial
            uint8_t cmd = Serial.read();

            // Any successful function within run_client will return 0
            // Critical fails should return 1 and break this loop
            if(run_client(cmd))
                break;
            
            // Clear out the serial buffer before continuing the loop
            while(Serial.available() > 0)
                Serial.read();
        }

        // Cleanup function to prevent memory leaks and free memory
        cleanup_client();
    }

    // Server mode loop
    if(PSEM_MODE == SERVER) {
        Serial.println("-------------------- Starting in Server Mode --------------------");

        // Before beginning the loop, the server needs to be initialized
        intialize_server();

        while(1) {
            // Any successful function within run_server will return 0
            // Critical fails should return 1 and break this loop
            if(run_server())
                break;
        }

        // Cleanup function to prevent memory leaks and free memory
        cleanup_server();
    }
}

// Loop function is required on the arduino, but this should never be reached unless the device is terminated
// (in which case it should probably just turn off)
void loop() {}