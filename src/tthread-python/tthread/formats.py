import csv
from tthread import accesslog


class TsvWriter:
    header = [
        "type",
        "return_address",
        "thread_id",
        "id/address",
        "heap/global"
    ]

    def __init__(self, log):
        self.log = log

    def write(self, csvfile):
        w = csv.writer(csvfile, delimiter='\t', quoting=csv.QUOTE_MINIMAL)
        w.writerow(self.header)
        for event in self.log.read():
            if type(event) is accesslog.ThunkEvent:
                self._write_thunk(w, event)
            elif type(event) is accesslog.FinishEvent:
                self._write_finish(w, event)
            else:
                self._write_access(w, event)

    def _write_finish(self, writer, event):
        writer.writerow(("finish",
                        event.return_address,
                        event.thread_id,
                        "-", "-"))

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
    header = [
        "access",
        "return_address",
        "thread_id",
        "thunk_id",
        "thunk_return_address",
        "heap/global"
    ]

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
            space = "mmap"
        writer.writerow((a,
                        event.return_address,
                        event.thread_id,
                        thunk[0],
                        thunk[1],
                        space))
