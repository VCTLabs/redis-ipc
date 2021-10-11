==================================================
 Contents of the ``.pre-commit-config.yaml`` file
==================================================

The file ``.pre-commit-config.yaml`` is used to configure the program
``pre-commit``, which controls the setup and execution of `Git hooks`_.

The ``.pre-commit-config.yaml`` file has a list of git repos, each repo may
define one or more hooks.

In this document we will review the various hooks. Some of the hooks will
modify files, some will not.

.. _pre-commit: https://pre-commit.com
.. _Git hooks: https://git-scm.com/book/en/v2/Customizing-Git-Git-Hooks


Hook Descriptions
=================

Basic warning checks include:

* ``check-added-large-files``
* ``check-case-conflict``
* ``check-executables-have-shebangs``
* ``check-shebang-scripts-are-executable``
* ``check-merge-conflict``
* ``detect-private-key``


``end-of-file-fixer``
---------------------

This will modify files by making sure that each file ends in a blank line.

If a commit fails due to this hook, just commit again.


``trailing-whitespace``
-----------------------

This will modify files by ensuring there is no trailing whitespace on any line.

If a commit fails due to this hook, just commit again.

``mixed-line-ending``
---------------------

This will modify files by ensuring there are no mixed line endings in any file.

If a commit fails due to this hook, just commit again.

``check-yaml``
--------------

This will NOT modify files. It will examine YAML files and report any
issues. The rules for its configuration are defined in
``.pre-commit-config.yaml`` in the ``exclude`` section.

If a commit fails due to this hook, all reported issues must be manually
fixed before committing again.

``cmake-format``
----------------

This will modify files. It will examine Cmake files and fix some
formatting/style issues. The rules for its configuration are currently
just upstream defaults.

If a commit fails due to this hook, review the proposed changes in the
console, and check the files using ``git diff <file1> <file2> ...``

``cpplint``
-----------

This will NOT modify files. It will examine source files and report any
issues. The rules for its configuration are defined in
``.pre-commit-config.yaml`` in the ``args`` section.

If a commit fails due to this hook, all reported issues must be manually
fixed before committing again.

``beautysh``
------------

This will modify files. It will examine shell files and fix some
formatting issues. The rules for its configuration are defined in
``.pre-commit-config.yaml`` in the ``args`` section.

If a commit fails due to this hook, review the proposed changes in the
console, and check the files using ``git diff <file1> <file2> ...``

Doc formatting (.rst files)
---------------------------

* doc8
* pygrep

  - rst-backticks
  - rst-directive-colons
  - rst-inline-touching-normal

These checks will NOT modifiy files. They will examine all RST files
(except ChangeLog.rst) and report any issues.

If a commit fails due to any of these hooks, all reported issues must be
manually fixed before committing again.
