#include "stop_and_wait.h"

int main(int argc, char *argv[])
{
    Send sender = Send(argv[1]);

    Receive receiver = Receive(argv[1]);

    if (argc == 3)
    {
        sender.setup();

        cout << "setup complete... send messages now" << endl;

        sender.send();
        // sender.teardown();
    }

    if (argc == 2)
    {
        receiver.setup();

        cout << "receiver: setup complete. Listening for messages now..." << endl;

        receiver.receive();
        // receiver.teardown();
    }

    return 0;
}