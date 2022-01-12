# AsyncComTCP
Async receive and send data between COM port and TCP port

Send data from COM port to TCP port.

Receive data from TCP port and send to COM port.

All receive process are asynchronous.

Receive and send realized on threads.

You can test with com0com project. http://com0com.sourceforge.net/

Create two Virtual Port Pair(check usePorts Class for all ports)

Example:

First pair: COM1 <--> COM2

Second pair: COM3 <--> COM4

In AsyncComTCP Client use COM1. 
Set IP: 127.0.0.1 
TCPport: 8035

In AsyncComTCP Server use COM3. 
Set IP: 127.0.0.1 
TCPport: 8035

Now COM2 and COM4 connected to each other via TCP
