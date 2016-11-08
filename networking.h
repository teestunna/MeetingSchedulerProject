#define MAX_PENDING_CONNECTIONS 10
#define MAX_BUFF_LEN 1024
#define NETWORKING_DEBUG 0

#if NETWORKING_DEBUG
#define NETWORKING_LOG(x) do { std::cout << "NETWORKING: " << x << endl; } while (0)
#else
#define NETWORKING_LOG(x)
#endif

#include <string>
using namespace std;

class Meeting;

int sendMessage(string& buff, int descriptor);
int receiveMessage(string& buff, int descriptor);
void setupListenSocket(int port, int *listen_socket);
void acceptIncomingConnection(int *listen_socket, int *accept_socket);
void connectToServer(char *ip_address, int port, int *descriptor);
void sendMeeting(Meeting& meeting, int descriptor);
void receiveMeeting(Meeting& meeting, int descriptor);
