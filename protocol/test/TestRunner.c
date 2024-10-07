#include "unity.h"
#include "crc_hash.h"
#include "packet_encode.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define NUM_SIZES 7
int sizes[NUM_SIZES];
    
void setUp(void) {
    // set stuff up here
    sizes[0] = 30;
    sizes[1] = 100;
    sizes[2] = 97; 
    sizes[3] = 96;
    sizes[4] = 87;
    sizes[5] = 86;
    sizes[6] = 253;

    time_t t = time(NULL);
    printf("Time for seed: %li \n", t);
    srand(t);
}

void tearDown(void) {
    // clean stuff up here
}

void testHashGeneration() {
    for (int t = 0; t < NUM_SIZES; t++) {
        uint8_t *buffer = malloc((sizes[t]+1)*sizeof(uint8_t));

        for (int attempt = 0; attempt < 1000; attempt++) {
            for (int i = 0; i < sizes[t]; i++) {
                buffer[i] = rand();
            }

            uint8_t hash1 = calculate_crc8_hash(buffer, sizes[t]);
            uint8_t hash2 = calculate_crc8_hash(buffer, sizes[t]);

            TEST_ASSERT_EQUAL_UINT8(hash1, hash2);

            buffer[sizes[t]] = hash1;

            TEST_ASSERT_TRUE(verify_crc8_hash(buffer, sizes[t]+1));
            TEST_ASSERT_TRUE(verify_crc8_hash(buffer, sizes[t]+1)); 

            // Now break the hash
            int indexChosen = rand() % sizes[t];
            buffer[indexChosen] = rand();

            uint8_t newHash = calculate_crc8_hash(buffer, sizes[t]);

            if (newHash == hash1) {
                printf("hash collision! (ignoring)\n");
            } else {
                TEST_ASSERT_NOT_EQUAL_UINT8(hash1, newHash);
                TEST_ASSERT_FALSE(verify_crc8_hash(buffer, sizes[t]+1));
            }
        }

        free(buffer);
    }



}

void testPacketEncode() {
    for (int t = 0; t < NUM_SIZES; t++) {
        uint8_t *original = malloc(sizes[t]*sizeof(uint8_t));
        uint8_t *packet = malloc(270*sizeof(uint8_t));
        uint8_t *extracted = malloc((10+sizes[t])*sizeof(uint8_t));
        uint8_t *buffer = malloc(500*sizeof(uint8_t));

        uint8_t emptyByte = rand();

        int miracle_baby_counter = 0;

        for (int attempt = 0; attempt < 1000; attempt++) {
            for (int i = 0; i < sizes[t]; i++) {
                original[i] = rand();
            }
            for (int i = 0; i < 270; i++) {
                packet[i] = emptyByte;
            }
            for (int i = 0; i < sizes[t] + 10; i++) {
                extracted[i] = emptyByte;
            }


            uint8_t messageId = rand();
            if (messageId == 0) messageId = 1; // Message ID cannot be 1

            int packetLen = generate_packet(original, sizes[t], packet + 10, messageId);
            TEST_ASSERT_EQUAL_INT(sizes[t]+5, packetLen);


            for (int i = 0; i < 10; i++) TEST_ASSERT_EQUAL_UINT8(emptyByte, packet[i]); // Test for out-of-bounds
            for (int i = packetLen + 10; i < 270; i++) TEST_ASSERT_EQUAL_UINT8(emptyByte, packet[i]); // Test for out-of-bounds 
            for (int i = 11; i < packetLen + 10; i++) TEST_ASSERT_NOT_EQUAL_UINT8(0, packet[i]); // Make sure no extra 0 bits

            int current_size = 0;
            for (int i = 0; i < 134; i++) {
                current_size = process_incoming_byte((uint8_t) rand(), buffer, current_size);
                if (current_size < 0) {
                    if (verify_packet(buffer, -current_size))
                        miracle_baby_counter++;
                    current_size = 0;
                }
            }
      
            for (int i = 0; i < sizes[t]+5; i++) {
                TEST_ASSERT_TRUE(current_size >= 0);
                current_size = process_incoming_byte((packet+10)[i], buffer, current_size);
            }
            TEST_ASSERT_TRUE(current_size < 0);
            TEST_ASSERT_EQUAL_UINT8(sizes[t] + 5, -current_size);

            for (int i = 0 ; i < sizes[t]+5; i++) {
                TEST_ASSERT_EQUAL_UINT8((packet+10)[i], buffer[i]);
            }
            
            TEST_ASSERT_EQUAL_UINT8(0, packet[10]);
            TEST_ASSERT_EQUAL_UINT8(messageId, packet[11]);

            TEST_ASSERT_TRUE(verify_packet(packet+10, packetLen));

            extract_packet(packet+10, packetLen, extracted+5);

            for (int i = 0; i < 5; i++) TEST_ASSERT_EQUAL_UINT8(emptyByte, extracted[i]); // Test for out-of-bounds
            for (int i = sizes[t] + 5; i < sizes[t] + 10; i++) TEST_ASSERT_EQUAL_UINT8(emptyByte, extracted[i]); // Test for out-of-bounds 
            for (int i = 0; i < sizes[t]; i++) {
                TEST_ASSERT_EQUAL_UINT8(original[i], extracted[i+5]);
            }
        }


        printf("Created %d miracle babies\n", miracle_baby_counter);

        free(packet);
        free(original);
        free(extracted);
        free(buffer);
    }
}

// not needed when using generate_test_runner.rb
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(testHashGeneration);
    RUN_TEST(testPacketEncode);
    return UNITY_END();
}