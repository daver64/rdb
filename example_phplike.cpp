#include "include/rdb.h"
#include <iostream>

using namespace rdb;

int main() {
    std::cout << "=== PHP-like API Example ===" << std::endl;
    
    // Open database using PHP-like API
    DBConnect db("test_phplike.db");
    
    // Create table
    db.query("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)");
    
    // Insert some data
    db.query("DELETE FROM users");  // Clear existing data
    db.query("INSERT INTO users (name, age) VALUES ('Alice', 30)");
    db.query("INSERT INTO users (name, age) VALUES ('Bob', 25)");
    db.query("INSERT INTO users (name, age) VALUES ('Charlie', 35)");
    
    std::cout << "Last inserted row ID: " << db.last_rowid() << std::endl;
    
    // Query with results (PHP-like)
    SQLResults results;
    db.query(&results, "SELECT * FROM users WHERE age > 25");
    
    std::cout << "Found " << results.num_rows << " rows with " 
              << results.num_fields << " fields" << std::endl;
    
    // Method 1: Iterate using fetch_array (like PHP's mysql_fetch_array)
    std::cout << "\n--- Using fetch_array() ---" << std::endl;
    SQLRow row;
    while (db.fetch_array(&results, &row)) {
        std::cout << "ID: " << row["id"] 
                  << ", Name: " << row["name"] 
                  << ", Age: " << row["age"] << std::endl;
    }
    
    // Method 2: Iterate directly over results
    std::cout << "\n--- Direct iteration ---" << std::endl;
    for (const auto& r : results.results) {
        std::cout << "ID: " << r.at("id") 
                  << ", Name: " << r.at("name") 
                  << ", Age: " << r.at("age") << std::endl;
    }
    
    // Using SQL escape function
    std::string unsafe_input = "O'Brien";
    std::string safe = sql_escape(unsafe_input);
    std::cout << "\n--- SQL Escape ---" << std::endl;
    std::cout << "Original: " << unsafe_input << std::endl;
    std::cout << "Escaped: " << safe << std::endl;
    
    std::string sql = "INSERT INTO users (name, age) VALUES ('" + safe + "', 40)";
    db.query(sql);
    
    // Check if table exists
    std::cout << "\n--- Table exists check ---" << std::endl;
    std::cout << "Table 'users' exists: " << (db.does_table_exist("users") ? "yes" : "no") << std::endl;
    std::cout << "Table 'foo' exists: " << (db.does_table_exist("foo") ? "yes" : "no") << std::endl;
    
    // You can still access the modern API through getDatabase()
    std::cout << "\n=== Mixing with Modern API ===" << std::endl;
    auto stmt = db.getDatabase()->prepare("SELECT COUNT(*) FROM users");
    if (stmt->step()) {
        std::cout << "Total users: " << stmt->getInt(0) << std::endl;
    }
    
    return 0;
}
