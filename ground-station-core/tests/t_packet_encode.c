#include "tst.h"
#include "packet_encode.h"

const int NUM_FUZZY_ATTEMPTS_PACKET_ENCODE = 5000;

tstsuite("Extract Packet") {
    
    tstcase("Extract all zeroes") {
        uint8_t buffer[252+5], extracted[252+5];

        for (int i = 0; i < 252+5; i++) buffer[i] = 0;

        tstassert(!verify_packet(buffer, 252+5));
        tstassert(0 == extract_packet(buffer, 252+5, extracted));
    }

    tstcase("Extract all zeroes except message id") {
        uint8_t buffer[252+5], extracted[252+5];

        for (int i = 0; i < 252+5; i++) buffer[i] = 0;
        buffer[1] = 10;

        tstassert(!verify_packet(buffer, 252+5));
        tstassert(0 == extract_packet(buffer, 252+5, extracted));
    }

    tstcase("Extract tiny packet") {
        uint8_t buffer[4];

        for (int i = 0; i < 252+5; i++) buffer[i] = 0;
        buffer[1] = 10; buffer[2] = 12; buffer[3] = 2;

        tstassert(false == verify_packet(buffer, 4));
    }

    tstcase("Try verify garbage") {
        tstcheck(true);
    }

    int sizes[] = {0, 13, 20, 23, 40, 53, 252}; 

    for (int size_i = 0; size_i < 7; size_i++) {
        tstcase("Fuzzy Testing Generate Packet with size %d bytes", sizes[size_i]) {
            uint8_t *raw_data = malloc(sizes[size_i]);
            uint8_t *packetized_data = malloc(sizes[size_i] + 5);
            uint8_t *extracted_data = malloc(sizes[size_i]);
            
            for (int t = 0; t < NUM_FUZZY_ATTEMPTS_PACKET_ENCODE; t++) {
                // Generate raw data and message id
                for (int i = 0; i < sizes[size_i]; i++) raw_data[i] = rand();
                uint8_t msg_id = rand();
                if(msg_id == 0) msg_id = 1; 

                int packet_size = generate_packet(raw_data, sizes[size_i], packetized_data, msg_id);

                tstassert(packet_size == sizes[size_i] + 5); // Check size is correct
                for (int i = 1; i < sizes[size_i] + 5; i++) tstassert(packetized_data[i] != 0, "Check %d byte from first %d %d %d %d", i, packetized_data[0], packetized_data[1], packetized_data[2], packetized_data[3]); // Make sure 0 bytes were removed
                
                tstassert(packetized_data[0] == 0);             // Make sure first byte is 0
                tstassert(packetized_data[1] == msg_id, "d1: %d mid: %d ", packetized_data[1], msg_id);        // Make sure second byte is message id
                tstassert(packetized_data[3] == sizes[size_i] || sizes[size_i] == 0); // Make sure fourth byte is payload size

                tstassert(verify_packet(packetized_data, sizes[size_i]+5));
                tstassert(msg_id == extract_packet(packetized_data, sizes[size_i]+5, extracted_data));
                for (int i = 0; i < sizes[size_i]; i++) tstassert(raw_data[i] == extracted_data[i], "byte %d val %d vs %d", i, raw_data[i], packetized_data[i]);
            }

            free (raw_data);
            free(packetized_data);
            free(extracted_data);
        }
    }

    tstcase("Generate packet with all zeroes") {
        uint8_t buffer[252];
        uint8_t packet[252+5];
        uint8_t extracted[252];
        for (int i = 0; i < 252; i++) buffer[i] = 0;

        for (int s = 1; s <= 252; s++) {
            int packet_size = generate_packet(buffer, s, packet, 5);
            tstassert(packet_size == s + 5);

            for (int i = 1; i < s + 5; i++) tstassert(packet[i] != 0); // Make sure 0 bytes were removed
                
            tstassert(packet[0] == 0); // Make sure first byte is 0
            tstassert(packet[1] == 5); // Make sure second byte is message id
            tstassert(packet[3] == s || s == 0); // Make sure fourth byte is payload size

            tstassert(verify_packet(packet, s+5));
            tstassert(5 == extract_packet(packet, s+5, extracted));
            for (int i = 0; i < s; i++) tstassert(buffer[i] == extracted[i]);
        }
    }

    tstcase("Generate 252 byte packet with no 0") {
        uint8_t buffer[252];
        uint8_t packet[252+5];
        uint8_t extracted[252];
        for (int i = 0; i < 252; i++) {
            buffer[i] = rand();
            if (buffer[i] == 0) buffer[i] = 1;
        }

        tstassert(252+5 == generate_packet(buffer, 252, packet, 5));

        for (int i = 1; i < 252+5; i++) tstassert(packet[i] != 0); // Make sure 0 bytes were removed
            
        tstassert(packet[0] == 0); // Make sure first byte is 0
        tstassert(packet[1] == 5); // Make sure second byte is message id
        tstassert(packet[3] == 252); // Make sure third byte is payload size

        tstassert(verify_packet(packet, 252+5));
        tstassert(5 == extract_packet(packet, 252+5, extracted));
        for (int i = 0; i < 252; i++) tstassert(buffer[i] == extracted[i]);
    }

    tstcase("Read all zero bytes") {
        uint8_t buffer[1] = {0};
        for (int i = 0; i < NUM_FUZZY_ATTEMPTS_PACKET_ENCODE; i++) {
            tstassert(process_incoming_byte(0, buffer, 1) == 1);
            tstassert(buffer[0] == 0);
        }
    }

    tstcase("Read random gibberish") {
        uint8_t buffer[252+5];
        for (int i = 0; i < NUM_FUZZY_ATTEMPTS_PACKET_ENCODE*10; i++) {
            uint8_t new_bit = rand();
            int buff_size = process_incoming_byte(new_bit, buffer, 1);
            
            if (buff_size < 0) {
                tstassert(buff_size <= -5);
                tstassert( (buffer[2] == 1)?0:buffer[3] == -buff_size - 5);
            } else {
                tstassert((buff_size == 1) == (new_bit == 0));
                if (buff_size > 0) tstassert(buffer[0] == 0);
            }
        }
    }

    tstcase("Read 0 byte message") {
        uint8_t packet[5];
        uint8_t data[0];
        uint8_t buffer[5];

        generate_packet(data, 0, packet, 5);

        for (int i = 0; i < 4; i++) {
            tstassert(i+1 == process_incoming_byte(packet[i], buffer, i));
            tstassert(packet[i] == buffer[i]);
        }
        int a = process_incoming_byte(packet[4], buffer, 4);
        tstassert(-5 == a, "%d", a);
        tstassert(packet[4] == buffer[4]);
    }

    tstcase("Read 252 byte message") {
        uint8_t packet[252+5];
        uint8_t data[252];
        uint8_t buffer[252+5];

        for (int i = 0; i < 252; i++) data[i] = rand();

        generate_packet(data, 252, packet, 5);

        for (int i = 0; i < 252+4; i++) {
            tstassert(i+1 == process_incoming_byte(packet[i], buffer, i));
            tstassert(packet[i] == buffer[i]);
        }
        tstassert(-252-5 == process_incoming_byte(packet[252+4], buffer, 252+4));
        tstassert(packet[252+4] == buffer[252+4]);
    }

    for (int size_i = 0; size_i < 7; size_i++) {
        tstcase("Read packet with size %d bytes", sizes[size_i]) {
            uint8_t *raw_data = malloc(sizes[size_i]);
            uint8_t *packetized_data = malloc(sizes[size_i] + 5);
            uint8_t read_buffer[270];

            for (int i = 0; i < sizes[size_i]; i++) raw_data[i] = rand();

            uint8_t msg_id = rand();
            if (msg_id == 0x00) msg_id = 0x01;

            generate_packet(raw_data, sizes[size_i], packetized_data, msg_id);

            int buff_size = 0;
            for (int i = 0; i < 50; i++) {
                buff_size = process_incoming_byte(rand(), read_buffer, buff_size);
                if (buff_size < 0) buff_size = 0;
            }

            for (int i = 0; i < sizes[size_i] + 4; i++) {
                tstassert(i+1 == process_incoming_byte(packetized_data[i], read_buffer, buff_size));
                tstassert(packetized_data[i] == read_buffer[i]);
                buff_size = i+1;
            }

            tstassert(-sizes[size_i]-5 == process_incoming_byte(packetized_data[sizes[size_i] + 4], read_buffer, buff_size));
            tstassert(packetized_data[sizes[size_i] + 4] == read_buffer[sizes[size_i] + 4]);
        }
    }
}

