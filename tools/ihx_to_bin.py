import struct
import argparse
import math

if __name__ == "__main__":
    parser = argparse.ArgumentParser("ihx_to_bin.py", description="Convert an Intel HEX output compiled file to an Amstrad BIN file")
    parser.add_argument("input")
    parser.add_argument("-c", "--code-location", required=True)
    parser.add_argument("-d", "--initialized-location")
    parser.add_argument("-o", "--output", required=True)

    args = parser.parse_args()
    code_location = int(args.code_location, base=16)
    if args.initialized_location is not None:
        initialized_location = int(args.initialized_location, base=16)
    else:
        initialized_location = 0

    raw_binary_data = bytearray()
    with open(args.input, "r") as f:
        for line in f:
            if line[0] != ":":
                continue
            size = int(line[1:3], base=16)
            start_addr = int(line[3:7], base=16)
            data = bytes.fromhex(line[9:-3])
            
            if start_addr == 0:
                continue
            
            if start_addr < min(code_location, initialized_location):
                print("Error: segment outside of range: 0x%x" % start_addr)
                exit(1)
            
            if len(raw_binary_data) < start_addr + size:
                raw_binary_data += bytes.fromhex("00") * (start_addr + size - len(raw_binary_data))
            
            raw_binary_data[start_addr:start_addr + size] = data
    
    with open(args.output, "wb") as f_code:
        if code_location < initialized_location:
            f_code.write(raw_binary_data[code_location:initialized_location].strip(b"\x00"))
        else:
            f_code.write(raw_binary_data[code_location:])

    if args.initialized_location is not None:
        with open("build/initialized.bin", "wb") as f_init:
            if code_location < initialized_location:
                f_init.write(raw_binary_data[initialized_location:])
            else:
                f_init.write(raw_binary_data[initialized_location:code_location])
