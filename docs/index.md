# rdb API documentation

Welcome to the API reference for `rdb`, a lightweight C++ database wrapper for SQLite with optional PostgreSQL and MySQL/MariaDB support.

## Overview

This project provides two main layers:

- a modern RAII-based C++ API with prepared statements, transactions, and type-safe result access
- a PHP-like compatibility layer that mirrors familiar query/fetch patterns for quick scripting and migration work

The library is intentionally compact and header-friendly, making it easy to drop into small tools, prototypes, and embedded database access layers.

## Main API areas

### Modern SQLite API

- @ref rdb::Database
- @ref rdb::Statement
- @ref rdb::SQLiteException
- @ref rdb::SQLResults
- @ref rdb::DBConnect

### Unified cross-database interface

- @ref rdb::IDatabase
- @ref rdb::IStatement
- @ref rdb::BackendType
- @ref rdb::UnifiedException
- @ref rdb::SQLiteUnifiedDatabase
- @ref rdb::PostgreSQLUnifiedDatabase
- @ref rdb::MySQLUnifiedDatabase

### Driver adapters

- @ref rdb::PostgreSQLDBConnect
- @ref rdb::MySQLDBConnect
- @ref rdb::MariaDBDBConnect

## Key functions

- @ref rdb::sql_escape
- @ref rdb::makeSQLiteDatabase
- @ref rdb::makePostgreSQLDatabase
- @ref rdb::makeMySQLDatabase
- @ref rdb::makeMariaDBDatabase

## Source files

- @ref rdb.h
- @ref rdb_drivers.h
- @ref rdb_unified.h
- @ref example.cpp
- @ref example_phplike.cpp
- @ref demo_complete.cpp
- @ref README.md

## Examples

The repository includes several examples that demonstrate both the modern API and the PHP-like API:

- @ref example.cpp — modern RAII transaction and row-mapping pattern
- @ref example_phplike.cpp — compatibility style query/fetch workflow
- @ref demo_complete.cpp — complete end-to-end database demo

## Quick links

- [Project README](../README.html)
- [Classes](annotated.html)
- [Files](files.html)
- [Functions](functions.html)
- [Namespaces](namespaces.html)
- [Graph overview](graph_legend.html)

## Notes

The generated documentation is intended to help navigation across the public API surface and the examples, with automatic indexing for classes, functions, files, and types from the project sources.
