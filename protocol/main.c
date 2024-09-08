#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "packet_encode.h"

struct SAMPLE_DATA_TYPE {
    long a;
    double b;
    int c;
    float d;
    bool e;
} sent, recv;

uint8_t send_buffer[300];

int main() {
    sent.a = 4930249231l;
    sent.b = 3241.1251;
    sent.c = 4124;
    sent.d = .01241;
    sent.e = true;

    int packet_size = generate_packet((uint8_t*)&sent, sizeof(sent), send_buffer, 14);

    printf("Sent packet: ");
    for (int i = 0; i < packet_size; i++) {
        printf("%d ", send_buffer[i]);
    }
    printf("\n\n");

    bool good = verify_packet(send_buffer, packet_size);

    if (good) {
        printf("Packet verified!\n\n");
    } else {
        printf("Packet not verified!\n\n");
    }

    extract_packet(send_buffer, packet_size, (uint8_t*)&recv);

    printf("%li %f %i %f %d", recv.a, recv.b, recv.c, recv.d, recv.e);

}

