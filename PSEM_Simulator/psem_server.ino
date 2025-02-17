//TODO: Change the Serial calls here to Serial1 once we can interface w/ the second board !!!

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

recv_pkt:
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
        goto recv_pkt;
    }

    //print the received packet
    print_rx(recv_buf, (recv_buf_sz - 1));

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

//function that actaully creates and sends the logon request 
int send_psem_logon(uint16_t userID, uint8_t *user, int userArrSize) {
    /*
    psuedo code 

    declare a request as a unit8_t 
    set the header to type LOGON (1)
    split the MSB and LSB of the user id (2) 
    split the user idenetification into 10 8 bit integers (10)
    send the packet of size 13   
    */

    //create a new request
    uint8_t logon_req[13]; 
    //split the user id into halves because it is a 16 bit word  
    //hi bit for user id 
    logon_req[0] = LOGON; //set type 

    //the user ID can be as large as 10 bytes (80 bits). this is the equivalant of 10 8 bit integers, so we will represent as such. each int is a hex value   
    for(int i = 0; i < userArrSize; i++) {
        //if i >= size, fill with zeros 
        if(i>= userArrSize) {
            logon_req[i] = 0; 
        }

        logon_req[i] = user[i]; //otherwise, just set to whatever was passed in as the user identification value 
    }
    //send the packet 
    return send_psem_pkt(logon_req, 13); //size 13 because: 1 byte for types, 2 bytes for user id, 10 bytes for user id = 1+2+10 = 13
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

//function to send a psem read request
int send_psem_read(uint16_t table_id, uint32_t offset, uint16_t octet_count) {
    //multi-partial request aka "<pread-offset>"
    uint8_t read_req[8];
    read_req[0] = READ_OFFSET_REQ; //req header
    read_req[1] = table_id >> 8; //hi byte
    read_req[2] = table_id & 0x00FF; //lo byte
    read_req[3] = offset >> 16; //hi byte
    read_req[4] = (offset & 0x00FF00) >> 8; //mid byte
    read_req[5] = offset & 0x0000FF; ///lo byte
    read_req[6] = octet_count >> 8; //hi byte
    read_req[7] = octet_count & 0x00FF; //lo byte

    return send_psem_pkt(read_req, 8);
}