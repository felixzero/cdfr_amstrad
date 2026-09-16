import struct
import argparse
import math

NUMBER_OF_TRACKS = 40
NUMBER_OF_SECTOR_PER_TRACK = 9
SECTOR_SIZE_BYTES = 512
GAP3_LENGTH = 0x2A
HEADER_BLOCK_SIZE = 0x100
SIDE_ID = 0
SECTOR_ORDERING = [0, 5, 1, 6, 2, 7, 3, 8, 4]

if __name__ == "__main__":
    parser = argparse.ArgumentParser("ihx_to_bin.py", description="Convert an Intel HEX output compiled file to an Amstrad BIN file")
    parser.add_argument("input")
    parser.add_argument("-l", "--location", default="0x4000")
    parser.add_argument("-o", "--output", required=True)

    args = parser.parse_args()
    
    raw_binary_data = bytearray()
    with open(args.input, "r") as f:
        for line in f:
            if line[0] != ":":
                continue
            size = int(line[1:3], base=16)
            start_addr = int(line[3:7], base=16) - int(args.location, base=16)
            data = bytes.fromhex(line[9:-3])
            
            if len(raw_binary_data) < start_addr + size:
                raw_binary_data += bytes.fromhex("00") * (start_addr + size - len(raw_binary_data))
            
            raw_binary_data[start_addr:start_addr + size] = data
    
    with open(args.output, "wb") as f:
        f.write(raw_binary_data)
