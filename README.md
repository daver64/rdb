# rdb

A modern, lightweight C++ wrapper for SQLite3 with RAII semantics and type-safe operations.

## Features

- **RAII Resource Management**: Automatic cleanup of database connections and statements
- **Exception Safety**: Custom exception handling for SQLite errors
- **Transaction Support**: RAII-based transactions with automatic rollback
- **Flexible Binding**: Support for both positional and named parameter binding
- **Type-Safe Results**: Generic row mapping with lambda support
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

```cpp
#include "include/rdb.h"
#include <iostream>

int main() {
    try {
        // Open/create database
        rdb::Database db("mydata.db");

        // Execute DDL
        db.execute("CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY, name TEXT);");

        // Insert with transaction
        {
            rdb::Database::Transaction txn(db);
            auto stmt = db.prepare("INSERT INTO users(name) VALUES(:name);");
            stmt->bind(":name", "Alice");
            while(stmt->step());
            txn.commit();
        }

        // Query with row mapping
        auto selectStmt = db.prepare("SELECT id, name FROM users;");
        selectStmt->forEachRow([](rdb::Statement& row) {
            std::cout << row.getInt(0) << ": " << row.getText(1) << "\n";
        });

    } catch(const rdb::SQLiteException& e) {
        std::cerr << "SQLite error: " << e.what() << "\n";
    }
}
```

## API Overview

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

## Example

See `example.cpp` for a complete working example demonstrating transactions, named parameters, and row mapping.

## License

MIT License - see LICENSE file for details

## Author

David Rowbotham (2025)
