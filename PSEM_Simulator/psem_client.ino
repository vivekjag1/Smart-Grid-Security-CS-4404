//higher level function to handle client-side psem read interaction (send and recv)
int client_psem_read() {
    int res = 0;

    //Firstly, send the PSEM read packet
    if(res = send_psem_read(SECURITY_TBL_ID, 0, 24)) {
        //Serial.write("Sent PSEM read successfully\n");
    } else {
        //Serial.write("couldn't send PSEM read, error %d\n", res);
        return 0;
    }
}

//question for later -- do we just want to use a single user for the purpose of our project, or do we want to support lots of users? 
//function that acts as an entry point for sending a logon request
void client_psem_logon(void) {
    int res = 0;
    //Firstly, send the PSEM read packet
    uint8_t userArray[13]; 
    for(int i = 0; i < 13; i++)
        userArray[i] = i; 

    if(res = send_psem_logon(LOGON_USERID, userArray, 13 )) {
        //Serial.write("Sent logon successfully\n");
    } else {
        //Serial.write("couldn't send logon, error %d\n", res);
        return 0;
    }
}

void client_psem_ident(void) {
    int res = 0;
    //Firstly, send the PSEM read packet
    if(res = send_psem_ident()) {
        //Serial.write("Sent logon successfully\n");
    } else {
        //Serial.write("couldn't send logon, error %d\n", res);
        return 0;
    }
}

int send_psem_ident(void){
    uint8_t ident_req[1]; //ident can just have 1 item if we use all the default parameters for version (which we are)
    ident_req[0] = IDENT; //set type to ident 

    return send_psem_pkt(ident_req, 1); 
}