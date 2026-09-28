import argparse
import socket
import json
import time

HOST = "localhost"
PORT = 6128
BLOCK_SIZE = 1024
TIMEOUT = 5.0

def query_server(query: dict) -> dict:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(json.dumps(query).encode("ascii"))
        data = s.recv(16 * 1024).decode("ascii")
        return json.loads(data)

def write_block(code: bytes, starting_addr: int):
    while len(code) > 0:
        print("Writing at address 0x%x" % starting_addr)
        query_server({
            "cmd": "writeMemory",
            "address": starting_addr,
            "bytes": list(code[:BLOCK_SIZE]),
            "memType": "ram"
        })
        code = code[BLOCK_SIZE:]
        starting_addr += BLOCK_SIZE
        

if __name__ == "__main__":
    parser = argparse.ArgumentParser("load_to_emulator.py", description="Load the compiled program to AceDL via sockets")
    parser.add_argument("--code", required=True)
    parser.add_argument("--initialized", required=True)
    parser.add_argument("--background", required=True)
    parser.add_argument("-c", "--code-location", default="0x8000")
    parser.add_argument("-b", "--background-location", default="0x4000")
    parser.add_argument("-d", "--initialized-location", default="0x9600")

    args = parser.parse_args()
    code_location = int(args.code_location, base=16)
    background_location = int(args.background_location, base=16)
    initialized_location = int(args.initialized_location, base=16)

    initial_time = time.time()
    while True:
        try:
            query_server({ "cmd": "getStatus" })
            break
        except ConnectionRefusedError:
            if time.time() - initial_time > TIMEOUT:
                exit(1)
            time.sleep(1.0)

    query_server({"cmd": "halt"})
    
    with open(args.background, "rb") as f:
        background = f.read()
    write_block(background, background_location)
    
    with open(args.initialized, "rb") as f:
        initialized = f.read()
    write_block(initialized, initialized_location)

    with open(args.code, "rb") as f:
        code = f.read()
    write_block(code, code_location)
    
    query_server({
        "cmd": "setRegisters",
        "pc": initialized_location
    })
    query_server({"cmd": "continue"})
    
