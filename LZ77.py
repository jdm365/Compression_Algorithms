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
    remove_indices = []
    new_string = ''
    for i in range(len(look_ahead_buffer)):
        if search_buffer[idx % len(search_buffer)] == look_ahead_buffer[i]:
            score += 1
            idx += 1
            matching = True
            remove_indices.append(i + len(search_buffer))
        elif matching:
            matches.append((look_ahead_buffer[i], score, i))
            idx = 0
            score = 0
            matching = False
            new_string += look_ahead_buffer[i]
        else:
            new_string += look_ahead_buffer[i]
    return matches, new_string
            

def encode_file(input_string):
    matches = []
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
    return matches, input_string


string = 'asdfa'
search_buffer = 'abcd'
look_ahead_buffer = 'apples and oranges are great and good and delicious abcd!'
print(encode_file(look_ahead_buffer))