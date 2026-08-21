#pragma once

#include "rdb.h"
#include <cstdint>
#include <stdexcept>
#include <string>

#if defined(RDB_ENABLE_POSTGRESQL)
#include <libpq-fe.h>
#endif
#if defined(RDB_ENABLE_MYSQL)
#include <mysql.h>
#endif

namespace rdb {

#if defined(RDB_ENABLE_POSTGRESQL)
class PostgreSQLDBConnect {
    PGconn* connection_ = nullptr;
    void checkConnection() const {
        if (!connection_ || PQstatus(connection_) != CONNECTION_OK)
            throw std::runtime_error(connection_ ? PQerrorMessage(connection_) : "PostgreSQL connection is not open");
    }
public:
    explicit PostgreSQLDBConnect(const std::string& conninfo) { open(conninfo); }
    ~PostgreSQLDBConnect() { if (connection_) PQfinish(connection_); }
    PostgreSQLDBConnect(const PostgreSQLDBConnect&) = delete;
    PostgreSQLDBConnect& operator=(const PostgreSQLDBConnect&) = delete;
    void open(const std::string& conninfo) {
        if (connection_) PQfinish(connection_);
        connection_ = PQconnectdb(conninfo.c_str());
        if (!connection_ || PQstatus(connection_) != CONNECTION_OK) {
            const std::string error = connection_ ? PQerrorMessage(connection_) : "PostgreSQL initialization failed";
            if (connection_) PQfinish(connection_);
            connection_ = nullptr;
            throw std::runtime_error(error);
        }
    }
    void query(SQLResults* results, const std::string& sql) {
        results->clear();
        checkConnection();
        PGresult* result = PQexec(connection_, sql.c_str());
        if (!result || PQresultStatus(result) != PGRES_TUPLES_OK) {
            results->error_message = result ? PQresultErrorMessage(result) : PQerrorMessage(connection_);
            if (result) PQclear(result);
            return;
        }
        const int fields = PQnfields(result);
        results->num_fields = static_cast<size_t>(fields);
        for (int row_index = 0; row_index < PQntuples(result); ++row_index) {
            SQLRow row;
            for (int field = 0; field < fields; ++field)
                row[PQfname(result, field)] = PQgetisnull(result, row_index, field) ? "" : PQgetvalue(result, row_index, field);
            results->results.push_back(row);
        }
        results->num_rows = results->results.size();
        results->num_tuples = results->num_rows;
        results->row_iterator = results->results.begin();
        PQclear(result);
    }
    void query(SQLResults* results, const char* sql) { query(results, std::string(sql)); }
    void query(const std::string& sql) {
        checkConnection();
        PGresult* result = PQexec(connection_, sql.c_str());
        if (!result || (PQresultStatus(result) != PGRES_COMMAND_OK && PQresultStatus(result) != PGRES_TUPLES_OK)) {
            const std::string error = result ? PQresultErrorMessage(result) : PQerrorMessage(connection_);
            if (result) PQclear(result);
            throw std::runtime_error(error);
        }
        PQclear(result);
    }
    void query(const char* sql) { query(std::string(sql)); }
    bool fetch_array(SQLResults* results, SQLRow* row) {
        if (results->row_iterator == results->results.end()) return false;
        *row = *results->row_iterator++;
        return true;
    }
    int64_t last_rowid() { return 0; }
    bool does_table_exist(const std::string& table_name) {
        SQLResults results;
        query(&results, "SELECT 1 FROM information_schema.tables WHERE table_schema = current_schema() AND table_name = '" + sql_escape(table_name) + "'");
        return results.error_message.empty() && results.num_rows != 0;
    }
};
#else
class PostgreSQLDBConnect {
public:
    explicit PostgreSQLDBConnect(const std::string&) { unavailable(); }
    void open(const std::string&) { unavailable(); }
private:
    static void unavailable() { throw std::runtime_error("PostgreSQL support requires RDB_ENABLE_POSTGRESQL and libpq"); }
};
#endif

// MariaDB exposes the MySQL client protocol and uses the same C client API.
#if defined(RDB_ENABLE_MYSQL)
class MySQLDBConnect {
    MYSQL* connection_ = nullptr;
    void checkConnection() const {
        if (!connection_) throw std::runtime_error("MySQL connection is not open");
    }
    void executeQuery(SQLResults* results, const std::string& sql) {
        results->clear();
        checkConnection();
        if (mysql_query(connection_, sql.c_str()) != 0) {
            results->error_message = mysql_error(connection_);
            return;
        }
        MYSQL_RES* result = mysql_store_result(connection_);
        if (!result) {
            if (mysql_field_count(connection_) != 0) results->error_message = mysql_error(connection_);
            return;
        }
        MYSQL_FIELD* fields = mysql_fetch_fields(result);
        results->num_fields = mysql_num_fields(result);
        MYSQL_ROW values;
        while ((values = mysql_fetch_row(result))) {
            SQLRow row;
            for (unsigned int field = 0; field < results->num_fields; ++field)
                row[fields[field].name] = values[field] ? values[field] : "";
            results->results.push_back(row);
        }
        results->num_rows = results->results.size();
        results->num_tuples = results->num_rows;
        results->row_iterator = results->results.begin();
        mysql_free_result(result);
    }
public:
    MySQLDBConnect(const std::string& host, const std::string& user, const std::string& password,
                   const std::string& database, unsigned int port = 0) { open(host, user, password, database, port); }
    ~MySQLDBConnect() { if (connection_) mysql_close(connection_); }
    MySQLDBConnect(const MySQLDBConnect&) = delete;
    MySQLDBConnect& operator=(const MySQLDBConnect&) = delete;
    void open(const std::string& host, const std::string& user, const std::string& password,
              const std::string& database, unsigned int port = 0) {
        if (connection_) mysql_close(connection_);
        connection_ = mysql_init(nullptr);
        if (!connection_ || !mysql_real_connect(connection_, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0)) {
            const std::string error = connection_ ? mysql_error(connection_) : "MySQL initialization failed";
            if (connection_) mysql_close(connection_);
            connection_ = nullptr;
            throw std::runtime_error(error);
        }
    }
    void query(SQLResults* results, const std::string& sql) { executeQuery(results, sql); }
    void query(SQLResults* results, const char* sql) { query(results, std::string(sql)); }
    void query(const std::string& sql) {
        checkConnection();
        if (mysql_query(connection_, sql.c_str()) != 0) throw std::runtime_error(mysql_error(connection_));
    }
    void query(const char* sql) { query(std::string(sql)); }
    bool fetch_array(SQLResults* results, SQLRow* row) {
        if (results->row_iterator == results->results.end()) return false;
        *row = *results->row_iterator++;
        return true;
    }
    int64_t last_rowid() { return connection_ ? static_cast<int64_t>(mysql_insert_id(connection_)) : 0; }
    bool does_table_exist(const std::string& table_name) {
        SQLResults results;
        query(&results, "SELECT 1 FROM information_schema.tables WHERE table_schema = DATABASE() AND table_name = '" + sql_escape(table_name) + "'");
        return results.error_message.empty() && results.num_rows != 0;
    }
};
using MariaDBDBConnect = MySQLDBConnect;
#else
class MySQLDBConnect {
public:
    MySQLDBConnect(const std::string&, const std::string&, const std::string&, const std::string&, unsigned int = 0) { unavailable(); }
private:
    static void unavailable() { throw std::runtime_error("MySQL/MariaDB support requires RDB_ENABLE_MYSQL and a MySQL-compatible client library"); }
};
using MariaDBDBConnect = MySQLDBConnect;
#endif

} // namespace rdb