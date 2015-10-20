import sys
import json
import re
import numpy as np
import matplotlib.pyplot as plt
from collections import defaultdict


def to_alphanum(key):
    def convert(text):
        return int(text) if text.isdigit() else text

    def split(s):
        return [convert(c) for c in re.split('([0-9]+)', key(s))]
    return split


def autolabel(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        plt.text(rect.get_x()+rect.get_width()/2.,
                 height * 0.5,
                 '%.2f' % height,
                 ha='center',
                 va='bottom')


def generate_graph(log):
    data = json.load(open(log))
    items = sorted(data.items(),
                   key=to_alphanum(lambda pair: pair[0]),
                   reverse=False)
    benchmark_by_name = defaultdict(list)
    for bench, row in items:
        name, _ = bench.split("-", 1)
        name = name.replace("_", "-")
        benchmark_by_name[name].append(row[1:])

    threads = (2, 4, 8, 16)
    plt.subplots()
    bar_width = 0.20
    opacity = 0.4
    index = np.arange(len(threads))
    patterns = ('-', '+', 'x', '\\', '*', 'o', 'O', '.')
    colors = ('b', 'g', 'r', 'c', 'm', 'y',)
    libraries = ("pthread", "tthread", "pt", "inspector")

    for name, values_by_thread in benchmark_by_name.items():
        pthread_values = [v[0] for v in values_by_thread]
        for _i, library in enumerate(libraries[1:]):
            i = _i + 1
            normalized_values = []
            for v, w in zip(values_by_thread, pthread_values):
                normalized_values.append(v[i] / w)
            rect = plt.bar(index + bar_width * (i + 0.5),
                           normalized_values,
                           bar_width,
                           color=colors[i],
                           alpha=opacity,
                           label=libraries[i],
                           hatch=patterns[i])
            autolabel(rect)
        plt.xlabel('Threads')
        plt.ylabel('Time / Time(pthread)')
        plt.title(name, y=1.02)
        plt.suptitle("Times by threads and libraries")
        plt.xticks(index + bar_width * i, threads)
        plt.legend(loc='best')
        plt.grid()

        plt.savefig("%s.pdf" % name)
        plt.clf()


def die(msg):
    print(msg, file=sys.stderr)
    sys.exit(1)


def main():
    if len(sys.argv) < 2:
        die("USAGE: %s log.json" % sys.argv[0])
    generate_graph(sys.argv[1])

if __name__ == "__main__":
    main()
