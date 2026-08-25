/**
 * @file example.cpp
 * @brief Demonstrates the modern RAII-based SQLite API with transactions and row mapping.
 */

#include "include/rdb.h"
#include <iostream>

/**
 * @brief Simple record used to map query results into a C++ struct.
 */
struct Player {
    int id;
    std::string name;
};

/**
 * @brief Example ship model used for demonstration purposes.
 */
struct Ship {
    int id;
    std::string type;
};

/**
 * @brief Runs the example program.
 * @return 0 on success, non-zero on failure.
 */
int main() {
    try {
        rdb::Database db("fleet.db");

        db.execute("CREATE TABLE IF NOT EXISTS players(id INTEGER PRIMARY KEY, name TEXT);");
        db.execute("CREATE TABLE IF NOT EXISTS ships(id INTEGER PRIMARY KEY, type TEXT);");

        // Insert using transaction
        {
            rdb::Database::Transaction txn(db);
            auto stmt = db.prepare("INSERT INTO players(name) VALUES(:name);");
            stmt->bind(":name", "Alice"); while(stmt->step());
            stmt->reset();
            stmt->bind(":name", "Bob"); while(stmt->step());
            txn.commit();
        }

        // Generic query mapping
        auto selectPlayers = db.prepare("SELECT id, name FROM players;");
        std::vector<Player> players = selectPlayers->mapRows<Player>(
            [](rdb::Statement& row){
                return Player{ row.getInt(0), row.getText(1) };
            }
        );

        for(auto& p : players)
            std::cout << p.id << ": " << p.name << "\n";

    } catch(const rdb::SQLiteException& e) {
        std::cerr << "SQLite error: " << e.what() << "\n";
    }
}
