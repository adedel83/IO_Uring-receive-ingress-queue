#!/bin/bash

set -x

# Create namespace
ip netns add A
ip netns add B

# Create links
ip link add veth-A type veth peer name veth-brA

# plug links to namespace
ip link set veth-A netns A
ip link set veth-brA netns B

# set addresses
ip -n A addr add 192.168.15.1/24 dev veth-A
ip -n B addr add 192.168.15.2/24 dev veth-brA

# setting interface up
ip -n B link set veth-brA up
ip -n A link set veth-A up
