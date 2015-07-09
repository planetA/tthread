#!/usr/bin/env python3

import os, sys
import unittest
import tthread

TEST_ROOT = os.path.realpath(os.path.dirname(__file__))

class TthreadTest(unittest.TestCase):
    def test_accesslog(self):
        test_binary = os.path.join(TEST_ROOT, "../../test/usage-test")
        if not os.path.isfile(test_binary):
            self.fail("test binary '%s' does not exists, please run 'make' first" % test_binary)
        path = tthread.default_library_path()
        if not os.path.isfile(path):
            self.fail("'%s' does not exists, please run 'make' first" % path)
        process = tthread.run(test_binary, path)
        log = process.wait()
        self.assertEqual(log.return_code, 0)
        events = list(log.read())
        self.assertEqual(len(events), 1)
        ev = events[0]
        self.assertIsInstance(ev, tthread.log.WriteEvent)
        self.assertGreater(ev.return_address, 0)
        self.assertGreater(ev.address, 0)
        self.assertGreater(ev.thread_id, 0)
        log.close()

if __name__ == '__main__':
    unittest.main()
