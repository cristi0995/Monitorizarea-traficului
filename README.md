# A server for traffic monitoring (C++)
Features:
-can accept concurrent connections from multiple clients
-periodically receives the speed of the client and the street and informs the driver if he
passed the speed limit, it can also verify the speed limit on a specific street if a client
asks
-receives requests about the weather, gas prices and news and responds with the
information requested
-can receive reports about accidents from clients. If a client reports an accident, all
the other clients connected will be informed as well
I also have implemented the client part for this application. I used TCP/IP protocol for
communication.