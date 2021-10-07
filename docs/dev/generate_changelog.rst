Changelog Generation
====================

Changelogs help document important changes.

To generate a (full) changelog from the repository root, run:

.. code-block:: bash

    (venv) $ gitchangelog

We use ``gitchangelog`` to create the changelog automatically.  It
examines git commit history and uses custom "filters" to produce its
output. The configurations for this are in the files
``.gitchangelog.rc`` and ``.gitchangelog-keepachangelog.tpl``.

To make your changelog even more useful/readable, you should use good
commit messages and consider using the gitchangelog message modifiers.
Since the ``.gitchangelog.rc`` is actually written in Python, it becomes
quite dynamic, thus the configured modifiers and associated documentation
are usually documented in the file itself (unless someone strips out all
the comments).  For this config, the message format uses 3 types of
modifier::

  Message Format
    ACTION: [AUDIENCE:] COMMIT_MSG [!TAG ...]

  Description
    ACTION is one of 'chg', 'fix', 'new'

      Is WHAT the change is about.

      'chg' is for refactor, small improvement, cosmetic changes...
      'fix' is for bug fixes
      'new' is for new features, big improvement

    AUDIENCE is optional and one of 'dev', 'usr', 'pkg', 'test', 'doc'

      Is WHO is concerned by the change.

      'dev'  is for developpers (API changes, refactors...)
      'usr'  is for final users (UI changes)
      'pkg'  is for packagers   (packaging changes)
      'test' is for testers     (test only related changes)
      'doc'  is for doc guys    (doc only changes)

    COMMIT_MSG is ... well ... the commit message itself.

    TAGs are additionnal adjective as 'refactor' 'minor' 'cosmetic'

      They are preceded with a '!' or a '@' (prefer the former, as the
      latter is wrongly interpreted in github.) Commonly used tags are:

      'refactor' is obviously for refactoring code only
      'minor' is for a very meaningless change (a typo, adding a comment)
      'cosmetic' is for cosmetic driven change (re-indentation, 80-col...)
      'wip' is for partial functionality but complete subfunctionality.

  Example:

    new: usr: support of bazaar implemented
    chg: re-indentend some lines !cosmetic
    new: dev: updated code to be compatible with last version of killer lib.
    fix: pkg: updated year of licence coverage.
    new: test: added a bunch of test around user usability of feature X.
    fix: typo in spelling my name in comment. !minor


See the current `.gitchangelog.rc`_ in the repo for more details.

Read more about ``gitchangelog`` here_.

.. _.gitchangelog.rc: https://github.com/VCTLabs/redis-ipc/blob/develop/.gitchangelog.rc
.. _here: https://github.com/sarnold/gitchangelog


Git Tags
--------

Git tags are a way to bookmark commits, and come in two varieties:
lightweight and signed/annotated. Both signed and annotated tags
contain author information and when used they will help organize the
changelog.

To create an annotated tag for a version ``0.1.1`` release:

.. code-block:: bash

    $ git tag -a v0.1.1 -m "v0.1.1"

Using tags like this will break the changelog into sections based on
versions. If you forgot to make a tag you can checkout an old commit
and make the tag (don't forget to adjust the date - you may want to
google this...)


Sections
--------

The sections in the changelog are created from the git log commit
messages, and are parsed using the regex defined in the
``.gitchangelog.rc`` configuration file.
