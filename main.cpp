#include "include/db.h"
#include <httplib.h>
#include <iostream>

const std::string kDbName = "database_2024";
const std::string kDbUser = "user";
const std::string kDbPassword = "password";
const std::string kDbHost = "172.19.0.2";
const int kDbPort = 5432;

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
    {1, "JANUARY"},   {2, "FEBRUARY"}, {3, "MARCH"},     {4, "APRIL"},
    {5, "MAY"},       {6, "JUNE"},     {7, "JULY"},      {8, "AUGUST"},
    {9, "SEPTEMBER"}, {10, "OCTOBER"}, {11, "NOVEMBER"}, {12, "DECEMBER"}};

std::string getMonthString(Month month) {
  auto it = monthToString.find(static_cast<int>(month));
  return (it != monthToString.end()) ? it->second : "Invalid Month";
}

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
  DatabaseConnector dbConnector(kDbName, kDbUser, kDbPassword, kDbHost,
                                kDbPort);
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
  // Extract the month from the URL
  std::string month_str = req.matches[1];
  int month = std::stoi(month_str);

  // Check if the month is valid (1 to 12)
  if (month < 1 || month > 12) {
    res.set_content("Invalid month", "text/plain");
    return;
  }

  // Create DatabaseConnector instance
  DatabaseConnector dbConnector(kDbName, kDbUser, kDbPassword, kDbHost,
                                kDbPort);

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
    if (!data.contains("columns") ||
        !data.contains("values")) {
      res.set_content(
          "Missing required parameters (columns, and/or values)",
          "text/plain");
      return;
    }
    
    std::string table =
    getMonthString(static_cast<Month>(month));
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
  if (!data.contains("columns") ||
      !data.contains("values") || !data.contains("where")) {
    res.set_content(
        "Missing required parameters (table, columns, values, and/or where)",
        "text/plain");
    return;
  }

  std::string table =
    getMonthString(static_cast<Month>(month));
  std::vector<std::string> columns = data["columns"];
  std::vector<std::string> values = data["values"];
  std::string where_clause = data["where"];

  // Create DatabaseConnector instance
  DatabaseConnector dbConnector(kDbName, kDbUser, kDbPassword, kDbHost,
                                kDbPort);

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
  if (!data.contains("where")) {
    res.set_content("Missing required parameters (table and/or where)",
                    "text/plain");
    return;
  }

  std::string table = getMonthString(static_cast<Month>(month));
  std::string where_clause = data["where"];

  // Create DatabaseConnector instance
  DatabaseConnector dbConnector(kDbName, kDbUser, kDbPassword, kDbHost,
                                kDbPort);

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

  // Post endpoint for insert with integer month
  server.Post(R"(/insert/(\d+))",
              [](const httplib::Request &req, httplib::Response &res) {
                handle_post(req, res);
              });

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
