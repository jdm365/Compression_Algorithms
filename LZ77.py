import sys

def get_init_buffer(input_string):
    search_buffer = []
    for idx, element in enumerate(input_string):
        search_buffer.append(element)
        if element == search_buffer[0] and idx != 0:
            return search_buffer[:-1]
    return search_buffer

def encode_once(search_buffer, look_ahead_buffer):
    idx = 0
    score = 0
    matching = False
    matches = []
    new_string = ''
    for i in range(len(look_ahead_buffer)):
        if search_buffer[idx % len(search_buffer)] == look_ahead_buffer[i]:
            score += 1
            idx += 1
            matching = True
        elif matching:
            matches.append((look_ahead_buffer[i], score, i))
            idx = 0
            score = 0
            matching = False
            new_string += look_ahead_buffer[i]
        else:
            new_string += look_ahead_buffer[i]
    return matches, new_string

def stringify_txt_file(filename):
    with open(filename, 'r') as file:
        return file.read().replace('\n', '')
            

def encode_file(filename):
    input_string = stringify_txt_file(filename)
    matches = []
    print(f'Size of input {sys.getsizeof(input_string)}')
    while True:
        search_buffer = get_init_buffer(input_string)
        start_idx = len(search_buffer)
        file_length = len(input_string)

        if start_idx == file_length:
            print('File is maximally LZ77 compressed!')
            break
        look_ahead_buffer = input_string[start_idx:file_length]
        
        new_matches, input_string = encode_once(search_buffer, look_ahead_buffer)
        matches += new_matches
    print(f'Size of output {sys.getsizeof(matches) + sys.getsizeof(input_string)}')
    return matches, input_string


def decode_string(input_string):
    return


print(encode_file('declaration_of_independence.txt'))