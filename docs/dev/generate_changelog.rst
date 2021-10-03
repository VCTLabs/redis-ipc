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
commit messages and consider using the gitchangelog message modifiers,
as well as proper tags.

Read more about ``gitchangelog`` here_.

.. _here: https://github.com/sarnold/gitchangelog

Tags
----

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
