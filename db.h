#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

using json = nlohmann::json;

class DatabaseConnector {
public:
  DatabaseConnector(const std::string &dbname, const std::string &user,
                    const std::string &password, const std::string &host,
                    int port)
      : connection_string_("dbname=" + dbname + " user=" + user +
                           " password=" + password + " hostaddr=" + host +
                           " port=" + std::to_string(port)),
        is_open_(false) {}

  ~DatabaseConnector() = default;

  bool connect() {
    try {
      // Establish a connection to the PostgreSQL database
      connection_ = std::make_unique<pqxx::connection>(connection_string_);

      if (connection_->is_open()) {
        std::cout << "Opened database successfully: " << connection_->dbname()
                  << std::endl;
        is_open_ = true;
        return true;
      } else {
        std::cerr << "Failed to open database" << std::endl;
        return false;
      }
    } catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  }

  bool isOpen() const { return is_open_; }

  pqxx::result executeQuery(const std::string &query) {
    try {
      pqxx::work txn(*connection_);
      return txn.exec(query);
    } catch (const std::exception &e) {
      std::cerr << "SQL error: " << e.what() << std::endl;
      return pqxx::result();
    }
  }

  bool executeInsert(const std::string &table,
                     const std::vector<std::string> &columns,
                     const std::vector<std::string> &values) {
    try {
      if (!is_open_) {
        std::cerr << "Database connection is not open." << std::endl;
        return false;
      }

      if (columns.size() != values.size()) {
        std::cerr << "Number of columns and values do not match." << std::endl;
        return false;
      }

      std::string query = "INSERT INTO " + table + " (";
      appendListToQuery(query, columns);
      query += ") VALUES (";
      appendListToQuery(query, values, true);
      query += ")";

      executeTransactionalQuery(query);
      return true;
    } catch (const std::exception &e) {
      std::cerr << "SQL error: " << e.what() << std::endl;
      return false;
    }
  }

  bool executeUpdate(const std::string &table, const std::string &set_clause,
                     const std::string &where_clause) {
    try {
      if (!is_open_) {
        std::cerr << "Database connection is not open." << std::endl;
        return false;
      }

      std::string query = "UPDATE " + table + " SET " + set_clause + " WHERE " +
                          where_clause + ";";

      executeTransactionalQuery(query);
      return true;
    } catch (const std::exception &e) {
      std::cerr << "SQL error: " << e.what() << std::endl;
      return false;
    }
  }

  bool executeDelete(const std::string &table,
                     const std::string &where_clause) {
    try {
      if (!is_open_) {
        std::cerr << "Database connection is not open." << std::endl;
        return false;
      }

      std::string query =
          "DELETE FROM " + table + " WHERE " + where_clause + ";";

      executeTransactionalQuery(query);
      return true;
    } catch (const std::exception &e) {
      std::cerr << "SQL error: " << e.what() << std::endl;
      return false;
    }
  }

private:
  std::unique_ptr<pqxx::connection> connection_;
  std::string connection_string_;
  bool is_open_;

  void appendListToQuery(std::string &query,
                         const std::vector<std::string> &items,
                         bool is_value = false) const {
    for (size_t i = 0; i < items.size(); ++i) {
      if (is_value) {
        query += "'" + items[i] + "'";
      } else {
        query += items[i];
      }

      if (i < items.size() - 1) {
        query += ", ";
      }
    }
  }

  void executeTransactionalQuery(const std::string &query) const {
    pqxx::work txn(*connection_);
    txn.exec(query);
    txn.commit();
  }
};
