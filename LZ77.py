


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
    old_string = search_buffer + look_ahead_buffer
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
    return matches
            

def encode_file(input_string):
    search_buffer = get_init_buffer(input_string)
    start_idx = len(search_buffer)
    file_length = len(input_string)

    if start_idx == file_length:
        raise ValueError('File is maximally LZ77 compressed!')

    look_ahead_buffer = input_string[start_idx:file_length]

    score = 0
    for idx, element in enumerate(look_ahead_buffer):
        if element == search_buffer[idx]:
            score += 1


string = 'asdfa'
search_buffer = 'abcd'
look_ahead_buffer = 'apples and oranges are great and good and delicious abcd!'
print(encode_once(search_buffer, look_ahead_buffer))
#encode(string)