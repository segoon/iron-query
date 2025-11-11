#pragma once

#include <cassert>
#include <string>
#include <vector>

namespace sql_builder {

/// @brief SQL query builder
///
/// Architectual decisions:
/// 1) This is a plain C++ -> SQL mapping builder;
/// 2) This is NOT an ORM;
/// 3) The main idea behind this is a possibility to avoid typical
///    syntax mistakes/typos in SQL query typing;
/// 4) The builder DOES NOT check for identifier definition,
///    expression data types, member names, etc.
/// 5) The builder DOES NOT check for missed required clause.
///
/// The benefits of using the builder:
/// 1) Dynamically build complex WHERE clauses with something
///    better than raw strings concatenation;
/// 2) Automatically use full set of table column names after schema migration;
/// 3)

class Table;

class [[nodiscard]] VirtualTable {
public:
    virtual std::string ToString() const = 0;

    virtual std::string ToStringBracketed() const { return "(" + ToString() + ")"; }

    Table As(std::string_view name) const;
};

class [[nodiscard]] Table /* not final! */ : public VirtualTable {
public:
    Table(std::string name) : name_(std::move(name)) {}

    std::string ToString() const override { return name_; }

    std::string ToStringBracketed() const override { return ToString(); }

private:
    std::string name_;
};

inline Table VirtualTable::As(std::string_view name) const {
    return Table("(" + ToString() + " AS " + std::string(name) + ")");
}

class [[nodiscard]] Expr {
public:
    Expr(std::string s) : expr_(std::move(s)) {}

    Expr(const char* s) : expr_(s) {}

    Expr(int i) : Expr(std::to_string(i)) {}

    Expr operator<(const Expr& other) const { return Expr(expr_ + " < " + other.expr_); }

    Expr operator>(const Expr&) const;

    Expr operator==(const Expr&) const;

    Expr operator||(const Expr&) const;

    Expr operator&&(const Expr&) const;

    std::string Extract() const { return expr_; }

private:
    std::string expr_;
};

class [[nodiscard]] SelectExpr final : public VirtualTable {
public:
    SelectExpr(const Table& tbl) : from_(tbl.ToStringBracketed()) {}

    SelectExpr Select(Expr exp) && {
        select_ = exp.Extract();
        return std::move(*this);
    }

    SelectExpr Select(std::initializer_list<Expr> exps) && {
        for (const auto& exp : exps) {
            if (!select_.empty()) select_ += ", ";
            select_ += exp.Extract();
        }
        return std::move(*this);
    }

    SelectExpr Where(Expr exp) && {
        where_ = exp.Extract();
        return std::move(*this);
    }

    // TODO: multiple args
    SelectExpr OrderBy(std::string_view by) && {
        order_by_ = by;
        return std::move(*this);
    }

    std::string ToString() const override {
        assert(!select_.empty());
        assert(!from_.empty());

        auto s = "SELECT " + select_ + " FROM " + from_;
        if (!where_.empty()) s += " WHERE " + where_;
        if (!order_by_.empty()) s += " ORDER BY " + order_by_;
        return s;
    }

private:
    std::string from_;
    std::string select_;
    std::string where_;
    std::string order_by_;
};

SelectExpr From(const Table& tbl) { return SelectExpr(tbl); }

class [[nodiscard]] DeleteFrom final : public VirtualTable {
public:
    DeleteFrom(const Table& tbl) : from_(tbl.ToStringBracketed()) {}

    DeleteFrom Where(Expr exp) && {
        where_ = exp.Extract();
        return std::move(*this);
    }

    std::string ToString() const override {
        assert(!from_.empty());

        auto s = "DELETE FROM " + from_;
        if (!where_.empty()) s += " WHERE " + where_;
        return s;
    }

private:
    std::string from_;
    std::string where_;
};

struct [[nodiscard]] JoinKind {
    virtual std::string_view ToString() const = 0;
};

struct [[nodiscard]] Inner final : JoinKind {
    std::string_view ToString() const override { return "INNER"; }
};
struct [[nodiscard]] Cross final : JoinKind {
    std::string_view ToString() const override { return "CROSS"; }
};

class [[nodiscard]] Join final : public VirtualTable {
public:
    Join(const VirtualTable& a, const VirtualTable& b, const JoinKind& kind)
        : a_(a.ToStringBracketed()), b_(b.ToStringBracketed()), kind_(kind.ToString()) {}

    Join On(Expr exp) && {
        on_ = exp.Extract();
        return std::move(*this);
    }

    std::string ToString() const override {
        auto s = a_ + " " + kind_ + " JOIN " + b_;
        if (!on_.empty()) s += " ON " + on_;
        return s;
    }

private:
    std::string kind_;
    std::string a_, b_;
    std::string on_;
};

// userver (???) PostgreSQL part:

struct [[nodiscard]] Column final {
    std::string name;
    std::string type;
    bool is_nullable{false};

    operator Expr() const { return Expr(name); }

    Expr operator<(const Expr& other) const;

    Expr operator>(const Expr&) const;

    Expr operator==(const Expr&) const;
};

// TODO: ugly/non-intuitive :(
Expr Dot(std::string_view table_name, const Column&);

class [[nodiscard]] TableWithColumns final : public Table {
public:
    TableWithColumns(std::string name, std::initializer_list<Column> columns)
        : Table(std::move(name)), columns_(columns) {}

    Expr SelectArgAll() const {
        std::string s;
        for (const auto& col : columns_) {
            if (!s.empty()) s += ", ";
            s += col.name;
        }
        return Expr(s);
    }

private:
    std::vector<Column> columns_;
};

}  // namespace sql_builder
