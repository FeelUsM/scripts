#! /bin/bash
datetime=$(date '+%F-%H-%M-%S')

ls -alR --time-style=+%s 2>>.ls-log | runcpp ll2lpath .files .files-log

cat .files '-' .files-log .ls-log >.files-$datetime <<EOF

----------------------------------------
------------ ERROR LOG -----------------
----------------------------------------
EOF

rm .files .files-log .ls-log
