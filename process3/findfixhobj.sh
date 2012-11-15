#!/bin/bash

set -u
set -e

find . -name "*.class" -exec fixhobj.sh '{}' \;
