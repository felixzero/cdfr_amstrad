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
        print(data)
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
    parser.add_argument("--background", required=True)

    args = parser.parse_args()
    
    initial_time = time.time()
    while True:
        try:
            query_server({ "cmd": "getStatus" })
            break
        except ConnectionRefusedError:
            if time.time() - initial_time > TIMEOUT:
                exit(1)
            time.sleep(1.0)
    
    query_server({"cmd": "reset"})
    
    with open(args.code, "rb") as f:
        code = f.read()
    write_block(code, 0x8000)

    with open(args.background, "rb") as f:
        background = f.read()
    write_block(background, 0x4000)
    
    query_server({
        "cmd": "setRegisters",
        "pc": 0x8000
    })
