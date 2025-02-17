#include "math.h"

#include "psem_info.h"

#define PSEM_MODE SERVER

uint8_t psem_ctrl_byte = 0x00; //ctrl_byte starts as zero, but can be toggled accordingly
uint8_t recv_buf[RECV_BUFSIZE]; //global buffer for receiving a PSEM packet
uint16_t recv_buf_sz; //global index for above recv_buf buffer

void setup() {
    //Configure serial drivers
    Serial.begin(PSEM_DEFAULT_BAUD);
    Serial1.begin(PSEM_DEFAULT_BAUD);

    Serial.print("\n\n\n");

    if(PSEM_MODE == CLIENT) {
        Serial.println("Running in Client Mode.");
        run_client();
    }

    if(PSEM_MODE == SERVER) {
        Serial.println("Running in Server Mode.");

        intialize_server();

        while(1) {
            if(run_server())
                break;
        }

        cleanup_server();
    }
}

void loop() {}