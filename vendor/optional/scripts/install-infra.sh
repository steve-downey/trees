#!/bin/bash

# scripts/install-infra.sh -*-shell-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Install beman infra subtree

git remote add -f bemanproject-infra https://github.com/bemanproject/infra.git
git fetch bemanproject-infra main
git subtree add -P infra bemanproject-infra main --squash
