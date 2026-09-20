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
AMS_HEADER_LENGTH = 128

class DiskManager:
    def __init__(self):
        self._free_sector = 2
        self._amsdos_catalog = bytes()
        self._binary_files = []
    
    def add_file(self, filename: str, file_extension: str, raw_binary_data: bytes, data_location: int, binary=True) -> None:
        filename = filename[:8]
        filename += " " * (8 - len(filename))
        enc_filename = bytes(filename, encoding="ascii")
        enc_file_extension = bytes(file_extension, encoding="ascii")
        
        total_size = len(raw_binary_data)
        if binary:
            total_size += AMS_HEADER_LENGTH
        if total_size % (2 * SECTOR_SIZE_BYTES) != 0:
            raw_binary_data += b"\x00" * (2 * SECTOR_SIZE_BYTES - (total_size % (2 * SECTOR_SIZE_BYTES)))

        data_length = len(raw_binary_data)
        extent = 0
        while data_length > 32 * SECTOR_SIZE_BYTES:
            self._add_catalog_entry(enc_filename, enc_file_extension, 32 * SECTOR_SIZE_BYTES, extent)
            data_length -= 32 * SECTOR_SIZE_BYTES
            extent += 1
        self._add_catalog_entry(enc_filename, enc_file_extension, data_length, extent)      

        if binary:
            self._add_binary_file(raw_binary_data, enc_filename, enc_file_extension, data_location)
        else:
            self._add_text_file(raw_binary_data)
        

    def _add_catalog_entry(self, enc_filename: bytes, enc_file_extension: bytes, data_length: int, extent=0) -> None:
        entry = struct.pack(
            "<B8s3s3xB",
            extent, enc_filename, enc_file_extension, math.ceil(data_length / 128)
        )
        for i in range(math.ceil(data_length / SECTOR_SIZE_BYTES / 2)):
            entry += struct.pack("B", self._free_sector + i)
        self._free_sector += i + 1
        
        entry += b"\x00" * (32 - len(entry))
        self._amsdos_catalog += entry


    def _close_catalog(self) -> None:
        self._amsdos_catalog += b"\xE5" * (4 * SECTOR_SIZE_BYTES - len(self._amsdos_catalog))


    def _add_binary_file(self, raw_binary_data: bytes, enc_filename: bytes, enc_file_extension: bytes, data_location: int) -> None:
        data_length = len(raw_binary_data)

        ams_headers = struct.pack(
            "<B8s3s4xxxBHHBHH",
            0, enc_filename, enc_file_extension, 2, data_length, data_location, 1, data_length, data_location
        )
        ams_headers += b"\x00" * (AMS_HEADER_LENGTH // 2 - len(ams_headers))
        checksum = (sum(c for c in ams_headers) + (data_length >> 8) + (data_length & 0xFF)) & 0xFFFF
        ams_headers += struct.pack(
            "<HxH",
            data_length, checksum
        )
        ams_headers += b"\x00" * (AMS_HEADER_LENGTH - len(ams_headers))
        
        self._binary_files.append(ams_headers + raw_binary_data)


    def _add_text_file(self, raw_binary_data: bytes) -> None:
        self._binary_files.append(raw_binary_data)    


    def write_to_file(self, filename: str) -> none:
        self._close_catalog()
        raw_binary_data = self._amsdos_catalog
        for bfile in self._binary_files:
            raw_binary_data += bfile

        with open(filename, "wb") as f:
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


if __name__ == "__main__":
    parser = argparse.ArgumentParser("bin_to_dsk.py", description="Convert a binary file compiled file to an Amstrad disk file")
    parser.add_argument("input")
    parser.add_argument("-l", "--location", default="0x8000")
    parser.add_argument("--background-image")
    parser.add_argument("--basic-loader")
    parser.add_argument("-o", "--output", required=True)

    args = parser.parse_args()
    
    with open(args.input, "rb") as f:
        raw_binary_data = f.read()
    data_location = int(args.location, base=16)
        
    manager = DiskManager()
    manager.add_file("CDFR", "BIN", raw_binary_data, data_location)
    
    if args.background_image is not None:
        with open(args.background_image, "rb") as f:
            raw_binary_data = f.read()
        manager.add_file("BACKGND", "BIN", raw_binary_data, 0x4000)
    
    if args.basic_loader is not None:
        with open(args.basic_loader, "rb") as f:
            raw_binary_data = f.read()
        raw_binary_data += b"\x1A"
        manager.add_file("CDFR", "BAS", raw_binary_data, 0, binary=False)

    manager.write_to_file(args.output)
