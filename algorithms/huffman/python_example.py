import heapq
from collections import Counter, defaultdict

class Node:
    def __init__(self, char, freq):
        self.char  = char
        self.freq  = freq
        self.left  = None
        self.right = None

    # For the priority queue to work
    def __lt__(self, other):
        return self.freq < other.freq

def build_huffman_tree(text):
    # Count frequency of each character
    frequency = Counter(text)

    # Create a priority queue to store nodes
    priority_queue = [Node(char, freq) for char, freq in frequency.items()]
    heapq.heapify(priority_queue)

    # Iteratively combine two smallest nodes
    while len(priority_queue) > 1:
        left = heapq.heappop(priority_queue)
        right = heapq.heappop(priority_queue)

        merged = Node(None, left.freq + right.freq)
        merged.left = left
        merged.right = right

        heapq.heappush(priority_queue, merged)

    return priority_queue[0]

def build_code_table(node, prefix="", code_table=defaultdict(str)):
    if node is not None:
        if node.char is not None:
            code_table[node.char] = prefix
        build_code_table(node.left, prefix + "0", code_table)
        build_code_table(node.right, prefix + "1", code_table)
    return code_table

def huffman_encode(text, code_table):
    return ''.join(code_table[char] for char in text)

def huffman_decode(encoded_text, root):
    decoded_text = ""
    current_node = root
    for bit in encoded_text:
        if bit == '0':
            current_node = current_node.left
        else:
            current_node = current_node.right

        if current_node.char is not None:
            decoded_text += current_node.char
            current_node = root

    return decoded_text

if __name__ == '__main__':
    # Example usage
    filename = '../../data/declaration_of_independence.txt'
    with open(filename, 'r') as f:
        text = f.read()

    root = build_huffman_tree(text)
    code_table = build_code_table(root)

    encoded_text = huffman_encode(text, code_table)
    decoded_text = huffman_decode(encoded_text, root)

    ## Convert string 0's and 1's to bits
    compressed_buffer = int(encoded_text, 2).to_bytes((len(encoded_text) + 7) // 8, byteorder='big')

    print("Original text:", text)

    print(f'Original buffer size:   {len(text)}')
    print(f'Compressed buffer size: {len(compressed_buffer)}')


    ## Print out all the codes
    for char, code in code_table.items():
        print(f'{char}: {code}')
