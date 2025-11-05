#!/usr/bin/env python3

'''
    down.py

    Copyright (C) 2025  Isaac Wood

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
'''

import os
import sys
import shutil
import subprocess
import random
import glob
import time
import threading
from pathlib import Path

l = sys.argv[1] if len(sys.argv) > 1 else None
m = sys.argv[2] if len(sys.argv) > 2 else None
n = sys.argv[3] if len(sys.argv) > 3 else None
PREFIX = sys.argv[4] if len(sys.argv) > 4 else None

if None in [l, m, n, PREFIX]:
    print("Usage: python down.py <l> <m> <n> <prefix> [pathlength] [failed reductions needed] [reductions needed] [processors] [splits] [split_distance]")
    sys.exit(1)

PATHLENGTH = sys.argv[5] if len(sys.argv) > 5 else '10000000'
TOTAL_NO_REDUCTION_FLIPS_THRESHOLD = int(sys.argv[6]) if len(sys.argv) > 6 else 100 # Total ./flip calls (across all threads) that yield no reduction before halting.
TOTAL_REDUCTIONS_FOUND_THRESHOLD = int(sys.argv[7]) if len(sys.argv) > 7 else 25# Number of *distinct rank reductions* found before moving to the new target rank.
PARALLEL_INSTANCES = int(sys.argv[8]) if len(sys.argv) > 8 else 8
splits = sys.argv[9] if len(sys.argv) > 9 else "1" # should the flips be doing splits or not?
split_distance = sys.argv[10] if len(sys.argv) > 10 else "1"


def find_lowest_rank_dir():
    for lowest_rank in range(256):
        lowest_rank_dir = f"solutions/{l},{m},{n}/{PREFIX}{lowest_rank}"
        if os.path.isdir(lowest_rank_dir):
            return lowest_rank_dir
    print(f"Error: No starting directory found in solutions/{l},{m},{n}/ with prefix '{PREFIX}'")
    print("Please create one, e.g., 'solutions/2,2,2/s8' containing 222.exp")
    sys.exit(1)
    return ""

def get_random_file_from_dir(directory):
    if os.path.isdir(directory):
        files = [f for f in Path(directory).iterdir() if f.is_file()]
        return str(random.choice(files)) if files else ""
    return ""


start_overall_time = time.time()


print(f"\n--- Processing target tensor <{l},{m},{n}> with prefix '{PREFIX}' ---")

base_dir = Path("solutions") / f"{l},{m},{n}"

print("\n Starting flips")
current_dir = [Path(find_lowest_rank_dir())]
try:
    current_rank = [int(current_dir[0].name[len(PREFIX):])]
except:
    print(f"Couldn't get rank from folder name [{current_dir[0].name}]")
    sys.exit(1)
print(f"--Trying to reduce from rank {current_rank[0]}--\n")
failed_attempts = [0]
reductions = [0]
stop = [False]
lock = [threading.Lock()]

print(f"Failed reductions ({failed_attempts[0]} / {TOTAL_NO_REDUCTION_FLIPS_THRESHOLD})",end='')

def flip_worker(thread_id, home_dir):
    while True:
        #choose a file
        with lock[0]:
            input_file = get_random_file_from_dir(home_dir / f"{PREFIX}{current_rank[0]}")
        proc = subprocess.Popen([f"./flip", input_file, n, m, l, PATHLENGTH, splits, "0", split_distance, "0"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True) # Run a flip graph search with desired splits, no resets and *no correctness checks*.
        out, err = proc.communicate()
        out = out.strip()

        # Move it to the right place
        try:
            new_file_path_str, new_rank_str = out.split(',', 1)
        except:
            print(f"ERROR: C++ program output was not parsable.")
            print(f"       Input file: {input_file}")
            print(f"       STDOUT: {out}")
            print(f"       STDERR: {err}\n\n")
            # Don't stop the worker, just try again
            continue

        new_rank = int(new_rank_str.strip())
        new_file_path = Path(new_file_path_str.strip())

        if not new_file_path.exists():
            print(f"ERROR: C++ program reported file {new_file_path} but it does not exist.\n")
            continue

        with lock[0]:
            if new_rank < current_rank[0]:
            #if it is a reduction, add one to the reduction counter
                # home_dir is base_dir (solutions/{l},{m},{n}), so this path is correct
                new_dir = home_dir / f"{PREFIX}{new_rank}"
                new_dir.mkdir(exist_ok=True)

                try:
                    shutil.move(new_file_path, new_dir / new_file_path.name)
                except Exception as e:
                    print(f"ERROR: Could not move file {new_file_path} to {new_dir}. {e}\n")
                    # If move fails, at least delete it so it's not processed again
                    new_file_path.unlink(missing_ok=True)

                reductions[0] += 1
                print(f"\r Reduction found! ({reductions[0]} / {TOTAL_REDUCTIONS_FOUND_THRESHOLD})             ",end='')
                #if we now have enough reductions, reduce one to the rank counter
                if reductions[0] >= TOTAL_REDUCTIONS_FOUND_THRESHOLD:
                    current_rank[0] -= 1
                    current_dir[0] = home_dir / f"{PREFIX}{current_rank[0]}"
                    reductions[0] = 0
                    failed_attempts[0] = 0
                    print(f"\n\n--Trying to reduce from rank {current_rank[0]}--\n")
            #else stop all workers if too many things
            else:
                new_file_path.unlink(missing_ok=True) # Delete non-reduction
                failed_attempts[0] += 1
                print(f"\rFailed reductions ({failed_attempts[0]} / {TOTAL_NO_REDUCTION_FLIPS_THRESHOLD})           ",end='')
                if reductions[0]==0 and failed_attempts[0]>=TOTAL_NO_REDUCTION_FLIPS_THRESHOLD:
                    stop[0] = True

        time.sleep(0.1) # Short sleep to prevent busy-looping if something goes wrong
        with lock[0]:
            if stop[0]:
                return

flip_threads = []
for i in range(PARALLEL_INSTANCES):
    t = threading.Thread(target=flip_worker,args=(i,base_dir))
    t.start()
    flip_threads.append(t)

# Main thread loop
try:
    while not stop[0]:
        # This loop logic is slightly changed to be more robust
        with lock[0]:
            if reductions[0] > 0:
                # If we are finding reductions, reset the fail counter
                failed_attempts[0] = 0

            if reductions[0] == 0 and failed_attempts[0] >= TOTAL_NO_REDUCTION_FLIPS_THRESHOLD:
                stop[0] = True

        time.sleep(1)
except KeyboardInterrupt:
    print("\n--- KeyboardInterrupt received, stopping workers... ---")
    with lock[0]:
        stop[0] = True

for t in flip_threads:
    t.join()


overall_time = time.time() - start_overall_time

print(f"\n---Finished size after {overall_time // 3600:.0f} hrs {(overall_time // 60) % 60:.0f} mins {overall_time % 60:.2f} secs---")
