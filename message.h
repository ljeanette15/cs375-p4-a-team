#define MAXBUFLEN 100

struct Message
{
    char seqnum1;
    char seqnum2;
    char seqnum3;
    char seqnum4;

    char ack;

    char control;

    char len1;
    char len2;

    char body[MAXBUFLEN];
};
