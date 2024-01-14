# http-gres

## Description

This repository contains a simple HTTP server application named "http-gres" that interacts with a PostgreSQL database. The application is built using the httplib library for handling HTTP requests and responses, and it includes functionalities for performing database operations such as SELECT, INSERT, UPDATE, and DELETE.

## Usage

To use this application, follow these steps:

1. Ensure you have a PostgreSQL database running.
1. Update the connection details in the DatabaseConnector constructor within main.cpp.
1. Build the application using an appropriate compiler.
1. Run the compiled executable.

## Endpoints:

- **GET /get/{month}**: Retrieve data for a specific month from the database.

- **POST /insert**: Insert data into the database. Requires a JSON payload with the `table`, `columns`, and `values` parameters.

- **PUT /update/{month}**: Update data in the database for a specific month. Requires a JSON payload with the `table`, `columns`, `values`, and `where` parameters.

- **DELETE /delete/{month}**: Delete data from the database for a specific month. Requires a JSON payload with the `table` and `where` parameters.

## Dependencies:

- [httplib](https://github.com/yhirose/cpp-httplib): C++ HTTP library for handling HTTP requests and responses.

- [nlohmann/json](https://github.com/nlohmann/json): JSON library for C++.

- [pqxx](https://github.com/jtv/libpqxx): C++ library for interacting with PostgreSQL databases.

## Note:

Ensure to customize the database connection details and handle security considerations, such as input validation and authentication, based on your specific use case and deployment environment.