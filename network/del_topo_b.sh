#!/bin/bash 

set -x

ip link del veth-A

ip -all netns del
