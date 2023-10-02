from sys import argv
import os.path as path
import os
import argparse
import math


DEFAULT_EXTS = [".h", ".hpp", ".c", ".cc", ".cpp", ".inl"]

def lpad(s: str, w: int) -> str:
    if len(s) >= w:
        return s
    else:
        return str(" " * (w - len(s))) + s

def rpad(s: str, w: int) -> str:
    if len(s) >= w:
        return s
    else:
        return s + str(" " * (w - len(s)))
    
def cpad(s: str, w: int) -> str:
    if len(s) >= w:
        return s
    else:
        padl = int((w - len(s)) / 2)
        padr = (w - len(s)) - padl
        return padl * " " + s + padr * " "

def bucket_label(b):
        low = 0 if len(b) == 0 else min(b)
        high = 0 if len(b) == 0 else max(b)
        s = f"[{low}, {high}]" #  ({len(b)})
        return s

def how_many_buckets(n):
    return int(math.log(n, 2))

def hist(vals, lo, hi, title):
    if (len(vals) == 0):
        print("No data to plot")
        return
    
    buckets = []
    for i in range(how_many_buckets(len(vals)) + 2): buckets.append([]) # 5 + 2 overflow
    for c in vals:#range(1, 6):
        if c > hi:
            buckets[len(buckets) - 1].append(c)
        elif c < lo:
            buckets[0].append(c)
        else:
            idx = 1 + int((len(buckets) - 2) * ((c - lo) / (hi - lo)))
            buckets[idx].append(c)
    n = sum([len(b) for b in buckets])

    max_h = 16
    #bucket_sums = list(map(lambda b: 0 if len(b) == 0 else sum(b), buckets))
    bar_labels = list(map(lambda b: bucket_label(b), buckets))
    bar_labels2 = list(map(lambda b: f"({len(b)})", buckets))
    bar_labels3 = list(map(lambda b: f"{len(b)/n * 100:.1f}%", buckets))
    bar_printw = max(max(map(len, bar_labels)), max(map(len, bar_labels2)), max(map(len, bar_labels3)))
    max_bucket = max([len(b) for b in buckets])

    print(cpad(title, len(buckets) * (bar_printw + 1)))
    for i in range(max_h):
        l = max_h - i
        for b in buckets:
            h = int(max_h * (len(b) / max_bucket))
            bar = ""
            if h >= l:
                bar = "*" * bar_printw
            else:
                bar = " " * bar_printw
            bar += " "
            print(bar, end='')
        print()

    for bl in bar_labels:
        s = cpad(bl, bar_printw) + " "
        print(s, end='')
    print()
    for bl in bar_labels2:
        s = cpad(bl, bar_printw) + " "
        print(s, end='')
    print()
    for bl in bar_labels3:
        s = cpad(bl, bar_printw) + " "
        print(s, end='')
    print()

class LineCount:
    def __init__(self, dir) -> None:
        self.total = 0
        self.counts = []
        self.dir = dir
    
    def push_count(self, count):
        self.total += count
        self.counts.append(count)

    def count(self):
        return len(self.counts)

    def sum(self):
        return self.total

    def mean(self):
        return 0 if len(self.counts) == 0 else self.total / len(self.counts)
    
    def stddev(self):
        if len(self.counts) == 0:
            return 0
        avg = self.mean()
        sumsqr = sum(map(lambda x: (x - avg) ** 2, self.counts))
        var = sumsqr / len(self.counts)
        return math.sqrt(var)
    
    def min(self):
        return 0 if len(self.counts) == 0 else min(self.counts)
    
    def max(self):
        return 0 if len(self.counts) == 0 else max(self.counts)
    
    def percentile(self, pct):
        if (len(self.counts) == 0):
            return 0
        idx = min(len(self.counts) - 1, int(pct * len(self.counts)))
        return sorted(self.counts)[idx]
    
    def hist(self):
        lo = self.percentile(0.03)
        hi = self.percentile(0.97)
        return hist(self.counts, lo, hi, f"Line Count Histogram for {self.dir}")
        

def count_file(args, fname, lc: LineCount):
    if (not fname):
        raise RuntimeError("No file given!")
    with open(fname, 'r') as file:
        contents = file.readlines()
        nlines = len(contents)
        print(f"{fname} has {nlines} lines")
        lc.push_count(nlines)


def main(argv: "list[str]") -> int:
    parser = argparse.ArgumentParser(description="Count the number of lines in a project so I can feel good about myself")
    parser.add_argument("-d", "--dir", help="add for all folders in directory (not compatible with custom name)", default=".")
    parser.add_argument("-e", "--ext", help="supply extensions", nargs="+", default=DEFAULT_EXTS)
    args = parser.parse_args(argv[1:])
    
    if args.dir:
        lc = LineCount(args.dir)
        for (root, dirs, files) in os.walk(args.dir):
            for fname in files:
                ext = os.path.splitext(fname)[1]
                if ext in args.ext:
                    count_file(args, os.path.join(root, fname), lc)
        print(f"Total: {lc.sum()} in {lc.count()} files")
        print(f"AVG: {lc.mean():.1f}")
        print(f"SD: {lc.stddev():.1f}")
        print(f"IQR: {lc.percentile(.75) - lc.percentile(.25):.1f}")
        print(f"25th: {lc.percentile(.25):.1f}")
        print(f"50th: {lc.percentile(.50):.1f}")
        print(f"75th: {lc.percentile(.75):.1f}")
        print(f"Min: {lc.min()}")
        print(f"Max: {lc.max()}")
        lc.hist()
    else:
        print("No directory given")

if __name__ == '__main__':
    exit(main(argv))