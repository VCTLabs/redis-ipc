==================
 Using Pre-Commit
==================

`pre-commit`_ is a program used to configure and run Git hooks. These
hooks can be triggered in different Git stages, though typically we use
them in only commit and push stages.

See the `pre-commit config contents`_ document for descriptions of the
current hooks.

Each of the hooks will run in its own small virtual environment.

.. _pre-commit: https://pre-commit.com
.. _pre-commit config contents: pre-commit-config.rst


Setup
-----

The program must be installed and the hooks must be configured. The
program should be installed in your usual virtual environment, for
example, "venv" (this could also be a conda environment).

After activating your environment, run the following commands:

.. code-block:: bash

    (venv) $ pip install pre-commit
    (venv) $ pre-commit autoupdate
    (venv) $ pre-commit install
    (venv) $ pre-commit install-hooks


Automatic Usage
---------------

In normal usage, ``pre-commit`` will trigger with every ``git commit``
and every ``git push``. The hooks that trigger in each stage can be
configured by editing the ``.pre-commit-config.yaml`` file. The files
that have changed will be passed to the various hooks before the git
operation completes. If one of the hooks exits with a non-zero
exit-code, then the commit (or push) will fail.

Manual Usage
------------

To manually trigger ``pre-commit`` to run all hooks on CHANGED files:

.. code-block:: bash

    (venv) $ pre-commit run

To manually trigger ``pre-commit`` to run all hooks on ALL files,
regardless if they are changed or not:

.. code-block:: bash

    (venv) $ pre-commit run --all-files

To manually trigger ``pre-commit`` to run a single hook on changed files:

.. code-block:: bash

    (venv) $ pre-commit run <hook-id>

To manually trigger ``pre-commit`` to run a single hook on all files:

.. code-block:: bash

    (venv) $ pre-commit run <hook-id> --all-files

For example, to run ``cpplint`` on all files:

.. code-block:: bash

    (venv) $ pre-commit run cpplint --all-files
