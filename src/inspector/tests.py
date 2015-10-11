import os
import unittest
import inspector
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


class PerfTest(unittest.TestCase):
    def test_run(self):
        sample_app = os.path.join(TEST_ROOT, "../../test/usage-test")
        perf_cmd = os.getenv("PERF_COMMAND", "perf")
        process = inspector.run([sample_app], perf_cmd=perf_cmd)
        rc = process.wait()
        self.assertEqual(0, rc)

if __name__ == '__main__':
    unittest.main()
