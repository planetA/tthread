import csv
from collections import defaultdict
from tthread import accesslog

class TsvWriter:
    header = ["type", "return_address", "thread_id", "id/address", "heap/global"]
    def __init__(self, log):
        self.log = log
    def write(self, csvfile):
        w = csv.writer(csvfile, delimiter='\t', quoting=csv.QUOTE_MINIMAL)
        w.writerow(self.header)
        for event in self.log.read():
            if type(event) is accesslog.ThunkEvent:
                self._write_thunk(w, event)
            else:
                self._write_access(w, event)
    def _write_thunk(self, writer, event):
        writer.writerow(("thunk",
                event.return_address,
                event.thread_id,
                event.id,
                "-"))
    def _write_access(self, writer, event):
        if type(event) is accesslog.WriteEvent:
            t = "write"
        elif type(event) is accesslog.ReadEvent:
            t = "read"
        else:
            return
        if self.log.is_heap(event.address):
            space = "heap"
        elif self.log.is_global(event.address):
            space = "global"
        else:
            space = "mmap"
        writer.writerow((t,
                event.return_address,
                event.thread_id,
                event.address,
                space))

class Tsv2Writer:
    header = ["access", "return_address", "thread_id", "thunk_id", "thunk_return_address", "heap/global"]
    def __init__(self, log):
        self.log = log
    def write(self, csvfile):
        w = csv.writer(csvfile, delimiter='\t', quoting=csv.QUOTE_MINIMAL)
        w.writerow(self.header)
        threads = {}
        for event in self.log.read():
            if type(event) is accesslog.ThunkEvent:
                threads[event.thread_id] = (event.id, event.return_address)
            else:
                self._write_access(w, event, threads)
    def _write_access(self, writer, event, threads):
        if type(event) is accesslog.WriteEvent:
            a = "write"
        elif type(event) is accesslog.ReadEvent:
            a = "read"
        else:
            return
        thunk = threads.get(event.thread_id, (0, "_start"))
        if self.log.is_heap(event.address):
            space = "heap"
        elif self.log.is_global(event.address):
            space = "global"
        else:
            space = "unknown"
        writer.writerow((a, event.return_address, event.thread_id, thunk[0], thunk[1], space))

#class JsonWriter:
#    def __init__(self, log):
#        self.log = log
#    def write(self, jsonfile):
#        threads = defaultdict(list)
#        for event in log.read():
#            if type(event) is accesslog.ThunkEvent:
#                threads[event.thread_id].append([])
#            else:
#                self._add_access(threads, log, event)
#        data = { "threads": threads }
#        json.dumps(data, indent=4)
#    def _add_access(self, threads, log, event):
#        if event is accesslog.WriteEvent:
#            a = "write"
#        elif event is accesslog.ReadEvent:
#            a = "read"
#        if log.is_heap(event):
#            space = "heap"
#        elif log.is_global(event):
#            space = "global"
#        else:
#            space = "unknown"
#        obj = {
#                a, event.return_address, event.thread_id, thunk_id, space
#              }
#        threads[event.thread_id][-1].append()
