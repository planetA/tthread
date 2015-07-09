#!/usr/bin/env python
import tthread
path = "../libtthread.so"
binary = "../../test/usage-test"
process = tthread.run(binary, path)
log = process.wait()
for event in log.read():
    print(event)
