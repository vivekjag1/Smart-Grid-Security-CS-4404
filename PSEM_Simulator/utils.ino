// -------------------------------------------------------- UTIL ONLY FUNCTIONS --------------------------------------------------------

//Helper function to send serial TX data, Serial.write() is a blocking function
int psem_tx(uint8_t *buf, uint16_t buf_size) {
    //IMPORTANT: use print_tx for when we want to print something to a debug !!!
    //console (COM9) when dealing with another serial interface for UART (Serial1)
    print_tx(buf, buf_size);
    Serial1.write(buf, buf_size);
    return 1;
}

// ------------------------------------------------------ END UTIL ONLY FUNCTIONS ------------------------------------------------------

// ---------------------------------------------------------- PRINT FUNCTIONS ----------------------------------------------------------

//Helper function to print serial data on the TX line (like a debug console, read important note in psem_tx)
void print_tx(uint8_t *buf, uint16_t buf_size) {
    //Write each byte of the buffer to serial output
    Serial.write("TX: ");
    for(int i = 0; i < buf_size; i++) { 
        if((i != 0) && (i % 16 == 0)) //add a newline every 16 bytes 
            Serial.write("\n    ");

        Serial.print(buf[i], HEX); //otherwise just print the information in integer array (packet)
        Serial.print(" ");
    }
    Serial.write("\n");
}

//Helper function to print serial data on the RX line. Prints data that is being recieved as HEX 
void print_rx(uint8_t *buf, uint16_t buf_size) {
    Serial.write("RX: ");

    for(int i = 0; i < buf_size; i++) {
        if((i != 0) && (i % 16 == 0))
            Serial.write("\n    ");

        Serial.print(buf[i], HEX);
        Serial.print(" ");
    }

    Serial.write("\n");
}

// -------------------------------------------------------- END PRINT FUNCTIONS --------------------------------------------------------

// -------------------------------------------------------- PSEM DATA FUNCTIONS --------------------------------------------------------

//Helper function to send a PSEM ACK
void send_psem_ack() {
    Serial1.write(PSEM_ACK);
}

//Helper function to send a PSEM NAK
void send_psem_nak() {
    Serial1.write(PSEM_NAK);
}

//Function to wrap contents in a PSEM packet and sends it out over TX
/*
workflow 
1. build the packet using the following butes 
  1. STP - start of packet (for parsing)
  2. IDENTITY - 
*/
int send_psem_pkt(uint8_t *buf, uint16_t buf_size) {
    //build the full psem_pkt
    uint16_t pkt_size = buf_size + 8; //add overhead of the packet size to the size of the buffer to get the packet size 
    uint8_t psem_pkt[pkt_size]; //declare the packet 
    psem_pkt[0] = PSEM_STP; //psem start of packet 
    psem_pkt[1] = PSEM_IDENTITY; //psem identity
    psem_pkt[2] = psem_ctrl_byte; //ctrl byte, we'll toggle the seq bit upon successfully receiving a response 
    psem_pkt[3] = 0x00; //seq-number
    psem_pkt[4] = buf_size >> 8; //length hi byte
    psem_pkt[5] = buf_size & 0x00FF; //length lo byte

    //copy the buffer data into the packet
    for(int i = 0; i < buf_size; i++)
        psem_pkt[i + 6] = buf[i];

    //calculate checksum and set
    uint16_t crc = calculate_psem_crc(psem_pkt, (pkt_size - 2));
    psem_pkt[pkt_size - 2] = crc >> 8; //crc hi byte
    psem_pkt[pkt_size - 1] = crc & 0x00FF; //crc lo byte
    //send it on the tx line 
    return psem_tx(psem_pkt, pkt_size);
}

//Helper function to verify a PSEM packet
int verify_psem_pkt() {
    //get given CRC from pkt
    uint16_t in_crc = ((uint16_t)recv_buf[recv_buf_sz - 2] << 8) | recv_buf[recv_buf_sz - 1];

    //calculate CRC ourselves
    uint16_t crc = calculate_psem_crc(recv_buf, recv_buf_sz - 2);

    //compare CRC
    if(in_crc != crc)
        return 0;
    
    //TODO: add support for sequence bit of ctrl byte (we cant really do this until we have both boards) !!!

    return 1;
}

//function to receive a psem packet and place it in recv_buf and set recv_buf_sz accordingly
int recv_psem_pkt() {
    uint8_t in_byte;
    uint8_t in_pkt_ctrl;
    uint8_t in_pkt_seq;
    uint16_t in_pkt_length;

    //While we have no bytes...
    recv_buf_sz = 0;
    while(recv_buf_sz < 1) {
        //If we get a byte...
        if(Serial1.available() > 0) {
            //read the byte
            in_byte = Serial1.read();

            //If we have PSEM_STP, put it into the buffer
            if(in_byte == PSEM_STP) {
                recv_buf[recv_buf_sz] = in_byte;
                recv_buf_sz++;
            }
        }
    }

    //Now that we are reading a PSEM packet, read the remaining 5 pkt header bytes
    //remaining pkt header bytes are identity, ctrl, seq-number, length hi byte, and length lo byte
    while(recv_buf_sz < 6) {
        //If we get a byte...
        if(Serial1.available() > 0) {
            //read the byte
            in_byte = Serial1.read();
            recv_buf[recv_buf_sz] = in_byte;
            recv_buf_sz++;
        }
    }

    //Take note of important header values
    in_pkt_ctrl = recv_buf[2];
    in_pkt_seq = recv_buf[3];
    in_pkt_length = ((uint16_t)recv_buf[4] << 8) | recv_buf[5]; //length hi byte | length lo byte

    //receive the packet data
    while(recv_buf_sz < (6 + in_pkt_length)) {
        //If we get a byte...
        if(Serial1.available() > 0) {
            //read the byte
            in_byte = Serial1.read();
            recv_buf[recv_buf_sz] = in_byte;
            recv_buf_sz++;
        }
    }

    //receive the packet crc
    while(recv_buf_sz < (6 + in_pkt_length + 2)) {
        //If we get a byte...
        if(Serial1.available() > 0) {
            //read the byte
            in_byte = Serial1.read();
            recv_buf[recv_buf_sz] = in_byte;
            recv_buf_sz++;
        }
    }

    //now that we've received the full packet, verify it
    if(verify_psem_pkt()) {
        // Once the packet has been verified, send acknowledgement to the sender
        send_psem_ack();
    } else {
        // If the packet fails to verify, notify the sender with a non-acknowledgement
        // This should be a more descriptive error message, but a NAK is fine for our purposes
        send_psem_nak();
        // The function should wait for more input if the packet couldn't be verified
        recv_psem_pkt();
    }

    //print the received packet
    print_rx(recv_buf, recv_buf_sz);

    return 0;
}

// ------------------------------------------------------ END PSEM DATA FUNCTIONS ------------------------------------------------------

// -------------------------------------------------------- MISC PSEM FUNCTIONS --------------------------------------------------------

//Helper function to calculate the CCITT CRC standard (taken from some datasheet online)
uint16_t calculate_psem_crc(uint8_t *buf, uint16_t buf_size) {
    int i, byte;
    uint16_t crc = 0;
    unsigned short C;
    
    for (byte = 1; byte <= buf_size; byte ++, buf ++) {
        C = ((crc >> 8) ^ *buf) << 8;
        for (i = 0; i < 8; i++) {
            if (C & 0x8000)
                C = (C << 1) ^ 0x1021;
            else
                C = C << 1;
        }
        crc = C ^ (crc << 8);
    }

    return crc;
}

// ------------------------------------------------------ END MISC PSEM FUNCTIONS ------------------------------------------------------