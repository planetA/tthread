# Inspector

- Combines tthread and [Intel Pt](https://software.intel.com/en-us/blogs/2013/09/18/processor-tracing) to build
  a concurrent data provenance graph

## Preqrequisites

- build libtthread (see https://github.com/Mic92/tthread#building-tthread).
- hardware which is capable of Intel PT (processors since the Broadwell
  architecture)
- A linux kernel with the Intel PT PMU driver for perf (was merged in v3.16-rc4) and `perf_event` cgroup:
  - check if `/sys/bus/event_source/devices/intel_pt` exists
  - check if kernel option is activated `zgrep CONFIG_CGROUP_PERF /proc/config.gz`
  - check if perf\_event cgroup is mounted in `/sys/fs/cgroup/perf_event`
- Perf tool with support for Intel PT (merged release candiates of v4.3):
  - check with `perf list | grep intel_pt`
- python3 to run the script

## Usage

To trace system wide and manage cgroups inspector needs **root privileges**

Suppose you have enabled testing with `cmake -DTESTING=ON .`:

```bash
# record
root> ./bin/inspector ../../test/parallel-sum
root> du ./perf.data
1,1M    ./perf.data
1,1M    total
# show and decode log
root> perf script | less
```

```
usage: inspector [-h] [--libtthread-path LIBTTHREAD_PATH]
                 [--perf-command PERF_COMMAND] [--perf-log PERF_LOG]
                 [--set-user SET_USER] [--set-group SET_GROUP]
                 [--no-processor-trace] [--quiet]
                 command [arguments [arguments ...]]

Run program with tthread and intel PT

positional arguments:
  command               command to execute with
  arguments             arguments passed to command

optional arguments:
  -h, --help            show this help message and exit
  --libtthread-path LIBTTHREAD_PATH
                        path to libtthread.so (default: ../../libtthread.so -
                        relative to script path)
  --perf-command PERF_COMMAND
                        Path to perf tool
  --perf-log PERF_LOG   File name to write log
  --set-user SET_USER   Run command as user
  --set-group SET_GROUP
                        Run command as group
  --no-processor-trace  disable processor trace
  --quiet               not output (suitable for scripting)
```
