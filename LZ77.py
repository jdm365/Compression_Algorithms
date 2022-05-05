import sys
import numpy as np
from utils import *

class LZ77:
    def __init__(self, buffer_max_len=32000, lookahead_len=32):
        self.buffer_max_len = buffer_max_len
        self.lookahead_len = lookahead_len

    def find_longest_match(self, buffer, lookahead):
        idx = 0
        substr = lookahead[idx]
        cycles = 0

        if not substr in buffer:
            return 0, 0

        while substr in buffer:
            last_substr = cycles * lookahead + lookahead[:idx]
            idx += 1
            cycles = idx // len(lookahead)
            substr = cycles * lookahead + lookahead[:idx]
        buffer_idx = buffer.index(last_substr)
        length = len(last_substr)

        offset = len(buffer) - buffer_idx
        return offset, length


    def encode_string(self, string):
        output_stream = [[0, 0, string[0]]]
        idx = 1
        while idx < len(string):
            lookahead = string[idx:idx+self.lookahead_len]
            start = max(0, idx - self.buffer_max_len)
            buffer = string[start:idx]
            offset, length = self.find_longest_match(buffer, lookahead)
            break_char_idx = min(idx+length, len(string)-1)
            break_char = string[break_char_idx]
            output_stream.append([offset, length, break_char])
            idx += length + 1
        return output_stream


    def decode_string(self, encoded):
        reconstructed_string = ''
        for tup in encoded:
            offset, length, break_char = tup
            if length == 0:
                reconstructed_string += break_char
            else:
                idx = len(reconstructed_string) - offset
                string = reconstructed_string[idx:idx+length]
                reconstructed_string += string + break_char
        return reconstructed_string


    def encode(self, filename, verbose=True):
        filename = 'text_files/' + filename
        string = stringify_txt_file(filename)
        original_size = sys.getsizeof(string)
        encoded = self.encode_string(string)
        final_size = sys.getsizeof(encoded)
        if verbose:
            print('\n------------------------------------------------------')
            print('LZ77 Compression:')
            print(f'original size: {original_size//1000}KB')
            print(f'new size: {final_size//1000}KB')
            print(f'Compressed file is {100 * final_size // original_size}% of the original.')
        return encoded


    def decode(self, encoded_string, verbose=True):
        decoded = self.decode_string(encoded_string)
        reconstructed_size = sys.getsizeof(decoded)
        if verbose:
            print(f'reconstructed size: {reconstructed_size//1000}KB')
        return decoded

    def verify(self, filename, reconstructed):
        filename = 'text_files/'+ filename
        original = stringify_txt_file(filename)
        reconstructed_size = sys.getsizeof(reconstructed)
        original_size = sys.getsizeof(original)
        if original == reconstructed:
            print('Reconstructed file is the same as the original.')
            print('------------------------------------------------------\n')



if __name__ == '__main__':
    compressor = LZ77()
    filename = 'Frankenstein.txt'
    encoded = compressor.encode(filename)
    decoded = compressor.decode(encoded)
    compressor.verify(filename, decoded)
