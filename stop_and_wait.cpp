// Stop and wait implementation test file
// April 18th, 2022
// Liam Jeanette, Giorgi Alavidze, and Evan Lang

#include "stop_and_wait.h"

Send sender;
Receive receiver;

void exit_send(int x)
{
    sender.teardown();
    exit(0);
}

void exit_receive(int x)
{
    receiver.teardown();
    exit(0);
}

int main(int argc, char *argv[])
{
    sender.hostname = argv[1];
    receiver.hostname = argv[1];

    // Set up signal processing (interrupts)
    struct sigaction sa;
    if (argc == 3)
    {
        sa.sa_handler = exit_send;
    }
    else if (argc == 2)
    {
        sa.sa_handler = exit_receive;
    }
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("client: sigaction");
        exit(1);
    }

    if (argc == 3)
    {
        sender.setup();

        cout << "setup complete... send messages now" << endl;

        sender.send();

        cout << "sender: closing connection..." << endl;
    }

    if (argc == 2)
    {
        receiver.setup();

        cout << "receiver: setup complete. Listening for messages now..." << endl;

        receiver.receive();

        cout << "receiver: closing connection..." << endl;
    }

    return 0;
}