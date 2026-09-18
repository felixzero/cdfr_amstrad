import argparse
import struct
from PIL import Image

IMG_WIDTH = 160
IMG_HEIGHT = 200
LINE_INTERLEAF = 8

def bit(x: int, n: int, shift: int) -> bool:
    return (1 << shift) if bool(x & (1 << n)) else 0

def interleaf(p1: int, p2: int) -> int:
    return (
        bit(p1, 0, 7) | bit(p2, 0, 6) | bit(p1, 2, 5) | bit(p2, 2, 4) | bit(p1, 1, 3) | bit(p2, 1, 2) | bit(p1, 3, 1) | bit(p2, 3, 0)
    )

if __name__ == "__main__":
    parser = argparse.ArgumentParser("png_to_background.py", description="Convert a PNG file into an ASM file with raw memory blocks")
    parser.add_argument("input")
    parser.add_argument("-o", "--output", required=True)

    args = parser.parse_args()
    
    img = Image.open(args.input)
    pixel_values = list(img.get_flattened_data())
    
    lines = []
    while len(pixel_values) > 0:
        line = pixel_values[:IMG_WIDTH]
        lines.append(struct.pack(f"{IMG_WIDTH // 2}B", *[interleaf(x, y) for x, y in zip(line[0::2], line[1::2])]))
        pixel_values = pixel_values[IMG_WIDTH:]

    with open(args.output, "wb") as f:
        for offset in range(LINE_INTERLEAF):
            for line in lines[offset::LINE_INTERLEAF]:
                f.write(line)
            if offset != 7:
                f.write(b"\x00" * 48)
        