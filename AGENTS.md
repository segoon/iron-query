# IronQuery

A SQL query builder for C++.

- @docs/VISION.md - product definition, read it first

# Development

* C++17, cmake, doxygen
* clang-format, clang-tidy, sanitizers
* gtest
* git, .github
* `make test`
* doxygen comments for all types, methods, functions in `include/`

* DRY, KISS, SOLID.
* Prefer SRP, avoid god objects.
* Use OOP where appropriate.

* Do not write 'Co-Authored-By' in commit description
* Avoid dropping files git history, prefer `git mv`

* If you catch a bug in the code, write a regression test for that.
* After you add/edit a file, check the whole file for code duplication in tests. Don't leave similar boilerplate.
* When fixing a bug, search for similar bugs in the nearby code.
* When found a bug, elaborate whether it is possible to redesign the system to make such bugs impossible

* Prefer `auto` for auto deduced types

## Comments

* Code comments have to describe "why", not "how".
* Code comments must not duplicate the code, must be brief.
* Avoid obvious comments.
* Document complex/TODO/weird code briefly.
