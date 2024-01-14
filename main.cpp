#include <iostream>
#include <pqxx/pqxx>

int main() {
    try {
        // Establish a connection to the PostgreSQL database
        pqxx::connection conn("dbname=database_2024 user=user password=password hostaddr=172.19.0.2 port=5432");

        if (conn.is_open()) {
            std::cout << "Opened database successfully: " << conn.dbname() << std::endl;
        } else {
            std::cout << "Failed to open database" << std::endl;
            return 1;
        }

        // Perform database operations her
        std::cout << "Closed database connection" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
