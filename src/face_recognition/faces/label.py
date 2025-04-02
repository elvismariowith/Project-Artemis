import csv
import glob
import os

with open('info.csv','w',newline='') as f:
    writer = csv.writer(f)
    for file in glob.glob('*.jpg'):
        print(file)
        if file.startswith('hector'):
            writer.writerow([file,1])