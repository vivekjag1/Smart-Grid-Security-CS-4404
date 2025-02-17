//TODO: Change the Serial calls here to Serial1 once we can interface w/ the second board !!!

int run_server() {
    recv_psem_pkt();

    uint8_t request_type = recv_buf[6];
    switch(request_type) {
        case IDENT:
            Serial.println("Ident received");
            break;

        case LOGON:
            Serial.println("Logon received");
            break;

        case READ_FULL:
            Serial.println("Full read received");
            break;

        case READ_OFFSET_REQ:
            Serial.println("Offset read received");
            break;

        default:
            Serial.println("Unknown request received");
            break;
    }

    return 0;
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
        send_psem_ack();
    } else {
        send_psem_nak();
        recv_psem_pkt();
    }

    //print the received packet
    print_rx(recv_buf, recv_buf_sz);

    return 1;
}

//higher level function to handle server-side psem read interaction (recv  send)
int server_psem_read() {
    int res = 0;
    //Firstly, receive a packet
    if(res = recv_psem_pkt()) {
        //Serial.write("Received PSEM packet successfully\n");
    } else {
        //Serial.write("couldn't read PSEM packet, error %d\n", res);
        return 0;
    }
}

//server function to recieve the logon request (does nothing else currently)
void server_psem_logon(uint8_t userID, uint8_t user) {
    //recieve the packet (this also verifys the checksum)
    //if the packet recieve method is fine then do nothing and just print that the logon was successful 
    //if not do nothing and print tht it didn't work 
    int res = 0;
    if(res = recv_psem_pkt()) {
        //for testing: 
        //Serial.println("Recieved logon packet"); 
        return 1; 
    } else { 
        //for testing: 
        //Serial.println("Logon failed"); 
        return 0; 
    }
}

void server_psem_ident(void) {
    int res = 0; 
    if((res = recv_psem_pkt())) {
        //for testing
        //Serial.println("IDENT recieved"); 
        return 1; 
    }
    return 0; 
}