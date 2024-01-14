#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <boost/exception/to_string.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <pqxx/pqxx>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include "include/db.h"
#include "include/util.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

std::unordered_map<int, std::string> monthToString{
    {1, "JANUARY"},   {2, "FEBRUARY"}, {3, "MARCH"},     {4, "APRIL"},
    {5, "MAY"},       {6, "JUNE"},     {7, "JULY"},      {8, "AUGUST"},
    {9, "SEPTEMBER"}, {10, "OCTOBER"}, {11, "NOVEMBER"}, {12, "DECEMBER"}};

std::string getMonthString(Month month) {
  auto it = monthToString.find(static_cast<int>(month));
  return (it != monthToString.end()) ? it->second : "Invalid Month";
}

void handle_request(const http::request<http::string_body> &req,
                    http::response<http::string_body> &res) {
  std::string month_str(req.target().data(), req.target().size());
  month_str.erase(0, 1); // remove the leading '/'
  int month = std::stoi(month_str);

  if (month < 1 || month > 12) {
    res.result(http::status::bad_request);
    res.set(http::field::content_type, "text/plain");
    res.body() = "Invalid month";
    return;
  }

  DatabaseConnector dbConnector(kDbName, kDbUser, kDbPassword, kDbHost,
                                kDbPort);

  if (req.method() == http::verb::get) {
    // Handle GET request
    std::string query =
        "SELECT * FROM " + getMonthString(static_cast<Month>(month)) + ";";

    if (dbConnector.connect()) {
      // Fetch both column names and data
      pqxx::result result = dbConnector.executeQuery(query);
      json jsonResult;

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

      res.result(http::status::ok);
      res.set(http::field::content_type, "application/json");
      res.body() = jsonResult.dump();
    } else {
      res.result(http::status::internal_server_error);
      res.set(http::field::content_type, "text/plain");
      res.body() = "Failed to execute SQL select query";
    }
  } else if (req.method() == http::verb::post) {
    // Handle POST request
    if (req.body().empty() || req.body()[0] != '{') {
      res.result(http::status::bad_request);
      res.set(http::field::content_type, "text/plain");
      res.body() = "Invalid or empty JSON data in the request body";
      return;
    }

    try {
      json data = json::parse(req.body());

      if (!data.contains("columns") || !data.contains("values")) {
        res.result(http::status::bad_request);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Missing required parameters (columns, and/or values)";
        return;
      }

      std::string table = getMonthString(static_cast<Month>(month));
      std::vector<std::string> columns = data["columns"];
      std::vector<std::string> values = data["values"];

      if (dbConnector.connect() &&
          dbConnector.executeInsert(table, columns, values)) {
        res.result(http::status::ok);
        res.set(http::field::content_type, "text/plain");
        res.body() = "SQL insert query executed successfully";
      } else {
        res.result(http::status::internal_server_error);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Failed to execute SQL insert query";
      }
    } catch (const std::exception &e) {
      res.result(http::status::bad_request);
      res.set(http::field::content_type, "text/plain");
      res.body() = "Error parsing JSON data: " + std::string(e.what());
    }
  } else if (req.method() == http::verb::put) {
    // Handle PUT (update) request inline
    json data = json::parse(req.body());

    if (!data.contains("columns") || !data.contains("values") ||
        !data.contains("where")) {
      res.result(http::status::bad_request);
      res.set(http::field::content_type, "text/plain");
      res.body() =
          "Missing required parameters (table, columns, values, and/or where)";
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
      res.result(http::status::ok);
      res.set(http::field::content_type, "text/plain");
      res.body() = "SQL update query executed successfully";
    } else {
      res.result(http::status::internal_server_error);
      res.set(http::field::content_type, "text/plain");
      res.body() = "Failed to execute SQL update query";
    }
  } else if (req.method() == http::verb::delete_) {
    // Handle DELETE request inline
    json data = json::parse(req.body());

    if (!data.contains("where")) {
      res.result(http::status::bad_request);
      res.set(http::field::content_type, "text/plain");
      res.body() = "Missing required parameters (table and/or where)";
      return;
    }

    std::string table = getMonthString(static_cast<Month>(month));
    std::string where_clause = data["where"];

    std::string query = "DELETE FROM " +
                        getMonthString(static_cast<Month>(month)) + " WHERE " +
                        where_clause + ";";

    if (dbConnector.connect()) {
      dbConnector.executeDelete(table, where_clause);
      res.result(http::status::ok);
      res.set(http::field::content_type, "text/plain");
      res.body() = "SQL delete query executed successfully";
    } else {
      res.result(http::status::internal_server_error);
      res.set(http::field::content_type, "text/plain");
      res.body() = "Failed to execute SQL delete query";
    }
  } else {
    // Unsupported HTTP method
    res.result(http::status::bad_request);
    res.set(http::field::content_type, "text/plain");
    res.body() = "Unsupported HTTP method";
  }
}

// Report a failure
void fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session> {
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  std::shared_ptr<std::string const> doc_root_;
  http::request<http::string_body> req_;

public:
  // Take ownership of the stream
  session(tcp::socket &&socket,
          std::shared_ptr<std::string const> const &doc_root)
      : stream_(std::move(socket)), doc_root_(doc_root) {}

  // Start the asynchronous operation
  void run() {
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(&session::do_read, shared_from_this()));
  }

  void do_read() {
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    req_ = {};

    // Set the timeout.
    stream_.expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(
        stream_, buffer_, req_,
        beast::bind_front_handler(&session::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    http::response<http::string_body> res;
    // This means they closed the connection
    if (ec == http::error::end_of_stream)
      return do_close();

    if (ec)
      return fail(ec, "read");

    // Send the response
    handle_request(req_, res);
    send_response(std::move(res));
  }

  void send_response(http::response<http::string_body> &&res) {
    bool keep_alive = res.keep_alive();

    // Write the response
    http::async_write(stream_, std::move(res),
                      beast::bind_front_handler(
                          &session::on_write, shared_from_this(), keep_alive));
  }

  void on_write(bool keep_alive, beast::error_code ec,
                std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    if (!keep_alive) {
      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      return do_close();
    }

    // Read another request
    do_read();
  }

  void do_close() {
    // Send a TCP shutdown
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
  }
};

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener> {
  net::io_context &ioc_;
  tcp::acceptor acceptor_;
  std::shared_ptr<std::string const> doc_root_;

public:
  listener(net::io_context &ioc, tcp::endpoint endpoint,
           std::shared_ptr<std::string const> const &doc_root)
      : ioc_(ioc), acceptor_(net::make_strand(ioc)), doc_root_(doc_root) {
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
      fail(ec, "open");
      return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
      fail(ec, "set_option");
      return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
      fail(ec, "bind");
      return;
    }

    // Start listening for connections
    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
      fail(ec, "listen");
      return;
    }
  }

  // Start accepting incoming connections
  void run() { do_accept(); }

private:
  void do_accept() {
    // The new connection gets its own strand
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(&listener::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
      fail(ec, "accept");
      return; // To avoid infinite loop
    } else {
      // Create the session and run it
      std::make_shared<session>(std::move(socket), doc_root_)->run();
    }

    // Accept another connection
    do_accept();
  }
};

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 5)
    {
        std::cerr <<
            "Usage: http-server-async <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    http-server-async 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const doc_root = std::make_shared<std::string>(argv[3]);
    auto const threads = std::max<int>(1, std::atoi(argv[4]));

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener>(
        ioc,
        tcp::endpoint{address, port},
        doc_root)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    return EXIT_SUCCESS;
}