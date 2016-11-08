#include "networking.h"
#include "Meeting.h"

#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <sstream>
using namespace std;

void printErrorMsg(string msg) {
  cout << "Error: " << msg << endl;
  exit(1);
}

/* Function to initialize my server attributes */
void createServerAddrStruct(struct sockaddr_in *address, int port) {
  bzero(address, sizeof(*address));
  address->sin_port = htons(port); // Store bytes in network byte order
  address->sin_family = AF_INET;
  address->sin_addr.s_addr = INADDR_ANY;
}

/* Function to initialize my client attributes */
void createClientAddrStruct(struct sockaddr_in *address, char *ip_address, int port) {
  bzero(address, sizeof(*address));
  address->sin_port = htons(port);
  address->sin_family = AF_INET;
  if (inet_pton(address->sin_family, ip_address, &(address->sin_addr.s_addr)) != 1) {
    printErrorMsg("Can't parse IP address or system error occurred\n");
  }
}

/* Create my scoket and check if it's successful or not */
void createSocket(int *descriptor) {
  *descriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (*descriptor < 0) {
    printErrorMsg("Can't create socket\n");
  }
}

/* Bind the socket if creating a socket was successful */
void bindSocket(struct sockaddr_in *address, int *descriptor) {
  socklen_t address_len = (socklen_t) sizeof(*address);
  if (::bind(*descriptor, (struct sockaddr *)address, address_len) < 0) {
    printErrorMsg("Can't bind socket\n");
  }
}

/* Listen for incomoing connection from the client accpets
 * only a maximum of 10 connections */
void setupListenSocket(int port, int *listen_socket) {
  struct sockaddr_in address;
  createServerAddrStruct(&address, port);
  createSocket(listen_socket);
  bindSocket(&address, listen_socket);
  listen(*listen_socket, MAX_PENDING_CONNECTIONS);
}

/* if a theres an incoming connection accept the incoming connection */
void acceptIncomingConnection(int *listen_socket, int *accept_socket) {
  struct sockaddr address;
  socklen_t address_len = (socklen_t) sizeof(address);
  if ((*accept_socket = accept(*listen_socket, &address, &address_len)) < 0) {
    printErrorMsg("Error accepting connection\n");
  }
}

/* After accepting incoming connection connect the client to the server */
void connectToServer(char *ip_address, int port, int *descriptor) {
  struct sockaddr_in address;
  createClientAddrStruct(&address, ip_address, port);
  createSocket(descriptor);
  if (connect(*descriptor, (struct sockaddr *) &address, sizeof(address)) < 0) {
    printErrorMsg("Can't initiate connection on socket\n");
  }
}

int sendMessage(string& buff, int descriptor) {
  const char *c_buff = buff.c_str();
  uint32_t stringSize = strlen(c_buff);

  NETWORKING_LOG("Sending string size");
  int numBytesSent = 0, totalNumBytes = sizeof(stringSize);
  while (numBytesSent < totalNumBytes) {
    int result = send(descriptor, &stringSize, totalNumBytes - numBytesSent, 0);
    if (result < 0) {
      cout << "Failed to transmit string size to socket " << descriptor << ". "
           << "(" << strerror(errno) << ")." << endl;
      return -1;
    }
    numBytesSent += result;
    NETWORKING_LOG(numBytesSent << " out of " << totalNumBytes << " sent.");
  }

  // Send message
  totalNumBytes = stringSize;
  numBytesSent = 0;

  NETWORKING_LOG("Sending message of size " << stringSize << " bytes.");
  int result = 0;
  while (numBytesSent < totalNumBytes) {
    result = send(descriptor, c_buff + result, totalNumBytes - numBytesSent, 0);
    if (result < 0) {
      cout << "Failed to send message from socket " << descriptor << ". "
           << "(" << strerror(errno) << ")." << endl;
      return -1;
    }
    numBytesSent += result;
    NETWORKING_LOG(numBytesSent << " out of " << totalNumBytes << " sent.");
  }

  return numBytesSent + sizeof(stringSize);
}

int receiveMessage(string& buff, int descriptor) {
  uint32_t stringSize;
  int totalNumBytes = sizeof(stringSize);
  int numBytesRcvd = 0;

  NETWORKING_LOG("Receiving string size");
  while (numBytesRcvd < totalNumBytes) {
    int result = recv(descriptor, &stringSize, totalNumBytes - numBytesRcvd, 0);
    if (result < 0) {
      cout << "Failed to receive string size from socket " << descriptor << ". "
           << "(" << strerror(errno) << ")." << endl;
      return -1;
    }

    numBytesRcvd += result;
    NETWORKING_LOG(numBytesRcvd << " out of " << totalNumBytes << " received.");
  }

  // receive the message
  totalNumBytes = stringSize;
  numBytesRcvd = 0;

  NETWORKING_LOG("Receiving message of size " << stringSize << " bytes.");
  char *c_buff = new char[MAX_BUFF_LEN];
  while (numBytesRcvd < totalNumBytes) {
    bzero(c_buff, MAX_BUFF_LEN);
    int bytesLeft = totalNumBytes - numBytesRcvd;
    int readSize = (bytesLeft - MAX_BUFF_LEN > 0) ? MAX_BUFF_LEN : bytesLeft;
    int result = recv(descriptor, c_buff, readSize, 0);
    if (result < 0) {
      cout << "Failed to receive string size from socket " << descriptor << ". "
           << "(" << strerror(errno) << ")." << endl;
      return -1;
    }
    numBytesRcvd += result;
    buff += c_buff;
    NETWORKING_LOG(numBytesRcvd << " out of " << totalNumBytes << " received.");
  }

  return numBytesRcvd + sizeof(stringSize);
}

void receiveMeeting(Meeting& meeting, int descriptor)
{
  string meeting_as_str;
  receiveMessage(meeting_as_str, descriptor);

  NETWORKING_LOG("Message Start");
  NETWORKING_LOG(meeting_as_str << flush);
  NETWORKING_LOG("Message End");

  istringstream iss(meeting_as_str);
  iss >> meeting;

}

void sendMeeting(Meeting& meeting, int descriptor)
{
  stringstream ss;
  ss << meeting;
  string str = ss.str();

  NETWORKING_LOG("Message Start");
  NETWORKING_LOG(str << flush);
  NETWORKING_LOG("Message End");

  sendMessage(str, descriptor);
}
