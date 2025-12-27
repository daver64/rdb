# rdb

A modern, lightweight C++ wrapper for SQLite3 with **two complementary APIs**:
- **PHP-like API**: Simple, familiar interface inspired by PHP's database functions
- **Modern C++ API**: RAII semantics with type-safe operations

## Features

- **Dual API Design**: Choose the API that fits your needs - or mix both!
- **PHP-like Interface**: `query()`, `fetch_array()`, string-based results (familiar and easy)
- **RAII Resource Management**: Automatic cleanup of database connections and statements
- **Exception Safety**: Custom exception handling for SQLite errors
- **Transaction Support**: RAII-based transactions with automatic rollback
- **Flexible Binding**: Support for both positional and named parameter binding (modern API)
- **Type-Safe Results**: Generic row mapping with lambda support (modern API)
- **Modern C++**: Move semantics, smart pointers, and functional programming patterns
- **Header-Only**: Single-header library for easy integration

## Requirements

- C++11 or later
- SQLite3 library

## Installation

Simply include the header in your project:

```cpp
#include "include/rdb.h"
```

Link against SQLite3:
```bash
g++ -o example example.cpp -lsqlite3
```

## Quick Start

### PHP-Like API

Simple and familiar interface, perfect for quick scripts and migrations:

```cpp
#include "include/rdb.h"
using namespace rdb;

int main() {
    // Open database
    DBConnect db("mydata.db");
    
    // Create table
    db.query("CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)");
    
    // Insert data
    db.query("INSERT INTO users (name, age) VALUES ('Alice', 30)");
    db.query("INSERT INTO users (name, age) VALUES ('Bob', 25)");
    
    // Get last inserted row ID
    std::cout << "Last ID: " << db.last_rowid() << std::endl;
    
    // Query with results
    SQLResults results;
    db.query(&results, "SELECT * FROM users WHERE age > 25");
    
    // Iterate with fetch_array (like PHP's mysql_fetch_array)
    SQLRow row;
    while (db.fetch_array(&results, &row)) {
        std::cout << row["name"] << " is " << row["age"] << " years old\n";
    }
    
    // SQL escaping for user input
    std::string name = "O'Brien";
    std::string safe = sql_escape(name);
    db.query("INSERT INTO users (name, age) VALUES ('" + safe + "', 40)");
    
    return 0;
}
```

### Modern C++ API

RAII-based interface with prepared statements and type safety:

```cpp
#include "include/rdb.h"
using namespace rdb;

```cpp
#include "include/rdb.h"
using namespace rdb;

int main() {
    try {
        // Open/create database
        Database db("mydata.db");

        // Execute DDL
        db.execute("CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY, name TEXT);");

        // Insert with transaction and prepared statement
        {
            Database::Transaction txn(db);
            auto stmt = db.prepare("INSERT INTO users(name) VALUES(:name);");
            stmt->bind(":name", "Alice");
            stmt->step();
            txn.commit();
        }

        // Query with row mapping
        auto selectStmt = db.prepare("SELECT id, name FROM users;");
        selectStmt->forEachRow([](Statement& row) {
            std::cout << row.getInt(0) << ": " << row.getText(1) << "\n";
        });

    } catch(const SQLiteException& e) {
        std::cerr << "SQLite error: " << e.what() << "\n";
    }
}
```

## API Comparison

Choose the API that best fits your use case, or mix both!

| Feature | PHP-like API | Modern API |
|---------|--------------|------------|
| **Style** | Procedural, PHP-inspired | RAII, modern C++ |
| **Learning Curve** | Minimal (if you know PHP/SQL) | Moderate (C++ patterns) |
| **Results** | All rows fetched at once | Stream rows on-demand |
| **Memory** | Higher (stores all rows) | Lower (streaming) |
| **Type safety** | String-based (all columns are strings) | Type-safe getters |
| **SQL injection protection** | Manual escaping with `sql_escape()` | Automatic via prepared statements |
| **Transactions** | Manual SQL | RAII Transaction class |
| **Use case** | Quick scripts, migrations, simple tools | Production code, performance-critical |

### When to Use PHP-like API

- ✓ Porting from old PHP or simple database code
- ✓ Writing quick scripts or utilities
- ✓ Working with small result sets
- ✓ You prefer the simplicity of string-based column access
- ✓ Prototyping or one-off tools

### When to Use Modern API

- ✓ Writing production code
- ✓ Need better performance
- ✓ Working with large result sets
- ✓ Want compile-time type safety
- ✓ Need streaming results
- ✓ Complex queries with prepared statements

## PHP-like API Reference

### DBConnect Class

```cpp
// Constructor
DBConnect db;                          // Empty constructor
DBConnect db("database.db");          // Open database

// Opening database
db.open("database.db");

// Executing queries
db.query("CREATE TABLE ...");                    // Execute without results
db.query(&results, "SELECT * FROM users");      // Execute with results

// Fetching results
SQLResults results;
db.query(&results, "SELECT * FROM users");
SQLRow row;
while (db.fetch_array(&results, &row)) {
    std::cout << row["name"] << std::endl;      // Access by column name
}

// Or iterate directly
for (const auto& row : results.results) {
    std::cout << row.at("name") << std::endl;
}

// Other methods
int64_t id = db.last_rowid();                   // Get last insert ID
bool exists = db.does_table_exist("users");     // Check if table exists

// SQL escaping
std::string safe = sql_escape("O'Brien");       // Escape single quotes
db.query("INSERT INTO users (name) VALUES ('" + safe + "')");
```

### SQLResults Structure

```cpp
SQLResults results;
results.num_rows;          // Number of rows returned
results.num_fields;        // Number of columns
results.error_message;     // Error message if query failed
results.results;           // std::vector<SQLRow> - all rows
```

### Mixing APIs

You can use both APIs on the same database:

```cpp
// Start with PHP-like API
DBConnect db("mydata.db");
db.query("INSERT INTO users (name) VALUES ('Alice')");

// Switch to modern API for complex operations
auto modern_db = db.getDatabase();
auto stmt = modern_db->prepare("SELECT COUNT(*) FROM users WHERE age > :min");
stmt->bind(":min", 18);
stmt->step();
int count = stmt->getInt(0);
```

## Modern C++ API Reference

## Modern C++ API Reference

### Database Class

```cpp
rdb::Database db("database.db");  // Open/create database
db.execute("SQL statement");       // Execute SQL
auto stmt = db.prepare("SELECT * FROM table;");  // Prepare statement
```

### Transaction Support

```cpp
rdb::Database::Transaction txn(db);
// ... perform operations ...
txn.commit();  // Or automatic rollback on destruction
```

### Statement Binding

Positional binding:
```cpp
stmt->bind(1, 42);           // int
stmt->bind(2, 3.14);         // double
stmt->bind(3, "text");       // string
```

Named binding:
```cpp
stmt->bind(":id", 42);
stmt->bind(":name", "Alice");
```

### Reading Results

```cpp
// Manual iteration
while(stmt->step()) {
    int id = stmt->getInt(0);
    std::string name = stmt->getText(1);
}

// Lambda-based iteration
stmt->forEachRow([](rdb::Statement& row) {
    std::cout << row.getInt(0) << ": " << row.getText(1) << "\n";
});

// Map to structs
struct User { int id; std::string name; };
auto users = stmt->mapRows<User>([](rdb::Statement& row) {
    return User{ row.getInt(0), row.getText(1) };
});

// Extract single column
auto ids = stmt->column<int>(0);
auto names = stmt->column<std::string>(1);
```

## Examples

- `example.cpp` - Modern C++ API demonstration with transactions and row mapping
- `example_phplike.cpp` - PHP-like API demonstration with fetch_array and SQL escaping  
- `demo_complete.cpp` - Comprehensive demo showing real-world usage patterns

## License

MIT License - see LICENSE file for details

## Author

David Rowbotham (2025)
