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

    ```
    http://localhost:8080/get/1
    ```

    ```
    some task	2024-01-14 06:33:00	2024-01-14 06:35:00	14	1
    ```

- **POST /insert**: Insert data into the database. Requires a JSON payload with the `table`, `columns`, and `values` parameters.
    ```
    /insert/{month} (POST)
    ```

    ```
    {
        "columns": ["day_id", "task"],
        "values": ["14", "another_task"]
    }
    ```

    ```
    SQL insert query executed successfully
    ```

- **PUT /update/{month}**: Update data in the database for a specific month. Requires a JSON payload with the `table`, `columns`, `values`, and `where` parameters.

    ```
    http://localhost:8080/update/1
    ```

    ```
    {
        "columns": ["task"],           // Specify the column you want to update
        "values": ["updated_value"],   // Specify the new value
        "where": "task_id=12"          // Specify the condition for the update
    }
    ```

    ```
    SQL update query executed successfully
    ```

- **DELETE /delete/{month}**: Delete data from the database for a specific month. Requires a JSON payload with the `table` and `where` parameters.
    ```
    http://localhost:8080/delete/1
    ```

    ```
    {
        "where": "task_id=11"
    }
    ```

    ```
    SQL delete query executed successfully
    ```
## Dependencies:

- [httplib](https://github.com/yhirose/cpp-httplib): C++ HTTP library for handling HTTP requests and responses.

- [nlohmann/json](https://github.com/nlohmann/json): JSON library for C++.

- [pqxx](https://github.com/jtv/libpqxx): C++ library for interacting with PostgreSQL databases.

- [docker](https://github.com/docker): Docker is an open platform for developing, shipping, and running applications

- [vue](https://github.com/vuejs/core): Vue.js is a progressive, incrementally-adoptable JavaScript framework for building UI on the web.

- [express](https://github.com/expressjs/express): Fast, unopinionated, minimalist web framework for node.

## Note:

Ensure to customize the database connection details and handle security considerations, such as input validation and authentication, based on your specific use case and deployment environment.