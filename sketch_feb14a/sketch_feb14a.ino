#include "psem_info.h"
#include "math.h"

uint8_t psem_ctrl_byte = 0x00; //ctrl_byte starts as zero, but can be toggled accordingly
uint8_t recv_buf[RECV_BUFSIZE]; //global buffer for receiving a PSEM packet
uint16_t recv_buf_sz; //global index for above recv_buf buffer

void setup() {
  //Configure serial drivers
  Serial.begin(PSEM_DEFAULT_BAUD);
  //Serial1.begin(PSEM_DEFAULT_BAUD);
  Serial.write("\n\n\n");
  //Serial1.begin(psem_default_baud) save for when we hook up to listener

  //handler function for psem_read
  //IMPORTANT: Right now I'm just changing this based on whichever one I want to test in debug output (Serial calls that output to COM9)
  client_psem_read();
  //server_psem_read();
}

void loop() {
  //put your main code here, to run repeatedly:
    
  //TODO: Make a state machine !!!
}

//-------------------------HELPER FUNCTIONS-------------------------//

//Helper function to print serial data on the TX line (like a debug console, read important note in psem_tx)
void print_tx(uint8_t* buf, uint16_t buf_size) {
  
  //Write each byte of the buffer to serial output
  Serial.write("TX: ");
  for(int i = 0; i < buf_size; i++) {
    if((i != 0) && (i % 16 == 0)) {
      Serial.write("\n    ");
    }
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }
  Serial.write("\n");
}

//Helper function to send serial TX data, Serial.write() is a blocking function
int psem_tx(uint8_t* buf, uint16_t buf_size) {

  //IMPORTANT: use print_tx for when we want to print something to a debug !!!
  //console (COM9) when dealing with another serial interface for UART (Serial1)
  print_tx(buf, buf_size);
  //Serial1.write(buf, buf_size);
  return 1;
}

//Helper function to print serial data on the RX line
void print_rx(uint8_t* buf, uint16_t buf_size) {
  Serial.write("RX: ");
  for(int i = 0; i < buf_size; i++) {
    if((i != 0) && (i % 16 == 0)) {
      Serial.write("\n    ");
    }
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }
  Serial.write("\n");
}

//Helper function to calculate the CCITT CRC standard (taken from some datasheet online)
uint16_t calculate_psem_crc(uint8_t* buf, uint16_t buf_size) {
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

//Helper function to send a PSEM ACK
void send_psem_ack() {
  Serial.write(PSEM_ACK);
}

//Helper function to send a PSEM NAK
void send_psem_nak() {
  Serial.write(PSEM_NAK);
}

//-------------------------CLIENT SIDE-------------------------//

//Function to wrap contents in a PSEM packet and sends it out over TX
int send_psem_pkt(uint8_t* buf, uint16_t buf_size) {

  //build the full psem_pkt
  uint16_t pkt_size = buf_size + 8;
  uint8_t psem_pkt[pkt_size];
  psem_pkt[0] = PSEM_STP; //psem start of packet
  psem_pkt[1] = PSEM_IDENTITY; //psem identity
  psem_pkt[2] = psem_ctrl_byte; //ctrl byte, we'll toggle the seq bit upon successfully receiving a response 
  psem_pkt[3] = 0x00; //seq-number
  psem_pkt[4] = buf_size >> 8; //length hi byte
  psem_pkt[5] = buf_size & 0x00FF; //length lo byte

  //fill psem_pkt data
  for(int i = 0; i < buf_size; i++) {
    psem_pkt[i + 6] = buf[i];
  }

  uint16_t crc = calculate_psem_crc(psem_pkt, (pkt_size - 2));
  psem_pkt[pkt_size - 2] = crc >> 8; //crc hi byte
  psem_pkt[pkt_size - 1] = crc && 0x00FF; //crc lo byte

  return psem_tx(psem_pkt, pkt_size);
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

//higher level function to handle client-side psem read interaction (send and recv)
int client_psem_read() {
  int res = 0;

  //Firstly, send the PSEM read packet
  if(res = send_psem_read(SECURITY_TBL_ID, 0, 24)) {
    //Serial.write("Sent PSEM read successfully\n");
  }
  else {
    //Serial.write("couldn't send PSEM read, error %d\n", res);
    return 0;
  }
}

//-------------------------SERVER SIDE-------------------------//

//TODO: Change the Serial calls here to Serial1 once we can interface w/ the second board !!!

//Helper function to verify a PSEM packet
int verify_psem_pkt() {
  
  //get given CRC from pkt
  uint16_t in_crc = ((uint16_t)recv_buf[recv_buf_sz - 2] << 8) | recv_buf[recv_buf_sz - 1];

  //calculate CRC ourselves
  uint16_t crc = calculate_psem_crc(recv_buf, recv_buf_sz - 2);

  //compare CRC
  if(in_crc != crc) {
    return 0;
  }
  
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
    if(Serial.available() > 0) {

      //read the byte
      in_byte = Serial.read();

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
    if(Serial.available() > 0) {

      //read the byte
      in_byte = Serial.read();
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
    if(Serial.available() > 0) {

      //read the byte
      in_byte = Serial.read();
      recv_buf[recv_buf_sz] = in_byte;
      recv_buf_sz++;
    }
  }

  //receive the packet crc
  while(recv_buf_sz < (6 + in_pkt_length + 2)) {
    //If we get a byte...
    if(Serial.available() > 0) {

      //read the byte
      in_byte = Serial.read();
      recv_buf[recv_buf_sz] = in_byte;
      recv_buf_sz++;
    }
  }

  //now that we've received the full packet, verify it
  if(verify_psem_pkt()) {
    send_psem_ack();
  }
  else {
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
  }
  else {
    //Serial.write("couldn't read PSEM packet, error %d\n", res);
    return 0;
  }
}