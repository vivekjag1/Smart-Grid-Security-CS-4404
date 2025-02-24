# PSEM Simulation for Data Interception Attack

## Description

Smart grid devices, specifically electrical meters, adhere to the Protocol Specification for Electricity Meters (PSEM), which is defined by the ANSI C12.18 standard. C12.18 negotiates the transfer of data between a client electrical meter and a server, which allows for electrical companies to properly bill their customers. This standard transports unencrypted (plaintext) data, which is vulnerable to an interception attack. The ANSI C12.18 standard also relies on another standard for data storage in tables, ANSI C12.19. 

C12.19 defines many tables for the operation of a meter, some of which may be exploited, but one stands out in particular as it holds user information. The password table contains user ID numbers, access levels, and, most importantly, passwords. C12.19 provides an option for basic encryption of these passwords in later revisions, but earlier implementations store them as plaintext. Even with encryption, brute-force password cracking may prove useful.

With this, the vulnerability is revealed. Assuming proper access to hardware, a bad actor can wait for a PSEM read request to be sent for the vulnerable password table. The plaintext passwords associated with their user ID numbers can then be intercepted via a "man-in-the-middle" attack. Once the password is intercepted, the attacker can send write requests to the server, causing the customer to be unfairly billed. This project simulates this attack. 

## Required Hardware

- **2x** - Arduino Mega 2560 microcontroller boards
- **1x** - Logic Analyzer with support for reading UART lines
- **6x** - Dupont breadboard jumper wires (connections depend on logic analyzer inputs)
- **1x** - Computer to monitor serial data from Arduinos and output of logic analyzer

## Required Software

- Arduino IDE
- Logic Analyzer monitoring program (typically bundled with the logic analyzer)

## Hardware Setup
1. Plug both Arduino boards into a usb port for power and data transfer.
2. Plug the logic analyzer into a usb port for power and data transfer.
3. Connect the TX lines of one arduino to the RX line of the other arduino (repeat this step for both boards).
4. Connect an alligator clip to the bottom of the wire connected to the RX line of the server. 
5. Connect an alligator clip to the bottom of the wire connected to the RX line of the client. 
6. Connect a wire to each alligator clip on one end, and to a channel of the logic analyzer on the other end (repeat for both boards).


## Software Setup

1. Clone this repository into its own directory
2. Open two instances of the PSEM_Simulator Arduino sketch in two different Arduino IDE windows
3. Connect both Arduino boards to the host computer
4. Ensure both instances of the Arduino IDE have a different board (COM port) selected
5. Designate one board as the client, and the other as the server
6. In the Arduino IDE instance for the client board, ensure the PSEM_MODE macro in PSEM_Simulator.ino is set to CLIENT
7. Verify and upload the program to the client board
8. In the Arduino IDE instance for the server board, ensure the PSEM_MODE macro in PSEM_Simulator.ino is set to SERVER
9. Verify and upload the program to the server board
10. In both instances, start the Serial Monitor with the 9600 baud rate option
11. The client instance should write "Starting in Client Mode" to its own serial output
12. The server instance should write "Starting in Server Mode" to its own serial output
13. Both the client and server should be ready and running at this time
14. Connect the logic analyzer to the host computer
15. Open the logic analyzer software
    - Note: This is dependent on the logic analyzer chosen, and may vary between users
16. Confirm that the logic analyzer is properly connected between the two boards and intercepting signals
    - Note: This can be done by sending a PSEM ident request between the boards (see "Performing the Attack" for more details)
18. Setup is complete

## Performing the Attack

### Part 1: Ensuring Proper Setup

1. In the client instance serial monitor input, type an upper case "I" and press the "Enter" key
    - This command sends a PSEM ident request from the client to the server
2. Ensure proper transmission of the packet by observing both the client and server serial monitor outputs
    - The client should display a message stating, "Successfully sent PSEM ident request."
    - The client should additionally display the bytes of the raw, transmitted PSEM packet in hex, preceded by "TX:", and encased between dashed lines
    - An image of the client's perspective can be seen below:
    - *insert client ident request image here*
    - The server should display a message stating, "Successfully received PSEM ident request."
    - The server should additionally display the bytes of the raw, received PSEM packet in hex, preceded by "RX:", and encased between dashed lines
    - An image of the server's perspective can be seen below:
    - *insert server ident received image here*
3. Observe the logic analyzer output
    - The raw UART packet containing the ident request should be visible in the output of the logic analyzer software
    - Note One: The data in the RAW UART packet will be different than the raw PSEM packet as UART contains additional protocol overhead with the data
    - Note Two: The logic analyzer output will differ from user to user depending on which device is being used
    - An example image of the logic analyzer output can be seen below:
    - *insert example logic analyzer output for ident request here*
4. Assuming proper setup, the outputs of these three components should match, or closely resemble the described outputs and the attack may continue

### Part 2: Intercepting Password Table Data

1. In the client instance serial monitor input, type an upper case "O" and press the "Enter" key
    - Note One: "O" stands for "offset read", or a read request that asks for data from a specifc table
    - Note Two: The offset read request is hard-coded to return only the passwords table for the sake of this simulation
    - Note Three: This offset read request would actually be initiated by the electrical company, not the bad actor
        - The bad actor would have to collect all incoming and outgoing data over a long period of time to hope to catch this packet
        - The bad actor would only have access to a transmission source (wire, optical connection, wireless transmission, etc.)
        - For the sake of this demonstration, the offset read request is isolated and initiated by the user performing the simulation
    - Note Four: Confirmation of the packet's transmission should be oberserved in a similar manner to the PSEM ident request described in Part 1 of this section
2. *Insert note on logic analyzer output*
3. *Explain to whomever is running this project what they're seeing*

## Credits

[Vivek Jagadeesh](https://github.com/vivekjag1)

[Nick Golparvar](https://github.com/ngolp)

[Charlie Engler](https://github.com/charlieengler)

This project was developed as a capstone submission for CS 4404, Tools and Techniques in Computer Network Security, at Worcester Polytechnic Institute.
