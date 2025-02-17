//TODO: Change the Serial calls here to Serial1 once we can interface w/ the second board !!!

int run_server() {
    // Receive an incoming packet for interpretation
    recv_psem_pkt();

    // Sixth byte of the received buffer should be the type of request
    uint8_t request_type = recv_buf[6];
    switch(request_type) {
        case IDENT:
            return server_psem_ident();
            break;

        case LOGON:
            return server_psem_logon();
            break;

        case READ_FULL:
            return server_psem_full_read();

        case READ_OFFSET_REQ:
            return server_psem_offset_read();

        default:
            Serial.println("Unknown request received");
            break;
    }

    return 0;
}

int server_psem_ident(void) {
    // No need to do anything else, just receive ident
    Serial.println("Ident received");

    return 0;
}

//server function to recieve the logon request (does nothing else currently)
int server_psem_logon() {
    //recieve the packet (this also verifies the checksum)
    //if the packet recieve method is fine then do nothing and just print that the logon was successful 
    //if not do nothing and print that it didn't work 
    Serial.println("Logon received");

    return 0;
}

//higher level function to handle server-side psem full read interaction
int server_psem_full_read() {
    Serial.println("Full read received");
    return 0;
}

//higher level function to handle server-side psem full read interaction
int server_psem_offset_read() {
    Serial.println("Offset read received");
    return 0;
}