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

int8_t read_data_cksum(uint16_t count, uint8_t *data) {
    int8_t cksum = 0;
    for(int i = 0; i < (uint8_t)count; i++, data++)
        cksum += *data;

    return -cksum;
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

// ------------------------------------------------------- ANSI C12.19 FUNCTIONS -------------------------------------------------------

// Initialize C12.19 tables for use in this exploit
int initialize_tables(uint8_t **tables) {
    Serial.println("Initializing PSEM Tables...");

    tables[PASSWORD_TBL_ID] = (uint8_t*)malloc(sizeof(uint8_t) * ENTRY_SIZE * NUM_USERS);

    uint8_t *password_table = (uint8_t*)tables[PASSWORD_TBL_ID];

    // Create entries for users
    for(int i = 0; i < NUM_USERS; i++) {
        struct password_entry *pwd_entry = (struct password_entry*)malloc(sizeof(password_entry));
        pwd_entry->user_id = (uint8_t)(i + 1);
        // Function to convert a constant char array to the uint8_t array used in the structure
        create_password((uint8_t*)&(pwd_entry->password), "twentyonecharacterpwd");
        // 1 = Readonly, 2 = Read/Write, 3 = Admin
        pwd_entry->access_level = (uint8_t)(i + 1);
        // Max number of attempts before account is locked
        pwd_entry->max_attempts = MAX_LOGIN_ATTEMPTS;
        // If the account has been locked or not
        pwd_entry->lock_status = 0;

        // C 12.19 compact datetime storage format
        struct datetime dt;
        dt.year = 0x07E9; // 2025
        dt.month = 0x02;  // February
        dt.day = 0x14;    // 20th
        dt.hour = 0x05;   // 5AM
        dt.minute = 0x1E; // 30 minutes
        dt.second = 0x00; // 0 seconds

        pwd_entry->dt = dt;

        create_table_entry(&(password_table[i * ENTRY_SIZE]), (uint8_t*)pwd_entry);

        Serial.println("Created Password Table Entry.");
    }

    Serial.println("Done Initializing PSEM Tables.");

    return 0;
}

// Create a C12.19 table entry (table and entry should both be 32 bytes in length)
int create_table_entry(uint8_t *table, uint8_t *entry) {
    for(int i = 0; i < ENTRY_SIZE; i++)
        table[i] = (uint8_t)entry[i];

    free(entry);

    return 0;
}

// Assemble a C12.19 password (21 bytes of data)
int create_password(uint8_t *buf, const char password[21]) {
    for(int i = 0; i < PASSWORD_LEN; i++, buf++)
        *buf = (uint8_t)password[i];

    return 0;
}

// Buffer should be the same size as octet_count
int read_table_entry(uint8_t **tables, uint16_t table_id, uint32_t offset, uint16_t octet_count, uint8_t *buffer) {
    // We only need a byte for the table id, offset, and octet_count in this implementation, not multiple
    uint8_t s_table_id = (uint8_t)table_id;
    uint8_t s_offset = (uint8_t)offset;
    uint8_t s_octet_count = (uint8_t)octet_count;

    uint8_t *table = tables[s_table_id + s_offset];
    for(int i = 0; i < s_octet_count; i++, table++) {
        buffer[i] = *table;
    }

    return 0;
}

// ----------------------------------------------------- END ANSI C12.19 FUNCTIONS -----------------------------------------------------