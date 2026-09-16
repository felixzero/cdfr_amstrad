import argparse
import struct
from PIL import Image

IMG_WIDTH = 160
IMG_HEIGHT = 200
LINE_INTERLEAF = 8

def interleaf(p1: int, p2: int) -> int:
    return (
        (p1 & 0b1000) << 4 | (p1 & 0b0100) << 3 | (p1 & 0b0010) << 2 | (p1 & 0b0001) << 1 |
        (p2 & 0b1000) << 3 | (p2 & 0b0100) << 2 | (p2 & 0b0010) << 1 | (p2 & 0b0001) << 0
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
        