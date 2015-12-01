import os
from .constants import BM_DATA

def ds_wrap(path):
    return os.path.join(BM_DATA, path)
# class Benchmark:
#     def __init__(self, name, datasets):
#         self.name = name
#         self.datasets = datasets

# class BenchmarkDB:
#     def __init__(self):
#         self.benchmarks = dict()

benchmarks = {
    'word_count' : {
        'dataset' : {
            'test' : ds_wrap('word_count_datafiles/word_10MB.txt'),
            'real' : ds_wrap('word_count_datafiles/word_100MB.txt')
        }
    },
    'kmeans' : {
        'dataset' : {
            'test' : '-d 3 -c 1000 -p 100000 -s 1000',
            'real' : '-d 3 -c 1000 -p 100000 -s 1000'
        }
    },
    'histogram' : {
        'dataset' : {
            'test' : ds_wrap('histogram_datafiles/small.bmp'),
            'real' : ds_wrap('histogram_datafiles/large.bmp')
        }
    },
    'linear_regression' : {
        'dataset' : {
            'test' : ds_wrap('linear_regression_datafiles/key_file_50MB.txt'),
            'real' : ds_wrap('linear_regression_datafiles/key_file_500MB.txt')
        }
    },
    'matrix_multiply' : {
        'dataset' : {
            'test' : '1000 1000',
            'real' : '2000 2000'
        }
    },
    'pca' : {
        'dataset' : {
            'test' : '-r 4000 -c 4000 -s 100',
            'real' : '-r 4000 -c 4000 -s 100'
        }
    },
    'string_match' : {
        'dataset' : {
            'test' : ds_wrap('string_match_datafiles/key_file_50MB.txt'),
            'real' : ds_wrap('string_match_datafiles/key_file_500MB.txt')
        }
    }
}
