mkdir -p data;
cd data;
wget https://archive.org/download/enwik9/enwik9.zip;
unzip enwik9.zip;
rm enwik9.zip;
head -c 100000000 enwik9 > enwik8;
head -c 10000000 enwik8 > enwik7;
head -c 1000000 enwik7 > enwik6;
cd ../;
