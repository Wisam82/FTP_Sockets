Class     :  CS-372 Introduction to Networking
Porject   :  Project#2
Data	  :  11-19-2015
Created by:  Wisam Thalij

File Included:
--------------
1- ftserver.c     [c program]
2- ftclient.py	  [python program]
3- Smallfile.txt
4- LargeFile.txt
5- README.txt

Compiling:
----------
1) I logged in to the flip1.engr.oregonstate.edu using the following command:

ssh flip1@engr.oregonstate.edu

Then created a file called "Server", in that file I added ftserver.c, SmallFile.txt and LargeFile.txt

compiled using the command [  gcc -o ftserver ftserver.c  ]

2) I logged in to the flip2.engr.oregonstate.edu in another window using the following command:

ssh flip2@engr.oregonstate.edu

Then created a file called "Client", in that file I added ftclient.py


Running the program:
--------------------
Note: I implemented an algorithem to wait for the meesages to be fully received on the client side, So executing the command takes about 5 seconds to see the results and display messages on the client side.

1) Server 

./ftserver <server Port>

2) Client

python ftclient.py <Hostname> <ServerPort> <-l> <DataPort>'
python fclient.py <Hostname> <ServerPort> <-g> <FileName> <DataPort>'


Extra Credit:
-------------
The server is a multi-thread server, it will accept up to 5 connection at the same time.

Reference Citation:
-------------------
I created a function to wait for the recieved messages on the client side, I found the concept in: Source: BinaryTides
URL   : http://www.binarytides.com/receive-full-data-with-the-recv-socket-function-in-python/