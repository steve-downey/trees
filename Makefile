# Makefile                                                       -*-makefile-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

export
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

NO_COLOR:=1

INSTALL_PREFIX ?= .install/
BUILD_DIR ?= .build
DEST ?= $(INSTALL_PREFIX)
CMAKE_FLAGS ?=

PYEXECPATH ?= $(shell which python3.13 || which python3.12 || which python3.11 || which python3.10 || which python3.9 || which python3.8 || which python3)
PYTHON ?= $(notdir $(PYEXECPATH))
VENV := .venv
UV := $(shell command -v uv 2> /dev/null)
ACTIVATE := $(UV) run
PYEXEC := $(UV) run python
MARKER = .initialized.venv.stamp

PRE_COMMIT := $(UV) run pre-commit
CLANG_FORMAT_BATCH_SIZE ?= 25
LOCAL_LINT_BATCH_SIZE ?= 25
LOCAL_TOOLS_DIR ?= .tools
LOCAL_BIN_DIR := $(LOCAL_TOOLS_DIR)/bin
LOCAL_NODE_BIN_DIR := $(LOCAL_TOOLS_DIR)/node_modules/.bin
LOCAL_MARKDOWNLINT := $(LOCAL_NODE_BIN_DIR)/markdownlint
LOCAL_CHECKMAKE := $(LOCAL_BIN_DIR)/checkmake
LOCAL_GITLEAKS := $(LOCAL_BIN_DIR)/gitleaks
MARKDOWNLINT_LOCAL_BASE_ARGS := --disable MD013 --
MARKDOWNLINT_GH_DISABLE := MD007 MD009 MD010 MD012 MD022 MD030 MD031 MD032 MD040
MARKDOWNLINT_GH_ARGS := --disable MD013 $(MARKDOWNLINT_GH_DISABLE) --
NPM ?= $(shell command -v npm 2> /dev/null)
GO ?= $(shell command -v go 2> /dev/null)

EMACS := $(shell command -v emacs 2> /dev/null)

.update-submodules:
	#git submodule update --init --recursive
	touch .update-submodules

.gitmodules: .update-submodules

CONFIG ?= Asan

export

TOOLCHAIN ?= clang

ifeq ($(strip $(TOOLCHAIN)),system)
	_build_name?=build-system/
	_build_dir?=.build/
	_local_toolchain?=$(CURDIR)/etc/toolchain.cmake
else
	_build_name?=build-$(TOOLCHAIN)
	_build_dir?=.build/
	_local_toolchain?=$(CURDIR)/etc/$(TOOLCHAIN)-toolchain.cmake
endif

_configuration_types ?= "RelWithDebInfo;Debug;Tsan;Asan;Gcov"

_build_path ?= $(_build_dir)/$(_build_name)
_build_path := $(subst //,/,$(_build_path))
_build_path := $(patsubst %/,%,$(_build_path))

VCPKG ?= $(shell command -v vcpkg 2> /dev/null)

ifeq ($(VCPKG),)
	_cmake_top_level?="infra/cmake/use-fetch-content.cmake"
	_toolchain:=$(_local_toolchain)
	_args=-DBEMANINFRA_googletest_REPO=file:///home/sdowney/bld/googletest/googletest.git
else
	_vcpkg_toolchain:=$(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
	_cmake_top_level?=$(_vcpkg_toolchain)
	export PROJECT_VCPKG_TOOLCHAIN=$(_local_toolchain)
	_toolchain:=$(_local_toolchain)
	_args=-DVCPKG_OVERLAY_TRIPLETS=$(CURDIR)/cmake -DVCPKG_TARGET_TRIPLET=x64-linux-custom
	# for debugging add 	-DVCPKG_INSTALL_OPTIONS="--debug"
endif

CMAKE ?= $(UV) run cmake
CTEST ?= $(UV) run ctest

define run_cmake =
	$(CMAKE) \
	-G "Ninja Multi-Config" \
	-DCMAKE_CONFIGURATION_TYPES=$(_configuration_types) \
	-DCMAKE_INSTALL_PREFIX=$(abspath $(INSTALL_PREFIX)) \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
	-DCMAKE_PREFIX_PATH=$(CURDIR)/infra/cmake \
	-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=$(_cmake_top_level) \
	-DCMAKE_TOOLCHAIN_FILE=$(_toolchain) \
	-DOPTIONAL_INSTALL_DIR=~/.local/lib/cmake/ \
	$(_args) \
	$(_cmake_args) \
	$(CURDIR)
endef

default: test
.PHONY: default

$(_build_path):
	mkdir -p $(_build_path)

$(_build_path)/CMakeCache.txt: | $(_build_path) .gitmodules $(VENV)
	cd $(_build_path) && $(run_cmake)

$(_build_path)/compile_commands.json: $(_build_path)/CMakeCache.txt

.PHONY: compile_commands.json
compile_commands.json: $(_build_path)/compile_commands.json
compile_commands.json: ## symlink the current compile commands db
	if [ "$(shell readlink compile_commands.json)" != "$(_build_path)/compile_commands.json" ] ; then \
		ln -sf $(_build_path)/compile_commands.json ; \
	fi

.PHONY: compile
compile: $(_build_path)/CMakeCache.txt compile_commands.json
compile: ## Compile the project
	@$(CMAKE) --build $(_build_path)  --config $(CONFIG) --target all -- -k 0

.PHONY: compile-headers
compile-headers: $(_build_path)/CMakeCache.txt ## Compile the headers
	 @$(CMAKE) --build $(_build_path)  --config $(CONFIG) --target all_verify_interface_header_sets -- -k 0

.PHONY: install
install: $(_build_path)/CMakeCache.txt compile ## Install the project
	@$(CMAKE) --install $(_build_path) --config $(CONFIG) --component example.name_Development --verbose

.PHONY: clean-install
clean-install:
	-rm -rf .install

.PHONY: realclean
realclean: clean-install

.PHONY: ctest
ctest: $(_build_path)/CMakeCache.txt ## Run CTest on current build
	@$(CTEST) --test-dir $(_build_path) --output-on-failure -C $(CONFIG)

.PHONY: ctest_
ctest_: compile
	@$(CTEST) --test-dir $(_build_path) --output-on-failure -C $(CONFIG)

.PHONY: test
test: ctest_ ## Rebuild and run tests

.PHONY: cmake
cmake: |  $(_build_path)
	cd $(_build_path) && ${run_cmake}

.PHONY: clean
clean: $(_build_path)/CMakeCache.txt ## Clean the build artifacts
	@$(CMAKE) --build $(_build_path)  --config $(CONFIG) --target clean

.PHONY: realclean
realclean: ## Delete the build directory
	rm -rf $(_build_path)

.PHONY: env
env:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))

.PHONY: papers
papers:
	$(MAKE) -C papers/P2988 papers

.PHONY: all
all: compile

.PHONY: venv
venv: ## Create python virtual env
venv: $(VENV)/$(MARKER)

.PHONY: clean-venv
clean-venv: ## Delete python virtual env
	-rm -rf $(VENV)

realclean: clean-venv

.PHONY: show-venv
show-venv: venv
show-venv: ## Debugging target - show venv details
	$(PYEXEC) -c "import sys; print('Python ' + sys.version.replace('\n',''))"
	@echo venv: $(VENV)

uv.lock: pyproject.toml
	$(UV) lock

$(VENV):
	$(UV) venv --python $(PYTHON)

$(VENV)/$(MARKER): uv.lock | $(VENV)
	$(UV) sync
	touch $(VENV)/$(MARKER)

.PHONY: dev-shell
dev-shell: venv
dev-shell: ## Shell with the venv activated
	$(ACTIVATE) $(notdir $(SHELL))

.PHONY: bash zsh
bash zsh: venv
bash zsh: ## Run bash or zsh with the venv activated
	$(ACTIVATE) $@

.PHONY: lint
lint: venv
lint: ## Run all configured tools in pre-commit
	SKIP=clang-format $(PRE_COMMIT) run -a
	$(MAKE) clang-format-all

.PHONY: clang-format-all
clang-format-all: venv
clang-format-all: ## Run clang-format in batches to avoid exhausting memory
	find . \
		\( -path './.build' -o -path './.build/*' \
		   -o -path './.venv' -o -path './.venv/*' \
		   -o -path './vendor' -o -path './vendor/*' \
		   -o -path './infra' -o -path './infra/*' \) -prune -o \
		-type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' \
		   -o -name '*.h' -o -name '*.hh' -o -name '*.hpp' -o -name '*.hxx' \
		   -o -name '*.ipp' \) \
		-print0 \
	| xargs -0 -r -n $(CLANG_FORMAT_BATCH_SIZE) $(PRE_COMMIT) run clang-format --files

.PHONY: lint-manual
lint-manual: venv
lint-manual: ## Run all manual tools in pre-commit
	$(PRE_COMMIT) run --hook-stage manual -a

$(LOCAL_TOOLS_DIR):
	mkdir -p $(LOCAL_TOOLS_DIR)

$(LOCAL_BIN_DIR): | $(LOCAL_TOOLS_DIR)
	mkdir -p $(LOCAL_BIN_DIR)

$(LOCAL_MARKDOWNLINT): | $(LOCAL_TOOLS_DIR)
	$(NPM) install --no-save --prefix $(LOCAL_TOOLS_DIR) markdownlint-cli@0.43.0

$(LOCAL_CHECKMAKE): | $(LOCAL_BIN_DIR)
	GOBIN=$(abspath $(LOCAL_BIN_DIR)) $(GO) install github.com/mrtazz/checkmake/cmd/checkmake@v0.2.2

$(LOCAL_GITLEAKS): | $(LOCAL_BIN_DIR)
	GOBIN=$(abspath $(LOCAL_BIN_DIR)) $(GO) install github.com/zricethezav/gitleaks/v8@v8.16.3

.PHONY: local-lint-tools
local-lint-tools: $(LOCAL_MARKDOWNLINT) $(LOCAL_CHECKMAKE) $(LOCAL_GITLEAKS)
local-lint-tools: ## Install non-Python local lint tools into the project tree

.PHONY: markdownlint-local
markdownlint-local: local-lint-tools
markdownlint-local: ## Run markdownlint on top-level hand-maintained docs
	find . -maxdepth 1 -type f -name '*.md' \
		! -name 'foldable-applicative-traversable.md' \
		-print0 \
	| xargs -0 -r $(LOCAL_MARKDOWNLINT) $(MARKDOWNLINT_LOCAL_BASE_ARGS)
	find ./docs -maxdepth 1 -type f -name '*.md' \
		! -name 'index.md' \
		! -name 'codestyle.md' \
		! -name 'live-src-main.md' \
		! -name 'live-src-main-built-targets.md' \
		-print0 \
	| xargs -0 -r $(LOCAL_MARKDOWNLINT) $(MARKDOWNLINT_LOCAL_BASE_ARGS)

.PHONY: markdownlint-gh
markdownlint-gh: local-lint-tools
markdownlint-gh: ## Run markdownlint with rules that matter for GitHub rendering
	find . -maxdepth 1 -type f -name '*.md' \
		! -name 'foldable-applicative-traversable.md' \
		-print0 \
	| xargs -0 -r $(LOCAL_MARKDOWNLINT) $(MARKDOWNLINT_GH_ARGS)
	find ./docs -maxdepth 1 -type f -name '*.md' \
		! -name 'index.md' \
		! -name 'codestyle.md' \
		! -name 'live-src-main.md' \
		! -name 'live-src-main-built-targets.md' \
		-print0 \
	| xargs -0 -r $(LOCAL_MARKDOWNLINT) $(MARKDOWNLINT_GH_ARGS)

.PHONY: lint-local
lint-local: venv local-lint-tools
lint-local: ## Run local lint tools without pre-commit environment downloads
	$(MAKE) markdownlint-gh
	$(UV) run codespell -I .codespell_ignore --uri-ignore-words-list '*' \
		-S './vendor,./infra,./etc,./docs/reference,./docs/haskell,./docs/index.md,./docs/codestyle.md,./docs/live-src-main.md,./docs/live-src-main-built-targets.md,./docs/doxygen-awesome.css,./docs/doxygen-awesome-darkmode-toggle.js,./foldable-applicative-traversable.html,./foldable-applicative-traversable-slides.html,./foldable-applicative-traversable.md'
	find . \
		\( -path './.build' -o -path './.build/*' \
		   -o -path './.venv' -o -path './.venv/*' \
		   -o -path './.tools' -o -path './.tools/*' \
		   -o -path './vendor' -o -path './vendor/*' \
		   -o -path './infra' -o -path './infra/*' \) -prune -o \
		-type f \( -name 'CMakeLists.txt' -o -name 'CMakeLists.txt.in' -o -name '*.cmake' -o -name '*.cmake.in' \) \
		-print0 \
	| xargs -0 -r -n $(LOCAL_LINT_BATCH_SIZE) $(UV) run gersemi -i
	find . \
		\( -path './.build' -o -path './.build/*' \
		   -o -path './.venv' -o -path './.venv/*' \
		   -o -path './.tools' -o -path './.tools/*' \
		   -o -path './vendor' -o -path './vendor/*' \
		   -o -path './infra' -o -path './infra/*' \) -prune -o \
		-type f \( -name 'Makefile' -o -name '*.mk' \) \
		-print0 \
	| xargs -0 -r $(UV) run mbake validate
	find . \
		\( -path './.build' -o -path './.build/*' \
		   -o -path './.venv' -o -path './.venv/*' \
		   -o -path './.tools' -o -path './.tools/*' \
		   -o -path './vendor' -o -path './vendor/*' \
		   -o -path './infra' -o -path './infra/*' \) -prune -o \
		-type f \( -name 'Makefile' -o -name '*.mk' \) \
		-print0 \
	| xargs -0 -r $(LOCAL_CHECKMAKE)
	find . \
		\( -path './.build' -o -path './.build/*' \
		   -o -path './.venv' -o -path './.venv/*' \
		   -o -path './.tools' -o -path './.tools/*' \
		   -o -path './vendor' -o -path './vendor/*' \
		   -o -path './infra' -o -path './infra/*' \) -prune -o \
		-type f -name '*.sh' \
		-print0 \
	| xargs -0 -r $(UV) run shellcheck
	$(LOCAL_GITLEAKS) detect --no-git --no-banner --redact --source .
	find . \
		\( -path './.build' -o -path './.build/*' \
		   -o -path './.venv' -o -path './.venv/*' \
		   -o -path './.tools' -o -path './.tools/*' \
		   -o -path './vendor' -o -path './vendor/*' \
		   -o -path './infra' -o -path './infra/*' \) -prune -o \
		-type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' \
		   -o -name '*.h' -o -name '*.hh' -o -name '*.hpp' -o -name '*.hxx' \
		   -o -name '*.ipp' \) \
		-print0 \
	| xargs -0 -r -n $(CLANG_FORMAT_BATCH_SIZE) $(UV) run clang-format -i

.PHONY: coverage
coverage: ## Build and run the tests with the GCOV profile and process the results
coverage: venv $(_build_path)/CMakeCache.txt
	$(CMAKE) --build $(_build_path) --config Gcov
	$(ACTIVATE) ctest --build-config Gcov --output-on-failure --test-dir $(_build_path)
	$(CMAKE) --build $(_build_path) --config Gcov --target process_coverage

.PHONY: view-coverage
view-coverage: ## View the coverage report
	sensible-browser $(_build_path)/coverage/coverage.html

.PHONY: docs
docs: ## Build the docs with Doxygen
	doxygen docs/Doxyfile

.PHONY: docs-live-src
docs-live-src: ## Regenerate docs/live-src-main.md from HEAD (excluding deadcode/conceptmap)
	@set -eu; \
	repo_root="$$(git rev-parse --show-toplevel)"; \
	out="$$repo_root/docs/live-src-main.md"; \
	commit="$$(git rev-parse --short HEAD)"; \
	{ \
		printf -- '---\n'; \
		printf 'title: Live Source Snapshot (main)\n'; \
		printf 'summary: Point-in-time fenced dump of live src C++ sources from main, excluding deadcode and smd/conceptmap.\n'; \
		printf 'source_of_truth: git HEAD on branch main\n'; \
		printf 'scope:\n'; \
		printf '  include:\n'; \
		printf '    - src/**/*.hpp\n'; \
		printf '    - src/**/*.h\n'; \
		printf '    - src/**/*.cpp\n'; \
		printf '  exclude:\n'; \
		printf '    - src/deadcode/**\n'; \
		printf '    - src/smd/conceptmap/**\n'; \
		printf '    - src/**/CMakeLists.txt\n'; \
		printf 'update_policy:\n'; \
		printf '  when_to_update:\n'; \
		printf '    - Any time live files under src are added, removed, renamed, or materially changed on main.\n'; \
		printf '    - Before using this file as a review/reference baseline.\n'; \
		printf '  how_to_update:\n'; \
		printf '    - Regenerate from main using the command block in the "Regeneration" section below.\n'; \
		printf '    - Replace this file atomically with regenerated output.\n'; \
		printf 'notes:\n'; \
		printf '  - Section headers are canonical paths without the leading src/ prefix.\n'; \
		printf '  - File contents are copied from git (HEAD), not the working tree.\n'; \
		printf -- '---\n\n'; \
		printf '# Live Source Snapshot (main)\n\n'; \
		printf 'Generated from main at commit %s.\n\n' "$$commit"; \
		printf 'Includes files under src that are live in current targets and examples, excluding deadcode and smd/conceptmap.\n'; \
		printf 'Canonical names below omit the leading src/ prefix.\n\n'; \
		git ls-tree -r --name-only HEAD src \
			| rg '^src/' \
			| rg -v '^src/deadcode/' \
			| rg -v '^src/smd/conceptmap/' \
			| rg -v '/CMakeLists\.txt$$' \
			| rg '\.(hpp|h|cpp)$$' \
			| sort -u \
			| while IFS= read -r f; do \
			canon="$${f#src/}"; \
			printf '## %s\n\n' "$$canon"; \
			printf '```cpp\n'; \
			git show "HEAD:$$f"; \
			printf '\n```\n\n'; \
		done; \
	} > "$$out"; \
	echo "Updated $$out"

.PHONY: docs-live-src-strict
docs-live-src-strict: ## Regenerate docs/live-src-main-built-targets.md from explicit CMake target_sources()
	@set -eu; \
	repo_root="$$(git rev-parse --show-toplevel)"; \
	out="$$repo_root/docs/live-src-main-built-targets.md"; \
	commit="$$(git rev-parse --short HEAD)"; \
	tmp_list="$$(mktemp)"; \
	trap 'rm -f "$$tmp_list"' EXIT; \
	find "$$repo_root/src" -name CMakeLists.txt -type f | sort | while IFS= read -r cmake; do \
		dir="$$(dirname "$$cmake")"; \
		sed -n '/target_sources(/,/)/p' "$$cmake" \
			| sed 's/#.*$$//' \
			| rg -o '[A-Za-z0-9_./-]+\.(hpp|h|cpp|t\.cpp|test\.cpp)' \
			| while IFS= read -r rel; do \
				if [[ "$$rel" == */* ]]; then \
					full="$$dir/$$rel"; \
				else \
					full="$$dir/$$rel"; \
				fi; \
				printf '%s\n' "$$full"; \
			done; \
	done \
		| sed "s#^$$repo_root/##" \
		| rg '^src/' \
		| rg -v '^src/deadcode/' \
		| rg -v '^src/smd/conceptmap/' \
		| rg '\.(hpp|h|cpp|t\.cpp|test\.cpp)$$' \
		| sort -u > "$$tmp_list"; \
	{ \
		printf -- '---\n'; \
		printf 'title: Live Source Snapshot (main, strict built-target graph)\n'; \
		printf 'summary: Point-in-time fenced dump of src C++ files explicitly listed in CMake target_sources() on main.\n'; \
		printf 'source_of_truth: git HEAD on branch main\n'; \
		printf 'strictness: only explicit target_sources entries; excludes transitively included headers\n'; \
		printf 'scope:\n'; \
		printf '  include:\n'; \
		printf '    - files directly named in src/**/CMakeLists.txt target_sources()\n'; \
		printf '  exclude:\n'; \
		printf '    - src/deadcode/**\n'; \
		printf '    - src/smd/conceptmap/**\n'; \
		printf 'update_policy:\n'; \
		printf '  when_to_update:\n'; \
		printf '    - Any time target_sources lists in src/**/CMakeLists.txt change.\n'; \
		printf '    - Any time a listed file changes on main and this snapshot is used as a baseline.\n'; \
		printf '  how_to_update:\n'; \
		printf '    - Rebuild the explicit built-file manifest from CMakeLists and regenerate from git HEAD.\n'; \
		printf 'notes:\n'; \
		printf '  - Section headers are canonical paths without the leading src/ prefix.\n'; \
		printf '  - File contents are copied from git (HEAD), not the working tree.\n'; \
		printf -- '---\n\n'; \
		printf '# Live Source Snapshot (main, strict built-target graph)\n\n'; \
		printf 'Generated from main at commit %s.\n\n' "$$commit"; \
		printf 'This file includes only source files explicitly listed in CMake target_sources() entries under src.\n\n'; \
		while IFS= read -r f; do \
			canon="$${f#src/}"; \
			printf '## %s\n\n' "$$canon"; \
			printf '```cpp\n'; \
			git show "HEAD:$$f"; \
			printf '\n```\n\n'; \
		done < "$$tmp_list"; \
	} > "$$out"; \
	echo "Updated $$out"

.PHONY: docs-index
docs-index: ## Regenerate docs/index.org from strict built-target snapshot
	@set -euo pipefail; \
	repo_root="$$(git rev-parse --show-toplevel)"; \
	out="$$repo_root/docs/index.org"; \
	strict_snapshot="$$repo_root/docs/live-src-main-built-targets.md"; \
	{ \
		printf '#+TITLE: Trees Active Source Index\n'; \
		printf '#+AUTHOR: Generated\n'; \
		printf '#+OPTIONS: toc:2 num:nil\n\n'; \
		printf '* Overview\n'; \
		printf 'This index transcludes active files under =src= that are explicitly listed in CMake =target_sources()= entries.\n'; \
		printf 'It excludes =deadcode= and =smd/conceptmap=.\n\n'; \
		printf '* Active Source Files\n\n'; \
		rg '^## ' "$$strict_snapshot" | sed 's/^## //' | sort -u | while IFS= read -r rel; do \
			ext="$${rel##*.}"; \
			lang='text'; \
			case "$$ext" in \
				cpp|cc|cxx|hpp|hh|hxx|h) lang='cpp' ;; \
				md|markdown) lang='markdown' ;; \
				org) lang='org' ;; \
			esac; \
			printf '** %s\n' "$$rel"; \
			printf '#+transclude: [[file:../src/%s]] :src %s\n\n' "$$rel" "$$lang"; \
		done; \
		printf '* Regeneration\n'; \
		printf 'Run from repository root to regenerate this file:\n\n'; \
		printf '#+name: regenerate-index-org\n'; \
		printf '#+begin_src bash :results output verbatim\n'; \
		printf 'set -euo pipefail\n'; \
		printf 'repo_root="$$(git rev-parse --show-toplevel)"\n'; \
		printf 'make -C "$$repo_root" docs-index\n'; \
		printf '#+end_src\n'; \
	} > "$$out"; \
	echo "Updated $$out"

.PHONY: docs-index-md
docs-index-md: docs-index docs/index.md ## Regenerate docs/index.md from docs/index.org with transcluded source rendered
	@echo "Updated docs/index.md"

docs/index.md: docs/index.org
	$(EMACS) --init-directory=.emacs.d/ \
	--batch --load .emacs.d/init.el  \
	-f package-initialize \
	--eval "(setq enable-local-variables :all)" \
	--visit $< \
	--eval "(org-transclusion-mode t)" \
	--eval "(org-export-to-file 'gfm \"$(CURDIR)/docs/index.md\")"

.PHONY: docs-refresh
docs-refresh: docs-live-src docs-live-src-strict docs-index docs-index-md ## Refresh source snapshots and both org/md indexes
	@echo "Refreshed live source snapshots and index docs"

.PHONY: mrdocs
mrdocs: ## Build the docs with MrDocs
	-rm -rf docs/adoc
	cd docs && NO_COLOR=1 mrdocs mrdocs.yml 2>&1 | sed 's/\x1b\[[0-9;]*m//g'
	find docs/adoc -name '*.adoc' | xargs asciidoctor

.PHONY: testinstall
testinstall: install
testinstall: CONFIG=RelWithDebInfo
testinstall: ## Test the installed package
	-$(RM) -rf installtest/.build
	$(CMAKE) -S installtest -B installtest/.build 	-G "Ninja Multi-Config"
	$(CMAKE) --build  installtest/.build --target test --config="RelWithDebInfo"

.PHONY: clean-testinstall
clean-testinstall:
	-rm -rf installtest/.build

realclean: clean-testinstall

ifeq ($(UV),)
define install_uv_cmd
pipx install uv
endef

define uv_error_message

'uv' command not found.
Please install uv or set the UV variable to the path of the uv binary.
The makefile target "install-uv" will run ``$(install_uv_cmd)''
endef

$(warn "$(uv_error_message)")
endif

.PHONY: install-uv
install-uv: ## install uv via `pipx install uv`
	$(install_uv_cmd)

ORGFILES := $(wildcard *.org)

%.html : %.org
	$(EMACS) --init-directory=.emacs.d/ \
	--batch --load .emacs.d/init.el  \
	-f package-initialize \
	--eval "(setq enable-local-variables :all)" \
	--load etc/bbg-footer.el \
	--load etc/set-footer.el \
	--visit $< \
	--eval "(org-transclusion-mode t)" \
	--eval "(org-export-to-file 'html \"$@\")"
	echo $@ : \\ > $@.deps
	echo "  $<" \\ >> $@.deps
	sed -n "s/^.*\[\[file:\(\S*\)::.*$$/\1/p" < $<  | sort -u | xargs printf "  %s \\\\\\n" >> $@.deps

-include $(ORGFILES:%.org=%.html.deps)

%-slides.html : %.org
	$(EMACS) --init-directory=.emacs.d/ \
	--batch --load .emacs.d/init.el  \
	-f package-initialize \
	--load etc/bbg-footer.el \
	--load etc/set-footer.el \
	--eval "(setq enable-local-variables :all)" \
	--visit $< \
	--eval "(org-transclusion-mode t)" \
	--eval "(org-export-to-file 're-reveal \"$@\")"
	echo $@ : \\ > $@.deps
	echo "  $<" \\ >> $@.deps
	sed -n "s/^.*\[\[file:\(\S*\)::.*$$/\1/p" < $<  | sort -u | xargs printf "  %s \\\\\\n" >> $@.deps

-include $(ORGFILES:%.org=%-slides.html.deps)

%.md : %.org
	$(EMACS) --init-directory=.emacs.d/ \
	--batch --load .emacs.d/init.el  \
	-f package-initialize \
	--load etc/bbg-footer.el \
	--load etc/set-footer.el \
	--eval "(setq enable-local-variables :all)" \
	--visit $< \
	--eval "(org-transclusion-mode t)" \
	--eval "(org-export-to-file 'gfm \"$@\")"
	echo $@ : \\ > $@.deps
	echo "  $<" \\ >> $@.deps
	sed -n "s/^.*\[\[file:\(\S*\)::.*$$/\1/p" < $<  | sort -u | xargs printf "  %s \\\\\\n" >> $@.deps

-include $(ORGFILES:%.org=%.md.deps)

.PHONY: clean-emacs.d
clean-emacs.d:
	-rm -rf .emacs.d/eln-cache
	-rm -rf .emacs.d/elpa*

realclean: clean-emacs.d

.PHONY: clean-example
clean-example:
	-rm example.html

clean: clean-example

.PHONY: clean-org-deps
clean-org-deps:
	-rm $(ORGFILES:%.org=%.org.deps)
clean: clean-org-deps

.PHONY: clean-org-html
clean-org-html:
	-rm $(ORGFILES:%.org=%.html) $(ORGFILES:%.org=%-slides.html)
clean: clean-org-html

.PHONY: presentation
presentation: docs-refresh
presentation: test
presentation: foldable-applicative-traversable.html
presentation: foldable-applicative-traversable-slides.html
presentation: foldable-applicative-traversable.md

.PHONY: elpa
elpa:
	$(EMACS) --init-directory=.emacs.d/ --batch --load .emacs.d/init.el

.PHONY: refresh
refresh:
	$(EMACS) --init-directory=.emacs.d/ --batch --load .emacs.d/init.el -f package-upgrade-all

# Help target
.PHONY: help
help: ## Show this help.
	@awk 'BEGIN {FS = ":.*?## "} /^[.a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'  $(MAKEFILE_LIST) | sort
