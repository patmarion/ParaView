import math


def pretty_print_partitions(partition_list):

    for partitioning in partition_list:
        id_strs = []
        for ids in partitioning:
            ids = [str(i) for i in ids]
            id_strs.append(",".join(ids))
        print " | ".join(id_strs)
        print ""


def compute_partition_sizes(number_of_procs, number_of_partitions):
    """Given a number of procs and a number of partitions, computes
    how many procs should be placed in each partition.  For example,
    given 5 procs and 3 partitiona, return [2, 2, 1]"""
    if number_of_partitions > number_of_procs:
        raise Exception("cannot split, more partitions than procs")

    partition_sizes = [0 for i in xrange(number_of_partitions)]
    for i in xrange(number_of_procs):
        partition_id = i % number_of_partitions
        partition_sizes[partition_id] += 1

    return partition_sizes


def assign_partition_ids(partition_sizes, proc_ids):
    """Give a list of partition sizes like [2, 2, 3] and a list of ids
    like [0, 1, 2, 3, 4, 5, 6, 7], partitions the ids into the given sizes
    and returns the result like [[0, 1], [2, 3], [4, 5, 6]]"""

    # sanity check
    if sum(partition_sizes) > len(proc_ids):
        raise Exception("Not enough proc ids to assign to partitions.")

    index_offset = 0
    partition_ids = []

    for partition_size in partition_sizes:

        id_list = [proc_ids[index_offset + i] for i in xrange(partition_size)]
        partition_ids.append(id_list)
        index_offset += partition_size

    return partition_ids


def compute_partitioning(proc_ids, number_of_partitions):
    """Give a list of ids like [0, 1, 2, 3, 4] and a number of partitions,
    returns the partitioned ids like [[0, 1, 2], [3, 4]] for number_of_partitions=2.
    See compute_partition_sizes and assign_partition_ids."""
    partition_sizes = compute_partition_sizes(len(proc_ids), number_of_partitions)
    return assign_partition_ids(partition_sizes, proc_ids)


def get_partition_roots(partitioning):
    """ Given a list like [[1,2,3], [4,5,6]] returns [1, 4]"""
    root_ids = [partition_ids[0] for partition_ids in partitioning]
    return root_ids


def compute_number_of_partitions(number_of_procs, procs_per_partition):
    return int(math.ceil(float(number_of_procs)/procs_per_partition))


def partition_helper(proc_ids, procs_per_partition, final_partition_size):

    if len(proc_ids) <= final_partition_size:
        return [[proc_ids]]

    number_of_partitions = compute_number_of_partitions(len(proc_ids), procs_per_partition)

    if number_of_partitions < final_partition_size:
        number_of_partitions = final_partition_size

    partitioning = compute_partitioning(proc_ids, number_of_partitions)
    proc_ids = get_partition_roots(partitioning)

    if len(proc_ids) > procs_per_partition:
        return [partitioning] + partition_helper(proc_ids,
                                  procs_per_partition, final_partition_size)

    elif len(proc_ids) > final_partition_size:
        final_partitioning = compute_partitioning(proc_ids, final_partition_size)
        return [partitioning, final_partitioning, [get_partition_roots(final_partitioning)]]

    else:
        return [partitioning, [get_partition_roots(partitioning)]]


def partition_ids(proc_ids, procs_per_partition, final_partition_size):

    partition_list = partition_helper(proc_ids, procs_per_partition, final_partition_size)

    final_partition = partition_list[-1]

    if len(final_partition) != 1:
        raise Exception("There should be only 1 final partition")

    final_partition_ids = final_partition[0]

    if len(final_partition_ids) != final_partition_size:
        raise Exception("The last partitioning has %d partitions which is more"
                        "than the requested final partition size %d" % (len(results[-1]),
                                                                    final_partition_size))

    return partition_list


def partition_id_sets(id_sets, final_partition_size):

    # This is not necessarily an error, but for some applications we
    # might want to flag it as an error or warning.
    #if final_partition_size > len(id_sets):
    #    raise Exception("Final partition size is greater than the number of id sets.")

    proc_ids = list()
    procs_per_partition = len(id_sets[0])
    for ids in id_sets:
        if len(ids) != procs_per_partition:
            raise Exception("Not all id lists have the same length.")
        proc_ids += ids
    return partition_ids(proc_ids, procs_per_partition, final_partition_size)


def partition(number_of_procs, procs_per_partition, final_partition_size):

    proc_ids = range(number_of_procs)
    return partition_ids(proc_ids, procs_per_partition, final_partition_size)
