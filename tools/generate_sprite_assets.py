import struct
import sys
import pathlib
from PIL import Image

def bit(x: int, n: int, shift: int) -> bool:
    return (1 << shift) if bool(x & (1 << n)) else 0

def interleaf(p1: int, p2: int) -> int:
    return (
        bit(p1, 0, 7) | bit(p2, 0, 6) | bit(p1, 2, 5) | bit(p2, 2, 4) | bit(p1, 1, 3) | bit(p2, 1, 2) | bit(p1, 3, 1) | bit(p2, 3, 0)
    )

if __name__ == "__main__":
    sprite_data = {}
    
    for input_image in sys.argv[1:]:
        img = Image.open(input_image)
        pixel_values = list(img.get_flattened_data())
        
        sprite_data[pathlib.Path(input_image).stem] = [
            [
                interleaf(x, y)
                for x, y in zip(
                    pixel_values[line_number * img.width:(line_number + 1) * img.width][0::2],
                    pixel_values[line_number * img.width:(line_number + 1) * img.width][1::2]
                )
            ]
            for line_number in range(img.height)
        ]

    with open("build/sprite_assets.s", "w") as f:
        for sprite_name in sprite_data:
            f.write(f".globl _{sprite_name}\n")
        f.write("\n")
        f.write(".area _INITIALIZED\n")
        for sprite_name, pixels in sprite_data.items():
            f.write(f"_{sprite_name}:\n")
            for line in pixels:
                f.write(".db " + ", ".join(str(x) for x in line) + "\n")
            f.write("\n")
