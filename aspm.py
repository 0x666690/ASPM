import subprocess
from enum import Enum

class ASPM(Enum):
    ASPM_DISABLED =   0b00
    ASPM_L0s_ONLY =   0b01
    ASPM_L1_ONLY =    0b10
    ASPM_L1_AND_L0s = 0b11

root_complex = "00:1c.4"
endpoint = "05:00.0"
value_to_set = ASPM.ASPM_L1_AND_L0s

def get_device_name(addr):
    p = subprocess.Popen([
        "lspci",
        "-s",
        addr,
    ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return p.communicate()[0].splitlines()[0].decode()

def read_all_bytes(device):
    all_bytes = bytearray()
    device_name = get_device_name(device)
    p = subprocess.Popen([
        "lspci",
        "-s",
        device,
        "-xxx"
    ], stdout= subprocess.PIPE, stderr=subprocess.PIPE)
    ret = p.communicate()
    ret = ret[0].decode()
    for line in ret.splitlines():
        if not device_name in line and ": " in line:
            all_bytes.extend(bytearray.fromhex(line.split(": ")[1]))
    if len(all_bytes) < 256:
        print(f"Expected 256 bytes, only got {len(all_bytes)} bytes!")
        print(f"Are you running this as root?")
        exit()
    return all_bytes

def find_byte_to_patch(bytes, pos):
    print(f"{hex(pos)} points to {hex(bytes[pos])}")
    pos = bytes[pos]
    print(f"Value at {hex(pos)} is {hex(bytes[pos])}")
    if bytes[pos] != 0x10:
        print("Value is not 0x10!")
        print("Reading the next byte...")
        pos += 0x1
        return find_byte_to_patch(bytes, pos)
    else:
        print(f"Found the byte at: {hex(pos)}")
        print("Adding 0x10 to the register...")
        pos += 0x10
        print(f"Final register reads: {hex(bytes[pos])}")
        return pos

def patch_byte(device, position, value):
    subprocess.Popen([
        "setpci",
        "-s",
        device,
        f"{hex(position)}.B={hex(value)}"
    ]).communicate()

def patch_device(addr):
    print(get_device_name(addr))
    endpoint_bytes = read_all_bytes(addr)
    byte_position_to_patch = find_byte_to_patch(endpoint_bytes, 0x34)

    print(f"Position of byte to patch: {hex(byte_position_to_patch)}")
    print(f"Byte is set to {hex(endpoint_bytes[byte_position_to_patch])}")
    print(f"-> {ASPM(int(endpoint_bytes[byte_position_to_patch]) & 0b11).name}")

    if int(endpoint_bytes[byte_position_to_patch]) & 0b11 != value_to_set.value:
        print("Value doesn't match the one we want, setting it!")
        
        patched_byte = int(endpoint_bytes[byte_position_to_patch])
        patched_byte = patched_byte >> 2
        patched_byte = patched_byte << 2
        patched_byte = patched_byte | value_to_set.value

        patch_byte(addr, byte_position_to_patch, patched_byte)
        new_bytes = read_all_bytes(addr)
        print(f"Byte is now set to {hex(new_bytes[byte_position_to_patch])}")
        print(f"-> {ASPM(int(new_bytes[byte_position_to_patch]) & 0b11).name}")
    else:
        print("Nothing to patch!")

def main():
    patch_device(root_complex)
    patch_device(endpoint)


if __name__ == "__main__":
    main()
