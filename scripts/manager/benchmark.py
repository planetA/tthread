import os
from .constants import BM_DATA, BM_APPS, BM_PARSEC

def ds_wrap(path):
    return os.path.join(BM_DATA, path)

def parsec_wrap(path):
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
        },
        'prepare' : [
            ('app', '1000 1000 1'),
            ('app', '2000 2000 1')
        ]
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
    },
    # 'reverse_index' : {
    #     'dataset' : {
    #         'test' : ds_wrap('reverse_index_datafiles'),
    #         'real' : ds_wrap('reverse_index_datafiles')
    #     }
    # }

    #
    # PARSEC
    'canneal' : {
        'dataset' : {
            'test' : '$NPROCS 15000 2000 ' + ds_wrap('canneal_datafiles/400000.nets') + ' 128'
        },
    },
    'blackscholes' : {
        'dataset' : {
            'test' : '$NPROCS ' + ds_wrap('blackscholes_datafiles/in_64K.txt') + \
                       ' ' + ds_wrap('blackscholes_datafiles/in_64K.txt')
        }
    },
    'dedup' : {
        'dataset' : {
            'test' : '-c -p -v -t $NPROCS  -i %s -o output.dat.ddp' % ds_wrap('dedup_datafiles/media.dat')
        }
    },
    'streamcluster' : {
        'dataset' : {
            'test' : '10 20 128 16384 16384 1000 none %s $NPROCS' + ds_wrap('streamcluster_datafiles/output.txt')
        }
    },
    'swaptions' : {
        'dataset' : {
            'test' : '-ns 64 -sm 40000 -nt $NPROCS'
        }
    }
}
