microdnf
========

A minimal `dnf` for (mostly) Docker containers that uses
[libdnf](https://github.com/rpm-software-management/libdnf)
and hence doesn't require Python.

This project was inspired by and derived from
https://github.com/cgwalters/micro-yuminst


Reporting issues
================

* [Red Hat Bugzilla](https://bugzilla.redhat.com/enter_bug.cgi?product=Fedora&component=microdnf) is the preferred way of filing issues. [[backlog](https://bugzilla.redhat.com/buglist.cgi?bug_status=__open__&product=Fedora&component=microdnf)]
* [GitHub issues](https://github.com/rpm-software-management/microdnf/issues/new) are also accepted. [[backlog](https://github.com/rpm-software-management/microdnf/issues)]


Contribution
============

Here's the most direct way to get your work merged into the project.

1. Fork the project
1. Clone down your fork
1. Implement your feature or bug fix and commit changes
1. If the change fixes a bug at [Red Hat bugzilla](https://bugzilla.redhat.com/), or if it is important to the end user, add the following block to the commit message:

       = changelog =
       msg:           message to be included in the changelog
       type:          one of: bugfix/enhancement/security (this field is required when message is present)
       resolves:      URLs to bugs or issues resolved by this commit (can be specified multiple times)
       related:       URLs to any related bugs or issues (can be specified multiple times)

   * For example::

         = changelog =
         msg: Don't print lines with (null) in transaction report
         type: bugfix
         resolves: https://bugzilla.redhat.com/show_bug.cgi?id=1691353

   * For your convenience, you can also use git commit template by running the following command in the top-level directory of this project:

         git config commit.template ./.git-commit-template

1. Push the branch to your fork
1. Send a pull request for your branch

