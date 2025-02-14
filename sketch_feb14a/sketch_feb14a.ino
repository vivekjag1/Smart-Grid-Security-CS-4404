#include "psem_info.h"
#include "math.h"

uint8_t psem_ctrl_byte = 0x00; //ctrl_byte starts as zero, but can be toggled accordingly

void setup() {
  //Configure serial drivers
  Serial.begin(PSEM_DEFAULT_BAUD);
  Serial1.begin(PSEM_DEFAULT_BAUD);
  Serial.write("\n\n\n");
  //Serial1.begin(psem_default_baud) save for when we hook up to listener

  //handler function for psem_read
  psem_read();
}

void loop() {
  // put your main code here, to run repeatedly:
    
}

//Helper function to print serial data on the TX line
void print_tx(uint8_t* buf, uint16_t buf_size) {
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

//Function to send serial TX data, Serial.write() is a blocking function
int psem_tx(uint8_t* buf, uint16_t buf_size) {
  print_tx(buf, buf_size); //IMPORTANT !!! use print_tx for when we want to print something to a debug console (COM9) when dealing with another serial interface for UART (Serial1)
  //Serial1.write(buf, buf_size);
  return 1;
}

//function to calculate the CCITT CRC standard (taken from a datasheet online)
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

  //calculate CRC (TODO !!!)
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
int psem_read() {
  int res = 0;

  //Firstly, send the PSEM read packet
  if(res = send_psem_read(SECURITY_TBL_ID, 0, 24)) {
    //Serial.write("Sent PSEM read successfully\n");
    return 1;
  }
  else {
    //Serial.write("couldn't send PSEM read, error %d\n", res);
    return 0;
  }
}