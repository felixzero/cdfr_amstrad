import struct
import sys
import pathlib
from PIL import Image

def interleaf(p1: int, p2: int) -> int:
    return (
        (p1 & 0b1000) << 4 | (p1 & 0b0100) << 3 | (p1 & 0b0010) << 2 | (p1 & 0b0001) << 1 |
        (p2 & 0b1000) << 3 | (p2 & 0b0100) << 2 | (p2 & 0b0010) << 1 | (p2 & 0b0001) << 0
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
        f.write(".area _DATA\n")
        for sprite_name, pixels in sprite_data.items():
            f.write(f"{sprite_name}:\n")
            for line in pixels:
                f.write(".db " + ", ".join(str(x) for x in line) + "\n")
            f.write("\n")
