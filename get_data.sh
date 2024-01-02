mkdir -p data;
cd data;
wget https://archive.org/download/enwik9/enwik9.zip;
unzip enwik9.zip;
rm enwik9.zip;
cd ../;
python make_datasets.py
