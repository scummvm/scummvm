"""
Allows converting header files from escape sequences and back to readable form.
"""

from string import printable

def escape_char(c):
    if chr(c) in printable:
        return bytes([c])
    ashex = hex(c)[2:].upper().encode()
    return b'\\x' + ashex

def encode_seq(seq):
    try:
        print(int(b'0x' + seq[:2], 16))
        return bytes([int(b'0x' + seq[:2], 16)]) + seq[2:]
    except:
        return seq

if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='convert character escape sequence')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--escape', '-e', action='store_true')
    group.add_argument('--read', '-r', action='store_true')
    parser.add_argument('input', help='filename to read from')
    parser.add_argument('output', help='filename to write to')
    args = parser.parse_args()

    with open(args.input, 'rb') as f:
        data = f.read()

    if args.escape:
        with open(args.output, 'wb') as f:
            f.write(b''.join(escape_char(c) for c in data))

    else:
        with open(args.output, 'wb') as f:
            f.write(b''.join(encode_seq(seq) for seq in data.split(b'\\x')))
