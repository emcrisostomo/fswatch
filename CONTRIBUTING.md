Contributing to fswatch
=======================

The easiest ways to contribute to `fswatch` are:

  * Creating a new [issue].
  * Forking the repository, make your contribution and submit a pull request.
    See [Git Flow](#git-flow) for further information.

[issue]: https://github.com/emcrisostomo/fswatch/issues/new

Git Flow
--------

I chose to use the [Git flow branching model](flow) for `fswatch`, so you are
kindly required to follow the same model when making your contributions.  That
basically means that:

  * If you are fixing a bug, you can create a *hotfix* branch from the affected
  release (preferably, the latest one) and send a pull request from that branch.
  * If you are creating a new feature, please checkout the `develop` branch and
  send a pull request from there.
  * I won't consider pull request coming from the `master` branch and if I do, I
    won't be quick.

[flow]: http://nvie.com/posts/a-successful-git-branching-model/

Coding Conventions and Style
----------------------------

The C and C++ code uses `snake_case` and is formatted using NetBeans with a
customized [Allman (BSD)][allman] style.  See [`README.codestyle`][codestyle]
for further information on how setting up your IDE.

[codestyle]: README.codestyle
[allman]: https://en.wikipedia.org/wiki/Indent_style#Allman_style
