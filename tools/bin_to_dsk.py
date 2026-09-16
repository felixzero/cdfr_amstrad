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
    parser = argparse.ArgumentParser("ihx_to_dsk.py", description="Convert an Intel HEX output compiled file to an Amstrad disk file")
    parser.add_argument("input")
    parser.add_argument("-l", "--location", default="0x4000")
    parser.add_argument("-o", "--output", required=True)

    args = parser.parse_args()
    
    with open(args.input, "rb") as f:
        raw_binary_data = f.read()
    
    filename = b"TEST    "
    file_extension = b"BIN"
    catalog = struct.pack(
        "<B8s3s3xB",
        0, filename, file_extension, math.ceil(len(raw_binary_data) / 128) + 1
    )

    allocation_start = 2
    for i in range(math.ceil(len(raw_binary_data) / SECTOR_SIZE_BYTES / 2)):
        catalog += struct.pack("B", allocation_start + i)
    catalog += b"\x00" * (32 - len(catalog))
    catalog += b"\xE5" * (4 * SECTOR_SIZE_BYTES - len(catalog))

    data_location = int(args.location, base=16)
    data_length = len(raw_binary_data)
    ams_headers = struct.pack(
        "<B8s3s4xxxBHHBHH",
        0, filename, file_extension, 0x02, data_length, data_location, 1, data_length, data_location
    )
    ams_headers += b"\x00" * (64 - len(ams_headers))
    checksum = (sum(c for c in ams_headers) + (data_length >> 8) + (data_length & 0xFF)) & 0xFFFF
    ams_headers += struct.pack(
        "<HxH",
        data_length, checksum
    )
    ams_headers += b"\x00" * (128 - len(ams_headers))
    
    raw_binary_data = catalog + ams_headers + raw_binary_data

    with open(args.output, "wb") as f:
        track_size = NUMBER_OF_SECTOR_PER_TRACK * 2 + 1
        disk_information_block = struct.pack(
            f"<34s14sBBH{NUMBER_OF_TRACKS}B",
            b"EXTENDED CPC DSK File\r\nDisk-Info\r\n",
            b"ihx_to_dsk.py\r\n",
            NUMBER_OF_TRACKS, 1, 0, *([track_size] * NUMBER_OF_TRACKS)
        )
        f.write(disk_information_block)

        f.write(b"\x00" * (HEADER_BLOCK_SIZE - len(disk_information_block)))

        sector_size = SECTOR_SIZE_BYTES >> 8
        for track_number in range(NUMBER_OF_TRACKS):
            track_information_block = struct.pack(
                "<13s3xBB2xBBBB",
                b"Track-Info\r\n",
                track_number,
                SIDE_ID,
                sector_size,
                NUMBER_OF_SECTOR_PER_TRACK,
                GAP3_LENGTH,
                0xE5
            )
            f.write(track_information_block)
            
            sector_information_list = b""
            for sector_id in [x + 0xC1 for x in SECTOR_ORDERING]:
                sector_information_list += struct.pack(
                    "<BBBB2xH",
                    track_number,
                    SIDE_ID,
                    sector_id,
                    sector_size,
                    SECTOR_SIZE_BYTES
                )
            f.write(sector_information_list)
            f.write(b"\x00" * (HEADER_BLOCK_SIZE - len(sector_information_list) - len(track_information_block)))

            for raw_offset in SECTOR_ORDERING:
                sliced = raw_binary_data[raw_offset * SECTOR_SIZE_BYTES : (raw_offset + 1) * SECTOR_SIZE_BYTES]
                if len(sliced) != SECTOR_SIZE_BYTES:
                    sliced += b"\xE5" * (SECTOR_SIZE_BYTES - len(sliced))
                f.write(sliced)
            raw_binary_data = raw_binary_data[SECTOR_SIZE_BYTES * NUMBER_OF_SECTOR_PER_TRACK:]
