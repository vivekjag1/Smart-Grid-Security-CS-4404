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
        run_client();
    }
}

void loop() {
    //put your main code here, to run repeatedly:
        
    // Only return on critical errors
    if(PSEM_MODE == SERVER) {
        if(run_server())
            return;
    }
}