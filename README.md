# Project 4: Stop and Wait Protocol
### By Giorgi Alavidze, Liam Jeanette, and Evan Lang

In this project, we implemented a send and receive function for both users in a communication, which is then able to be used in order to call a stop and wait protocol for sending data over a series of messages. Our project was created using C++.

## Sending Data

Ourr function, in implementing a stop and wait system, must first create a method of being able to have a user send data to another user, while making sure that all sent data gets received.

#### Setting up Sockets

Firstly, our program will create 2 sockets, one for sending data to our other user, and the other for acting as a method for us to receive acknowledgements for our data. We can then start using our receiving socket as an input for various send functions, having the result we obtain from the socket being used in adddition to user inputs, to determine if we will need to send data a second time, our if there is no need to do so.

#### Initial Sequence Numbers

Our sender will then first sent a packet of data to our receiver, where the first 4 bytes of data is our sequence number. We will then send over a 0 byte and a byte of value 1, to act as a placeholder for an ack and a control variable respectively. Finally, we will send over the length of our message. Our sender will then wait to see if it receives an acknowledgement of this packet from ourr other user. If we receive the acknowledgement, we will receive the process of sending over data. If we do not get a response back after a predetermined time, our sender will repeat the process, sending our packet until we receive a response.

#### Sending Data and Receiving Acknowledgements

After our initial sequnce numbers are defined, our sender will begin to send data pacets to our receiving user. among the data for these packets, the sender will give the receiver a sequence number for our specific packet, of which our receiver will send an ack for if it received the data or not. If we do  not receive this ack in our alloted time, we will simply send the data again. If we do receive our ack, we will iterate our sequence number by 1, and send our next packet of data, iterating until we have sent over all of the data in our overall set. We will then save that we have received this ack.


## Receiving Data

#### Setting up Sockets

For our receiving function, we will set up the same sockets as before for our sending function, we will then have our receiver wait until data appears in our receiving socket, to obtain the data described in our setup message for initiating sequence numbers. With this data, our reciever will then send an acknowledgment of the first packet by sending the cooresponding sequence number. 

#### Receiving Dsta and Sending Acknowledgements

Once a sequence number has been decided, our reciever will then repeatedly wait for the receiving socket to obtain more data. Once this data is obtained, we will then update our total data with the newest packet, should the sequence number be new, or we will ignore the data if we have received the packet before. Regardless of if our sequence number is new or not, we will continue to send acks to oour sender to help them update their system. 

Once our data receives a packet designated for teardown, it will send an acknowledgement of the packet, which when given to the receiver, has it shut down. Once we send the ackknowledgement, we will then have the reciever shut down itself.
