#!/usr/bin/python
from socket import *
import sys, getopt
import time

################################################################################################
#								recv_timeout()
# Description:
# This function will create a starting time for reading data from the dataSocket and waits till
# all the data received from the server and append it to the total data array.
#
# Input:
# a socket and a timeout = 2
# OutPut:
# All the received data
#
# Source: BinaryTides
# URL: http://www.binarytides.com/receive-full-data-with-the-recv-socket-function-in-python/
# 
################################################################################################

def recv_timeout(the_socket,timeout=2):
    #make socket non blocking
    the_socket.setblocking(0)
     
    #total data partwise in an array
    total_data=[];
    data='';
     
    #beginning time
    begin=time.time()
    while 1:
        #if you got some data, then break after timeout
        if total_data and time.time()-begin > timeout:
            break
         
        #if you got no data at all, wait a little longer, twice the timeout
        elif time.time()-begin > timeout*2:
            break
         
        #recv something
        try:
            data = the_socket.recv(8192)
            if data:
                total_data.append(data)
                #change the beginning time for measurement
                begin=time.time()
            else:
                #sleep for sometime to indicate a gap
                time.sleep(0.1)
        except:
            pass
    
    total_data.append('\n')
    #join all parts to make final string
    return ''.join(total_data)


# Initialize variables for the command line arguments
host = ''
command = ''
fileName = ''
test = sys.argv[3:]
CdPath = ''
dataPort = []

if len(sys.argv) < 5:
	print 'Usage: ftclient.py <Hostname> <ServerPort> <-l> <DataPort>'
	print 'OR Usage: ftclient.py <Hostname> <ServerPort> <-g> <FileName> <DataPort>'
	sys.exit(2)

# Get the host, contorl Port, Data port, command and the file name
if len(sys.argv) == 5:
	host = sys.argv[1]
	serverPort = int(sys.argv[2])
	command = sys.argv[3]
	dataPort = sys.argv[4]

if len(sys.argv) == 6:
	if sys.argv[3] == '-g':
		host = sys.argv[1]
		serverPort = int(sys.argv[2])
		command = sys.argv[3]
		fileName = sys.argv[4]
		dataPort = sys.argv[5]
	else:
		print 'Usage: ftclient.py <Hostname> <ServerPort> <-l> <DataPort>'
		print 'OR Usage: ftclient.py <Hostname> <ServerPort> <-g> <FileName> <DataPort>'
		sys.exit(2)

# Create a Contorl connection with the server
clientSocket = socket(AF_INET,SOCK_STREAM)
clientSocket.connect((host, serverPort))
TCPMessage = recv_timeout(clientSocket)
#print TCPMessage

# Send the Data port number, the command and the file name if exist
if command == '-l':
    clientSocket.send(dataPort)
    message = recv_timeout(clientSocket)
    clientSocket.send(host)
    message = recv_timeout(clientSocket)
    clientSocket.send(command)

elif command == '-g':
    clientSocket.send(dataPort)
    message = recv_timeout(clientSocket)
    clientSocket.send(host)
    message = recv_timeout(clientSocket)
    clientSocket.send(command)
    message = recv_timeout(clientSocket)
    clientSocket.send(str(len(fileName)))
    message = recv_timeout(clientSocket)
    clientSocket.send(fileName)

else:
    clientSocket.send(dataPort)
    message = recv_timeout(clientSocket)
    clientSocket.send(host)
    message = recv_timeout(clientSocket)
    clientSocket.send(command)


if command == '-l':
    print '\nReceiving directory structure from ', host ,':' , dataPort, '\n'
    # Create a DATA connection with the server
    try:
        DataSocket = socket(AF_INET, SOCK_STREAM)
    except socket.error:
        print "Error in socket creation"

    DataSocket.bind(('', int(dataPort)))
    DataSocket.listen(1)
    clientSocket.send("READY")
    while 1:
        conn, addr = DataSocket.accept()
        # Receive the directory list and display it
        TCPMessage = recv_timeout(conn)
        print TCPMessage
        # Close The Data Connection
        conn.close()
        DataSocket.close()
        break
    
elif command == '-g':
    # Create a DATA connection with the server
    try:
        DataSocket = socket(AF_INET, SOCK_STREAM)
    except socket.error:
        print "Error in socket creation"

    DataSocket.bind(('', int(dataPort)))
    DataSocket.listen(1)
    clientSocket.send("READY")
    while 1:
        conn, addr = DataSocket.accept()
        # receive an ok message
        text = ''
        text = conn.recv(5)
        if text == 'start':
            # receive the file size from server
            filesize = ''
            filesize = recv_timeout(conn)
            # Send and ok message
            conn.send("ok")
            print "Receiving \"" , fileName, "\" from :" , dataPort
            # Receive the file content from the server
            FileContent = ''
            FileContent = recv_timeout(conn)

            # Creating a file, write the content and close it
            NewFile = open(fileName, 'w')
            NewFile.write(FileContent)
            NewFile.close()

            print "File Transfer complete."
        else:
            ErrorMessage = ''
            ErrorMessage = recv_timeout(conn)
            print host, ":", dataPort, "says ", ErrorMessage
        DataSocket.close()
        conn.close()
        break

# Close both Data and Contorl sockets

clientSocket.close()







