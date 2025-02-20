// Directs control to the relevant psem helper function depending on serial input
int run_client(uint8_t cmd) {
    switch(cmd) {
        // Serial character I sends a PSEM ident request
        case 'I':
            return client_psem_ident();

        // Serial character L sends a PSEM logon request
        case 'L':
            return client_psem_logon();

        // Serial character F sends a PSEM full read request (does not currently work)
        case 'F':
            return client_psem_full_read();

        // Serial character O sends a PSEM offset read request with constant parameters in the function
        // TODO: Accept user input for the table ID, offset, and octet count
        case 'O':
            return client_psem_offset_read();

        // Serial character T just breaks the loop, but should send a PSEM terminate request
        // TODO: Implement termination for a graceful shutdown on both ends
        case 'T':
            return 1;

        // Any other command is considered unknown
        default:
            Serial.println("\nUnknown command received.\n");
            return 0;
    }

    // Run client returns zero on success and one on failure
    return 1;
}

// TODO: Implement me
void cleanup_client() {
    // If anything needs to be free, this is where it should be done
}

// Helper function for awaiting a PSEM ident request and printing a serial status update
int client_psem_ident(void) {
    // Send the PSEM ident packet
    if(send_psem_ident()) {
        // send_psem_ident returns 1 on failure, so an error is printed
        Serial.println("\nUnable to send PSEM ident request.\n");
        // client_psem_ident returns 1 on failure to indicate that the client should stop running
        return 1;
    }

    // Otherwise, a success message is printed to serial and zero is returned to continue execution
    Serial.println("\nSuccessfully sent PSEM ident request.\n");

    return 0;
}

// Helper function for assembling the ident packet and handing it to the general send function
int send_psem_ident(void) {
    uint8_t ident_req[1]; // Ident can just have 1 item if we use all the default parameters for version (which we are)
    ident_req[0] = IDENT; // Set type to ident 

    // The result of sending the packet is returned
    return send_psem_pkt(ident_req, 1); 
}

// Helper function for generating a user array and awaiting a PSEM logon request before printing a
// serial status update
int client_psem_logon(void) {
    // TODO: user_array is arbitrary and probably needs to change
    uint8_t user_array[13]; 
    for(int i = 0; i < 13; i++)
        user_array[i] = i; 

    // TODO: This logon function is also arbitrary and should probably prompt the user for some sort of input regarding who is logging on
    if(send_psem_logon(LOGON_USERID, user_array, 13)) {
        // send_psem_logon returns 1 on failure, so an error is printed
        Serial.println("\nUnable to send PSEM logon request.\n");
        // client_psem_logon returns 1 on failure to indicate that the client should stop running
        return 1;
    }

    // Otherwise, a success message is printed to serial and zero is returned to continue execution
    Serial.println("\nSuccessfully sent PSEM logon request.\n");

    return 0;
}

// Helper function for assembling the logon packet and handing it to the general send function
int send_psem_logon(uint16_t user_id, uint8_t *user, int user_arr_len) {
    // Create a new request
    uint8_t logon_req[13]; 
    // Split the user id into halves because it is a 16 bit word  
    // Hi bit for user id 
    logon_req[0] = LOGON; // Set type

    // The user ID can be as large as 10 bytes (80 bits)
    // This is the equivalant of 10 8 bit integers, so we will represent as such
    // Each int is a hex value
    for(int i = 1; i < user_arr_len; i++) {
        // If i >= size, fill with zeros 
        if(i>= user_arr_len)
            logon_req[i] = 0; 

        logon_req[i] = user[i]; // Otherwise, just set to whatever was passed in as the user identification value 
    }

    // The result of sending the packet is returned
    return send_psem_pkt(logon_req, 13); // Size 13 because: 1 byte for types, 2 bytes for user id, 10 bytes for user id = 1+2+10 = 13
}

// TODO: Implement me
int client_psem_full_read() {
    return 0;
}

// TODO: Request table ID, offset, and octet count as user input
// Helper function for generating awaiting a PSEM logon request before printing a serial status update
int client_psem_offset_read() {
    // Firstly, send the PSEM read packet
    // The table ID, offset, and octet count are all constant, but these should be requested from the user
    if(send_psem_read(PASSWORD_TBL_ID, 0, ENTRY_SIZE * NUM_USERS)) {
        // send_psem_read returns 1 on failure, so an error is printed
        Serial.println("\nUnable to send PSEM read request.\n");
        // client_psem_offset_read returns 1 on failure to indicate that the client should stop running
        return 1;
    }
    
    // Otherwise, a success message is printed to serial and zero is returned to continue execution
    Serial.println("\nSuccessfully sent PSEM offset read request.\n");

    // Since read is requesting data, we need to receive a packet afterwards
    // TODO: Verify response to make sure we get the data we wanted
    // TODO: Part of the verification process should include confirming the checksum
    recv_psem_pkt();

    return 0;
}

// Helper function for assembling the read packet and handing it to the general send function
int send_psem_read(uint16_t table_id, uint32_t offset, uint16_t octet_count) {
    // Multi-partial request aka "<pread-offset>"
    uint8_t read_req[8];
    read_req[0] = READ_OFFSET_REQ; // Req header
    read_req[1] = table_id >> 8; // Hi byte
    read_req[2] = table_id & 0x00FF; // Lo byte
    read_req[3] = offset >> 16; // Hi byte
    read_req[4] = (offset & 0x00FF00) >> 8; // Mid byte
    read_req[5] = offset & 0x0000FF; /// Lo byte
    read_req[6] = octet_count >> 8; // Hi byte
    read_req[7] = octet_count & 0x00FF; // Lo byte

    // The result of sending the packet is returned
    return send_psem_pkt(read_req, 8);
}