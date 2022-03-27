import sys

def encode(input_string):
    lookback_length = 6
    buffer_length = 4
    for i in range(len(input_string)):
        lookback_window = input_string[i-lookback_length:i]
        buffer = input_string
        
    