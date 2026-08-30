

## Packet Structure

| 0x00 | abxxxxxx | counter | ... | crc-8 |
| :------: | :------: | :------: | :------: | :------: |
| start byte | Packet identifier | Counter or COBS byte | optional payload | checksum (or 0xFF if a=1) |

#### Packet identifier rules:
| bit | description | details |
| :------: | :------: | :------: | 
| a | crc8 zero flag | If set, the packet checksum is 0 and the crc-8 bit will be 0xFF as a placeholder |
| b | acknowledgement flag  | If set, this packet is the an acknowledgement packet |
| xxxxxx | packet identifier | Unique packet identifier. Also determines if the packet contains a payload. |