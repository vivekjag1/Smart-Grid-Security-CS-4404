// Byte array containing the tables in the database (only ever reaches 82 tables, so a one byte pointer is plenty)
uint8_t **server_tables = (uint8_t**)malloc(sizeof(uint8_t*) * TOTAL_TABLES);

int intialize_server() {
    Serial.println("Initializing PSEM Server...");
    
    // Only setup required is to initialize the table(s) associated with the server
    initialize_tables(server_tables);

    Serial.println("Done Initializing PSEM Server.");
    Serial.println("PSEM Server is Running:\n");
}

// run_server is called in a loop to properly handle incoming packets for processing and response
int run_server() {
    // Receive an incoming packet for interpretation
    recv_psem_pkt();

    // Sixth byte of the received buffer should be the type of request
    uint8_t request_type = recv_buf[6];

    // We only need to support 4 types of calls (full read isn't even really supported)
    // TODO: Add a 5th terminate call to gracefully shut down the server
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

// Not much processing occurs with ident because it isn't important for our implementation
// TODO: Proper ident handling
int server_psem_ident(void) {
    // No need to do anything else, just receive ident
    Serial.println("\nSuccessfully received PSEM ident request.\n");

    // Zero is returned to continue the run_server loop
    return 0;
}

// Server function to recieve the logon request (does nothing else currently)
// TODO: Proper logon handling
int server_psem_logon() {
    // If the packet recieve method is fine then do nothing and just print that the logon was successful 
    Serial.println("\nSuccessfully received PSEM logon request.\n");

    // Zero is returned to continue the run_server loop
    return 0;
}

// Higher level function to handle server-side psem full read interaction
// TODO: Proper full read handling
int server_psem_full_read() {
    // We only receive and acknowledge the request, nothing else is done in our implementation
    Serial.println("\nSuccessfully received PSEM full read request.\n");
    
    // Zero is returned to continue the run_server loop
    return 0;
}

// Higher level function to handle server-side psem full read interaction
// TODO: We should probably check for errors in the helper functions
int server_psem_offset_read() {
    // We first acknowledge the successful reception of the offset read request
    Serial.println("\nSuccessfully received PSEM offset read request.\n");

    // The table ID is the first argument passed by the incoming request. Index 6 is the type of request, then arguments follow,
    // hence index 7. The table ID is a two-byte integer, so we take the first two bytes of the request and combine them into the
    // hi and lo bytes of the table ID.
    // Table ID = the index of the table to be read from
    uint16_t table_id = ((uint16_t)recv_buf[7] << 8) | recv_buf[8];
    // The offset is the second argument and consists of a 24-bit integer, so we represent it as a four-byte variable. This is the
    // next three bytes in the incoming buffer.
    // Offset = the amount of data to pass over in the table before reading entries
    uint32_t offset = ((uint32_t)recv_buf[9] << 16) | ((uint16_t)((uint16_t)recv_buf[10] & 0x00FF00) << 8) | recv_buf[11];
    // Finally, the octet count is the second argument in the request with a two-byte value, so its hi and lo bytes are also extracted
    // and combined.
    // Octet Count = the amount of data (in bytes) to read from the table
    uint16_t octet_count = ((uint16_t)recv_buf[12] << 8) | recv_buf[13];

    // We initialize a buffer of zeros to hold the data we wish to read
    uint8_t buffer[(uint8_t)octet_count] = {0};

    // Call a helper function to read from the table by ID, offset, and octet count passing in a buffer
    read_table_entry(server_tables, table_id, offset, octet_count, buffer);

    // Call a helper to calculate a simple checksum for the data being read from the table. This is different from the CRC
    int8_t cksum = read_data_cksum(octet_count, buffer);

    // Finally, call a helper to compile the data into a PSEM packet
    server_send_read_response(octet_count, buffer, cksum);

    // Zero is returned to continue the run_server loop
    return 0;
}

// Helper function that compiles a PSEM read response packet and sends the data back to the client
int server_send_read_response(uint16_t count, uint8_t *data, int8_t cksum) {
    // Buffer to hold the read data, +3 for the size of the count (2 bytes) plus the size of the checksum (1 byte)
    uint8_t send_buf[(uint8_t)count + 3] = {0};
    
    // The count (amount of data to send) is divided across two bytes in the buffer as it's a two-byte value
    send_buf[0] = (uint8_t)(count >> 8); // Count hi byte
    send_buf[1] = (uint8_t)(count & 0x00FF); // Count lo byte

    // The rest of the return data is read into the buffer
    int i;
    for(i = 0; i < (uint8_t)count; i++)
        send_buf[i+2] = data[i];

    // The final byte of the read packet is the checksum
    send_buf[i+2] = cksum;

    // Call a helper to formally compile and send the PSEM packet
    send_psem_pkt(send_buf, count + 3);

    // Zero is returned to continue the run_server loop
    return 0;
}