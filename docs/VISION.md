# IronQuery

A header-only SQL query builder for C++.

# A user

Our user is a C++ developer who is required to craft dynamic SQL queries.
The core risk is to make error with manual SQL typing:
- bad SQL syntax
- keyword typo
- missing escaping of string values
- non-existing table field
- lost field after migration
- erroneous expressions with a branch duplication like DELETE ... USING ... USING ...
