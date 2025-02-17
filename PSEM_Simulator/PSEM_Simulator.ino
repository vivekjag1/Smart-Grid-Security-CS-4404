#include "psem_info.h"
#include "math.h"

uint8_t psem_ctrl_byte = 0x00; //ctrl_byte starts as zero, but can be toggled accordingly
uint8_t recv_buf[RECV_BUFSIZE]; //global buffer for receiving a PSEM packet
uint16_t recv_buf_sz; //global index for above recv_buf buffer

void setup() {
    //Configure serial drivers
    Serial.begin(PSEM_DEFAULT_BAUD);
    Serial1.begin(PSEM_DEFAULT_BAUD);
    Serial.write("\n\n\n");
    //Serial1.begin(psem_default_baud) save for when we hook up to listener

    //handler function for psem_read
    //IMPORTANT: Right now I'm just changing this based on whichever one I want to test in debug output (Serial calls that output to COM9)
    //client_psem_read();
    recv_psem_pkt();
    //server_psem_read();
}

void loop() {
    //put your main code here, to run repeatedly:
        
    //TODO: Make a state machine !!!
}