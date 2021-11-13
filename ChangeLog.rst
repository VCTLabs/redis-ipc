Changelog
=========

v0.0.5 (2021-11-12)
-------------------

New
~~~
- json.hh: provide has_field() alternative to throwing missing-field
  exception. [S. Lockwood-Childs]


v0.0.4 (2021-11-04)
-------------------

New
~~~
- Add initial pre-commit config. [Stephen L Arnold]
- Add cmake module to generate coverage data. [Stephen L Arnold]

  * cmake module replicates makefile.am lcov/genhtml commands
  * tox bionic env updated to generate full coverage
  * move lcov to common deps in smoke workflow
  * note full coverage reqiures 2 passes, one with and one without
    the runtime socket path override (true for both autotools
    and cmake builds)
  * enabling cmake coverage also enables building test binaries;
    both are currently OFF by default

    - WITH_COVERAGE enables RIPC_BUILD_TESTING
    - enable RIPC_BUILD_TESTING to build test binaries only
- One more (fast) workflow to test setting path ENV var. [Stephen L
  Arnold]

  * split env path, build (cmake and autotools) across ubuntu versions
  * note bionic lcov is too old for autotools, use cmake instead
- Updates all tox envs, adds valgrind with ci workflow. [Stephen L
  Arnold]

  * valgrind args are currently hard-coded in tox.ini
  * uses three of five test executables
- Test ccache module in conda devenv (not enbaled currently) [Stephen L
  Arnold]

  * can be used on linux; coughs broken compiler error on macos
- Add FindJSONC module, update cmake cfg and recipe. [Stephen L Arnold]
- Add cmake test coverage, checked with ctest build-and-test/gcovr.
  [Stephen L Arnold]
- Add cmake module for conda env, disable windows. [Stephen L Arnold]
- Add conda CI workflow, cleanup cmake options. [Stephen L Arnold]
- Add local conda build recipe. [Stephen L Arnold]
- Update cmake to create and install pkgconfig file. [S. Lockwood-
  Childs]

  Note that library version is still being maintained in 2 places,
  both CMakeLists.txt and configure.ac -- would be good to move it to
  a separate version file that both can share, at some point.
- Update cmake to check for json-c with pkg-config. [S. Lockwood-Childs]

  Use consistent strategy for finding json-c and hiredis dependencies,
  which might be managed by pkg-config rather than native cmake modules
- Add cmake build cfg, fix configure version and template typo. [Stephen
  L Arnold]

Changes
~~~~~~~
- Update pre-commit hooks, bump version for next release. [Stephen L
  Arnold]
- Be more explicit with coverage badge. [Stephen L Arnold]
- Set cmake-format intending to 4 and reformat. [Stephen L Arnold]
- Add overview of gitchangelog commit message handling. [Stephen L
  Arnold]
- Apply post-rebase pre-commit format updates. [Stephen L Arnold]

  * disable cmake auto-format around custom coverage command
- Add some dev docs, add rst doc checks to pre-commit config. [Stephen L
  Arnold]

  * add doc8 and some pygrep hooks to validate .rst formatting
  * add some pre-commit dev docs, usage and config hooks
  * validate docs, reflow some text, fix some warnings
  * make ChangeLog.rst an actual .rst doc
- Add pre-commit bash formatter, apply changes. [Stephen L Arnold]

  * update .pre-commit-config.yaml with prettysh
  * add prettysh to tox -e lint command
  * commit autogen.sh format changes
- Add contributing section with pre-commit install steps. [Stephen L
  Arnold]
- Pre-commit badge and readme cleanup. [Stephen L Arnold]
- Add runtime path ENV to coverage workflow. [Stephen L Arnold]

  * test coverage results
- Limit PPA dep installs, add setenv for tox. [Stephen L Arnold]

  * add setenv with override for RIPC_SERVER_PATH (auto,bionic)
  * reorder dep install commands, limit PPA installs (smoke)
  * fix bash quoting for nested quotes
- Use gcovr from pip, update gcovr args for bionic. [Stephen L Arnold]

  * unexpected coverage output between ubuntu versions
- Let cmake use socket path ENV var or path as build option. [Stephen L
  Arnold]

  * leave option empty/define unset if not found
  * prefer path found in env over build option
  * pass ENV var in tox.ini
- Add section on tox commands, remove redis-ipc-py section. [Stephen L
  Arnold]
- Json_test coughs runtime exception on macos, disable for now. [Stephen
  L Arnold]

  * tested on macos-10/11 in github (macos-10 did not run)
- Bump action version, remove temporary fix for coverage data. [Stephen
  L Arnold]
- Still more coverage refactoring. [Stephen L Arnold]

  * issue https://github.com/irongut/CodeCoverageSummary/issues/9
- Still has parse error, cleanup misc, disable fix, upload data.
  [Stephen L Arnold]
- Upgrade CodeCoverageSummary action to latest, remove DTD. [Stephen L
  Arnold]

  * add more sed and rename script => fix_cov_file.sh
- Add required deps, remove cruft (workflows) [Stephen L Arnold]
- Add coverage workflow, update readme, nuke codecov. [Stephen L Arnold]
- Test new action branch. [Stephen L Arnold]
- Remove python/related files => moved to redis-ipc-py repo. [Stephen L
  Arnold]
- Something amiss with running cccc-action container? [Stephen L Arnold]

  * the same smoke workflow is fine in the pcr repos
- Remove python-only workflows and badges. [Stephen L Arnold]
- Disable full conda workflow on PR, add dispatch to cov-test. [Stephen
  L Arnold]
- Update readme build steps (add conda) and .gitignore. [Stephen L
  Arnold]

  * updates for deps, autotools, cmake, and conda
  * ignore generated environment.yml file
- Remove ccache; not enough payoff, too much baggage. [Stephen L Arnold]
- Add coverage/deps for devenv workflow, fix matrix. [Stephen L Arnold]
- Remove macos until more debug, re-enable ccache on linux. [Stephen L
  Arnold]
- Use conda-dev/env setup for conda-dev workflow. [Stephen L Arnold]
- General build cleanup in cmake cfg and conda recipe. [Stephen L
  Arnold]

  * disable cmake modules, prefer pkg-config over find_package
  * adjust conda recipe deps and tests, add extra macos flags
- Disable conda-dev and try full conda workflow. [Stephen L Arnold]
- Switch generators, add cmake threads_init, test macos exc. [Stephen L
  Arnold]

  * json_test coughs an exception on macos, syscall warning
- Add pkg-config dep and FindPython module. [Stephen L Arnold]
- Use agnostic build-test command across all platforms. [Stephen L
  Arnold]

Fixes
~~~~~
- Add missing arg in readme example. [Stephen L Arnold]
- Document fixes and pre-commit autoupdates. [Stephen L Arnold]
- Improve gitchangelog config, populate ChangeLog => ..v0.0.3. [Stephen
  L Arnold]

  * use updated cfg with built-in rest_py for ChangeLog.rst
  * add experimental md template file for release page
  * add initial gitchangelog doc, update readme
- Pre-commit whitespace/eol cleanup commit. [Stephen L Arnold]
- Tweak pre-commit cfg, apply cmake/shell changes. [Stephen L Arnold]

  * yaml checks cough parse error on std conda meta.yaml format
  * cmake-format needs fencing/rulers to mark comments
  * add excludes and fence markers
  * restore missing clang toolchain file
- Local autotools env and small nit in PR coverage xml report names.
  [Stephen L Arnold]

  * isolate internal env, override via ENV_RIPC_RUNTIME_DIR
  * define package (internal) env var names using tox defaults
  * move tox env commands to replicate workflow
- Create both html coverage reports, add inc/ dir to metrics artifact.
  [Stephen L Arnold]

  * create both html reports, one for functions and one for branches
  * note each report is created via separate tox cmds
  * sync up metrics source code with coverage
  * update conda devenv file, use Ninja generator

  Signed-off-by: Stephen L Arnold <nerdboy@gentoo.org>

  chg: dev: py38/39 is not resolving deps like 37, remove jinja py ver

  * this should really not be necessary, somehow devenv is inconsistent
  * it should work fine across all python versions 36 => 39
  * even on macos
- Cleanup ci cmds (per OS env), add python dep for conda devenv.
  [Stephen L Arnold]

  * bionic lcov is too old for required include usage
  * devenv needs jinja python dep per CI version
- Cleanup coverage flags, upload coverage report. [Stephen L Arnold]

  * speedup: switch coverage workflow to ctest
  * cleanup: make sure covrage builds are identical
  * add cov report artifact upload to smoke workflow (no gh-pages branch yet)
  * add/update coverage cfgs and tox commands
- Remove stale results until next scan (cov-test workflow) [Stephen L
  Arnold]

  * add check for data file before triggering convert/upload steps
- Use local lcov config file for make cov, fix name in ci. [Stephen L
  Arnold]
- Refactor coverage generation/reporting, add fix script. [Stephen L
  Arnold]

  * add autobuild to tox, use lcov => gcovr for report
  * xml seems more compliant, except for pkg name="."
  * add fix_pkg_name.sh and run it in coverage workflow
- Sort out coverage config, enable debug for branches/lines. [Stephen L
  Arnold]
- Switch metrics action to latest release => 0.3. [Stephen L Arnold]

  * fixes metrics report artifact uploads
- Remove action options until gh-pages branch is pushed. [Stephen L
  Arnold]

  * add readme note about python module move
- Make sure autotools and cmake use the same soname/version. [Stephen L
  Arnold]

  * add missing configure check for pthreads (autotools)
  * allow SCM_VERSION to override static version (cmake)
- Restore missing target property versions. [Stephen L Arnold]
- Set recipe soversion, add include guard for unistd.h !win. [Stephen L
  Arnold]

Other
~~~~~
- Revert macos ci test. [Stephen L Arnold]
- Update version in configure.ac, fix typo and check macos. [Stephen L
  Arnold]
- Json.hh: make cpplint happier. [S. Lockwood-Childs]
- Json.hh: throw custom exception for missing fields. [S. Lockwood-
  Childs]

  caller might want to specifically know about missing fields,
  so make that a specific exception that can be caught
- Json.hh: fix json constructor from json_object ptr. [S. Lockwood-
  Childs]

  * if ptr is null, create an empty object
  * if ptr is non-null, take a reference on it so it will not
    get freed until json wrapper object is done with it
- Fix package_version var for cmake and autotools. [Stephen L Arnold]
- Add cmake-format cfg file, apply formatting updates. [Stephen L
  Arnold]

  * this seems to keep more of the original format
  * although it does add more dangling close-parens
- Restore gcovr funtion report to coverage artifact. [Stephen L Arnold]
- Fix generated pkgconfig file. [S. Lockwood-Childs]

  Now the @prefix@ in redis-ipc.pc.in should get substituted when building
  with cmake (already worked for autotools builds)
- Update smoke/runtime workflows to use lcov from PPA on bionic.
  [Stephen L Arnold]
- Configurable path to redis server socket. [S. Lockwood-Childs]

  * compile-time configuration with RIPC_RUNTIME_DIR
    * with cmake
      cmake -DRIPC_RUNTIME_DIR=/var/tmp/redis-ipc
    * with automake
      export RIPC_RUNTIME_DIR=/var/tmp/redis-ipc ./configure

  In each case, default path will be $RIPC_RUNTIME_DIR/socket
  Note that matches the usage of RIPC_RUNTIME_DIR in scripts/run_redis.sh
  If you set RIPC_RUNTIME_DIR when building, use the same value
  when using run_redis.sh to setup for tests.

  Path can be overrident at runtime by setting RIPC_SERVER_PATH
  NOTE this is full path, not just the parent dir like RIPC_RUNTIME_DIR:
    export RIPC_SERVER_PATH=/var/tmp/redis-ipc/socket
- Allow building deb from git repo. [S. Lockwood-Childs]

  Make dpkg-buildpackage not require a pre-existing source tarball
- Include pkgconfig when building deb. [S. Lockwood-Childs]

  Also removed python from deb packaging, since it moved to separate repo
- Sync with deb packaging files from PPA. [S. Lockwood-Childs]

  PPA is at https://launchpad.net/~nerdboy/+archive/ubuntu/embedded
- Cheg: dev: test temp fix for coverage workflow data parse error.
  [Stephen L Arnold]

  * remove temp fix when upstream issue is fixed
  * limit	metrics	collection to src/ directory only
  * adjust gcovr cmd root/path args, cleanup cruft
- Updated coverity results from after cleanup commit. [S. Lockwood-
  Childs]
- Clean up current detections from code scanners. [S. Lockwood-Childs]
- Fix dev: use correct syntax in last devenv workflow step. [Stephen L
  Arnold]
- Use new action release and set source directories for analysis.
  [Stephen L Arnold]
- Update recipe, add pkg-config to test commands, disable inspect.
  [Stephen L Arnold]
- Add develop branch to all workflowa, inspect conda pkgs. [Stephen L
  Arnold]
- Dis-able conda dev workflow, debug on macos (segfault) [Stephen L
  Arnold]
- Revert msvc include changes, re-enable conda-dev workflow. [Stephen L
  Arnold]
- Remove windows (msvc) from CI workflows and conda recipe. [Stephen L
  Arnold]
- Add more (win) build deps, tweak simple pkg tests. [Stephen L Arnold]
- Set install libdir, update host deps and build scripts. [Stephen L
  Arnold]

v0.0.3 (2021-08-20)
-------------------

New
~~~
- Add issue/PR templates and base .gitignore file. [Stephen L Arnold]

Changes
~~~~~~~
- Add pkconfig.in file, update configure.ac. [Stephen L Arnold]
- Add readme section for overlay/ppa package installs. [Stephen L
  Arnold]

Other
~~~~~
- Test conversion and display of coverity results as SARIF data.
  [Stephen L Arnold]
- Cpplint cleanup and workflow (#8) [Steve Arnold]

  * add doctest to pylint workflow, with minimal nose cfg
  * cpplint cleanup commit, mainly whitespace, if/else, and curly braces
  * cleanup indenting, revert if/else brace changes, add cfg file
  * fix constructor warnings in inc/json.hh, add cpplint worklow
- Revert action to @main and set branch for metrics. [Stephen L Arnold]
- Use new action release and set source directories for analysis.
  [Stephen L Arnold]
- Silence "/tmp" path socket warning with a usage comment. [Stephen L
  Arnold]
- Add pylint workflow (check only, fail under 9.25) [Stephen L Arnold]
- More fun with badges. [Stephen L Arnold]
- Add bandit workflow (with github annotaions), disable flake8 ignores.
  [Stephen L Arnold]
- Update readme status, use status table. [Stephen L Arnold]
- Add codeql analysis to its own workflow, enable extra queries.
  [Stephen L Arnold]
- Add python examples to readme (doctest-able even) [Stephen L Arnold]
- Pylint cleanup commit, update pep8speaks config. [Stephen L Arnold]
- Flake8 cleanup commit, add modified gitchangelog.rc and flake8 cfg.
  [Stephen L Arnold]


v0.0.2 (2021-07-23)
-------------------
- Switch build status badge to (internal) github actions. [Stephen L
  Arnold]
- Test alternate github license provider 2. [Stephen L Arnold]
- Update license (filename) to GPL-2.0 generated by github. [Stephen L
  Arnold]
- Add status badges to readme file (#4) [Steve Arnold]

  * add status badges to readme file
  * fix license file parsing (on github) and add SPDX id to primary sources
- Make gettid() conditional on glibc version. [Stephen L Arnold]
- Add project-level codecov config file. [Stephen L Arnold]
- Separate src prepare from src configure, display coverage in CI.
  [Stephen L Arnold]
- Enable coverage with html default report, add to smoke workflow.
  [Stephen L Arnold]
- Fix gcc build error and remove obsolete json-c usage. [Stephen L
  Arnold]
- Add github CI and test across ubuntu/toolchain versions. [Stephen L
  Arnold]
- Ditch extraneous header file from python branch. [S. Lockwood-Childs]
- Some redis-py fixes in python module. [S. Lockwood-Childs]

  * redis.Connection is for tcp connections, not unix sockets,
    use redis.StrictRedis instead

  * blpop() returns None on timeout or (queue, value) if successful in
    popping value from queue
- Debug fix properly access globals. [nll]
- Deleted bogus comma. [nll]
- This is a version ready to be tested it is not checked out. [nll]
- Add server-side class to python module. [S. Lockwood-Childs]

  client-side class has one public method
    redis_ipc_send_and_receive()

  but server-side class has two
    redis_ipc_receive_command()
    redis_ipc_send_reply()

  because server has to do some processing between getting a command
  and sending back a reply
- C library encodes tid as integer, so match in python module. [S.
  Lockwood-Childs]
- Python module is really close to client-side functionality. [S.
  Lockwood-Childs]

  "client-side" means the code that generates commands and receives
  replies, as opposed to "server-side" code that waits for commands
  and services them.

  python now follows C-library conventions so it should (soon) interoperate
  with a server app written in C:

  * same mandatory fields for commands

    cmd["timestamp"]
    cmd["component"]
    cmd["thread"]
    cmd["tid"]
    cmd["results_queue"]
    cmd["command_id"]

  * same naming of queues for commands and their replies

    * command queue in format "queues.commands.$SERVER_COMPONENT"

    * reply queue in format "queues.results.$CLIENT_COMPONENT.$CLIENT_THREAD"

  TODO:

  Still need to fill in the actual redis connection bits,
  plus generate real timestamps for commands
- This version can do a few things it thinks it can send and receive
  messages, but it can not those functions are stubs the file can be
  imported into Python the code is written to raise exceptions, but none
  are handled yet no logging is performed. [nll]
- New version of skeleton and a tiny bit of meat. [nll]
- A little more client code for redis. [nll]
- A little more client code. [nll]
- Skeleton of redis client. [nll]
- A file was added proclaiming the vital features of the client library
  to implement in Python as a first phase no comment on what a new phase
  might bring. [nll]
- Redis_ipc.h: explicitly declare init/cleanup functions. [Steve Arnold]

  Fixes QA warnings about implicit declarations.


v0.0.1 (2017-03-11)
-------------------
- Make debian packaging straight from git work. [S. Lockwood-Childs]

  alternative is to do 'make dist' and use that as upstream tarball,
  either way should work...
- Fix date stamp in debian/copyright file. [Steve Arnold]
- Tweak debian/ubuntu packaging so it should work. [S. Lockwood-Childs]
- Add readme for example programs. [S. Lockwood-Childs]
- Add initial debian packaging files (still untested) [Steve Arnold]
- "tests" were more examples than tests, for now count running as
  "passed" [S. Lockwood-Childs]
- Cleanup cruft, need to test with running redis server. [Steve Arnold]
- Building lib and test programs works; custom test runner tweaks still
  needed. [Steve Arnold]
- Working libtool shared library build (no tests yet, so still a WIP)
  [Steve Arnold]

  Still not sure if that's what we want...
- Not quite working - WIP. [Steve Arnold]
- Make new autotools baseline, move to subdirs, add Makefile.am and
  configure.ac, populate initial GPL files. [Steve Arnold]


v0.0.0 (2017-03-10)
-------------------
- Fix build against current json-c paths and names. [Steve Arnold]
- Still filling holes in README doc. [Stephanie Lockwood-Childs]
- Another README formatting tweakage. [Stephanie Lockwood-Childs]
- README formatting fixes. [Stephanie Lockwood-Childs]
- Putting documentation README. [Stephanie Lockwood-Childs]

  Still a work in progress, some sections missing...
- Connect to unix socket instead of localhost tcp. [Stephanie Lockwood-
  Childs]

  Unix sockets are better for performance (and security, since permissions
  can constrain what clients use the socket) than localhost tcp
  connections, so a hard-coded path of /tmp/redis-ipc/socket replaces the old
  localhost & port in the category of things-that-probably-belong-in-a-config
- Provide sample output file for each test prog. [Stephanie Lockwood-
  Childs]
- Fix crashes after failing to connect to redis server. [Stephanie
  Lockwood-Childs]
- Support 'make testprogs' [Stephanie Lockwood-Childs]
- Test program cleanup. [Stephanie Lockwood-Childs]

  test.c was a dupe, json_test can now be built from Makefile
- Native build should be default. [Stephanie Lockwood-Childs]

  When cross-compiling, set CROSS_COMPILE and SYSROOT. Skip setting
  them for native compiles.
- Just a couple more debug messages. [Stephanie Lockwood-Childs]

  Helped with debugging an app crash when a field was missing from redis
- Added GNU hash to linker args. [Stephen Arnold]
- Update makefile and added missing includes to test source files.
  [Stephen Arnold]
- Switched to thread-local storage. [Stephanie Lockwood-Childs]

  Discovered that arm compiler should support __thread variables, so was
  able to switch per-thread struct to thread-local without having to roll
  my own via different entries in a list.

  Multi-thread test program which would segfault about 1 in 3 times (due to the
  threads stomping on each other) now succeeded 100 times in a row.
- Make header C++ safe. [Stephanie Lockwood-Childs]

  use ifdef's to insert 'extern C' block in header when compiled under C++
- Added wrapper class for json-c access from C++ [Stephanie Lockwood-
  Childs]

  json-c brings with it the typical reference-tracking pain of
  dynamically allocated C objects, but redis_ipc uses it anyway
  to provide C compatibility. This wrapper class is intended
  to make use of a returned json_object * much less painful for
  applications that are written in C++ instead.
- Fix segfault after redis connection error. [Stephanie Lockwood-Childs]
- Fix single-field read of settings/status hashes. [Stephanie Lockwood-
  Childs]

  Return value was pointing to stuff that was going out of scope, so
  needed to stdup() a copy. Made note in the .h that, as usual, caller
  is responsible for cleanup when done with returned value.
- Add targets for test programs and install. [Stephanie Lockwood-Childs]
- Implemented single-field hash operations. [Stephanie Lockwood-Childs]

  Single-field versions of setting and status hash operations
  seem to be working now.
- Setting hash read/write implemented. [Stephanie Lockwood-Childs]

  Successfully performed setting write and read back with test
  program. Currently library is looking for component "db" as
  the one privileged to update settings, but that probably isn't
  the right name.

  Still need to implement single-field operations for both status
  and setting hashes.
- Status hash read/write implemented. [Stephanie Lockwood-Childs]

  Successfully performed status write and read back with test program.

  While adding hash support, discovered that redis syntax errors do not
  result in NULL replies but rather REDIS_REPLY_ERROR type replies,
  so redis reply checking had to be reworked accordingly.
- Pairing of command and result. [Stephanie Lockwood-Childs]

  After sending a command, the submitter will now discard result entries
  that do not have a matching ID and keep looking for the one that belongs
  to the just-submitted command.
- Send command and receive result almost works. [Stephanie Lockwood-
  Childs]

  Command processing is close to finished: one process can queue a
  command, and another process can receive and send back a result.

  Still need to put in the check to see that a received reply matches
  the recently-sent command (compare the command id strings).
- Queueing commands. [Stephanie Lockwood-Childs]

  The first half of sending commands has been implemented and exercised
  with test program. The command is being properly formatted and pushed to
  a redis queue, but parsing the result still needs to be filled out.

  Also cleaned up internal func ipc_path() to be less redundant.
- Cleanup func. [Stephanie Lockwood-Childs]

  Implemented cleanup func, though will need to revisit both init and
  cleanup to make them work with multi-thread processes (switch to a
  list of per-thread structs for saving state, as noted in FIXMEs).
- Subscribers can listen on channels. [Stephanie Lockwood-Childs]

  Finished up initial cut at pub/sub API by implementing the blocking
  listener function. Caller is responsible for not trying to listen
  until one or more channels have been subscribed, though library
  could track subscriptions if that became a problem.

  Test program is now able to post messages from one process and
  receive them from another.
- Implement subscribe/unsubscribe. [Stephanie Lockwood-Childs]

  Implemented functions for subscribe/unsubscribe from event or debug
  channels. The correct redis commands appear to be sent by the test
  program, and further verification awaits implementation of receiving
  published events/debug messages.
- Implement sending of events. [Stephanie Lockwood-Childs]

  events can now be published on redis
- Added timestamps to debug. [Stephanie Lockwood-Childs]
- Debug channel is working. [Stephanie Lockwood-Childs]

  Haven't implemented timestamp field for debug messages yet,
  but other than that debug messages are working -- JSON message
  looks correct and gets sent to redis pub/sub channel.
- Implementing init and send-debug funcs. [Stephanie Lockwood-Childs]

  Init function seems to work, debug function is mostly there -- generates
  json text, but prints to stdout instead of really publishing to redis.
- Starting library implementation. [Stephanie Lockwood-Childs]

  Started implementing functions. Init and send-debug functions are mostly
  implemented and compile now (not run-tested yet)
- Initial design but not implementation. [Stephanie Lockwood-Childs]

  Library include file has proposed function signatures

  None of the functions have been implemented yet however
