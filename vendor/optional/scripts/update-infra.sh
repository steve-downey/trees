#!/bin/bash

# scripts/install-infra.sh -*-shell-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Install beman infra subtree

git fetch bemanproject-infra main
git subtree pull --prefix=infra bemanproject-infra main --squash
