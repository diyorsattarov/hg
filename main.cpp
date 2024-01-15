#include "include/db.h"
#include "include/util.h"
#include <httplib.h>
#include <iostream>
#include <unordered_map>

std::unordered_map<int, std::string> monthToString{
    {1, "JANUARY"},   {2, "FEBRUARY"}, {3, "MARCH"},     {4, "APRIL"},
    {5, "MAY"},       {6, "JUNE"},     {7, "JULY"},      {8, "AUGUST"},
    {9, "SEPTEMBER"}, {10, "OCTOBER"}, {11, "NOVEMBER"}, {12, "DECEMBER"}};

std::string getMonthString(Month month) {
  auto it = monthToString.find(static_cast<int>(month));
  return (it != monthToString.end()) ? it->second : "Invalid Month";
}

void handle_request(const httplib::Request &req, httplib::Response &res) {
  std::string month_str = req.matches[1];
  int month = std::stoi(month_str);

  if (month < 1 || month > 12) {
    res.set_content("Invalid month", "text/plain");
    return;
  }

  DatabaseConnector dbConnector(kDbName, kDbUser, kDbPassword, kDbHost,
                                kDbPort);

 if (req.method == "GET") {
    // Handle GET request
    std::string query =
        "SELECT * FROM " + getMonthString(static_cast<Month>(month)) + ";";

    if (dbConnector.connect()) {
        // Fetch both column names and data
        pqxx::result result = dbConnector.executeQuery(query);
        json jsonResult;

        // Check if there are no rows returned
        if (result.size() == 0) {
            res.set_content("No elements found", "text/plain");
            return;
        }

        // Extract column names
        json columnNames;
        for (size_t i = 0; i < result.columns(); ++i) {
            columnNames.push_back(result.column_name(i));
        }
        jsonResult["columns"] = columnNames;

        // Extract data rows
        json dataRows;
        for (const auto &row : result) {
            json jsonRow;
            for (size_t i = 0; i < row.size(); ++i) {
                jsonRow[result.column_name(i)] = row[i].c_str();
            }
            dataRows.push_back(jsonRow);
        }
        jsonResult["data"] = dataRows;

        res.set_content(jsonResult.dump(), "application/json");
    } else {
        res.set_content("Failed to execute SQL select query", "text/plain");
    }
} else if (req.method == "POST") {
    // Handle POST request
    if (req.body.empty() || req.body[0] != '{') {
      res.set_content("Invalid or empty JSON data in the request body",
                      "text/plain");
      return;
    }

    try {
      json data = json::parse(req.body);

      if (!data.contains("columns") || !data.contains("values")) {
        res.set_content("Missing required parameters (columns, and/or values)",
                        "text/plain");
        return;
      }

      std::string table = getMonthString(static_cast<Month>(month));
      std::vector<std::string> columns = data["columns"];
      std::vector<std::string> values = data["values"];

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
  } else if (req.method == "PUT") {
    // Handle PUT (update) request inline
    json data = json::parse(req.body);

    if (!data.contains("columns") || !data.contains("values") ||
        !data.contains("where")) {
      res.set_content(
          "Missing required parameters (table, columns, values, and/or where)",
          "text/plain");
      return;
    }

    std::string table = getMonthString(static_cast<Month>(month));
    std::vector<std::string> columns = data["columns"];
    std::vector<std::string> values = data["values"];
    std::string where_clause = data["where"];

    std::string set_clause;
    for (size_t i = 0; i < columns.size(); ++i) {
      set_clause += columns[i] + "='" + values[i] + "'";
      if (i < columns.size() - 1) {
        set_clause += ", ";
      }
    }

    if (dbConnector.connect()) {
      dbConnector.executeUpdate(table, set_clause, where_clause);
      res.set_content("SQL update query executed successfully", "text/plain");
    } else {
      res.set_content("Failed to execute SQL update query", "text/plain");
    }
  } else if (req.method == "DELETE") {
    // Handle DELETE request inline
    json data = json::parse(req.body);

    if (!data.contains("where")) {
      res.set_content("Missing required parameters (table and/or where)",
                      "text/plain");
      return;
    }

    std::string table = getMonthString(static_cast<Month>(month));
    std::string where_clause = data["where"];

    std::string query = "DELETE FROM " +
                        getMonthString(static_cast<Month>(month)) + " WHERE " +
                        where_clause + ";";

    if (dbConnector.connect()) {
      dbConnector.executeDelete(table, where_clause);
      res.set_content("SQL delete query executed successfully", "text/plain");
    } else {
      res.set_content("Failed to execute SQL delete query", "text/plain");
    }
  } else {
    // Unsupported HTTP method
    res.set_content("Unsupported HTTP method", "text/plain");
  }
}

int main() {
  httplib::Server server;

  server.Post(kInsertEndpoint,
              [](const httplib::Request &req, httplib::Response &res) {
                handle_request(req, res);
              });

  server.Get(kGetEndpoint,
             [](const httplib::Request &req, httplib::Response &res) {
               handle_request(req, res);
             });

  server.Put(kUpdateEndpoint,
             [](const httplib::Request &req, httplib::Response &res) {
               handle_request(req, res);
             });

  server.Delete(kDeleteEndpoint,
              [](const httplib::Request &req, httplib::Response &res) {
                handle_request(req, res);
              });

  server.listen("localhost", 8080);

  return 0;
}
