#include <iostream>
#include <pqxx/pqxx>

class DatabaseConnector {
public:
    DatabaseConnector(const std::string& dbname, const std::string& user, const std::string& password,
                      const std::string& host, int port)
        : connection_string_("dbname=" + dbname + " user=" + user + " password=" + password +
                             " hostaddr=" + host + " port=" + std::to_string(port)), is_open_(false) {}

    ~DatabaseConnector() {}

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

    // void disconnect() {
    //     // Close the connection
    //     if (connection_ && connection_->is_open()) {
    //         connection_->disconnect();
    //         std::cout << "Closed database connection" << std::endl;
    //         is_open_ = false;
    //     }
    // }

    bool isOpen() const {
        return is_open_;
    }

    // Extend this class with additional database operations as needed

private:
    std::unique_ptr<pqxx::connection> connection_;
    std::string connection_string_;
    bool is_open_;
};



int main() {
    // Example usage of DatabaseConnector
    DatabaseConnector dbConnector("database_2024", "user", "password", "172.19.0.2", 5432);

    // Connect to the database
    if (dbConnector.connect()) {
    }

    return 0;
}
