#include "include/rdb.h"
#include <iostream>
#include <iomanip>

using namespace rdb;

void printSeparator() {
    std::cout << std::string(60, '-') << std::endl;
}

int main() {
    std::cout << "RDB - Complete PHP-like API Demo" << std::endl;
    printSeparator();
    
    // 1. Open database
    DBConnect db("demo.db");
    
    // 2. Create schema
    std::cout << "\n1. Creating schema..." << std::endl;
    db.query("DROP TABLE IF EXISTS products");
    db.query("DROP TABLE IF EXISTS orders");
    
    db.query(R"(
        CREATE TABLE products (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            price REAL,
            stock INTEGER
        )
    )");
    
    db.query(R"(
        CREATE TABLE orders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            product_id INTEGER,
            quantity INTEGER,
            customer TEXT
        )
    )");
    
    std::cout << "✓ Tables created" << std::endl;
    
    // 3. Insert data
    std::cout << "\n2. Inserting products..." << std::endl;
    db.query("INSERT INTO products (name, price, stock) VALUES ('Laptop', 999.99, 10)");
    db.query("INSERT INTO products (name, price, stock) VALUES ('Mouse', 29.99, 50)");
    db.query("INSERT INTO products (name, price, stock) VALUES ('Keyboard', 79.99, 30)");
    db.query("INSERT INTO products (name, price, stock) VALUES ('Monitor', 299.99, 15)");
    
    std::cout << "✓ Last product ID: " << db.last_rowid() << std::endl;
    
    // 4. Query all products
    std::cout << "\n3. Listing all products..." << std::endl;
    SQLResults products;
    db.query(&products, "SELECT * FROM products ORDER BY price DESC");
    
    std::cout << std::left 
              << std::setw(5) << "ID" 
              << std::setw(15) << "Name" 
              << std::setw(10) << "Price" 
              << std::setw(10) << "Stock" << std::endl;
    printSeparator();
    
    SQLRow row;
    while (db.fetch_array(&products, &row)) {
        std::cout << std::left
                  << std::setw(5) << row["id"]
                  << std::setw(15) << row["name"]
                  << "$" << std::setw(9) << row["price"]
                  << std::setw(10) << row["stock"] << std::endl;
    }
    
    // 5. Filtered query
    std::cout << "\n4. Products under $100..." << std::endl;
    SQLResults cheap;
    db.query(&cheap, "SELECT name, price FROM products WHERE price < 100");
    
    for (const auto& p : cheap.results) {
        std::cout << "  • " << p.at("name") << " - $" << p.at("price") << std::endl;
    }
    
    // 6. Insert with user input (demonstrating SQL escaping)
    std::cout << "\n5. Inserting product with special characters..." << std::endl;
    std::string product_name = "32\" Monitor (Bob's Edition)";
    std::string escaped_name = sql_escape(product_name);
    
    std::string sql = "INSERT INTO products (name, price, stock) VALUES ('" 
                     + escaped_name + "', 449.99, 5)";
    db.query(sql);
    std::cout << "✓ Inserted: " << product_name << " (ID: " << db.last_rowid() << ")" << std::endl;
    
    // 7. Insert orders
    std::cout << "\n6. Creating orders..." << std::endl;
    db.query("INSERT INTO orders (product_id, quantity, customer) VALUES (1, 2, 'Alice')");
    db.query("INSERT INTO orders (product_id, quantity, customer) VALUES (2, 5, 'Bob')");
    db.query("INSERT INTO orders (product_id, quantity, customer) VALUES (1, 1, 'Charlie')");
    std::cout << "✓ Orders created" << std::endl;
    
    // 8. JOIN query
    std::cout << "\n7. Order details (with JOIN)..." << std::endl;
    SQLResults orders;
    db.query(&orders, R"(
        SELECT o.id, o.customer, p.name, o.quantity, p.price,
               (o.quantity * p.price) as total
        FROM orders o
        JOIN products p ON o.product_id = p.id
        ORDER BY o.id
    )");
    
    std::cout << std::left
              << std::setw(5) << "ID"
              << std::setw(12) << "Customer"
              << std::setw(15) << "Product"
              << std::setw(6) << "Qty"
              << std::setw(10) << "Total" << std::endl;
    printSeparator();
    
    for (const auto& order : orders.results) {
        std::cout << std::left
                  << std::setw(5) << order.at("id")
                  << std::setw(12) << order.at("customer")
                  << std::setw(15) << order.at("name")
                  << std::setw(6) << order.at("quantity")
                  << "$" << order.at("total") << std::endl;
    }
    
    // 9. Aggregate query
    std::cout << "\n8. Sales summary..." << std::endl;
    SQLResults summary;
    db.query(&summary, R"(
        SELECT 
            COUNT(*) as total_orders,
            SUM(o.quantity * p.price) as total_revenue
        FROM orders o
        JOIN products p ON o.product_id = p.id
    )");
    
    if (summary.num_rows > 0) {
        const auto& stats = summary.results[0];
        std::cout << "  Total orders: " << stats.at("total_orders") << std::endl;
        std::cout << "  Total revenue: $" << stats.at("total_revenue") << std::endl;
    }
    
    // 10. Table existence check
    std::cout << "\n9. Checking tables..." << std::endl;
    std::cout << "  products table exists: " 
              << (db.does_table_exist("products") ? "✓" : "✗") << std::endl;
    std::cout << "  customers table exists: " 
              << (db.does_table_exist("customers") ? "✓" : "✗") << std::endl;
    
    // 11. Error handling
    std::cout << "\n10. Testing error handling..." << std::endl;
    SQLResults error_results;
    db.query(&error_results, "SELECT * FROM nonexistent_table");
    
    if (!error_results.error_message.empty()) {
        std::cout << "  ✓ Error caught: " << error_results.error_message << std::endl;
    }
    
    // 12. Mix with modern API
    std::cout << "\n11. Using modern API for aggregation..." << std::endl;
    auto stmt = db.getDatabase()->prepare(
        "SELECT name, stock FROM products WHERE stock < 20 ORDER BY stock"
    );
    
    std::cout << "  Low stock items:" << std::endl;
    while (stmt->step()) {
        std::cout << "    • " << stmt->getText(0) 
                  << " (only " << stmt->getInt(1) << " left)" << std::endl;
    }
    
    printSeparator();
    std::cout << "\n✓ Demo completed successfully!" << std::endl;
    
    return 0;
}
