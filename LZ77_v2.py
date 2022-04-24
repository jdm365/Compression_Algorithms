import sys


def encode(input_string):
    compressed = []
    # format -> (offset, length, start_charachter)
    idx = 0
    while idx < len(input_string):
        if idx + 1 == len(input_string):
            break
        if idx != 0:
            buffer = input_string[:idx]
        else:
            buffer = []
        if input_string[idx] in buffer:
            loc_in_buffer = buffer.index(input_string[idx])
            offset = len(buffer) - loc_in_buffer
            i = 0
            while input_string[idx - offset + i] == input_string[idx + i]:
                i += 1
            length = i
            compressed.append([offset, length, input_string[idx]])
            idx += length
        else:
            compressed.append([0, 0, input_string[idx]])
            idx += 1
    return compressed

def convert_to_bytes(encoded_list):
    bytes_string = ''
    for tup in encoded_list:
        offset, length, char = tup


def stringify_txt_file(filename):
    with open(filename, 'r') as file:
        return file.read().replace('\n', '')

def encode_file(filename):
    input_string = stringify_txt_file(filename)
    print(f'Size of input {sys.getsizeof(input_string)}')
    compressed = encode(input_string)
    print(f'Size of output {sys.getsizeof(compressed)}')

encode_file('declaration_of_independence.txt')
    
