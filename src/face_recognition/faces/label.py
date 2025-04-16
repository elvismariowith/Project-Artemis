import csv
import glob
import os

MAX_SAMPLES = 100
with open('info.csv','w',newline='') as f:
    writer = csv.writer(f)
    samples = [0 for _ in range(6)] 
    for file in glob.glob('*.jpg'):
        print(file)
        if file.startswith('hector'):
            samples[0] += 1
            if samples[0] > MAX_SAMPLES: continue
            writer.writerow([file,1])
        elif file.startswith('Daniel'):
            samples[1] += 1
            if samples[1] > MAX_SAMPLES: continue
            writer.writerow([file,2])
        elif file.startswith('franco'):
            samples[2] += 1
            if samples[2] > MAX_SAMPLES: continue
            writer.writerow([file,3])
        elif file.startswith('lx'):
            samples[3] += 1
            if samples[3] > MAX_SAMPLES: continue
            writer.writerow([file,4])
        elif file.startswith('luciano'):
            samples[4] += 1
            if samples[4] > MAX_SAMPLES: continue
            writer.writerow([file,5])
        elif file.startswith('elvis'):
            samples[5] += 1
            if samples[5] > MAX_SAMPLES: continue
            writer.writerow([file,6])