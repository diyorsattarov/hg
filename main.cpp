#include <iostream>
#include <pqxx/pqxx>
#include <httplib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class DatabaseConnector {
public:
    DatabaseConnector(const std::string& dbname, const std::string& user, const std::string& password,
                      const std::string& host, int port)
        : connection_string_("dbname=" + dbname + " user=" + user + " password=" + password +
                             " hostaddr=" + host + " port=" + std::to_string(port)), is_open_(false) {}

    ~DatabaseConnector() = default;

    bool connect() {
        try {
            // Establish a connection to the PostgreSQL database
            connection_ = std::make_unique<pqxx::connection>(connection_string_);

            if (connection_->is_open()) {
                std::cout << "Opened database successfully: " << connection_->dbname() << std::endl;
                is_open_ = true;
                return true;
            } else {
                std::cout << "Failed to open database" << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return false;
        }
    }

    bool isOpen() const {
        return is_open_;
    }

    pqxx::result executeQuery(const std::string& query) {
        try {
            pqxx::work txn(*connection_);
            return txn.exec(query);
        } catch (const std::exception& e) {
            std::cerr << "SQL error: " << e.what() << std::endl;
            // Returning an empty result in case of an error
            return pqxx::result();
        }
    }

    bool executeInsert(const std::string& table, int day_id, const std::string& task) {
        try {
            if (!is_open_) {
                std::cerr << "Database connection is not open." << std::endl;
                return false;
            }

            // Construct the SQL insert query
            std::string query = "INSERT INTO " + table + " (day_id, task) VALUES (" + std::to_string(day_id) + ", '" + task + "')";

            pqxx::work txn(*connection_);
            txn.exec(query);
            txn.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "SQL error: " << e.what() << std::endl;
            return false;
        }
    }

private:
    std::unique_ptr<pqxx::connection> connection_;
    std::string connection_string_;
    bool is_open_;
};

void handle_get(const httplib::Request& req, httplib::Response& res) {
    // Create DatabaseConnector instance
    DatabaseConnector dbConnector("database_2024", "user", "password", "172.19.0.2", 5432);

    // Construct the SQL select query for the month of January
    std::string query = "SELECT * FROM JANUARY;";

    // Log the query for debugging
    std::cout << "Executing get query: " << query << std::endl;

    // Execute the SQL select query
    if (dbConnector.connect()) {
        pqxx::result result = dbConnector.executeQuery(query);

        // Log the result for debugging
        std::cout << "Query result: " << result.size() << " rows" << std::endl;

        // Convert the result to a string
        std::string result_str;
        for (const auto& row : result) {
            for (const auto& field : row) {
                result_str += field.c_str();
                result_str += "\t";  // Add tab as a delimiter
            }
            result_str += "\n";  // Add newline after each row
        }

        // Set the response content with the SQL query result as plaintext
        res.set_content(result_str, "text/plain");
    } else {
        res.set_content("Failed to execute SQL select query", "text/plain");
    }
}

void handle_post(const httplib::Request& req, httplib::Response& res) {
    // Create DatabaseConnector instance
    DatabaseConnector dbConnector("database_2024", "user", "password", "172.19.0.2", 5432);

    // Check if the request body is not empty and is valid JSON
    if (req.body.empty() || req.body[0] != '{') {
        res.set_content("Invalid or empty JSON data in the request body", "text/plain");
        return;
    }

    try {
        // Parse the JSON data
        json data = json::parse(req.body);

        // Check if necessary parameters are present
        if (!data.contains("table") || !data.contains("day_id") || !data.contains("task")) {
            res.set_content("Missing required parameters (table, day_id, and/or task)", "text/plain");
            return;
        }

        std::string table = data["table"];
        int day_id = data["day_id"];
        std::string task = data["task"];

        // Execute the SQL insert query
        if (dbConnector.connect() && dbConnector.executeInsert(table, day_id, task)) {
            res.set_content("SQL insert query executed successfully", "text/plain");
        } else {
            res.set_content("Failed to execute SQL insert query", "text/plain");
        }
    } catch (const std::exception& e) {
        res.set_content("Error parsing JSON data: " + std::string(e.what()), "text/plain");
    }
}

int main() {
    // Create HTTP server
    httplib::Server server;

    // Get endpoint
    server.Get("/get", [](const httplib::Request& req, httplib::Response& res) {
        handle_get(req, res);
    });

    server.Post("/insert", [](const httplib::Request& req, httplib::Response& res) {
        handle_post(req, res);
    });

    // Run the server on port 8080
    server.listen("localhost", 8080);

    return 0;
}
