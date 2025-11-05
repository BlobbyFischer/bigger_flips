#!/usr/bin/env python3

'''
    expand.py

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


import sys
import re
import shutil
import threading
import time
from pathlib import Path

def format_term(i,j,k,isBig):
    if isBig:
        return f"\n(a{i:02d}{j:02d})*(b{j:02d}{k:02d})*(c{k:02d}{i:02d})"
    return f"\n(a{i}{j})*(b{j}{k})*(c{k}{i})"

def convert_scheme_at_dest(dest_path): # should convert a *.exp to a *.lexp
    try:
        with open(dest_path,'r') as f:
            content = f.read()
        new_content = re.sub(r'(a|b|c)(\d)(\d)',r'\10\20\3',content)
        new_dest_path = dest_path.with_suffix('.lexp')
        with open(new_dest_path,'w') as f:
            f.write(new_content)
        dest_path.unlink()
        return new_dest_path
    except Exception as e:
        with print_lock:
            print(f"ERROR in convert_scheme_at_dest for {dest_path}: {e}")
        return None


def get_text(oldl,oldm,oldn,l,m,n): # computes the text we need to add to all our files
    text = ""
    isBig = l>9 or m>9 or n>9
    for i in range(1,l+1):
            for j in range(1,m+1):
                for k in range(1,n+1):
                    if i>oldl or j>oldm or k>oldn:
                        text+=format_term(i,j,k,isBig)
    return text

print_lock = threading.Lock()

def expand_worker(thread_id, src_files_batch, dest_dir, is_new_large, text): # this is what each thread should do
    with print_lock:
        print(f"Worker {thread_id} starting. Processing {len(src_files_batch)} files.")
    for src_file in src_files_batch:
        try:
            dest_path = dest_dir / src_file.name
            shutil.copy2(src_file, dest_path)
            if is_new_large and dest_path.suffix == '.exp':
                new_path = convert_scheme_at_dest(dest_path)
                if new_path:
                    dest_path = new_path
                else:
                    continue
            if text:
                with open(dest_path,'a') as f:
                    f.write(text)
        except Exception as e:
            with print_lock:
                print(f"Worker {thread_id}: ERROR processing {src_file.name}. {e}")
    with print_lock:
        print(f"Worker {thread_id} finished.")

def main():
    if len(sys.argv) != 8:
        print("Usage: python3 expand.py <oldl> <oldm> <oldn> <l> <m> <n> [num_threads]")
        sys.exit(1)
    oldl = int(sys.argv[1])
    oldm = int(sys.argv[2])
    oldn = int(sys.argv[3])
    l = int(sys.argv[4])
    m = int(sys.argv[5])
    n = int(sys.argv[6])
    num_threads = int(sys.argv[7]) if len(sys.argv) > 7 else 1
    assert num_threads>0
    print(f"Expanding <{oldl},{oldm},{oldn}> to <{l},{m},{n}> using {num_threads} threads...")
    start_time = time.time()
    src_dir = None
    for r in range(oldl*oldm*oldn+2):
        src_dir_check = Path("solutions") / f"{oldl},{oldm},{oldn}" / f"y{r}"
        if src_dir_check.is_dir():
            src_dir = src_dir_check
            print(f"Found source directory at rank {r}: {src_dir}")
            break
    if src_dir is None:
        print("No source directory found")
        sys.exit(1)
    new_rank = l*m*n - oldl*oldm*oldn + r
    dest_dir = Path("solutions") / f"{l},{m},{n}" / f"y{new_rank}"
    print(f"New schemes will be rank {new_rank} in {dest_dir}")
    dest_dir.mkdir(parents=True,exist_ok=True)
    new_terms = get_text(oldl,oldm,oldn,l,m,n)
    is_new_large = (l>9 or m>9 or n>9) and (oldl<10 and oldm<10 and oldn<10)
    all_files = [f for f in src_dir.iterdir() if f.is_file()]
    if not all_files:
        print("No files found in source directory")
        sys.exit(0)
    print(f"Found {len(all_files)} files to process")
    thread_batches = []
    for i in range(num_threads):
        thread_batches.append(all_files[i::num_threads]) # split up the files into the threads roughly evenly
    threads = []
    for i in range(num_threads):
        if not thread_batches[i]:
            print(f"Worker {i} has been assigned no files, skipping")
            continue
        t = threading.Thread(target = expand_worker, args = (i,thread_batches[i],dest_dir,is_new_large,new_terms))
        t.start()
        threads.append(t)
    for t in threads:
        t.join()
    end_time = time.time()
    print(f"\n--- Expanding Complete ---\n\n Processed {len(all_files)} files in {end_time - start_time:.2f} seconds")


if __name__ == "__main__":
    main()
