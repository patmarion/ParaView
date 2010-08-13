from PartitionUtils import *

import sys

if len(sys.argv) != 4:
    print "Usage: %s <number of procs> <procs per partition> <final partition size>" % sys.argv[0]
    sys.exit(1)


nprocs = int(sys.argv[1])
procs_per_partition = int(sys.argv[2])
final_partition_size = int(sys.argv[3])


p = partition(nprocs, procs_per_partition, final_partition_size)
pretty_print_partitions(p)

