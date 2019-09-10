@ Author: David Heck (dheck@udel.edu)

This program is a UDP file transfer socket programming example written in C.
Data transfer follows the Go-Back-N pipeline protocol to detect bit errors on the client side.  
All code has been tested in an Ubuntu Linux environment using the gcc compiler and libmhash library.

Files + Folders:                Desription:
    /README.txt                     The file you are reading.
    /Experiment.pdf                 A .pdf file of the figures and results for task 3.
    /Experiment_bonus.pdf           A .pdf file with results for the BONUS task. Includes timeout figures and additional tests.
    /client/bonus_client.c          Source C code for the client socket WITH bonus task implementation.
    /client/client.c                Source C code for the client socket WITHOUT bonus task implementation.
    /server/bonus_server.c          Source C code for the server socket WITH bonus task implementation.
    /server/server.c                Source C code for the server socket WITHOUT bonus task implementation.
    /server/long_test.txt           A long text file for transferring (on server)
    /server/med_test.txt            A medium text file for transferring (on server)
    /server/short_test.txt          A shorter text file for transferring (on server)
    /server/test.jpg                A .jpg file for transferring (on server)
    /server/test.pdf                A .pdf file for transferring (on server)
    /server/test.png                A .png file for transferring (on server)
    /server/testA.txt               A short text file for transferring (on server)
    /server/testB.txt               A short text file for transferring (on server)


Compile Instructions:
    gcc server.c -o server -lmhash
    gcc client.c -o client -lmhash

    OR

    gcc bonus_server.c -o server -lmhash
    gcc bonus_client.c -o client -lmhash


To Run:

    ./server [listening port number] [window size] [bit error probability]
    ./client [destination ip] [destination port number]

    OR

    ./server [listening port number] [window size] [bit error probability] [timeout interval]
    ./client [destination ip] [destination port number] [packet drop probability]

    Be sure to run the server file before using the client, as it will need to intercept UDP messages.

    Both programs can be run simply by "./exec" no arguments are necessary; however, arguments can be passed through.
    Both client and server are set to have default values (IP = 127.0.0.1) (port = 127.0.0.1) (window size = 1) (probability = 0).
    Bonus version default to the same but with (packet drop probability = 0) (timeout interval = 0.5)

    Note: Probability should be a decimal value between 0 and 1.

    Note: To run the code libmhash must be installed and set up on the machine. http://mhash.sourceforge.net/

    On an ubuntu machine mhash can be installed with:
        sudo apt-get update
        sudo apt-get install libmhash-dev


Program Flow:
    1. The server starts and waits for UDP requests from the client.
    2. The client sends a filename to the server.
    3. The server receives the filename.
    4. If the file is not present, the server informs the client by sending back an error message.
    5. If the file is present, the server continuously reads the file into a buffer and sends the buffer content to the client until file-end is reached. 
    6. The file is transfered using the Go-Back-N pipeline protocol: 
            - The server allows a window size of N max unacked packets.
            - When the server sends a packet, it appends an 8 digit CRC32 hex checksum resentation of the packet data (32 bit binary)
            - The client continuously receives packets only sending cumulative ACKs for the highest in order segment number.
            - The client computes an 8 digit CRC32 checksum hex representation of the packet. 
            - The client then compares this value to the 8 bit CRC32 payload on the packet. If values don't match a bit error is detected.
            - The client ignores segments with bit errors, and segments that are not the next in order segment number.
            - The server slides the Go-Back-N window to the highest received ACK (Only keeps up to N unacked packets in window)
    7. Once  the  transmission  finishes,  the  client  verifies  that  the  received  file  is  correct.
    8. Client can either terminate connection or receive another file.


BONUS Program Flow:
    1. The server starts and waits for UDP requests from the client.
    2. The client sends a filename to the server.
    3. The server receives the filename.
    4. If the file is not present, the server informs the client by sending back an error message.
    5. If the file is present, the server continuously reads the file into a buffer and sends the buffer content to the client until file-end is reached. 
    6. The file is transfered using the Go-Back-N pipeline protocol: 
            - The server allows a window size of N max unacked packets.
            - When the server sends a packet and starts a timeout timer for said segment,
              it appends an 8 digit CRC32 hex checksum resentation of the packet data (32 bit binary)
            - The client continuously receives packets only sending cumulative ACKs for the highest in order segment number.
            - The client may drop a received packet with input probability q.
            - If packet not dropped the client computes an 8 digit CRC32 checksum hex representation of the packet. 
            - The client then compares this value to the 8 bit CRC32 payload on the packet. If values don't match a bit error is detected.
            - The client ignores segments with bit errors, and segments that are not the next in order segment number.
            - If the server's packet timer exceeds the timeout period S, the server detects packet drop and resends appropriate packet.
            - The server slides the Go-Back-N window to the highest received ACK (Only keeps up to N unacked packets in window)
    7. Once  the  transmission  finishes,  the  client  verifies  that  the  received  file  is  correct.
    8. Client can either terminate connection or receive another file.


Sample Output Client:
    david@DESKTOP-K1GE480:/DavidHeck/client$ gcc client.c -o client -lmhash
    david@DESKTOP-K1GE480:/Programming 2/DavidHeck/client$ ./client 127.0.0.1 8080
    CLIENT: Socket creation successful.
    CLIENT: Enter the filename you wish to download, or type 'exit' to close connection: fakefile
    SERVER: File fakefile not found on server, please try another file.
    CLIENT: Enter the filename you wish to download, or type 'exit' to close connection: short_file.txt
    SERVER: File short_file.txt not found on server, please try another file.
    CLIENT: Enter the filename you wish to download, or type 'exit' to close connection: short_test.txt
    SERVER: OK
    CLIENT: Receiveing short_test.txt from Server and saving it.
    CLIENT: Receiveing segment 0
    CLIENT: Sending acknowledge 0
    CLIENT: Receiveing segment 1
    CLIENT: Sending acknowledge 1
    CLIENT: Receiveing segment 2
    CLIENT: Sending acknowledge 2
    CLIENT: Receiveing segment 3
    CLIENT: Sending acknowledge 3
    CLIENT: Receiveing segment 4
    CLIENT: Sending acknowledge 4
    CLIENT: Receiveing segment 5
    CLIENT: Bit error detected - Expected CRC32 Value of 68e99f94 but got 68e9a177 instead! Ignoring packets..
    CLIENT: Sending acknowledge 4
    CLIENT: Receiveing segment 6
    CLIENT: Sending acknowledge 4
    CLIENT: Receiveing segment 7
    CLIENT: Sending acknowledge 4
    CLIENT: Receiveing segment 5
    CLIENT: Sending acknowledge 5
    CLIENT: Receiveing segment 6
    CLIENT: Sending acknowledge 6
    CLIENT: Receiveing segment 7
    CLIENT: Bit error detected - Expected CRC32 Value of 934376e but got 934394d instead! Ignoring packets..
    CLIENT: Sending acknowledge 6
    CLIENT: Receiveing segment 8
    CLIENT: Sending acknowledge 6
    CLIENT: Receiveing segment 7
    CLIENT: Sending acknowledge 7
    CLIENT: Receiveing segment 8
    CLIENT: Sending acknowledge 8
    CLIENT: Receiveing segment 9
    CLIENT: Sending acknowledge 9
    CLIENT: Receiveing segment 10
    CLIENT: Sending acknowledge 10
    CLIENT: Receiveing segment 11
    CLIENT: Bit error detected - Expected CRC32 Value of cfd00ca4 but got cfd00e82 instead! Ignoring packets..
    CLIENT: Sending acknowledge 10
    CLIENT: Receiveing segment 12
    CLIENT: Sending acknowledge 10
    CLIENT: Receiveing segment 13
    CLIENT: Sending acknowledge 10
    CLIENT: Receiveing segment 14
    CLIENT: Sending acknowledge 10
    CLIENT: Receiveing segment 11
    CLIENT: Sending acknowledge 11
    CLIENT: Receiveing segment 12
    CLIENT: Sending acknowledge 12
    CLIENT: Receiveing segment 13
    CLIENT: Bit error detected - Expected CRC32 Value of e4127993 but got e4127b7b instead! Ignoring packets..
    CLIENT: Sending acknowledge 12
    CLIENT: Receiveing segment 14
    CLIENT: Sending acknowledge 12
    CLIENT: Receiveing segment 13
    CLIENT: Sending acknowledge 13
    CLIENT: Receiveing segment 14
    CLIENT: Sending acknowledge 14
    CLIENT: Receiveing segment 15
    CLIENT: Sending acknowledge 15
    CLIENT: Receiveing segment 16
    CLIENT: Sending acknowledge 16
    CLIENT: Receiveing segment 17
    CLIENT: Bit error detected - Expected CRC32 Value of ed49eaeb but got ed49eccb instead! Ignoring packets..
    CLIENT: Sending acknowledge 16
    CLIENT: Receiveing segment 18
    CLIENT: Sending acknowledge 16
    CLIENT: Receiveing segment 19
    CLIENT: Sending acknowledge 16
    CLIENT: Receiveing segment 20
    CLIENT: Sending acknowledge 16
    CLIENT: Receiveing segment 17
    CLIENT: Sending acknowledge 17
    CLIENT: Receiveing segment 18
    CLIENT: Sending acknowledge 18
    CLIENT: Receiveing segment 19
    CLIENT: Bit error detected - Expected CRC32 Value of 61429183 but got 6142935c instead! Ignoring packets..
    CLIENT: Sending acknowledge 18
    CLIENT: Receiveing segment 20
    CLIENT: Sending acknowledge 18
    CLIENT: Receiveing segment 19
    CLIENT: Sending acknowledge 19
    CLIENT: Receiveing segment 20
    CLIENT: Sending acknowledge 20
    CLIENT: Receiveing segment 21
    CLIENT: Bit error detected - Expected CRC32 Value of b4f87f1e but got b4f880f7 instead! Ignoring packets..
    CLIENT: Sending acknowledge 20
    CLIENT: Receiveing segment 22
    CLIENT: Sending acknowledge 20
    CLIENT: Receiveing segment 21
    CLIENT: Sending acknowledge 21
    CLIENT: Receiveing segment 22
    CLIENT: Sending acknowledge 22
    CLIENT: Receiveing segment 23
    CLIENT: Sending acknowledge 23
    CLIENT: Receiveing segment 24
    CLIENT: Bit error detected - Expected CRC32 Value of f979fd50 but got f979ff36 instead! Ignoring packets..
    CLIENT: Sending acknowledge 23
    CLIENT: Receiveing segment 24
    CLIENT: Sending acknowledge 24
    CLIENT: Receiveing segment 25
    CLIENT: Sending acknowledge 25
    CLIENT: Receiveing segment 26
    CLIENT: Sending acknowledge 26
    CLIENT: Receiveing segment 27
    CLIENT: Sending acknowledge 27
    CLIENT: Receiveing segment 28
    CLIENT: Bit error detected - Expected CRC32 Value of f5a70193 but got f5a7036b instead! Ignoring packets..
    CLIENT: Sending acknowledge 27
    CLIENT: Receiveing segment 29
    CLIENT: Sending acknowledge 27
    CLIENT: Receiveing segment 30
    CLIENT: Sending acknowledge 27
    CLIENT: Receiveing segment 31
    CLIENT: Sending acknowledge 27
    CLIENT: Receiveing segment 28
    CLIENT: Sending acknowledge 28
    CLIENT: Receiveing segment 29
    CLIENT: Sending acknowledge 29
    CLIENT: Receiveing segment 30
    CLIENT: Sending acknowledge 30
    CLIENT: Receiveing segment 31
    CLIENT: Sending acknowledge 31
    CLIENT: Receiveing segment 32
    CLIENT: Sending acknowledge 32
    CLIENT: Receiveing segment 33
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 34
    CLIENT: Bit error detected - Expected CRC32 Value of 54faa19a but got 54faa376 instead! Ignoring packets..
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 35
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 34
    CLIENT: Bit error detected - Expected CRC32 Value of 54faa199 but got 54faa376 instead! Ignoring packets..
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 35
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 36
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 37
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 34
    CLIENT: Bit error detected - Expected CRC32 Value of 54faa195 but got 54faa376 instead! Ignoring packets..
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 35
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 36
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 37
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 34
    CLIENT: Sending acknowledge 34
    CLIENT: Receiveing segment 35
    CLIENT: Sending acknowledge 35
    CLIENT: File received from server in 0.046875 seconds!
    CLIENT: Enter the filename you wish to download, or type 'exit' to close connection: exit
    SERVER: Closing connection.
    CLIENT: Exiting...


Sample Output Server:
    david@DESKTOP-K1GE480:/Programming 2/DavidHeck/server$ gcc server.c -o server -lmhash
    david@DESKTOP-K1GE480:/Programming 2/DavidHeck/server$ ./server 8080 4 .3
    SERVER: Socket creation successful.
    SERVER: Socket binding successful.
    CLIENT: Requesting fakefile
    SERVER: ERROR file fakefile not found on server.
    CLIENT: Requesting short_file.txt
    SERVER: ERROR file short_file.txt not found on server.
    CLIENT: Requesting short_test.txt
    SERVER: Sending file short_test.txt to Client...
    SERVER: Sending segment 0
    SERVER: Receiving acknowledge 0
    SERVER: Sending segment 1
    SERVER: Receiving acknowledge 1
    SERVER: Sending segment 2
    SERVER: Receiving acknowledge 2
    SERVER: Sending segment 3
    SERVER: Receiving acknowledge 3
    SERVER: Sending segment 4
    SERVER: Receiving acknowledge 4
    SERVER: Sending segment 5
    SERVER: Receiving acknowledge 4
    SERVER: Sending segment 6
    SERVER: Receiving acknowledge 4
    SERVER: Sending segment 7
    SERVER: Receiving acknowledge 4
    SERVER: Sending segment 5
    SERVER: Receiving acknowledge 5
    SERVER: Sending segment 6
    SERVER: Receiving acknowledge 6
    SERVER: Sending segment 7
    SERVER: Receiving acknowledge 6
    SERVER: Sending segment 8
    SERVER: Receiving acknowledge 6
    SERVER: Sending segment 7
    SERVER: Receiving acknowledge 7
    SERVER: Sending segment 8
    SERVER: Receiving acknowledge 8
    SERVER: Sending segment 9
    SERVER: Receiving acknowledge 9
    SERVER: Sending segment 10
    SERVER: Receiving acknowledge 10
    SERVER: Sending segment 11
    SERVER: Receiving acknowledge 10
    SERVER: Sending segment 12
    SERVER: Receiving acknowledge 10
    SERVER: Sending segment 13
    SERVER: Receiving acknowledge 10
    SERVER: Sending segment 14
    SERVER: Receiving acknowledge 10
    SERVER: Sending segment 11
    SERVER: Receiving acknowledge 11
    SERVER: Sending segment 12
    SERVER: Receiving acknowledge 12
    SERVER: Sending segment 13
    SERVER: Receiving acknowledge 12
    SERVER: Sending segment 14
    SERVER: Receiving acknowledge 12
    SERVER: Sending segment 13
    SERVER: Receiving acknowledge 13
    SERVER: Sending segment 14
    SERVER: Receiving acknowledge 14
    SERVER: Sending segment 15
    SERVER: Receiving acknowledge 15
    SERVER: Sending segment 16
    SERVER: Receiving acknowledge 16
    SERVER: Sending segment 17
    SERVER: Receiving acknowledge 16
    SERVER: Sending segment 18
    SERVER: Receiving acknowledge 16
    SERVER: Sending segment 19
    SERVER: Receiving acknowledge 16
    SERVER: Sending segment 20
    SERVER: Receiving acknowledge 16
    SERVER: Sending segment 17
    SERVER: Receiving acknowledge 17
    SERVER: Sending segment 18
    SERVER: Receiving acknowledge 18
    SERVER: Sending segment 19
    SERVER: Receiving acknowledge 18
    SERVER: Sending segment 20
    SERVER: Receiving acknowledge 18
    SERVER: Sending segment 19
    SERVER: Receiving acknowledge 19
    SERVER: Sending segment 20
    SERVER: Receiving acknowledge 20
    SERVER: Sending segment 21
    SERVER: Receiving acknowledge 20
    SERVER: Sending segment 22
    SERVER: Receiving acknowledge 20
    SERVER: Sending segment 21
    SERVER: Receiving acknowledge 21
    SERVER: Sending segment 22
    SERVER: Receiving acknowledge 22
    SERVER: Sending segment 23
    SERVER: Receiving acknowledge 23
    SERVER: Sending segment 24
    SERVER: Receiving acknowledge 23
    SERVER: Sending segment 24
    SERVER: Receiving acknowledge 24
    SERVER: Sending segment 25
    SERVER: Receiving acknowledge 25
    SERVER: Sending segment 26
    SERVER: Receiving acknowledge 26
    SERVER: Sending segment 27
    SERVER: Receiving acknowledge 27
    SERVER: Sending segment 28
    SERVER: Receiving acknowledge 27
    SERVER: Sending segment 29
    SERVER: Receiving acknowledge 27
    SERVER: Sending segment 30
    SERVER: Receiving acknowledge 27
    SERVER: Sending segment 31
    SERVER: Receiving acknowledge 27
    SERVER: Sending segment 28
    SERVER: Receiving acknowledge 28
    SERVER: Sending segment 29
    SERVER: Receiving acknowledge 29
    SERVER: Sending segment 30
    SERVER: Receiving acknowledge 30
    SERVER: Sending segment 31
    SERVER: Receiving acknowledge 31
    SERVER: Sending segment 32
    SERVER: Receiving acknowledge 32
    SERVER: Sending segment 33
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 34
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 35
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 34
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 35
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 36
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 37
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 34
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 35
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 36
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 37
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 34
    SERVER: Receiving acknowledge 34
    SERVER: Sending segment 35
    SERVER: Receiving acknowledge 35
    SERVER: File successfully sent to client in 0.046875 seconds!
    CLIENT: Requesting exit
    SERVER: Exiting...


Sample Output Bonus Client:
    david@DESKTOP-K1GE480:/DavidHeck/client$ gcc client_bonus.c -o client -lmhash
    david@DESKTOP-K1GE480:/DavidHeck/client$ ./client 127.0.0.1 8080 .50
    CLIENT: Socket creation successful.
    CLIENT: Enter the filename you wish to download, or type 'exit' to close connection: short_test.txt
    SERVER: OK
    CLIENT: Receiveing short_test.txt from Server and saving it.
    CLIENT: Receiveing segment 0
    CLIENT: Sending acknowledge 0
    CLIENT: Receiveing segment 1
    CLIENT: Sending acknowledge 1
    CLIENT: Receiveing segment 2
    CLIENT: Sending acknowledge 1
    CLIENT: Receiveing segment 2
    CLIENT: Sending acknowledge 2
    CLIENT: Receiveing segment 3
    CLIENT: Sending acknowledge 3
    CLIENT: Receiveing segment 4
    CLIENT: Sending acknowledge 4
    CLIENT: Receiveing segment 5
    CLIENT: Sending acknowledge 4
    CLIENT: Receiveing segment 5
    CLIENT: Sending acknowledge 5
    CLIENT: Receiveing segment 6
    CLIENT: Sending acknowledge 6
    CLIENT: Receiveing segment 7
    CLIENT: Sending acknowledge 7
    CLIENT: Receiveing segment 8
    CLIENT: Sending acknowledge 8
    CLIENT: Receiveing segment 9
    CLIENT: Sending acknowledge 9
    CLIENT: Receiveing segment 10
    CLIENT: Sending acknowledge 9
    CLIENT: Receiveing segment 10
    CLIENT: Sending acknowledge 9
    CLIENT: Receiveing segment 10
    CLIENT: Sending acknowledge 10
    CLIENT: Receiveing segment 11
    CLIENT: Sending acknowledge 11
    CLIENT: Receiveing segment 12
    CLIENT: Sending acknowledge 12
    CLIENT: Receiveing segment 13
    CLIENT: Sending acknowledge 12
    CLIENT: Receiveing segment 13
    CLIENT: Sending acknowledge 12
    CLIENT: Receiveing segment 13
    CLIENT: Sending acknowledge 13
    CLIENT: Receiveing segment 14
    CLIENT: Sending acknowledge 14
    CLIENT: Receiveing segment 15
    CLIENT: Sending acknowledge 14
    CLIENT: Receiveing segment 15
    CLIENT: Sending acknowledge 15
    CLIENT: Receiveing segment 16
    CLIENT: Sending acknowledge 16
    CLIENT: Receiveing segment 17
    CLIENT: Sending acknowledge 17
    CLIENT: Receiveing segment 18
    CLIENT: Sending acknowledge 18
    CLIENT: Receiveing segment 19
    CLIENT: Sending acknowledge 19
    CLIENT: Receiveing segment 20
    CLIENT: Sending acknowledge 19
    CLIENT: Receiveing segment 20
    CLIENT: Sending acknowledge 20
    CLIENT: Receiveing segment 21
    CLIENT: Sending acknowledge 20
    CLIENT: Receiveing segment 21
    CLIENT: Sending acknowledge 21
    CLIENT: Receiveing segment 22
    CLIENT: Sending acknowledge 21
    CLIENT: Receiveing segment 22
    CLIENT: Sending acknowledge 21
    CLIENT: Receiveing segment 22
    CLIENT: Sending acknowledge 21
    CLIENT: Receiveing segment 22
    CLIENT: Sending acknowledge 21
    CLIENT: Receiveing segment 22
    CLIENT: Sending acknowledge 21
    CLIENT: Receiveing segment 22
    CLIENT: Sending acknowledge 22
    CLIENT: Receiveing segment 23
    CLIENT: Sending acknowledge 22
    CLIENT: Receiveing segment 23
    CLIENT: Sending acknowledge 22
    CLIENT: Receiveing segment 23
    CLIENT: Sending acknowledge 22
    CLIENT: Receiveing segment 23
    CLIENT: Sending acknowledge 22
    CLIENT: Receiveing segment 23
    CLIENT: Sending acknowledge 23
    CLIENT: Receiveing segment 24
    CLIENT: Sending acknowledge 23
    CLIENT: Receiveing segment 24
    CLIENT: Sending acknowledge 23
    CLIENT: Receiveing segment 24
    CLIENT: Sending acknowledge 23
    CLIENT: Receiveing segment 24
    CLIENT: Sending acknowledge 23
    CLIENT: Receiveing segment 24
    CLIENT: Sending acknowledge 24
    CLIENT: Receiveing segment 25
    CLIENT: Sending acknowledge 25
    CLIENT: Receiveing segment 26
    CLIENT: Sending acknowledge 25
    CLIENT: Receiveing segment 26
    CLIENT: Sending acknowledge 26
    CLIENT: Receiveing segment 27
    CLIENT: Sending acknowledge 26
    CLIENT: Receiveing segment 27
    CLIENT: Sending acknowledge 26
    CLIENT: Receiveing segment 27
    CLIENT: Sending acknowledge 26
    CLIENT: Receiveing segment 27
    CLIENT: Sending acknowledge 26
    CLIENT: Receiveing segment 27
    CLIENT: Sending acknowledge 27
    CLIENT: Receiveing segment 28
    CLIENT: Sending acknowledge 27
    CLIENT: Receiveing segment 28
    CLIENT: Sending acknowledge 28
    CLIENT: Receiveing segment 29
    CLIENT: Sending acknowledge 29
    CLIENT: Receiveing segment 30
    CLIENT: Sending acknowledge 29
    CLIENT: Receiveing segment 30
    CLIENT: Sending acknowledge 29
    CLIENT: Receiveing segment 30
    CLIENT: Sending acknowledge 30
    CLIENT: Receiveing segment 31
    CLIENT: Sending acknowledge 30
    CLIENT: Receiveing segment 31
    CLIENT: Sending acknowledge 31
    CLIENT: Receiveing segment 32
    CLIENT: Sending acknowledge 31
    CLIENT: Receiveing segment 32
    CLIENT: Sending acknowledge 31
    CLIENT: Receiveing segment 32
    CLIENT: Sending acknowledge 32
    CLIENT: Receiveing segment 33
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 34
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 34
    CLIENT: Sending acknowledge 33
    CLIENT: Receiveing segment 34
    CLIENT: Sending acknowledge 34
    CLIENT: Receiveing segment 35
    CLIENT: Sending acknowledge 35
    CLIENT: File received from server in 0.031250 seconds!
    CLIENT: Enter the filename you wish to download, or type 'exit' to close connection: exit
    SERVER: Closing connection.
    CLIENT: Exiting...

Sample Output Bonus Server:
    david@DESKTOP-K1GE480:/DavidHeck/server$ gcc server_bonus.c -o server -lmhash
    david@DESKTOP-K1GE480:/DavidHeck/server$ ./server 8080 1 0 .2
    SERVER: Socket creation successful.
    SERVER: Socket binding successful.
    CLIENT: Requesting short_test.txt
    SERVER: Sending file short_test.txt to Client...
    SERVER: Sending segment 0
    SERVER: Receiving acknowledge 0
    SERVER: Sending segment 1
    SERVER: Receiving acknowledge 1
    SERVER: Sending segment 2
    SERVER: Receiving acknowledge 1
    SERVER: TIME-OUT. Packet drop detected! Resending segment 2
    SERVER: Sending segment 2
    SERVER: Receiving acknowledge 2
    SERVER: Sending segment 3
    SERVER: Receiving acknowledge 3
    SERVER: Sending segment 4
    SERVER: Receiving acknowledge 4
    SERVER: Sending segment 5
    SERVER: Receiving acknowledge 4
    SERVER: TIME-OUT. Packet drop detected! Resending segment 5
    SERVER: Sending segment 5
    SERVER: Receiving acknowledge 5
    SERVER: Sending segment 6
    SERVER: Receiving acknowledge 6
    SERVER: Sending segment 7
    SERVER: Receiving acknowledge 7
    SERVER: Sending segment 8
    SERVER: Receiving acknowledge 8
    SERVER: Sending segment 9
    SERVER: Receiving acknowledge 9
    SERVER: Sending segment 10
    SERVER: Receiving acknowledge 9
    SERVER: TIME-OUT. Packet drop detected! Resending segment 10
    SERVER: Sending segment 10
    SERVER: Receiving acknowledge 9
    SERVER: TIME-OUT. Packet drop detected! Resending segment 10
    SERVER: Sending segment 10
    SERVER: Receiving acknowledge 10
    SERVER: Sending segment 11
    SERVER: Receiving acknowledge 11
    SERVER: Sending segment 12
    SERVER: Receiving acknowledge 12
    SERVER: Sending segment 13
    SERVER: Receiving acknowledge 12
    SERVER: TIME-OUT. Packet drop detected! Resending segment 13
    SERVER: Sending segment 13
    SERVER: Receiving acknowledge 12
    SERVER: TIME-OUT. Packet drop detected! Resending segment 13
    SERVER: Sending segment 13
    SERVER: Receiving acknowledge 13
    SERVER: Sending segment 14
    SERVER: Receiving acknowledge 14
    SERVER: Sending segment 15
    SERVER: Receiving acknowledge 14
    SERVER: TIME-OUT. Packet drop detected! Resending segment 15
    SERVER: Sending segment 15
    SERVER: Receiving acknowledge 15
    SERVER: Sending segment 16
    SERVER: Receiving acknowledge 16
    SERVER: Sending segment 17
    SERVER: Receiving acknowledge 17
    SERVER: Sending segment 18
    SERVER: Receiving acknowledge 18
    SERVER: Sending segment 19
    SERVER: Receiving acknowledge 19
    SERVER: Sending segment 20
    SERVER: Receiving acknowledge 19
    SERVER: TIME-OUT. Packet drop detected! Resending segment 20
    SERVER: Sending segment 20
    SERVER: Receiving acknowledge 20
    SERVER: Sending segment 21
    SERVER: Receiving acknowledge 20
    SERVER: TIME-OUT. Packet drop detected! Resending segment 21
    SERVER: Sending segment 21
    SERVER: Receiving acknowledge 21
    SERVER: Sending segment 22
    SERVER: Receiving acknowledge 21
    SERVER: TIME-OUT. Packet drop detected! Resending segment 22
    SERVER: Sending segment 22
    SERVER: Receiving acknowledge 21
    SERVER: TIME-OUT. Packet drop detected! Resending segment 22
    SERVER: Sending segment 22
    SERVER: Receiving acknowledge 21
    SERVER: TIME-OUT. Packet drop detected! Resending segment 22
    SERVER: Sending segment 22
    SERVER: Receiving acknowledge 21
    SERVER: TIME-OUT. Packet drop detected! Resending segment 22
    SERVER: Sending segment 22
    SERVER: Receiving acknowledge 21
    SERVER: TIME-OUT. Packet drop detected! Resending segment 22
    SERVER: Sending segment 22
    SERVER: Receiving acknowledge 22
    SERVER: Sending segment 23
    SERVER: Receiving acknowledge 22
    SERVER: TIME-OUT. Packet drop detected! Resending segment 23
    SERVER: Sending segment 23
    SERVER: Receiving acknowledge 22
    SERVER: TIME-OUT. Packet drop detected! Resending segment 23
    SERVER: Sending segment 23
    SERVER: Receiving acknowledge 22
    SERVER: TIME-OUT. Packet drop detected! Resending segment 23
    SERVER: Sending segment 23
    SERVER: Receiving acknowledge 22
    SERVER: TIME-OUT. Packet drop detected! Resending segment 23
    SERVER: Sending segment 23
    SERVER: Receiving acknowledge 23
    SERVER: Sending segment 24
    SERVER: Receiving acknowledge 23
    SERVER: TIME-OUT. Packet drop detected! Resending segment 24
    SERVER: Sending segment 24
    SERVER: Receiving acknowledge 23
    SERVER: TIME-OUT. Packet drop detected! Resending segment 24
    SERVER: Sending segment 24
    SERVER: Receiving acknowledge 23
    SERVER: TIME-OUT. Packet drop detected! Resending segment 24
    SERVER: Sending segment 24
    SERVER: Receiving acknowledge 23
    SERVER: TIME-OUT. Packet drop detected! Resending segment 24
    SERVER: Sending segment 24
    SERVER: Receiving acknowledge 24
    SERVER: Sending segment 25
    SERVER: Receiving acknowledge 25
    SERVER: Sending segment 26
    SERVER: Receiving acknowledge 25
    SERVER: TIME-OUT. Packet drop detected! Resending segment 26
    SERVER: Sending segment 26
    SERVER: Receiving acknowledge 26
    SERVER: Sending segment 27
    SERVER: Receiving acknowledge 26
    SERVER: TIME-OUT. Packet drop detected! Resending segment 27
    SERVER: Sending segment 27
    SERVER: Receiving acknowledge 26
    SERVER: TIME-OUT. Packet drop detected! Resending segment 27
    SERVER: Sending segment 27
    SERVER: Receiving acknowledge 26
    SERVER: TIME-OUT. Packet drop detected! Resending segment 27
    SERVER: Sending segment 27
    SERVER: Receiving acknowledge 26
    SERVER: TIME-OUT. Packet drop detected! Resending segment 27
    SERVER: Sending segment 27
    SERVER: Receiving acknowledge 27
    SERVER: Sending segment 28
    SERVER: Receiving acknowledge 27
    SERVER: TIME-OUT. Packet drop detected! Resending segment 28
    SERVER: Sending segment 28
    SERVER: Receiving acknowledge 28
    SERVER: Sending segment 29
    SERVER: Receiving acknowledge 29
    SERVER: Sending segment 30
    SERVER: Receiving acknowledge 29
    SERVER: TIME-OUT. Packet drop detected! Resending segment 30
    SERVER: Sending segment 30
    SERVER: Receiving acknowledge 29
    SERVER: TIME-OUT. Packet drop detected! Resending segment 30
    SERVER: Sending segment 30
    SERVER: Receiving acknowledge 30
    SERVER: Sending segment 31
    SERVER: Receiving acknowledge 30
    SERVER: TIME-OUT. Packet drop detected! Resending segment 31
    SERVER: Sending segment 31
    SERVER: Receiving acknowledge 31
    SERVER: Sending segment 32
    SERVER: Receiving acknowledge 31
    SERVER: TIME-OUT. Packet drop detected! Resending segment 32
    SERVER: Sending segment 32
    SERVER: Receiving acknowledge 31
    SERVER: TIME-OUT. Packet drop detected! Resending segment 32
    SERVER: Sending segment 32
    SERVER: Receiving acknowledge 32
    SERVER: Sending segment 33
    SERVER: Receiving acknowledge 33
    SERVER: Sending segment 34
    SERVER: Receiving acknowledge 33
    SERVER: TIME-OUT. Packet drop detected! Resending segment 34
    SERVER: Sending segment 34
    SERVER: Receiving acknowledge 33
    SERVER: TIME-OUT. Packet drop detected! Resending segment 34
    SERVER: Sending segment 34
    SERVER: Receiving acknowledge 34
    SERVER: Sending segment 35
    SERVER: Receiving acknowledge 35
    SERVER: File successfully sent to client in 0.031250 seconds!
    CLIENT: Requesting exit
    SERVER: Exiting...

