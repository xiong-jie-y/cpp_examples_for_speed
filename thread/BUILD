cc_binary(
    name = "mutex_example",
    srcs = ["mutex_example.cc"],
    linkopts = ["-lpthread"],
)

cc_binary(
    name = "condvar_example",
    srcs = ["condvar_example.cc"],
    linkopts = ["-lpthread"],
)

cc_binary(
    name = "rwlock_barrier_example",
    srcs = ["rwlock_barrier_example.cc"],
    copts = ["-std=c++17"],
    linkopts = ["-lpthread", "-lboost_thread"],
)