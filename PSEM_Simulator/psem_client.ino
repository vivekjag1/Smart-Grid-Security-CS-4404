// TODO: Change this out of its test config
int run_client(uint8_t cmd) {
    switch(cmd) {
        case 'I':
            return client_psem_ident();

        case 'L':
            return client_psem_logon();

        case 'F':
            return client_psem_full_read();

        case 'O':
            return client_psem_offset_read();

        case 'T':
            // TODO: Terminate should shut both off
            return 1;

        default:
            Serial.println("Unknown command received.");
            return 0;
    }

    return 1;
}

// TODO: Implement me
void cleanup_client() {
    // If anything needs to be free, this is where it should be done
}

int client_psem_ident(void) {
    //Firstly, send the PSEM read packet
    if(send_psem_ident()) {
        Serial.println("Unable to send PSEM ident request.");
        return 1;
    }

    Serial.println("Successfully sent PSEM ident request.");

    return 0;
}

int send_psem_ident(void){
    uint8_t ident_req[1]; //ident can just have 1 item if we use all the default parameters for version (which we are)
    ident_req[0] = IDENT; //set type to ident 

    return send_psem_pkt(ident_req, 1); 
}

//question for later -- do we just want to use a single user for the purpose of our project, or do we want to support lots of users? 
//function that acts as an entry point for sending a logon request
int client_psem_logon(void) {
    // TODO: userArray is arbitrary and probably needs to change
    //Firstly, send the PSEM read packet
    uint8_t userArray[13]; 
    for(int i = 0; i < 13; i++)
        userArray[i] = i; 

    // TODO: This logon function is also arbitrary and should probably prompt the user for some sort of input regarding who is logging on
    if(send_psem_logon(LOGON_USERID, userArray, 13)) {
        Serial.println("Unable to send PSEM logon request.");
        return 1;
    }

    Serial.println("Successfully sent PSEM logon request.");

    return 0;
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
    for(int i = 1; i < userArrSize; i++) {
        //if i >= size, fill with zeros 
        if(i>= userArrSize) {
            logon_req[i] = 0; 
        }

        logon_req[i] = user[i]; //otherwise, just set to whatever was passed in as the user identification value 
    }

    //send the packet 
    return send_psem_pkt(logon_req, 13); //size 13 because: 1 byte for types, 2 bytes for user id, 10 bytes for user id = 1+2+10 = 13
}

// TODO: Implement me
int client_psem_full_read() {
    return 0;
}

// TODO: Request table ID, offset, and octet count
//higher level function to handle client-side psem read interaction (send and recv)
int client_psem_offset_read() {
    //Firstly, send the PSEM read packet
    if(send_psem_read(PASSWORD_TBL_ID, 0, ENTRY_SIZE * NUM_USERS)) {
        Serial.println("Unable to send PSEM read request.");
        return 1;
    }
    
    Serial.println("Successfully sent PSEM offset read request.");

    // TODO: Need to make sure this is the appropriate response to the read request
    recv_psem_pkt();

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