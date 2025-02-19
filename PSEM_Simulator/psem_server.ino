//TODO: Change the Serial calls here to Serial1 once we can interface w/ the second board !!!

// Can be a double byte array because the number of total tables remains below 255
// TODO: Free me at the end
uint8_t **server_tables = (uint8_t**)malloc(sizeof(uint8_t*) * TOTAL_TABLES);

int intialize_server() {
    Serial.println("Initializing PSEM Server...");
    
    initialize_tables(server_tables);

    Serial.println("Done Initializing PSEM Server.");
    Serial.println("PSEM Server is Running:\n");
}

int run_server() {
    // Receive an incoming packet for interpretation
    recv_psem_pkt();

    // Sixth byte of the received buffer should be the type of request
    uint8_t request_type = recv_buf[6];
    switch(request_type) {
        case IDENT:
            return server_psem_ident();

        case LOGON:
            return server_psem_logon();

        case READ_FULL:
            return server_psem_full_read();

        case READ_OFFSET_REQ:
            return server_psem_offset_read();

        default:
            Serial.println("\nUnknown PSEM request received.\n");
            break;
    }

    return 0;
}

void cleanup_server() {
    for(int i = 0; i < TOTAL_TABLES; i++) {
        free(server_tables[i]);
    }

    free(server_tables);
}

int server_psem_ident(void) {
    // No need to do anything else, just receive ident
    Serial.println("\nSuccessfully received PSEM ident request.\n");

    return 0;
}

//server function to recieve the logon request (does nothing else currently)
int server_psem_logon() {
    //recieve the packet (this also verifies the checksum)
    //if the packet recieve method is fine then do nothing and just print that the logon was successful 
    //if not do nothing and print that it didn't work 
    Serial.println("\nSuccessfully received PSEM logon request.\n");

    return 0;
}

//higher level function to handle server-side psem full read interaction
int server_psem_full_read() {
    Serial.println("\nSuccessfully received PSEM full read request.\n");

    return 0;
}

//higher level function to handle server-side psem full read interaction
int server_psem_offset_read() {
    Serial.println("\nSuccessfully received PSEM offset read request.\n");

    uint16_t table_id = ((uint16_t)recv_buf[7] << 8) | recv_buf[8];
    uint32_t offset = ((uint32_t)recv_buf[9] << 16) | ((uint16_t)((uint16_t)recv_buf[10] & 0x00FF00) << 8) | recv_buf[11];
    uint16_t octet_count = ((uint16_t)recv_buf[12] << 8) | recv_buf[13];

    uint8_t buffer[(uint8_t)octet_count] = {0};

    read_table_entry(server_tables, table_id, offset, octet_count, buffer);

    int8_t cksum = read_data_cksum(octet_count, buffer);

    server_send_read_response(octet_count, buffer, cksum);

    return 0;
}

int server_send_read_response(uint16_t count, uint8_t *data, int8_t cksum) {
    // +3 for the size of the count (2 bytes) plus the size of the checksum (1 byte)
    uint8_t send_buf[(uint8_t)count + 3] = {0};
    
    send_buf[0] = (uint8_t)(count >> 8);
    send_buf[1] = (uint8_t)(count & 0x00FF);

    int i;
    for(i = 0; i < (uint8_t)count; i++)
        send_buf[i+2] = data[i];

    send_buf[i+2] = cksum;

    send_psem_pkt(send_buf, count + 3);

    free(send_buf);
    
    return 0;
}