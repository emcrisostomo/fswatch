# Stress Test Backlog

These tests are intentionally not part of the default test suite yet.  They
need careful runtime limits and environment checks because fanotify availability
depends on kernel, filesystem, and privilege support.

## fanotify: huge recursive trees

Exercise `fswatch -m fanotify_monitor -r` against a large pre-existing tree and
verify that startup marking, event delivery, and stop latency remain bounded.

Suggested coverage:

- create a configurable directory tree, for example 5,000 to 50,000
  directories, below a temporary root
- start `fswatch` with a high latency value and confirm SIGTERM exits promptly
  through the eventfd wake path
- create, modify, rename, and delete files at shallow, middle, and deep levels
- create new nested directories after the monitor starts and verify synthetic
  child creation events still appear
- record elapsed startup time and total marks attempted so regressions can be
  compared across runs

## fanotify: high event-rate workloads

Exercise sustained bursts against an already marked directory tree.

Suggested coverage:

- create many files in parallel below one watched directory and below many
  watched subdirectories
- mix create, append, rename, and delete operations
- compare emitted event categories against expected lower bounds rather than
  exact counts, because kernel event coalescing and scheduling can vary
- verify the process exits promptly while events are still being generated
- run with and without `--access` to check that access/open/close-nowrite
  traffic remains opt-in

## fanotify: queue overflow

Deliberately stress the kernel queue and assert the monitor reports overflow
through the existing overflow path.

Suggested coverage:

- use a constrained test environment where the fanotify queue can realistically
  overflow without consuming excessive host resources
- run once with default overflow handling and expect the monitor to fail with an
  overflow error
- run once with `--allow-overflow` and expect an `Overflow` event
- keep this test opt-in unless it can be made deterministic and cheap
