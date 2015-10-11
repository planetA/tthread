import os
import unittest
from inspector import cgroups

TEST_ROOT = os.path.realpath(os.path.dirname(__file__))


class CgroupTest(unittest.TestCase):
    def test_create(self):
        with cgroups.PerfEvent("inspector-test") as c:
            pid = str(os.getpid())
            c.addPids(pid)
            f = open(os.path.join(c.mountpoint, "tasks"))
            line = f.readline()
            f.close()
            self.assertEqual(pid + "\n", line)

if __name__ == '__main__':
    unittest.main()
