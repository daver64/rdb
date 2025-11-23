#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <memory>
#include <functional>
#include <iostream>

namespace rdb {

class SQLiteException : public std::runtime_error {
public:
    SQLiteException(const std::string& msg) : std::runtime_error(msg) {}
};

// ---------------------------------
// Database
// ---------------------------------
class Database {
    sqlite3* db_ = nullptr;

public:
    Database(const std::string& filename) {
        if (sqlite3_open(filename.c_str(), &db_) != SQLITE_OK) {
            throw SQLiteException(sqlite3_errmsg(db_));
        }
    }

    ~Database() { if (db_) sqlite3_close(db_); }

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    Database(Database&& other) noexcept : db_(other.db_) { other.db_ = nullptr; }
    Database& operator=(Database&& other) noexcept {
        if (db_) sqlite3_close(db_);
        db_ = other.db_;
        other.db_ = nullptr;
        return *this;
    }

    sqlite3* get() { return db_; }

    std::unique_ptr<class Statement> prepare(const std::string& sql);

    void execute(const std::string& sql) {
        char* errmsg = nullptr;
        if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
            std::string msg = errmsg ? errmsg : "Unknown error";
            sqlite3_free(errmsg);
            throw SQLiteException(msg);
        }
    }

    // RAII transaction
    class Transaction {
        Database& db_;
        bool active_ = true;
    public:
        Transaction(Database& db) : db_(db) { db_.execute("BEGIN;"); }
        ~Transaction() { if(active_) db_.execute("ROLLBACK;"); }
        void commit() { db_.execute("COMMIT;"); active_ = false; }
        void rollback() { db_.execute("ROLLBACK;"); active_ = false; }
    };
};

// ---------------------------------
// Statement
// ---------------------------------
class Statement {
    sqlite3_stmt* stmt_ = nullptr;

public:
    Statement(sqlite3* db, const std::string& sql) {
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_, nullptr) != SQLITE_OK) {
            throw SQLiteException(sqlite3_errmsg(db));
        }
    }

    ~Statement() { if(stmt_) sqlite3_finalize(stmt_); }

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    Statement(Statement&& other) noexcept : stmt_(other.stmt_) { other.stmt_ = nullptr; }
    Statement& operator=(Statement&& other) noexcept {
        if(stmt_) sqlite3_finalize(stmt_);
        stmt_ = other.stmt_;
        other.stmt_ = nullptr;
        return *this;
    }

    // Positional binding
    void bind(int index, int val) { sqlite3_bind_int(stmt_, index, val); }
    void bind(int index, double val) { sqlite3_bind_double(stmt_, index, val); }
    void bind(int index, const std::string& val) {
        sqlite3_bind_text(stmt_, index, val.c_str(), -1, SQLITE_TRANSIENT);
    }

    // Named binding
    void bind(const std::string& name, int val) {
        int idx = sqlite3_bind_parameter_index(stmt_, name.c_str());
        if(idx) sqlite3_bind_int(stmt_, idx, val);
    }
    void bind(const std::string& name, double val) {
        int idx = sqlite3_bind_parameter_index(stmt_, name.c_str());
        if(idx) sqlite3_bind_double(stmt_, idx, val);
    }
    void bind(const std::string& name, const std::string& val) {
        int idx = sqlite3_bind_parameter_index(stmt_, name.c_str());
        if(idx) sqlite3_bind_text(stmt_, idx, val.c_str(), -1, SQLITE_TRANSIENT);
    }

    bool step() {
        int rc = sqlite3_step(stmt_);
        if(rc == SQLITE_ROW) return true;
        if(rc == SQLITE_DONE) return false;
        throw SQLiteException(sqlite3_errmsg(sqlite3_db_handle(stmt_)));
    }

    void reset() { sqlite3_reset(stmt_); }

    int getInt(int col) { return sqlite3_column_int(stmt_, col); }
    double getDouble(int col) { return sqlite3_column_double(stmt_, col); }
    std::string getText(int col) {
        const char* txt = reinterpret_cast<const char*>(sqlite3_column_text(stmt_, col));
        return txt ? txt : "";
    }

    // Map each row using a lambda
    void forEachRow(const std::function<void(Statement&)>& fn) {
        while(step()) fn(*this);
        reset();
    }

    // Generic row-to-struct mapping
    template<typename T>
    std::vector<T> mapRows(std::function<T(Statement&)> mapper) {
        std::vector<T> results;
        forEachRow([&](Statement& row){ results.push_back(mapper(row)); });
        return results;
    }

    // Fetch single column as vector
    template<typename T>
    std::vector<T> column(int colIndex);
};

// ---------------------------------
// Database::prepare
// ---------------------------------
inline std::unique_ptr<Statement> Database::prepare(const std::string& sql) {
    return std::make_unique<Statement>(db_, sql);
}

// ---------------------------------
// Template specializations
// ---------------------------------
template<>
inline std::vector<int> Statement::column<int>(int colIndex) {
    std::vector<int> res;
    while(step()) res.push_back(getInt(colIndex));
    reset();
    return res;
}

template<>
inline std::vector<double> Statement::column<double>(int colIndex) {
    std::vector<double> res;
    while(step()) res.push_back(getDouble(colIndex));
    reset();
    return res;
}

template<>
inline std::vector<std::string> Statement::column<std::string>(int colIndex) {
    std::vector<std::string> res;
    while(step()) res.push_back(getText(colIndex));
    reset();
    return res;
}

} // namespace rdb
