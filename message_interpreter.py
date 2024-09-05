import struct

MAX_MESSAGE_LEN = 255

def get_message_struct(message_id: int):
    return '<ii?f'


class Interpreter:

    def __init__(self) -> None:
        self.reset()

    def reset(self):
        self._msg = bytearray()
        self._msg_len = MAX_MESSAGE_LEN
        self._reading_msg = False

    def read_byte(self, data: int):
        if data == 0: # New packet
            if len(self._msg) > 0:
                print("Deleting partial message due to new 0 byte!!!")

            self.reset()
            self._reading_msg = True
            print("reading message!")

        elif not self._reading_msg: # Data not part of a packet
            print(f"Warning: unpacketized byte found: {data}")

        else: # Data that is part of an active packet!
            self._msg.append(data)
            if len(self._msg) == 2: # This is the payload size byte
                self._msg_len = 4 + data
                print(f"Determined message length to be {self._msg_len}")

            if len(self._msg) >= self._msg_len:
                print(f"Recv packet: {self._msg.hex(' ').upper()}")

                cobs_check = 2
                while cobs_check < len(self._msg):
                    increment = self._msg[cobs_check]
                    self._msg[cobs_check] =  0
                    cobs_check += increment

                print(f"De-COBSED packet: {self._msg.hex(' ').upper()}")

                print(len(self._msg[3:-1]))
                print(struct.unpack('<ii????f', self._msg[3:-1]))

                self.reset()
                

itpt = Interpreter()

data = bytearray(b'\x56\x00\x43\xFF\x00\x0f\x10\x04\xf0\xe0\x01\x04\x6f\x52\x42\x02\x01\x01\x01\x06\xd8\x56\xde\x3e\xab')
print(len(data))
for byte in data:
    itpt.read_byte(byte)

