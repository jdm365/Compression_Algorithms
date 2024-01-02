with open('data/enwik9', 'r') as f:
    data = f.read()

enwik8 = data[:100000000]
enwik7 = data[:10000000]
enwik6 = data[:1000000]

with open('data/enwik8', 'w') as f:
    f.write(enwik8)
    print('Created enwik8')

with open('data/enwik7', 'w') as f:
    f.write(enwik7)
    print('Created enwik7')

with open('data/enwik6', 'w') as f:
    f.write(enwik6)
    print('Created enwik6')
