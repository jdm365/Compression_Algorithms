import sys
from utils import *



class LZ77Base:
    def __init__(self):
        return

    def encode(self, input_string):
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



class GZip:
    def __init__(self, min_length):
        self.min_length = min_length


    def encode(self, input_string):
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
                #if length < self.length:
                #    compressed.append([])
                compressed.append([offset, length, input_string[idx]])
                idx += length
            else:
                compressed.append([0, 0, input_string[idx]])
                idx += 1
        return compressed


