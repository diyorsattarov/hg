#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

using json = nlohmann::json;

enum class Month {
  January = 1,
  February,
  March,
  April,
  May,
  June,
  July,
  August,
  September,
  October,
  November,
  December
};

std::unordered_map<int, std::string> monthToString{
    {1, "January"},   {2, "February"}, {3, "March"},     {4, "April"},
    {5, "May"},       {6, "June"},     {7, "July"},      {8, "August"},
    {9, "September"}, {10, "October"}, {11, "November"}, {12, "December"}};

std::string getMonthString(Month month) {
  auto it = monthToString.find(static_cast<int>(month));
  return (it != monthToString.end()) ? it->second : "Invalid Month";
}

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
        std::cout << "Failed to open database" << std::endl;
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
      // Returning an empty result in case of an error
      return pqxx::result();
    }
  }

  /* Needs to parameters table, column<vector>, value<vector> */
  // do we treat everything as strings for simplicty? most likely json is the
  // answer..
  bool executeInsert(const std::string &table,
                     const std::vector<std::string> &columns,
                     const std::vector<std::string> &values) {
    try {
      if (!is_open_) {
        std::cerr << "Database connection is not open." << std::endl;
        return false;
      }

      // Check if the number of columns and values match
      if (columns.size() != values.size()) {
        std::cerr << "Number of columns and values do not match." << std::endl;
        return false;
      }

      // Construct the SQL insert query
      std::string query = "INSERT INTO " + table + " (";

      // Append column names
      for (size_t i = 0; i < columns.size(); ++i) {
        query += columns[i];
        if (i < columns.size() - 1) {
          query += ", ";
        }
      }

      query += ") VALUES (";

      // Append values
      for (size_t i = 0; i < values.size(); ++i) {
        query += "'" + values[i] + "'";
        if (i < values.size() - 1) {
          query += ", ";
        }
      }

      query += ")";

      pqxx::work txn(*connection_);
      txn.exec(query);
      txn.commit();
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

      // Construct the SQL update query
      std::string query = "UPDATE " + table + " SET " + set_clause + " WHERE " +
                          where_clause + ";";

      pqxx::work txn(*connection_);
      txn.exec(query);
      txn.commit();
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

      // Construct the SQL delete query
      std::string query =
          "DELETE FROM " + table + " WHERE " + where_clause + ";";

      pqxx::work txn(*connection_);
      txn.exec(query);
      txn.commit();
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
};

void handle_get(const httplib::Request &req, httplib::Response &res) {
  // Extract the month from the URL
  std::string month_str = req.matches[1];
  int month = std::stoi(month_str);

  // Check if the month is valid (1 to 12)
  if (month < 1 || month > 12) {
    res.set_content("Invalid month", "text/plain");
    return;
  }

  // Create DatabaseConnector instance
  DatabaseConnector dbConnector("database_2024", "user", "password",
                                "172.19.0.2", 5432);
  // Construct the SQL select query using the provided month
  std::string query =
      "SELECT * FROM " + getMonthString(static_cast<Month>(month)) + ";";

  // Log the query for debugging
  std::cout << "Executing get query: " << query << std::endl;

  // Execute the SQL select query
  if (dbConnector.connect()) {
    pqxx::result result = dbConnector.executeQuery(query);

    // Log the result for debugging
    std::cout << "Query result: " << result.size() << " rows" << std::endl;

    // Convert the result to a nicely formatted string
    std::ostringstream formatted_result;
    for (const auto &row : result) {
      for (const auto &field : row) {
        formatted_result << field.c_str() << "\t"; // Add tab as a delimiter
      }
      formatted_result << "\n"; // Add newline after each row
    }

    // Set the response content with the nicely formatted SQL query result as
    // plaintext
    res.set_content(formatted_result.str(), "text/plain");
  } else {
    res.set_content("Failed to execute SQL select query", "text/plain");
  }
}

void handle_post(const httplib::Request &req, httplib::Response &res) {
  // Create DatabaseConnector instance
  DatabaseConnector dbConnector("database_2024", "user", "password",
                                "172.19.0.2", 5432);

  // Check if the request body is not empty and is valid JSON
  if (req.body.empty() || req.body[0] != '{') {
    res.set_content("Invalid or empty JSON data in the request body",
                    "text/plain");
    return;
  }

  try {
    // Parse the JSON data
    json data = json::parse(req.body);

    // Check if necessary parameters are present
    if (!data.contains("table") || !data.contains("columns") ||
        !data.contains("values")) {
      res.set_content(
          "Missing required parameters (table, columns, and/or values)",
          "text/plain");
      return;
    }

    std::string table = data["table"];
    std::vector<std::string> columns = data["columns"];
    std::vector<std::string> values = data["values"];

    // Execute the SQL insert query
    if (dbConnector.connect() &&
        dbConnector.executeInsert(table, columns, values)) {
      res.set_content("SQL insert query executed successfully", "text/plain");
    } else {
      res.set_content("Failed to execute SQL insert query", "text/plain");
    }
  } catch (const std::exception &e) {
    res.set_content("Error parsing JSON data: " + std::string(e.what()),
                    "text/plain");
  }
}

void handle_update(const httplib::Request &req, httplib::Response &res) {
  // Extract the month from the URL
  std::string month_str = req.matches[1];
  int month = std::stoi(month_str);

  // Check if the month is valid (1 to 12)
  if (month < 1 || month > 12) {
    res.set_content("Invalid month", "text/plain");
    return;
  }

  // Extract data from the request body
  json data = json::parse(req.body);

  // Check if necessary parameters are present
  if (!data.contains("table") || !data.contains("columns") ||
      !data.contains("values") || !data.contains("where")) {
    res.set_content(
        "Missing required parameters (table, columns, values, and/or where)",
        "text/plain");
    return;
  }

  std::string table = data["table"];
  std::vector<std::string> columns = data["columns"];
  std::vector<std::string> values = data["values"];
  std::string where_clause = data["where"];

  // Create DatabaseConnector instance
  DatabaseConnector dbConnector("database_2024", "user", "password",
                                "172.19.0.2", 5432);

  // Construct the SQL update query using the provided month and clauses
  std::string set_clause;
  for (size_t i = 0; i < columns.size(); ++i) {
    set_clause += columns[i] + "='" + values[i] + "'";
    if (i < columns.size() - 1) {
      set_clause += ", ";
    }
  }

  std::string query = "UPDATE " + getMonthString(static_cast<Month>(month)) +
                      " SET " + set_clause + " WHERE " + where_clause + ";";

  // Log the query for debugging
  std::cout << "Executing update query: " << query << std::endl;

  // Execute the SQL update query
  if (dbConnector.connect()) {
    dbConnector.executeUpdate(table, set_clause, where_clause);
    res.set_content("SQL update query executed successfully", "text/plain");
  } else {
    res.set_content("Failed to execute SQL update query", "text/plain");
  }
}

void handle_delete(const httplib::Request &req, httplib::Response &res) {
  // Extract the month from the URL
  std::string month_str = req.matches[1];
  int month = std::stoi(month_str);

  // Check if the month is valid (1 to 12)
  if (month < 1 || month > 12) {
    res.set_content("Invalid month", "text/plain");
    return;
  }

  // Extract data from the request body
  json data = json::parse(req.body);

  // Check if necessary parameters are present
  if (!data.contains("table") || !data.contains("where")) {
    res.set_content("Missing required parameters (table and/or where)",
                    "text/plain");
    return;
  }

  std::string table = data["table"];
  std::string where_clause = data["where"];

  // Create DatabaseConnector instance
  DatabaseConnector dbConnector("database_2024", "user", "password",
                                "172.19.0.2", 5432);

  // Construct the SQL delete query using the provided month and clauses
  std::string query = "DELETE FROM " +
                      getMonthString(static_cast<Month>(month)) + " WHERE " +
                      where_clause + ";";

  // Log the query for debugging
  std::cout << "Executing delete query: " << query << std::endl;

  // Execute the SQL delete query
  if (dbConnector.connect()) {
    dbConnector.executeDelete(table, where_clause);
    res.set_content("SQL delete query executed successfully", "text/plain");
  } else {
    res.set_content("Failed to execute SQL delete query", "text/plain");
  }
}

int main() {
  // Create HTTP server
  httplib::Server server;

  // Get endpoint with a placeholder for the month
  server.Get(R"(/get/(\d+))",
             [](const httplib::Request &req, httplib::Response &res) {
               handle_get(req, res);
             });

  // Post endpoint for insert
  server.Post("/insert", [](const httplib::Request &req,
                            httplib::Response &res) { handle_post(req, res); });

  // Put endpoint for update
  server.Put(R"(/update/(\d+))",
             [](const httplib::Request &req, httplib::Response &res) {
               handle_update(req, res);
             });

  server.Delete(R"(/delete/(\d+))",
                [](const httplib::Request &req, httplib::Response &res) {
                  handle_delete(req, res);
                });

  // Run the server on port 8080
  server.listen("localhost", 8080);

  return 0;
}
